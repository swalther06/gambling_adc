/* Helper to send a single byte and wait for transmission completion */
#include "nhd_lib.h"
#include "cybsp.h"
static void send_byte(uint8_t data)
{
    XMC_UART_CH_Transmit(LCD_UART_HW, data);
    /* Wait until the transmit buffer is ready for the next byte */
    while (XMC_USIC_CH_GetTransmitBufferStatus(LCD_UART_HW) == XMC_USIC_CH_TBUF_STATUS_BUSY) {
        __NOP();
    }
    /* Small delay to prevent overrunning the PIC16F690 controller */
    XMC_DelayUs(150);
}

/* Helper to send an escape command (0xFE + command) */
static void send_cmd(uint8_t cmd)
{
    send_byte(0xFE);
    send_byte(cmd);
}

void NHD_LCD_Clear()
{
    send_cmd(0x51);
    XMC_Delay(2);   // Clear screen requires ~1.5ms execution time
}

void NHD_LCD_Init()
{
    /* Wait 100ms after power-up for the display's internal PIC to boot */
    XMC_Delay(100);
    send_cmd(0x41); // Display ON
    NHD_LCD_Clear();
}

void NHD_LCD_SetCursor(uint8_t row, uint8_t col)
{
    /* Row starting addresses per NHD-0420D3Z datasheet */
    const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    
    if (row > 3) row = 3;
    if (col > 19) col = 19;

    send_cmd(0x45);
    send_byte(row_offsets[row] + col);
    XMC_DelayUs(100);
}

void NHD_LCD_SetBacklight(uint8_t brightness)
{
    if (brightness < 1) brightness = 1;
    if (brightness > 8) brightness = 8;

    send_cmd(0x53);
    send_byte(brightness);
    XMC_DelayUs(100);
}

void NHD_LCD_SetContrast(uint8_t contrast)
{
    if (contrast < 1) contrast = 1;
    if (contrast > 50) contrast = 50;

    send_cmd(0x52);
    send_byte(contrast);
    XMC_DelayUs(500);
}

void NHD_LCD_Print(const char *str)
{
    while (*str) {
        send_byte((uint8_t)(*str++));
    }
}

void NHD_LCD_PutChar(char c)
{
    send_byte((uint8_t)c);
}

void NHD_LCD_PrintUint8(uint8_t num)
{
    if (num >= 100) {
        send_byte((num / 100) + '0');
    }
    if (num >= 10) {
        send_byte(((num / 10) % 10) + '0');
    }
    send_byte((num % 10) + '0');
    send_byte(' ');
}

void NHD_LCD_PutByte(uint8_t byte)
{
    send_byte(byte);
}