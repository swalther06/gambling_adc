#include <stdint.h>
#include <math.h>

#include "cybsp.h"
#include "cy_utils.h"
#include "DC_Motor.h"

// input: motor object
// modifies: nothing
// returns: number of counts in one pwm_frequency
static uint64_t counts_per_period(dc_motor_t *motor) {
    uint32_t psiv = (motor->config.timer_slice->PSC & CCU4_CC4_PSC_PSIV_Msk) >> CCU4_CC4_PSC_PSIV_Pos;
    return 1.0f/motor->config.PWM_frequency_hz * XMC_SCU_CLOCK_GetCcuClockFrequency() / (1U << psiv);
}

// input: motor object
// modifies: 
static void ramp_to(dc_motor_t * motor, float new_duty) {
    // clamp out of range duty cycle
    if (new_duty > motor->config.max_duty_cycle) {
        new_duty = motor->config.max_duty_cycle;
    }
    else if (new_duty < 0.0f) {
        new_duty = 0.0f;
    }
    motor->curr_duty_cycle = new_duty;
    
    // calculate the new compare value
    uint32_t total_counts_in_period = counts_per_period(motor);
    uint32_t new_compare_value = (1 - new_duty) * total_counts_in_period;
    
    // gradually increment to new compare value from curr compare value
    uint32_t curr_compare_value = XMC_CCU4_SLICE_GetTimerCompareMatch(motor->config.timer_slice);
    int delta = (new_compare_value > curr_compare_value) ? 1 : -1;
    for (; curr_compare_value != new_compare_value; curr_compare_value += delta) {
        XMC_CCU4_SLICE_SetTimerCompareMatch(motor->config.timer_slice, curr_compare_value);
        XMC_CCU4_EnableShadowTransfer(motor->config.timer_module, motor->config.slice_mask);
        // risk of overflow, optimally clock with the period reset event
        XMC_DelayUs(motor->config.ramp_time_sec * 1e6f / total_counts_in_period);
    }
    
    // correct off-by-one error
    XMC_CCU4_SLICE_SetTimerCompareMatch(motor->config.timer_slice, new_compare_value);
    XMC_CCU4_EnableShadowTransfer(motor->config.timer_module, motor->config.slice_mask);
}

bool dc_motor_init(dc_motor_t *motor, const dc_motor_config_t *config) {    
    // update all members
    motor->curr_duty_cycle = 0;
    motor->config = *config;

    // check that passive low is set
    if((motor->config.timer_slice->PSL & CCU4_CC4_PSL_PSL_Msk) != 0U) {return false;}

    // compute PSC and set
    if (!(motor->config.PWM_frequency_hz > 0.0f)) { return false; }
    float psc_f = ceilf(log2f(1.0f/motor->config.PWM_frequency_hz * XMC_SCU_CLOCK_GetCcuClockFrequency()/65535.0f));
    // Check PSC <= 15
    if (psc_f > 15.0f) { return false; }
    int8_t PSC_value = (psc_f < 0.0f) ? 0 : (int8_t)psc_f;
    XMC_CCU4_SLICE_SetPrescaler(motor->config.timer_slice, PSC_value);

    // set PR and CR values
    volatile uint64_t period_val = counts_per_period(motor);
    if (period_val > 65535 || period_val < 2) {return false;}
    XMC_CCU4_SLICE_SetTimerPeriodMatch(motor->config.timer_slice, period_val - 1);
    XMC_CCU4_SLICE_SetTimerCompareMatch(motor->config.timer_slice, period_val);
    XMC_CCU4_EnableShadowTransfer(motor->config.timer_module, motor->config.slice_mask);
    XMC_CCU4_SLICE_StartTimer(motor->config.timer_slice);
    return true;
}

void dc_motor_set_duty_cycle(dc_motor_t *motor, float new_duty) {
    ramp_to(motor, new_duty);
}