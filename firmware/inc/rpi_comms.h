#ifndef RPI_COMMS_H
#define RPI_COMMS_H

#include "xmc_uart.h"
#include "cycfg_peripherals.h"

#define SYNC (uint8_t)(0b101 << 5)

typedef struct {
    uint8_t left;
    uint8_t right;
    uint8_t center;
    uint8_t player;
    uint8_t skip;
} rpi_inp_t;


typedef struct {
    uint8_t reset;
    uint8_t dir;
} rpi_out_t;

bool read_rpi(rpi_inp_t* inp);

void write_rpi(rpi_out_t* out);


#endif