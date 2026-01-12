/*
 * rtc_task.c
 *
 *  Created on: Jul 10, 2024
 *      Author: gpaol
 */
#include "rtc_task.h"
#include "sensors_task.h"
#include "serial_com_task.h"
#include <string.h>
#include "utility.h"


#define MSECS_TASK 1000

#define RTC_YEAR_OFFSET   2000U
#define RTC_MIN_YEAR      2000U
#define RTC_MAX_YEAR      2099U
#define SECONDS_PER_DAY   86400U

static uint8_t is_leap_year(uint32_t year)
{
    return ((year % 4U == 0U && year % 100U != 0U) || (year % 400U == 0U));
}


static void rtc_task(void);
static void rtc_timer_task(void);
static uint8_t u8TickState = 0;

static time_t gUnixTimeStamp;
static time_t gStartUnixTime = 0;
static time_t gInterval = 60; // Intervallo di allarme in secondi

int   rtc_task_init(void)
{
	gStartUnixTime = rtc_unix_read_utc();
	add_mainloop_funct(rtc_task, "rtc_task",1,0);
	add_cyclical_funct(rtc_timer_task, MSECS_TASK, "rtc timer\0",CRITICAL_TASK);
    return 0;
}

void rtc_timer_task(void)
{
	u8TickState=1;
}
/**
 * @brief  Legge l'RTC e lo converte in Unix time UTC.
 * @retval Unix time (secondi da 1/1/1970 UTC).
 *         Ritorna 0 se l'anno è fuori range (2000..2099).
 */
time_t rtc_unix_read_utc(void)
{
    RTC_TimeTypeDef rtcTime;
    RTC_DateTypeDef rtcDate;

    /* IMPORTANTE: prima il tempo, poi la data */
    HAL_RTC_GetTime(&hrtc, &rtcTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &rtcDate, RTC_FORMAT_BIN);

    uint32_t year  = (uint32_t)rtcDate.Year + RTC_YEAR_OFFSET;
    uint32_t month = (uint32_t)rtcDate.Month;  // 1..12
    uint32_t day   = (uint32_t)rtcDate.Date;   // 1..31

    uint32_t hour   = (uint32_t)rtcTime.Hours;
    uint32_t minute = (uint32_t)rtcTime.Minutes;
    uint32_t second = (uint32_t)rtcTime.Seconds;

    if (year < 1970U || year > RTC_MAX_YEAR)
        return (time_t)0;   // fuori range, gestisci come errore se vuoi

    /* --- anni completi dal 1970 all'anno-1 --- */
    uint32_t days = 0U;
    for (uint32_t y = 1970U; y < year; y++)
    {
        days += is_leap_year(y) ? 366U : 365U;
    }

    /* --- mesi completi dentro l'anno corrente --- */
    static const uint8_t days_in_month[12] =
    {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    for (uint32_t m = 1U; m < month; m++)
    {
        uint32_t dim = days_in_month[m - 1U];
        if (m == 2U && is_leap_year(year))
        {
            dim = 29U;
        }
        days += dim;
    }

    /* --- giorni nel mese (day parte da 1) --- */
    days += (day - 1U);

    uint32_t t = days * SECONDS_PER_DAY
               + hour * 3600U
               + minute * 60U
               + second;

    return (time_t)t;
}

/**
 * @brief  Imposta l'RTC da un Unix time UTC (secondi da 1/1/1970).
 * @param  unix_time_utc: time_t considerato come UTC
 * @retval 0 se OK, <0 se errore
 */
int rtc_unix_write_utc(time_t unix_time_utc)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    uint32_t t;

    if (unix_time_utc < 0)
        return -1;   // prima del 1970 non gestito

    t = (uint32_t)unix_time_utc;   // per Unix 32 bit va bene

    /* --- scomponi in hh:mm:ss --- */
    uint32_t days          = t / SECONDS_PER_DAY;
    uint32_t seconds_of_day = t % SECONDS_PER_DAY;

    sTime.Hours   = (uint8_t)(seconds_of_day / 3600U);
    seconds_of_day %= 3600U;
    sTime.Minutes = (uint8_t)(seconds_of_day / 60U);
    sTime.Seconds = (uint8_t)(seconds_of_day % 60U);

    sTime.TimeFormat     = RTC_HOURFORMAT12_AM;      // irrilevante in 24h
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    /* --- giorni → anno/mese/giorno (sempre UTC) --- */
    uint32_t year = 1970U;
    while (1)
    {
        uint32_t diy = is_leap_year(year) ? 366U : 365U;
        if (days < diy)
            break;
        days -= diy;
        year++;
    }

    /* RTC può rappresentare solo 2000..2099 */
    if (year < RTC_MIN_YEAR || year > RTC_MAX_YEAR)
        return -2;

    static const uint8_t days_in_month[12] =
    {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    uint32_t month;
    uint32_t day;

    for (month = 0U; month < 12U; month++)
    {
        uint32_t dim = days_in_month[month];
        if (month == 1U && is_leap_year(year))
        {
            dim = 29U;
        }
        if (days < dim)
            break;
        days -= dim;
    }
    day = days + 1U;

    /* --- calcolo weekday (sempre UTC)
     * 1/1/1970 è giovedì.
     * tm_wday stile libc: 0=Dom, 1=Lun, ..., 4=Gio
     */
    uint32_t days_since_epoch = t / SECONDS_PER_DAY;
    uint8_t  tm_wday = (uint8_t)((days_since_epoch + 4U) % 7U);

    /* mappa tm_wday -> HAL:
     * HAL: Mon=1 .. Sat=6, Sun=7
     * tm:  Sun=0, Mon=1 .. Sat=6
     */
    uint8_t rtcWeekday = (tm_wday == 0U) ? RTC_WEEKDAY_SUNDAY : tm_wday;

    /* --- riempi struct data RTC --- */
    sDate.WeekDay = rtcWeekday;
    sDate.Month   = (uint8_t)(month + 1U);           // 0..11 → 1..12
    sDate.Date    = (uint8_t)day;
    sDate.Year    = (uint8_t)(year - RTC_YEAR_OFFSET); // 0..99 => 2000..2099

    /* --- scrittura RTC: prima ora, poi data --- */
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
        return -3;

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
        return -4;

    // opzionale: segna nei backup register che l'RTC è valido
    // HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x32F2);

    return 0;
}

void rtc_task()
{
	double diffUnixTime = 0;
	time_t tTimeElapsed     = 0;


	if(u8TickState == 0) return;
	u8TickState=0;

	gUnixTimeStamp = rtc_unix_read_utc();
	if(gUnixTimeStamp<gStartUnixTime)
	{
		gStartUnixTime = gUnixTimeStamp;
	}
	tTimeElapsed = gUnixTimeStamp - gStartUnixTime;

	if (tTimeElapsed >= gInterval) {
		gStartUnixTime += gInterval;
	}

	if(getBatteryData()->BatteryVoltage == 0) //batteria non presente
	{
		//gestire assenza di batteria
	}
	else if(getBatteryData()->BatteryChargeLevelRelative > 4)
	{
		//gestire alta/bassa carica della batteria..
	}
	return;
}

