#include <xmc_gpio.h>


#ifndef STEPPER
#define STEPPER

//define ports/pins for dir, sleep, step pins 
typedef struct{
    XMC_GPIO_PORT_t *const DIRPORT; 
    const uint8_t DIRPIN; 
    XMC_GPIO_PORT_t *const STEPPORT; 
    const uint8_t STEPPIN; 
    XMC_GPIO_PORT_t *const SLEEPPORT; 
    const uint8_t SLEEPPIN; 

    uint8_t direction; 
    int64_t position;
} stepper;

//input: stepper details
//modifies: nothing 
//returns: stepper object by value
stepper stepper_factory(XMC_GPIO_PORT_t *const dirport, const uint8_t dirpin,
                                   XMC_GPIO_PORT_t *const stepport, const uint8_t steppin,
                                   XMC_GPIO_PORT_t *const sleepport, const uint8_t sleeppin);

//input: specific stepper struct pointer
//modifies: configures gpio for step, direction, and enable line. Configures I/O and intializes output. Resets position. 
//returns: nothing
void stepper_init(stepper *const uniqueStepper);

//input: specific stepper struct pointer
//modifies: sets direction to ccw
//returns: nothing
void stepper_set_dir_ccw(stepper *const uniqueStepper);

//input: specific stepper struct pointer
//modifies: sets direction to cw
//returns: nothing
void stepper_set_dir_cw(stepper *const uniqueStepper);

//input: specific stepper struct pointer
//modifies: Increments motor by one unit and updates position
//returns: nothing
void stepper_increment(stepper *const uniqueStepper);

//input: position to move to, specific stepper struct pointer
//modifies: moves stepper to desired position 
//returns: nothing
void stepper_reposition(int64_t wanted_pos, stepper *const uniqueStepper);

//input: direction: 0 == ccw, 1 == cw, specific stepper struct pointer
//modifies: moves stepper one revolution in desired direction 
//returns: nothing
void stepper_revolution(uint64_t direction, stepper *const uniqueStepper); 

#endif