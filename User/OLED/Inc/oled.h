#ifndef OLED_H
#define OLED_H

#include "stdint.h"

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Fill(uint8_t fill);
void OLED_DisplayOn(void);
void OLED_DisplayOff(void);
void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t size);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size);
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t mode);
void OLED_Refresh(void);

#endif
