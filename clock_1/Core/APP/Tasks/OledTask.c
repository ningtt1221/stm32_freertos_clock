#include "OledTask.h"

#include <stdio.h>
#include <string.h>

#include "BootAnim.h"
#include "usart.h"
#include "WifiTask.h"  // 【新增】引入网络任务头文件，以获取天气和室外温度

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

    char str_buf[32];
    uint8_t display_hour = 0;
    uint8_t display_min = 0;
    uint8_t display_sec = 0;
    uint8_t blink_flag = 0; // 0: 实体显示, 1: 隐藏(实现闪烁)

    // u8g2 初始化
    u8g2_init();

    // 1. 开机动画 (仅执行一次)
    PlayBootAnimation();

    // 2. 【新增】：开机主动向 ESP8266 发送同步请求
    // 这样即使 STM32 单独按了 Reset 重启，也能立刻拿到最新数据
    char sync_cmd[] = "#SYNC\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)sync_cmd, strlen(sync_cmd), 100);

    for (;;) {
        // 1. 计算闪烁标志位: 系统每过 500ms，blink_flag 在 0 和 1 之间翻转一次
        blink_flag = (osKernelGetTickCount() / 500) % 2;

        // 2. 清除 u8g2 的内部缓冲区
        u8g2_ClearBuffer(&myDisplay);

        // 3. 根据当前页面渲染不同内容
        switch (CurrentPage) {

            // ================== 时间界面 & 闹钟界面 ==================
            case PAGE_TIME:
                // 获取底层真实 RTC 时间
                GetRealTimeForDisplay(&display_hour, &display_min, &display_sec);
                // --- 绘制日期数字 ---
                u8g2_SetFont(&myDisplay, u8g2_font_ncenB12_tr);
                sprintf(str_buf, "20%02d-%02d-%02d", sDate.Year, sDate.Month, sDate.Date);
                u8g2_DrawStr(&myDisplay, 20, 20, str_buf);
                // --- 绘制巨大的时间数字 ---
                u8g2_SetFont(&myDisplay, u8g2_font_ncenB18_tr);
                // 绘制小时
                sprintf(str_buf, "%02d", display_hour);
                u8g2_DrawStr(&myDisplay, 5, 55, str_buf);
                // 绘制中间的冒号 (固定显示，或者你也可以加上秒级闪烁逻辑)
                u8g2_DrawStr(&myDisplay, 40, 53, ":");
                // 绘制分钟
                sprintf(str_buf, "%02d", display_min);
                u8g2_DrawStr(&myDisplay, 50, 55, str_buf);
                // 绘制中间的冒号
                u8g2_DrawStr(&myDisplay, 85, 53, ":");
                // 绘制秒钟
                sprintf(str_buf, "%02d", display_sec);
                u8g2_DrawStr(&myDisplay, 95, 55, str_buf);
                break;
            case PAGE_ALARM:
                // --- 绘制顶部标题栏 ---
                u8g2_SetFont(&myDisplay, u8g2_font_ncenB12_tr);
                u8g2_DrawStr(&myDisplay, 0, 15, "Alarm");

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
                u8g2_SetFont(&myDisplay, u8g2_font_ncenB18_tr); // 24像素高纯数字大字体

                // 绘制小时：如果处于编辑小时状态，且闪烁标志为1，则跳过绘制(隐身)
                if (!(EditState == EDIT_HOUR && blink_flag == 1)) {
                    sprintf(str_buf, "%02d", display_hour);
                    u8g2_DrawStr(&myDisplay, 5, 55, str_buf);
                }

                // 绘制中间的冒号 (固定显示，或者你也可以加上秒级闪烁逻辑)
                u8g2_DrawStr(&myDisplay, 40, 53, ":");

                // 绘制分钟：如果处于编辑分钟状态，且闪烁标志为1，则跳过绘制
                if (!(EditState == EDIT_MIN && blink_flag == 1)) {
                    sprintf(str_buf, "%02d", display_min);
                    u8g2_DrawStr(&myDisplay, 50, 55, str_buf);
                }
                // 绘制中间的冒号
                u8g2_DrawStr(&myDisplay, 85, 53, ":");
                sprintf(str_buf, "%02d", display_sec);
                u8g2_DrawStr(&myDisplay, 95, 55, str_buf);
                break;
            // ================== 温度界面 ==================
            case PAGE_TEMP:
                // --- 绘制顶部标题栏 ---
                u8g2_SetFont(&myDisplay, u8g2_font_ncenB08_tr);
                u8g2_DrawStr(&myDisplay, 0, 10, "Temp & Weather");

                // 【修改点2】：重构温度界面，上下分两行显示室内和室外

                // --- 1. 室内温度 (IN - 来源于 LM75) ---
                u8g2_SetFont(&myDisplay, u8g2_font_ncenB08_tr);
                u8g2_DrawStr(&myDisplay, 0, 35, "IN:");

                u8g2_SetFont(&myDisplay, u8g2_font_ncenB12_tr); // 使用 16 像素的中号字体
                int temp_int = (int)CurrentTemp;
                int temp_frac = (int)(CurrentTemp * 10) % 10;
                sprintf(str_buf, "%d.%d  C", temp_int, temp_frac);
                u8g2_DrawStr(&myDisplay, 30, 36, str_buf);
                u8g2_DrawCircle(&myDisplay, 72, 22, 2, U8G2_DRAW_ALL); // 室内度数符号的小圆圈

                // --- 2. 室外温度 (OUT) & 天气 (来源于 ESP8266) ---
                u8g2_SetFont(&myDisplay, u8g2_font_ncenB08_tr);
                u8g2_DrawStr(&myDisplay, 0, 60, "OUT:");

                u8g2_SetFont(&myDisplay, u8g2_font_ncenB12_tr);
                if (GlobalWeatherCode == 99) {
                    // 如果值还是99，说明开机后还没收到 ESP8266 的数据
                    u8g2_DrawStr(&myDisplay, 30, 62, "-- C");
                } else {
                    sprintf(str_buf, "%d  C", GlobalNetTemp);
                    u8g2_DrawStr(&myDisplay, 30, 62, str_buf);
                    u8g2_DrawCircle(&myDisplay, 55, 48, 2, U8G2_DRAW_ALL); // 室外度数符号

                    // --- 3. 将天气代码转为英文单词显示在右下角 ---
                    const char* weat_str = "N/A";
                    if (GlobalWeatherCode <= 3) weat_str = "Sunny";          // 0~3 是各种晴天
                    else if (GlobalWeatherCode <= 8) weat_str = "Cloudy";    // 4~8 是各种多云
                    else if (GlobalWeatherCode == 9) weat_str = "Overcast";  // 9 是阴天
                    else if (GlobalWeatherCode <= 19) weat_str = "Rain";     // 10~19 是各种雨天
                    else if (GlobalWeatherCode <= 25) weat_str = "Snow";     // 20~25 是雪天

                    u8g2_SetFont(&myDisplay, u8g2_font_ncenB08_tr); // 换回小字体显示天气文本
                    u8g2_DrawStr(&myDisplay, 75, 60, weat_str);
                }
                break;
            // ================== 天气预报界面 ==================
            case PAGE_FORECAST:
                // 1. 绘制顶部标题栏
                u8g2_SetFont(&myDisplay, u8g2_font_ncenB08_tr);

                // 根据逻辑任务传来的翻页索引，动态显示标题
                if (FcstDayIndex == 0) {
                    u8g2_DrawStr(&myDisplay, 0, 10, "Forecast: Today");
                } else if (FcstDayIndex == 1) {
                    u8g2_DrawStr(&myDisplay, 0, 10, "Forecast: Tomorrow");
                } else {
                    u8g2_DrawStr(&myDisplay, 0, 10, "Forecast: Day After");
                }

                // 2. 检查网络数据是否已到达 (99为初始无效值)
                if (GlobalFcstCode[0] == 99) {
                    u8g2_DrawStr(&myDisplay, 30, 40, "Waiting...");
                } else {
                    // 获取当前选择页面的天气预报数据
                    uint8_t code = GlobalFcstCode[FcstDayIndex];
                    int8_t high = GlobalFcstHigh[FcstDayIndex];
                    int8_t low = GlobalFcstLow[FcstDayIndex];

                    // 3. 将温度范围靠左对齐绘制
                    u8g2_SetFont(&myDisplay, u8g2_font_ncenB12_tr);
                    sprintf(str_buf, "%d ~ %d  C", low, high);
                    u8g2_DrawStr(&myDisplay, 0, 38, str_buf);
                    u8g2_DrawCircle(&myDisplay, 65, 22, 2, U8G2_DRAW_ALL); // 度数符号的小圆圈

                    // 4. 将天气代码转换为文字，并匹配对应的【内置图标代码】
                    const char* f_weat = "N/A";
                    uint16_t icon_code = 0x0040; // 默认图标：云朵

                    if (code <= 3) {
                        f_weat = "Sunny";
                        icon_code = 0x0045; // u8g2图标：大太阳
                    } else if (code <= 8) {
                        f_weat = "Cloudy";
                        icon_code = 0x0041; // u8g2图标：多云 (云朵加太阳)
                    } else if (code == 9) {
                        f_weat = "Overcast";
                        icon_code = 0x0040; // u8g2图标：纯云朵
                    } else if (code <= 19) {
                        f_weat = "Rain";
                        icon_code = 0x0043; // u8g2图标：下雨
                    } else if (code <= 25) {
                        f_weat = "Snow";
                        icon_code = 0x0040; // u8g2图标：雪天没有独立图标，暂用云朵代替
                    }

                    // 5. 绘制天气状况英文字符串
                    u8g2_SetFont(&myDisplay, u8g2_font_ncenB12_tr);
                    u8g2_DrawStr(&myDisplay, 0, 60, f_weat);

                    // ==========================================================
                    // 6. 在屏幕右侧绘制 32x32 像素的巨大天气图标！
                    // ==========================================================
                    u8g2_SetFont(&myDisplay, u8g2_font_open_iconic_weather_4x_t);
                    // DrawGlyph函数专门用于绘制特殊字符/图标
                    // X=85(靠右), Y=55(底部基线)，刚好在屏幕右侧居中
                    u8g2_DrawGlyph(&myDisplay, 90, 55, icon_code);

                }
                break;
        }

        // 4. 将缓冲区数据推送到 OLED 屏幕
        u8g2_SendBuffer(&myDisplay);

        // 5. 延时 50ms (刷新率 20Hz)
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

