#ifndef DC_MOTOR_H
#define DC_MOTOR_H

#include <stdint.h>
#include <stdbool.h>
#include "xmc_ccu4.h"
#include "cybsp.h"
#include "cy_utils.h"

typedef struct{
    XMC_CCU4_MODULE_t *timer_module;
    XMC_CCU4_SLICE_t *timer_slice;
    uint32_t slice_mask;
    float ramp_time_sec;                // The amount of time it should take to ramp the motor
                                        // from 0% to 100% duty cycle. Value defined in hw_config.h
    float max_duty_cycle;               // Max duty cycle that can be safely supplied.
                                        // Value defined in hw_config.h
    float PWM_frequency_hz;             // Value defined in hw_config.h
} dc_motor_config_t;

typedef struct {
    float curr_duty_cycle;              // Duty cycle is a value in [0, max_duty_cycle]
    dc_motor_config_t config;
} dc_motor_t;

// requires: timer slice must not be started ("start on initialization" unchecked)
    // passive level on timer slice must be set to LOW
// input: motor details
// modifies: initializes motor according to config, modifies HW registers:
    // Prescaler, Period Register, Compare Register, starts timer
// returns: bool which is false if config is invalid, specifically:
    // The slice’s passive level isn’t LOW.
    // PWM_frequency_hz is ≤ 0 or NaN.
    // The frequency is too low for any prescaler (the required PSIV is greater than 15).
    // The counts per period fall outside [2, 65535].
// if true, motor is initialized with 0% duty
bool dc_motor_init(dc_motor_t *motor, const dc_motor_config_t *config);

// input: motor and new duty cycle
// modifies: ramps compare register value to new value such that the motor
// is controlled with a pwm signal with new_duty as its duty cycle.
    // values outside the allowable range [0, max_duty_cycle] will be clamped to nearest bound
// returns: nothing, only exits once ramp up has completed (blocking)
void dc_motor_set_duty_cycle(dc_motor_t *motor, float new_duty);

#endif