#include "WifiTask.h"
#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"
#include "rtc.h"

// ---------------------------------------------------------
// 【真正的全局变量定义区】 - 整个工程只能在这里定义一次！
// ---------------------------------------------------------
uint8_t GlobalWeatherCode = 99;
int8_t GlobalNetTemp = 0;

// 预报数组：必须在这里初始化为 99！这样 OLED 没收到数据前才会显示 Waiting...
uint8_t GlobalFcstCode[3] = {99, 99, 99};
int8_t GlobalFcstHigh[3] = {0, 0, 0};
int8_t GlobalFcstLow[3] = {0, 0, 0};

uint8_t is_network_synced = 0;
osMessageQueueId_t WifiQueueHandle = NULL;

extern UART_HandleTypeDef huart1;

/* 核心解析函数 */
void ParseNetworkData(char *rx_buf) {
    // 1. 解析时间协议 #TIME:26,04,20,21,40,23
    if (strncmp(rx_buf, "#TIME:", 6) == 0) {
        int yy, MM, dd, hh, mm, ss;
        if (sscanf(rx_buf, "#TIME:%d,%d,%d,%d,%d,%d", &yy, &MM, &dd, &hh, &mm, &ss) == 6) {

            RTC_TimeTypeDef sTime = {0};
            RTC_DateTypeDef sDate = {0};

            sTime.Hours = (uint8_t)hh;
            sTime.Minutes = (uint8_t)mm;
            sTime.Seconds = (uint8_t)ss;
            HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

            sDate.Year = (uint8_t)yy;
            sDate.Month = (uint8_t)MM;
            sDate.Date = (uint8_t)dd;
            sDate.WeekDay = RTC_WEEKDAY_MONDAY;
            HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

            is_network_synced = 1;
        }
    }
    // 2. 解析实时天气 #WEAT:09,18
    else if (strncmp(rx_buf, "#WEAT:", 6) == 0) {
        int code, temp;
        if (sscanf(rx_buf, "#WEAT:%d,%d", &code, &temp) == 2) {
            GlobalWeatherCode = (uint8_t)code;
            GlobalNetTemp = (int8_t)temp;
        }
    }
    // 3. 【核心修复】：解析天气预报 #FCST:09,19,13,09,24,13,04,29,15
    else if (strncmp(rx_buf, "#FCST:", 6) == 0) {
        int c0, h0, l0, c1, h1, l1, c2, h2, l2;
        if (sscanf(rx_buf, "#FCST:%d,%d,%d,%d,%d,%d,%d,%d,%d", &c0, &h0, &l0, &c1, &h1, &l1, &c2, &h2, &l2) == 9) {
            GlobalFcstCode[0] = (uint8_t)c0;
            GlobalFcstHigh[0] = (int8_t)h0;
            GlobalFcstLow[0]  = (int8_t)l0;

            GlobalFcstCode[1] = (uint8_t)c1;
            GlobalFcstHigh[1] = (int8_t)h1;
            GlobalFcstLow[1]  = (int8_t)l1;

            GlobalFcstCode[2] = (uint8_t)c2;
            GlobalFcstHigh[2] = (int8_t)h2;
            GlobalFcstLow[2]  = (int8_t)l2;
        }
    }
}

/* 任务入口 */
void StartWifiTask(void *argument) {
    (void)argument;

    if (WifiQueueHandle == NULL) {
        WifiQueueHandle = osMessageQueueNew(5, 64, NULL);
    }

    char msg_buf[64];
    for(;;) {
        // 阻塞等待串口中断发来的数据
        if (osMessageQueueGet(WifiQueueHandle, msg_buf, NULL, osWaitForever) == osOK) {
            ParseNetworkData(msg_buf);
        }
    }
}