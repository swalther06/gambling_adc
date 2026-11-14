#ifndef NHD_LCD_H_
#define NHD_LCD_H_

#include <stdint.h>
#include "xmc_uart.h"

/* Update this macro to match your Device Configurator UART peripheral name */
#define LCD_UART_HW   XMC_UART1_CH0

void NHD_LCD_Init(void);
void NHD_LCD_Clear(void);
void NHD_LCD_SetCursor(uint8_t row, uint8_t col);
void NHD_LCD_SetBacklight(uint8_t brightness); // 1 (off) to 8 (full)
void NHD_LCD_SetContrast(uint8_t contrast);      // 1 to 50 (default 40)
void NHD_LCD_Print(const char *str);
void NHD_LCD_PutChar(char c);
void NHD_LCD_PrintUint8(uint8_t num);
void NHD_LCD_PutByte(uint8_t byte);



#endif /* NHD_LCD_H_ */
