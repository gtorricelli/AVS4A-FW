#ifndef RTC_TASK_H_
#define RTC_TASK_H_

#include "KedOS.h"
#include "main.h"
#include "rtc.h"
#include "time.h"

int     rtc_task_init(void);
time_t  rtc_unix_read();
int     rtc_unix_write(time_t unixtime);


#endif /* RTC_TASK_H_ */
