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
#include "time.h"

#define MSECS_TASK 1000


static void rtc_task(void);
static void rtc_timer_task(void);
static uint8_t u8TickState = 0;
static int gnTimeLeft = 0;
int32_t date_read(struct tm *time);
static uint16_t gntpRequestActive = 0;
int setCurrRTC(RTC_TimeTypeDef*pTime,RTC_DateTypeDef*pDate);
int rtc_dow (int year, int month, int day);
int rtc_zeller (int year, int month, int day);
int rtc_leap (int year);

int   rtc_task_init(void)
{
	add_mainloop_funct(rtc_task, "rtc_task",1,0);
	add_cyclical_funct(rtc_timer_task, MSECS_TASK, "rtc timer\0",CRITICAL_TASK);
    return 0;
}

void rtc_timer_task(void)
{
	u8TickState=1;
}


struct tm rtc_to_tm(RTC_TimeTypeDef *t, RTC_DateTypeDef *d)
{
    struct tm tm_time;

    tm_time.tm_sec  = t->Seconds;
    tm_time.tm_min  = t->Minutes;
    tm_time.tm_hour = t->Hours;

    tm_time.tm_mday = d->Date;

    // RTC: 1–12 → struct tm: 0–11
    tm_time.tm_mon  = d->Month - 1;

    // RTC Year: 0–99 → struct tm: year since 1900
    tm_time.tm_year = d->Year;// + 100;
    // +100 perché 0 = 2000 → 100 = 2000 in struct tm

    // RTC WeekDay: 1=Mon .. 7=Sun → struct tm: 0=Sun .. 6=Sat
    tm_time.tm_wday = (d->WeekDay % 7);

    tm_time.tm_yday = 0;   // opzionale, puoi calcolarlo con mktime()
    tm_time.tm_isdst = -1; // -1 lascia che mktime lo determini

    return tm_time;
}

int32_t date_read(struct tm *time)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	*time=rtc_to_tm(&sTime,&sDate);
}

void rtc_task()
{
	struct tm time;
	int nSecondsTime=0;
	if(u8TickState == 0) return;
	u8TickState=0;

//	RTC_TimeTypeDef sTime;
//	RTC_DateTypeDef sDate;
//
//	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
//	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
//
////	uint8_t hours = sTime.Hours;
////	uint8_t minutes = sTime.Minutes;
////	uint8_t seconds = sTime.Seconds;
////
////	uint8_t day = sDate.Date;
////	uint8_t month = sDate.Month;
////	uint8_t year = sDate.Year;   // Solo ultimi due digits (00–99)

//	time=rtc_to_tm(&sTime,&sDate);
	date_read(&time);
	time_t unix_time = mktime(&time);
	nSecondsTime = time.tm_hour*3600+(time.tm_min*60)+time.tm_sec;

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

int setCurrRTC(RTC_TimeTypeDef*pTime,RTC_DateTypeDef*pDate)
{
	if (HAL_RTC_SetTime(&hrtc, pTime, RTC_FORMAT_BIN) != HAL_OK)
		return -3;

	if (HAL_RTC_SetDate(&hrtc, pDate, RTC_FORMAT_BIN) != HAL_OK)
		return -4;
	return 0;
}
/* Returns the number of days to the start of the specified year, taking leap
 * years into account, but not the shift from the Julian calendar to the
 * Gregorian calendar. Instead, it is as though the Gregorian calendar is
 * extrapolated back in time to a hypothetical "year zero".
 */
int rtc_leap (int year)
{
  return year*365 + (year/4) - (year/100) + (year/400);
}

/* Returns a number representing the number of days since March 1 in the
 * hypothetical year 0, not counting the change from the Julian calendar
 * to the Gregorian calendar that occured in the 16th century. This
 * algorithm is loosely based on a function known as "Zeller's Congruence".
 * This number MOD 7 gives the day of week, where 0 = Monday and 6 = Sunday.
 */
int rtc_zeller (int year, int month, int day)
{
  year += ((month+9)/12) - 1;
  month = (month+9) % 12;
  return rtc_leap (year) + month*30 + ((6*month+5)/10) + day + 1;
}

/* Returns the day of week (1=Monday, 7=Sunday) for a given date.
 */
int rtc_dow (int year, int month, int day)
{
  return (rtc_zeller (year, month, day) % 7) + 1;
}
