#ifndef __SERIAL_H
#define __SERIAL_H
#include "system.h"
//#include <stdio.h>

void Serial_Init(void);
void Serial1_SendByte(uint8_t Byte);
void Serial2_SendByte(uint8_t Byte);
void Serial1_SendNumber(uint32_t Number, uint8_t Length);
void Serial2_SendNumber(uint32_t Number, uint8_t Length);
uint8_t Serial1_GetRxFlag(void);
uint8_t Serial2_GetRxFlag(void);
uint8_t Serial1_GetRxData(void);
extern char Serial1_RxLine[16];
extern volatile uint8_t Serial1_LineReady;
uint8_t Serial2_GetRxData(void);
#endif
