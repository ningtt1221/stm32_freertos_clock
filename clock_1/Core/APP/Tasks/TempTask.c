//
// Created by 18454 on 2026/3/27.
//

/* TempTask.c */
#include "TempTask.h"

// 全局温度变量，供显示任务读取
float CurrentTemp = 0.0f;

/* ======================================================= */
/* 修正后的 LM75A 底层读取函数                             */
/* ======================================================= */
float LM75A_ReadTemp(void)
{
    uint8_t TempHL[2] = {0};
    int16_t temp_raw; // 必须使用 16 位有符号整型
    float t;

    // 阻塞式读取 I2C 数据，最大超时时间 100ms
    if (HAL_I2C_Mem_Read(&hi2c1, LM75A_ADD, LM75A_TempReg, I2C_MEMADD_SIZE_8BIT, TempHL, 2, 100) == HAL_OK)
    {
        // 拼接成 16 位数据
        temp_raw = (int16_t)((TempHL[0] << 8) | TempHL[1]);

        // 算术右移 5 位 (LM75A 为 11 位数据格式，低 5 位无效)
        // 因为 temp_raw 是 int16_t，遇到负数时会自动补 1，实现完美的符号扩展
        temp_raw = temp_raw >> 5;

        // 分辨率为 0.125°C
        t = temp_raw * 0.125f;
        return t;
    }

    // 如果 I2C 读取失败，返回一个错误标识值（或者返回上一次的旧值）
    return -999.0f;
}

/* ======================================================= */
/* FreeRTOS 温度采集任务                                   */
/* ======================================================= */
void StartTempTask(void *argument)
{
    float temp_val;
    for(;;)
    {
        // 调用底层函数获取最新温度
        temp_val = LM75A_ReadTemp();

        // 如果读取成功，更新全局变量
        if (temp_val != -999.0f) {
            CurrentTemp = temp_val;
        }

        // 延时 500ms (2Hz 采样率对于环境温度来说完全足够了)
        // 这会让出 CPU 资源给核心的逻辑和显示任务
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}