//
// Created by 18454 on 2026/4/20.
//

#ifndef CLOCK_1_WIFITASK_H
#define CLOCK_1_WIFITASK_H

#include "main.h"
#include "cmsis_os.h"
#include "rtc.h"

// 暴露给其他文件（比如 OledTask）使用的全局天气变量
extern uint8_t GlobalWeatherCode;
extern int8_t GlobalNetTemp;
// 【新增】天气预报全局变量暴露
extern uint8_t GlobalFcstCode[3];
extern int8_t GlobalFcstHigh[3];
extern int8_t GlobalFcstLow[3];
// 暴露给 main.c 串口中断使用的消息队列句柄
extern osMessageQueueId_t WifiQueueHandle;

// 任务入口函数声明
void StartWifiTask(void *argument);


#endif //CLOCK_1_WIFITASK_H