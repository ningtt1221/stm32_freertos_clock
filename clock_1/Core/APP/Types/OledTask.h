//
// Created by 18454 on 2026/3/27.
//

#ifndef CLOCK_1_OLEDTASK_H
#define CLOCK_1_OLEDTASK_H

#include "cmsis_os2.h"
#include "spi.h"
#include "u8g2.h"
#include "stdio.h"
#include "LogicTask.h"

// 假设 u8g2 对象已经在主函数中初始化完毕
extern u8g2_t myDisplay;
// 假设这是从 LM75 任务中获取的实时温度
extern float CurrentTemp;

#endif //CLOCK_1_OLEDTASK_H