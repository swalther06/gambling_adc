#ifndef RPI_COMMS_H
#define RPI_COMMS_H

#include <stdbool.h>

typedef struct {
    bool left;
    bool right;
    bool center;
    bool player;
    bool skip;
} rpi_inp_t;


typedef struct {
    bool reset;
    bool dir;
} rpi_out_t;


#endif