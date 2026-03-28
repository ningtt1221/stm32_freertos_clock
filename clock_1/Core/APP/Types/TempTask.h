//
// Created by 18454 on 2026/3/27.
//

#ifndef CLOCK_1_TEMPTASK_H
#define CLOCK_1_TEMPTASK_H

#include "LogicTask.h"
#include "i2c.h"

#define LM75A_ADD 0x90 // 器件地址
#define LM75A_TempReg 0x00 // 温度寄存器地址
extern float CurrentTemp;

#endif //CLOCK_1_TEMPTASK_H