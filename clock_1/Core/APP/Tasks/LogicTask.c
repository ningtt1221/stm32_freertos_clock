//
// Created by 18454 on 2026/3/27.
//

/* LogicTask.c */
#include "LogicTask.h"

Page_t CurrentPage = PAGE_TIME;
EditState_t EditState = EDIT_NONE;
uint8_t TempHour = 0;
uint8_t TempMin = 0;

RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};

/* 从RTC获取真实时间 */
void Get_RTC_Time(void) {
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    TempHour = sTime.Hours;
    TempMin = sTime.Minutes;
}

/* 将修改后的时间保存到RTC */
void Save_RTC_Time(void) {
    sTime.Hours = TempHour;
    sTime.Minutes = TempMin;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
}

RTC_AlarmTypeDef sAlarm = {0};

/* 从RTC获取当前设定的闹钟时间 */
void Get_RTC_Alarm(void) {
    // 获取闹钟 A 的配置
    HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
    TempHour = sAlarm.AlarmTime.Hours;
    TempMin = sAlarm.AlarmTime.Minutes;
}

/* 将修改后的闹钟时间保存到RTC并开启中断 */
void Save_RTC_Alarm(void) {
    // 先获取一次当前闹钟配置，为了保留 AlarmMask 等触发条件不被清零
    HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
    // 修改为我们设置的时和分
    sAlarm.AlarmTime.Hours = TempHour;
    sAlarm.AlarmTime.Minutes = TempMin;
    sAlarm.AlarmTime.Seconds = 0; // 强制设为0秒，保证准点触发
    sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
    // 写入RTC，并使能闹钟A的中断功能
    HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);
}

void  StartLogicTask(void *argument) {
    (void)argument;

    // 进入主界面
    CurrentPage = PAGE_TIME;
    KeyMessage_t msg;

    for (;;) {
        // 阻塞等待按键消息
        if (osMessageQueueGet(KeyQueueHandle, &msg, NULL, osWaitForever) == osOK) {

            // ================== KEY0: 切换页面逻辑 ==================
            if (msg.key_index == 0 && msg.event == BTN_EVENT_CLICK) {
                // 如果当前正在修改时间，强制放弃保存并退出闪烁状态
                if (EditState != EDIT_NONE) {
                    EditState = EDIT_NONE;
                }

                // 页面循环切换: TIME -> ALARM -> TEMP -> TIME
                if (CurrentPage == PAGE_TIME) CurrentPage = PAGE_ALARM;
                else if (CurrentPage == PAGE_ALARM) CurrentPage = PAGE_TEMP;
                else if (CurrentPage == PAGE_TEMP) CurrentPage = PAGE_TIME;

                continue; // 处理完毕，跳过后续逻辑
            }

            // ================== KEY1: 修改/确认逻辑 ==================
            if (msg.key_index == 1 && msg.event == BTN_EVENT_CLICK) {
                // 只有在时间或闹钟页面才允许修改
                if (CurrentPage == PAGE_TIME || CurrentPage == PAGE_ALARM) {

                    if (EditState == EDIT_NONE) {
                        // 第一次按：加载当前时间/闹钟，开始修改小时
                        EditState = EDIT_HOUR;
                        if (CurrentPage == PAGE_TIME) {
                            Get_RTC_Time();
                        } else {
                            Get_RTC_Alarm();
                        }
                    }
                    else if (EditState == EDIT_HOUR) {
                        // 第二次按：切换到修改分钟
                        EditState = EDIT_MIN;
                    }
                    else if (EditState == EDIT_MIN) {
                        // 第三次按：确认保存，停止修改
                        EditState = EDIT_NONE;
                        if (CurrentPage == PAGE_TIME) {
                            Save_RTC_Time();
                        } else {
                            Save_RTC_Alarm();
                        }
                    }
                }
            }

            // ================== KEY2 (加) & KEY3 (减) ==================
            // 无论是单击(CLICK)还是长按(LONG_PRESS)，只要处于编辑状态，都执行加减
            if ((msg.key_index == 2 || msg.key_index == 3) && EditState != EDIT_NONE) {

                int8_t step = (msg.key_index == 2) ? 1 : -1;

                if (EditState == EDIT_HOUR) {
                    // 小时 0-23 循环
                    int16_t new_hour = TempHour + step;
                    if (new_hour > 23) new_hour = 0;
                    if (new_hour < 0)  new_hour = 23;
                    TempHour = (uint8_t)new_hour;
                }
                else if (EditState == EDIT_MIN) {
                    // 分钟 0-59 循环
                    int16_t new_min = TempMin + step;
                    if (new_min > 59) new_min = 0;
                    if (new_min < 0)  new_min = 59;
                    TempMin = (uint8_t)new_min;
                }
            }
        }
    }
}