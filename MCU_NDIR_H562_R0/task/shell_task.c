#include <stdio.h>
#include <stdlib.h>
#include "KedOS.h"
#include "main.h"
#include "bsp.h"
#include "errno.h"
#include "string.h"
#include "shell_task.h"
#include "shell.h"
#include "sensors_task.h"
#include "bme_task.h"

#define ENOERR           0
#define MAX_OPTIONS      10             /**< Max number of options for a command */

/**
* @brief Command function pointer type definition
*/
typedef int32_t (*cmd_func)(const int8_t *[]);

/**
* @brief Command descriptor
*/
typedef struct shell_command
{
    int8_t *cmd_name;                   /**< Command name */
    int8_t *cmd_descr;                  /**< Command description name */
    int8_t *cmd_help;                   /**< Command string */
    cmd_func func;                      /**< Command function */
    int8_t *cmd_options[MAX_OPTIONS];   /**< Command options */
} shell_command_st;

static bool_t  init = TRUE;
static int32_t print_help        (int8_t *_opts[]);
static int32_t i2c_bus_scan      (char *_opts[]);
static int32_t mpp               (char *_opts[]);
static int32_t en20v             (char *_opts[]);
static int32_t en12v             (char *_opts[]);
static int32_t enirled           (char *_opts[]);
static int32_t enwhiteled        (char *_opts[]);
static int32_t term              (char *_opts[]);
static int32_t battery           (char *_opts[]);
static int32_t sensor            (char *_opts[]);

static shell_command_st commands[] =
{
	{"help","         Print this list"      		, NULL, print_help},
	{"i2c","          i2c bus scan"         		, NULL, i2c_bus_scan, {NULL}},
    {"mpp","          mpp (1/0)"   		            , NULL, mpp, {NULL}},
	{"en20v","        20V (1/0)"   		            , NULL, en20v, {NULL}},
	{"en12v","        12V (1/0)"   		            , NULL, en12v, {NULL}},
	{"enIR","         IR (1/0)"    		            , NULL, enirled, {NULL}},
	{"enwhite","      White (1/0)" 		            , NULL, enwhiteled, {NULL}},
    {"term","         termination (1/0)"            , NULL, term, {NULL}},
    {"batt","         battery data"                 , NULL, battery, {NULL}},
    {"sens","         sensor data"                  , NULL, sensor, {NULL}},
};                                                                  /**< Commands array */

/* API functions */
/**
* @brief Shell task set initialized
* @details Set flag to indicate shell module is initialized
* @param none
* @return none
* traceability: -
*/
void shell_task_init(void)
{
    init = TRUE;
	//UartInit(LINUX_CONSOLE,115200,MODE_RS232,0,0);
	//UartStart(LINUX_CONSOLE);// start uart interrupt for queue management
    add_mainloop_funct(shell_task, "Shell",0,0);
}


/**
* @brief Shell task
* @details Print shell command layout and manage received commands calling appropriate function
* @param none
* @return none
* traceability: -
*/
void shell_task(void)
{
    int32_t i, opt_idx;
    char *cmd;
    int8_t *str_command;
//    int32_t res;
    uint8_t command_valid;
    int8_t *cmd_opts[MAX_OPTIONS];

    memset(cmd_opts, 0, sizeof(int8_t *) * MAX_OPTIONS);
    command_valid = 0;

    if (init == TRUE)
    {
        init = FALSE;
        ShellInit();
    }

    cmd = ShellProcess();
    if (cmd != NULL)
    {
        str_command = ConsoleStrTok(cmd);

        for (i = 0; i < sizeof(commands) / sizeof(shell_command_st); ++i)
        {
            if (strcmp(str_command, commands[i].cmd_name) == 0)
            {
                /* Parse options */
                for (opt_idx = 0; opt_idx < MAX_OPTIONS; ++opt_idx)
                {
                    cmd_opts[opt_idx] = ConsoleStrTok(NULL);
                }

                if (commands[i].func(cmd_opts) != ENOERR && commands[i].cmd_help != NULL)
                {
                    printf("Usage:\r\n\t%s\r\n", commands[i].cmd_help);
                }
                command_valid = 1;
            }
        }

        if (command_valid == 0)
        {
        	printf("Command not found\r\n");
        }
        printshell();
    }
}


/**
* @brief Print help
* @details Print command list
* @param _opts, options array
* @return none
* traceability: -
*/
static int32_t print_help(int8_t *_opts[])
{
    int32_t i;
    int32_t exit_status_;

    exit_status_ = ENOERR;

    printf("Commands: \r\n");
    for (i = 0; i < sizeof(commands) / sizeof(shell_command_st); ++i)
    {
    	printf("  %s: %s\r\n", commands[i].cmd_name, commands[i].cmd_descr);
    }

    return exit_status_;
}

static int32_t i2c_bus_scan      (char *_opts[])
{
	static uint8_t i2cScanRes[128];
	int i= 0;
	int nDetect;
	uint8_t address = 0;
	printf("I2C SCAN :\r\n");

	nDetect = 0;
	memset(i2cScanRes,0x00,128);
	printf("Bus I2C1\r\n");
	for(i=0;i<128;i++)
	{
	  address = i<<1;
	  if (HAL_I2C_IsDeviceReady(&hi2c1, address, 2, 100)==HAL_OK)
	  {
		  i2cScanRes[nDetect]=address;
		  nDetect++;
		  printf(" %.2X",i);
	  }
	}
	printf("\r\n");

	nDetect = 0;
	memset(i2cScanRes,0x00,128);
	printf("Bus I2C2\r\n");
	for(i=0;i<128;i++)
	{
	  address = i<<1;
	  if (HAL_I2C_IsDeviceReady(&hi2c2, address, 2, 100)==HAL_OK)
	  {
		  i2cScanRes[nDetect]=address;
		  nDetect++;
		  printf(" %.2X",i);
	  }
	}
	printf("\r\n");

	return 0;
}

//static uint32_t shellFuncTime = 0;
//
//void * shell_measure()
//{
//	static uint32_t value = 0;
//	GasData* pGasData;
//	int res;
//	char ChValue;
//	if(time_elapsed_ms(shellFuncTime, 100)==0) return 0;
//	shellFuncTime = get_clock_ms();
//    printf(CLS CURPOS(0,0) ATTR_FRED "\r\nNDIR MEASURE [P:pump on S:pump off  O:Calibrate Oxygen C: Calibrate R: Reset Calibrate\r\n\r\n" );
//    printf(ATTR_FGREEN);
//	pGasData = getGasData();
//
//    printf("Temp:%.2f \r\n",pGasData->temp);
//    printf("Press:%.2f \r\n",pGasData->press);
//    printf("Oxygen Raw:%.2f \r\n",pGasData->oxygen);
//    printf("O2  :%.2f%%\r\n",get_o2());
//    printf("Air :%.2f%%\r\n",get_air());
//
//    printf("I[0]:%07.3f I[1]:%07.3f I[2]:%07.3f I[3]:%07.3f \r\n",
//    		pGasData->fVrmsCal[0],
//    		pGasData->fVrmsCal[1],
//    		pGasData->fVrmsCal[2],
//    		pGasData->fVrmsCal[3]
//			);
//    printf("A[0]:%07.3f A[1]:%07.3f A[2]:%07.3f A[3]:%07.3f \r\n",
//    		pGasData->absorbance[0],
//    		pGasData->absorbance[1],
//    		pGasData->absorbance[2],
//    		pGasData->absorbance[3]
//			);
//
//	res = getShellProcessChar();
//	if (res >= 0) {
//		ChValue = res;
//		switch (ChValue) {
//		case 'c':
//		case 'C':
//			sensor_calibration();
//			break;
//		case 'r':
//		case 'R':
//			sensor_reset();
//			break;
//		case 'p':
//		case 'P':
//			pump_on(1);
//			break;
//		case 's':
//		case 'S':
//			pump_off(0);
//			break;
//		case 'o':
//		case 'O':
//			set_o2_calibration21();
//			break;
//		case 'z':
//		case 'Z':
//			set_o2_calibration0();
//			break;
//		}
//
//	}
//	return 0;
//}
//#define HC_CHANNEL      0  //3.33um 160nm
//#define R134a_CHANNEL   1  //10.27um 210nm
//#define R1234yf_CHANNEL 2  //11.30um 200nm
//#define R22_CHANNEL     3  //12.40um 180nm
//static int32_t launch_mes        (char *_opts[])
//{
//	shellFuncTime = get_clock_ms();
//	launch_shell_process(shell_measure);
//	return 0;
//}
static int32_t mpp               (char *_opts[])
{
	int on = 0;

    if (_opts[0] != NULL)
    {
    	on = atoi(_opts[0]);
    	if(on>0)
    	{
    		HAL_GPIO_WritePin(MPP_EN_GPIO_Port, MPP_EN_Pin, GPIO_PIN_RESET);
    	}
    	else
    	{
    		HAL_GPIO_WritePin(MPP_EN_GPIO_Port, MPP_EN_Pin, GPIO_PIN_SET);
    	}
    }
	return 0;
}

static int32_t en20v               (char *_opts[])
{
	int on = 0;

    if (_opts[0] != NULL)
    {
    	on = atoi(_opts[0]);
    	if(on>0)
    	{
    		HAL_GPIO_WritePin(EN_20V_GPIO_Port, EN_20V_Pin, GPIO_PIN_RESET);
    	}
    	else
    	{
    		HAL_GPIO_WritePin(EN_20V_GPIO_Port, EN_20V_Pin, GPIO_PIN_SET);
    	}
    }
	return 0;
}


static int32_t en12v               (char *_opts[])
{
	int on = 0;

    if (_opts[0] != NULL)
    {
    	on = atoi(_opts[0]);
    	if(on>0)
    	{
    		HAL_GPIO_WritePin(EN_12V_GPIO_Port, EN_12V_Pin, GPIO_PIN_RESET);
    	}
    	else
    	{
    		HAL_GPIO_WritePin(EN_12V_GPIO_Port, EN_12V_Pin, GPIO_PIN_SET);
    	}
    }
	return 0;
}

static int32_t enirled               (char *_opts[])
{
	int on = 0;

    if (_opts[0] != NULL)
    {
    	on = atoi(_opts[0]);
    	if(on>0)
    	{
    		HAL_GPIO_WritePin(EN_IR_GPIO_Port, EN_IR_Pin, GPIO_PIN_RESET);
    	}
    	else
    	{
    		HAL_GPIO_WritePin(EN_IR_GPIO_Port, EN_IR_Pin, GPIO_PIN_SET);
    	}
    }
	return 0;
}


static int32_t enwhiteled               (char *_opts[])
{
	int on = 0;

    if (_opts[0] != NULL)
    {
    	on = atoi(_opts[0]);
    	if(on>0)
    	{
    		HAL_GPIO_WritePin(EN_WHITE_GPIO_Port, EN_WHITE_Pin, GPIO_PIN_RESET);
    	}
    	else
    	{
    		HAL_GPIO_WritePin(EN_WHITE_GPIO_Port, EN_WHITE_Pin, GPIO_PIN_SET);
    	}
    }
	return 0;
}

static int32_t term               (char *_opts[])
{
	int on = 0;

    if (_opts[0] != NULL)
    {
    	on = atoi(_opts[0]);
    	if(on>0)
    	{
    		HAL_GPIO_WritePin(TERM_EN_GPIO_Port, TERM_EN_Pin, GPIO_PIN_RESET);
    	}
    	else
    	{
    		HAL_GPIO_WritePin(TERM_EN_GPIO_Port, TERM_EN_Pin, GPIO_PIN_SET);
    	}
    }
	return 0;
}

static int32_t battery           (char *_opts[])
{
	int nValue;
	float fValue;


//	dumpe_register();


//	nValue    = getBatteryAbsoluteStateOfCharge();
//	printf("Charge Abs %d\r\n",nValue);
	nValue    = getBatteryRelativeStateOfCharge();
	printf("Charge Relative %d\r\n",nValue);
	nValue    = getBatteryCurrentMa();
	printf("Current %d\r\n",nValue);
	nValue    = getBatteryVoltageMv();
	printf("Voltage %d\r\n",nValue);
	nValue    = getBatteryAverageCurrentMa();
	printf("Average ma %d\r\n",nValue);
	fValue    = getBatteryTemperatureC();
	printf("Temperature %f\r\n",fValue);

	nValue    = get_charger_status();
    switch(nValue)
    {
    case CHARGE_SUSPEND:
    	printf("Charge Suspended\r\n");
    	break;
    case CHARGE_IN_PROGRESS:
    	printf("Charge In Progress\r\n");
    	break;
    case CHARGE_COMPLETE:
    	printf("Charge Complete\r\n");
    	break;
    case CHARGE_ERROR:
    	printf("Charge Standby\r\n");
    	break;
    }
	return 0;
}
static int32_t sensor            (char *_opts[])
{
	int nValue;
	float fValue;
	nValue    = get_sensor_humidity();
	printf("Moisture %d\r\n",nValue);
	nValue    = get_sensor_temp();
	printf("Temperature %d\r\n",nValue);
	fValue    = get_bme_pressure();
	printf("BME Pressure %ld\r\n",(int32_t)(fValue*10));
	fValue    = get_bme_temp();
	printf("BME temp %ld\r\n",(int32_t)(fValue*10));
	fValue    = get_bme_humidity();
	printf("BME humidity %ld\r\n",(int32_t)(fValue*10));
	return 0;
}

