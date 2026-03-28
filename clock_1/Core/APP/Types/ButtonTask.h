//
// Created by 18454 on 2026/3/26.
//

#ifndef CLOCK_1_BUTTONTASK_H
#define CLOCK_1_BUTTONTASK_H

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"


/* --- 参数配置 --- */
#define BTN_TASK_PERIOD_MS    10    // 扫描周期：10ms
#define BTN_DEBOUNCE_TICKS    2     // 消抖阈值：2 * 10ms = 20ms
#define BTN_LONG_PRESS_TICKS  100   // 长按阈值：100 * 10ms = 1000ms
#define BTN_COUNT             4     // 按键数量

/* 按键事件枚举 */
typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_CLICK,        // 单击
    BTN_EVENT_LONG_PRESS    // 长按
} BtnEvent_t;

/* 按键对象结构体 */
typedef struct {
    GPIO_TypeDef* port;     // GPIO 端口
    uint16_t pin;           // GPIO 引脚
    uint16_t active_ticks;  // 按下持续时间计数器
    uint8_t  long_pressed; // 标记长按事件是否已经触发过
} Button_t;

extern Button_t buttons[BTN_COUNT];

#endif //CLOCK_1_BUTTONTASK_H