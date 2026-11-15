#include "rpi_comms.h"

bool read_rpi(rpi_inp_t* inp) {
    bool valid = false;

    while (!XMC_USIC_CH_RXFIFO_IsEmpty(RPI_UART_HW)) {
        volatile uint8_t data = (uint8_t)XMC_UART_CH_GetReceivedData(RPI_UART_HW);

        if ((data & 0xE0) != SYNC) // invalid data sent
            continue;

        inp->player = (data >> 4) & 1;
        inp->skip = (data >> 3) & 1;
        inp->left = (data >> 2) & 1;
        inp->right = (data >> 1) & 1;
        inp->center = data & 1;
        valid = true;
    }

    return valid;
}

void write_rpi(rpi_out_t* out) {
    volatile uint8_t data = SYNC | out->dir << 1 | out->reset;
    XMC_UART_CH_Transmit(RPI_UART_HW, data);
}