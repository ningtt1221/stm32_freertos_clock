#include "ButtonTask.h"
#include "LogicTask.h"

Button_t buttons[BTN_COUNT] = {
    {Key0_GPIO_Port, Key0_Pin, 0, 0},
    {Key1_GPIO_Port, Key1_Pin, 0, 0},
    {Key2_GPIO_Port, Key2_Pin, 0, 0},
    {Key3_GPIO_Port, Key3_Pin, 0, 0}
};

/* 按键事件回调处理函数：只负责发消息，不处理具体业务 */
void Button_EventHandler(uint8_t btn_index, BtnEvent_t event) {
    KeyMessage_t msg;
    msg.key_index = btn_index;
    msg.event = event;

    // 发送消息到队列，如果队列满则不等待 (非阻塞)
    if (KeyQueueHandle != NULL) {
        osMessageQueuePut(KeyQueueHandle, &msg, 0, 0);
    }
}

/* FreeRTOS 按键扫描任务 */
void StartButtonTask(void *argument) {
    (void)argument;

    for (;;) {
        for (uint8_t i = 0; i < BTN_COUNT; i++) {
            uint8_t is_pressed = (HAL_GPIO_ReadPin(buttons[i].port, buttons[i].pin) == GPIO_PIN_RESET);

            if (is_pressed) {
                if (buttons[i].active_ticks < 0xFFFF) {
                    buttons[i].active_ticks++;
                }

                // 修改点：触发长按后，不锁死，而是回退时间戳，实现“连续触发”
                if (buttons[i].active_ticks >= BTN_LONG_PRESS_TICKS) {
                    Button_EventHandler(i, BTN_EVENT_LONG_PRESS);
                    buttons[i].long_pressed = 1;

                    // 让 tick 回退 15 个单位 (150ms)，这样 150ms 后会再次触发长按事件，实现连加/连减
                    buttons[i].active_ticks = BTN_LONG_PRESS_TICKS - 15;
                }
            }
            else {
                if (buttons[i].active_ticks >= BTN_DEBOUNCE_TICKS) {
                    if (!buttons[i].long_pressed) {
                        Button_EventHandler(i, BTN_EVENT_CLICK);
                    }
                }
                buttons[i].active_ticks = 0;
                buttons[i].long_pressed = 0;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(BTN_TASK_PERIOD_MS));
    }
}

