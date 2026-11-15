#include "stepper.h"
#include "xmc4_gpio.h"
#include "xmc_gpio.h"
#include <stdint.h>


//factory function to create stepper object and return by value
//position and direction set to 0 by default 
stepper stepper_factory(XMC_GPIO_PORT_t *const dirport, const uint8_t dirpin,
                                   XMC_GPIO_PORT_t *const stepport, const uint8_t steppin,
                                   XMC_GPIO_PORT_t *const sleepport, const uint8_t sleeppin){
    stepper instance = {dirport, dirpin, 
                        stepport, steppin,
                        sleepport, sleeppin,
                        0, 0};
    return instance; 
}


//config struct set to medium strength, push/pull, output 0
static const XMC_GPIO_CONFIG_t stpr_gpio_config_low = {XMC_GPIO_MODE_OUTPUT_PUSH_PULL, XMC_GPIO_OUTPUT_LEVEL_LOW,XMC_GPIO_OUTPUT_STRENGTH_MEDIUM};
//config struct set to medium strength, push/pull, output 1
static const XMC_GPIO_CONFIG_t stpr_gpio_config_high = {XMC_GPIO_MODE_OUTPUT_PUSH_PULL, XMC_GPIO_OUTPUT_LEVEL_HIGH,XMC_GPIO_OUTPUT_STRENGTH_MEDIUM};


//direction pin set to medium strength, push/pull, output 0
static void stepper_init_direction(stepper *const uniqueStepper){
    XMC_GPIO_Init(uniqueStepper->DIRPORT, uniqueStepper->DIRPIN, &stpr_gpio_config_low);
    uniqueStepper->direction = 0; 
}

//step pin set to medium strength, push/pull, output 0
static void stepper_init_step(stepper *const uniqueStepper){
    XMC_GPIO_Init(uniqueStepper->STEPPORT, uniqueStepper->STEPPIN, &stpr_gpio_config_low);
}

//sleep pin set to medium strength, push/pull, output 1
static void stepper_init_sleep(stepper *const uniqueStepper){
    XMC_GPIO_Init(uniqueStepper->SLEEPPORT, uniqueStepper->SLEEPPIN, &stpr_gpio_config_high);
}

void stepper_init(stepper *const uniqueStepper){
    //initalize direction 
    stepper_init_direction(uniqueStepper); 

    //intalize step 
    stepper_init_step(uniqueStepper); 

    //intalize sleep
    stepper_init_sleep(uniqueStepper); 

    uniqueStepper->position = 0;
}

void stepper_set_dir_ccw(stepper *const uniqueStepper){
    XMC_GPIO_SetOutputHigh(uniqueStepper->DIRPORT, uniqueStepper->DIRPIN); 
    uniqueStepper->direction = 0; 
    for(volatile int i = 0; i < 500; ++i){} //buffer to allow driver to register direction
}

void stepper_set_dir_cw(stepper *const uniqueStepper){
    XMC_GPIO_SetOutputLow(uniqueStepper->DIRPORT, uniqueStepper->DIRPIN);
    uniqueStepper->direction = 1;
    for(volatile int i = 0; i < 500; ++i){} //buffer to allow driver to register direction 
}

void stepper_increment(stepper *const uniqueStepper){
    XMC_GPIO_SetOutputLow(uniqueStepper->STEPPORT, uniqueStepper->STEPPIN); 
    for(volatile int i = 0; i < 500; ++i){} //buffer to allow driver to process
    XMC_GPIO_SetOutputHigh(uniqueStepper->STEPPORT, uniqueStepper->STEPPIN); 
    for(volatile int i = 0; i < 500; ++i){} //buffer to allow driver to process
    XMC_GPIO_SetOutputLow(uniqueStepper->STEPPORT, uniqueStepper->STEPPIN); 
    for(volatile int i = 0; i < 30000; ++i){} //buffer to allow motor to physically move. MUST TUNE AS WEIGHT INCREASES
    (uniqueStepper->direction == 1)? ++(uniqueStepper->position) : --(uniqueStepper->position);
}

void stepper_reposition(int64_t wanted_pos, stepper *const uniqueStepper){
    (wanted_pos > uniqueStepper->position)? stepper_set_dir_cw(uniqueStepper):stepper_set_dir_ccw(uniqueStepper);
    while(wanted_pos != uniqueStepper->position){stepper_increment(uniqueStepper);}
}


void stepper_revolution(uint64_t direction, stepper *const uniqueStepper){
    assert(direction == 0 || direction == 1); 
    (direction == 0)? stepper_reposition(uniqueStepper->position - 200, uniqueStepper) :stepper_reposition(uniqueStepper->position + 200, uniqueStepper); 
}


