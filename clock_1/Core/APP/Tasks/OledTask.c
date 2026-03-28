#include "OledTask.h"
#include "BootAnim.h"
// 硬件spi移植

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_DELAY_MILLI:
            osDelay(arg_int);
            break;
        case U8X8_MSG_GPIO_CS:
            HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, arg_int);
            break;
        case U8X8_MSG_GPIO_DC:
            HAL_GPIO_WritePin(SPI1_DC_GPIO_Port, SPI1_DC_Pin, arg_int);
            break;
        case U8X8_MSG_GPIO_RESET:
             HAL_GPIO_WritePin(SPI1_RES_GPIO_Port, SPI1_RES_Pin, arg_int);
            break;
    }
    return 1;
}

uint8_t u8x8_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_BYTE_SET_DC:
            HAL_GPIO_WritePin(SPI1_DC_GPIO_Port, SPI1_DC_Pin, arg_int);
            break;
        case U8X8_MSG_BYTE_SEND:
            HAL_SPI_Transmit(&hspi1, (uint8_t *)arg_ptr, arg_int, 1000);
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
            break;
    }
    return 1;
}

u8g2_t myDisplay;

void u8g2_init()
{
    u8g2_Setup_ssd1306_128x64_noname_f(&myDisplay, U8G2_R0, u8x8_spi, u8x8_gpio_and_delay);
    u8g2_InitDisplay(&myDisplay);
    u8g2_SetPowerSave(&myDisplay, 0);
}

/* 从底层获取要显示的实际时间/闹钟 (这里用伪代码代替你的实际获取逻辑) */
void GetRealTimeForDisplay(uint8_t *hour, uint8_t *min, uint8_t *sec) {
    if (CurrentPage == PAGE_TIME) {
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); // 必须连读
        *hour = sTime.Hours;
        *min = sTime.Minutes;
        *sec = sTime.Seconds;
    } else if (CurrentPage == PAGE_ALARM) {
        HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
        *hour = sAlarm.AlarmTime.Hours;
        *min = sAlarm.AlarmTime.Minutes;
        *sec = sAlarm.AlarmTime.Seconds;
    }
}

void PlayBootAnimation(void)
{
    // BOOT_FRAME_COUNT 和 BootFrames 已经在头文件里自动定义好了！
    for (int i = 0; i < BOOT_FRAME_COUNT; i++) {
        u8g2_ClearBuffer(&myDisplay);

        // 使用 DrawXBM 绘制，参数：句柄, X, Y, 宽, 高, 数据指针
        u8g2_DrawXBM(&myDisplay, 0, 0, 128, 64, BootFrames[i]);

        u8g2_SendBuffer(&myDisplay);

        osDelay(20  ); // 控制播放速度，约 20帧/秒
    }
}

/* FreeRTOS 显示刷新任务 */
void StartOledTask(void *argument) {
    (void)argument;

    char str_buf[16];
    uint8_t display_hour = 0;
    uint8_t display_min = 0;
    uint8_t display_sec = 0;
    uint8_t blink_flag = 0; // 0: 实体显示, 1: 隐藏(实现闪烁)

    // u8g2 初始化
    u8g2_init();

    // 1. 开机动画 (仅执行一次)
    PlayBootAnimation();

    for (;;) {
        // 1. 计算闪烁标志位: 系统每过 500ms，blink_flag 在 0 和 1 之间翻转一次
        blink_flag = (osKernelGetTickCount() / 500) % 2;

        // 2. 清除 u8g2 的内部缓冲区
        u8g2_ClearBuffer(&myDisplay);

        // 3. 根据当前页面渲染不同内容
        switch (CurrentPage) {

            // ================== 时间界面 & 闹钟界面 ==================
            case PAGE_TIME:
            case PAGE_ALARM:
                // --- 绘制顶部标题栏 ---
                u8g2_SetFont(&myDisplay, u8g2_font_ncenB08_tr); // 8像素高英文字体
                if (CurrentPage == PAGE_TIME) {
                    u8g2_DrawStr(&myDisplay, 0, 10, "Current Time");
                } else {
                    u8g2_DrawStr(&myDisplay, 0, 10, "Alarm Setting");
                }

                // --- 确定当前要绘制的数据源 ---


                
                if (EditState != EDIT_NONE) {
                    // 正在编辑中：显示逻辑任务中暂存的变量 (随按键实时变化)
                    display_hour = TempHour;
                    display_min = TempMin;
                } else {
                    // 正常显示：获取底层真实 RTC 时间或闹钟时间
                    GetRealTimeForDisplay(&display_hour, &display_min, &display_sec);
                }

                // --- 绘制巨大的时间数字 ---
                u8g2_SetFont(&myDisplay, u8g2_font_logisoso24_tn); // 24像素高纯数字大字体

                // 绘制小时：如果处于编辑小时状态，且闪烁标志为1，则跳过绘制(隐身)
                if (!(EditState == EDIT_HOUR && blink_flag == 1)) {
                    sprintf(str_buf, "%02d", display_hour);
                    u8g2_DrawStr(&myDisplay, 5, 45, str_buf);
                }

                // 绘制中间的冒号 (固定显示，或者你也可以加上秒级闪烁逻辑)
                u8g2_DrawStr(&myDisplay, 40, 43, ":");

                // 绘制分钟：如果处于编辑分钟状态，且闪烁标志为1，则跳过绘制
                if (!(EditState == EDIT_MIN && blink_flag == 1)) {
                    sprintf(str_buf, "%02d", display_min);
                    u8g2_DrawStr(&myDisplay, 50, 45, str_buf);
                }
                // 绘制中间的冒号
                u8g2_DrawStr(&myDisplay, 85, 43, ":");
                sprintf(str_buf, "%02d", display_sec);
                u8g2_DrawStr(&myDisplay, 95, 45, str_buf);

                break;

            // ================== 温度界面 ==================
            case PAGE_TEMP:
                // --- 绘制顶部标题栏 ---
                u8g2_SetFont(&myDisplay, u8g2_font_ncenB08_tr);
                u8g2_DrawStr(&myDisplay, 0, 10, "Temperature");

                // --- 绘制温度数值 ---
                u8g2_SetFont(&myDisplay, u8g2_font_logisoso24_tr); // 这里要选带字母的字体，tn后缀仅含数字

                int temp_int = (int)CurrentTemp; // 提取整数部分 (例如 27)
                int temp_frac = (int)(CurrentTemp * 10) % 10; // 提取一位小数 (例如 5)

                // 使用 %d.%d 完美绕过浮点打印限制！
                sprintf(str_buf, "%d.%d C", temp_int, temp_frac);
                u8g2_DrawStr(&myDisplay, 15, 45, str_buf);

                // 画一个小圆圈代表度数符号 "°"
                u8g2_DrawCircle(&myDisplay, 80, 20, 3, U8G2_DRAW_ALL);
                break;
        }

        // 4. 将缓冲区数据推送到 OLED 屏幕
        u8g2_SendBuffer(&myDisplay);

        // 5. 延时 50ms (刷新率 20Hz)
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

