//
// Created by 18454 on 2026/3/27.
//

#ifndef CLOCK_1_LOGICTASK_H
#define CLOCK_1_LOGICTASK_H

#include "main.h"
#include "cmsis_os.h"
#include "ButtonTask.h"
#include "rtc.h"


/* 1. 页面枚举 */
typedef enum {
    PAGE_TIME = 0,  // 时间界面
    PAGE_ALARM,     // 闹钟界面
    PAGE_TEMP       // 温度界面
} Page_t;

/* 2. 编辑状态枚举 */
typedef enum {
    EDIT_NONE = 0,  // 正常显示，不闪烁
    EDIT_HOUR,      // 修改小时 (小时闪烁)
    EDIT_MIN        // 修改分钟 (分钟闪烁)
} EditState_t;

/* 3. 消息队列传输的数据结构 */
typedef struct {
    uint8_t key_index;  // 哪个按键 (0~3)
    BtnEvent_t event;   // 什么事件 (CLICK / LONG_PRESS)
} KeyMessage_t;

/* 4. 全局变量声明 (供显示任务读取) */
extern Page_t CurrentPage;
extern EditState_t EditState;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;
extern RTC_AlarmTypeDef sAlarm;
extern uint8_t TempHour;  // 正在修改中的小时
extern uint8_t TempMin;   // 正在修改中的分钟

extern osMessageQueueId_t KeyQueueHandle;

#endif //CLOCK_1_LOGICTASK_H