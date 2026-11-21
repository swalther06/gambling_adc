#ifndef BUTTONS_H
#define BUTTONS_H

#include "stdbool.h"

typedef struct {
    bool up_pressed;
    bool middle_pressed;
    bool down_pressed;
} Buttons;

Buttons checkForUpdate();
#endif