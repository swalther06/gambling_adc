#include "stepper.h"
#include "xmc_common.h"
#include "xmc_gpio.h"
#include <stdint.h>

//stepper struct
//defines GPIO pins for DIRECTION, STEP, SLEEP pins on driver and stepper total steps and gear ratio
//maintains last set direction and  global position of stepper in steps (not in degrees)
struct stepper{
    XMC_GPIO_PORT_t *dirport; 
    uint8_t dirpin; 
    XMC_GPIO_PORT_t *stepport; 
    uint8_t steppin; 
    XMC_GPIO_PORT_t *sleepport; 
    uint8_t sleeppin; 
    
    uint16_t total_steps;
    float gear_ratio; 

    StepperDirection direction; 
    int64_t position;
};

enum{STEPPERS_CAPACITY = 5}; //global arrays size determined at build time thus must be compile time constant instead of varaible
static stepper steppers[STEPPERS_CAPACITY]; //array to hold stepper object created by factory fcn
static uint8_t steppers_count = 0; //count tracker 

//factory function to create stepper object and return by pointer
//position set to 0 and direction set to forward by default 
stepper *stepper_factory(XMC_GPIO_PORT_t *const dirport, const uint8_t dirpin,
                         XMC_GPIO_PORT_t *const stepport, const uint8_t steppin,
                         XMC_GPIO_PORT_t *const sleepport, const uint8_t sleeppin,
                         const uint16_t total_steps, const float gear_ratio){

    stepper instance = {dirport, dirpin, 
                        stepport, steppin,
                        sleepport, sleeppin,
                        total_steps, gear_ratio,
                        STEPPER_DIR_FWD, 0};

    if(gear_ratio <= 0){return NULL;}
    if(total_steps <= 0){return NULL;}

    if (steppers_count >= STEPPERS_CAPACITY){return NULL;} //return NULL if steppers array is full
    steppers[steppers_count++] = instance; //copy the local copy into steppers
    return &steppers[steppers_count-1]; //return address of non-local copy 
}


//config struct set to medium strength, push/pull, output 0
static const XMC_GPIO_CONFIG_t stpr_gpio_config_low = {XMC_GPIO_MODE_OUTPUT_PUSH_PULL, XMC_GPIO_OUTPUT_LEVEL_LOW,XMC_GPIO_OUTPUT_STRENGTH_MEDIUM};
//config struct set to medium strength, push/pull, output 1
static const XMC_GPIO_CONFIG_t stpr_gpio_config_high = {XMC_GPIO_MODE_OUTPUT_PUSH_PULL, XMC_GPIO_OUTPUT_LEVEL_HIGH,XMC_GPIO_OUTPUT_STRENGTH_MEDIUM};


//direction pin set to medium strength, push/pull, output 0
static void stepper_init_direction(stepper *const uniqueStepper){
    XMC_GPIO_Init(uniqueStepper->dirport, uniqueStepper->dirpin, &stpr_gpio_config_low);
    uniqueStepper->direction = STEPPER_DIR_FWD; 
}

//step pin set to medium strength, push/pull, output 0
static void stepper_init_step(stepper *const uniqueStepper){
    XMC_GPIO_Init(uniqueStepper->stepport, uniqueStepper->steppin, &stpr_gpio_config_low);
}

//sleep pin set to medium strength, push/pull, output 1
static void stepper_init_sleep(stepper *const uniqueStepper){
    XMC_GPIO_Init(uniqueStepper->sleepport, uniqueStepper->sleeppin, &stpr_gpio_config_high);
}

void stepper_init(stepper *const uniqueStepper){
    //initialize direction 
    stepper_init_direction(uniqueStepper); 

    //initialize step 
    stepper_init_step(uniqueStepper); 

    //initialize sleep
    stepper_init_sleep(uniqueStepper); 

    uniqueStepper->position = 0;

    XMC_Delay(2u);//delay for driver to wake after turned on with sleep pin

}


StepperDirection stepper_get_direction(const stepper *const uniqueStepper){
    return uniqueStepper->direction;
}

int64_t stepper_get_position_steps(const stepper *const uniqueStepper){
    return uniqueStepper->position;
}

int64_t stepper_get_position_degrees(const stepper *const uniqueStepper){
    if(uniqueStepper->position >=0){
        return (int64_t)(uniqueStepper->position * 360 / (float)uniqueStepper->total_steps / uniqueStepper->gear_ratio + 0.5f); //steps converted to degrees (nearest int)
    }
    else{
        return (int64_t)(uniqueStepper->position * 360 / (float)uniqueStepper->total_steps / uniqueStepper->gear_ratio - 0.5f); //steps converted to degrees (nearest int)
    }
}


//sets direction to FWD or REV
static void stepper_set_dir(stepper *const uniqueStepper, const StepperDirection dir){
   
    if(dir == STEPPER_DIR_FWD){
        XMC_GPIO_SetOutputLow(uniqueStepper->dirport, uniqueStepper->dirpin); 
        uniqueStepper->direction = STEPPER_DIR_FWD; 
    }
    else{
        XMC_GPIO_SetOutputHigh(uniqueStepper->dirport, uniqueStepper->dirpin);
        uniqueStepper->direction = STEPPER_DIR_REV;
    }

    XMC_DelayUs(2); //buffer to allow driver to register direction
}

//increments stepper motor with increment_delay as the delay in milliseconds between each step
//the delay allows motor shaft to physically move before attempting to step again
//this delay must be tuned as weight increases 
static void stepper_increment(stepper *const uniqueStepper, const uint32_t increment_delay){
    XMC_GPIO_SetOutputLow(uniqueStepper->stepport, uniqueStepper->steppin); 
    XMC_DelayUs(2); //buffer to allow driver to process
    XMC_GPIO_SetOutputHigh(uniqueStepper->stepport, uniqueStepper->steppin); 
    XMC_DelayUs(2); //buffer to allow driver to process
    XMC_GPIO_SetOutputLow(uniqueStepper->stepport, uniqueStepper->steppin); 
    XMC_Delay(increment_delay); 
    (uniqueStepper->direction == STEPPER_DIR_FWD)? ++(uniqueStepper->position) : --(uniqueStepper->position);
}

//used for moving to a certian step. all units in steps 
static void stepper_reposition_steps(stepper *const uniqueStepper, const int64_t wanted_pos_step, const uint32_t delay){
    (wanted_pos_step > uniqueStepper->position)? stepper_set_dir(uniqueStepper, STEPPER_DIR_FWD):stepper_set_dir(uniqueStepper,STEPPER_DIR_REV);
    while(wanted_pos_step != uniqueStepper->position){stepper_increment(uniqueStepper, delay);}
}

void stepper_reposition(stepper *const uniqueStepper, const int64_t wanted_pos, const uint32_t delay){
    //convert degress to position 
    int64_t wanted_pos_step;
    if(wanted_pos >=0){
        wanted_pos_step = (int64_t)(wanted_pos * uniqueStepper->gear_ratio / 360 * uniqueStepper->total_steps + 0.5f);  
    }
    else{
        wanted_pos_step = (int64_t)(wanted_pos * uniqueStepper->gear_ratio / 360 * uniqueStepper->total_steps - 0.5f);  
    }

    stepper_reposition_steps(uniqueStepper, wanted_pos_step, delay);
}

void stepper_revolution(stepper *const uniqueStepper, const StepperDirection dir, const uint32_t delay){
    int64_t revolution_steps = (int64_t)(uniqueStepper->total_steps * uniqueStepper->gear_ratio + 0.5f);
    if(dir == STEPPER_DIR_FWD){
        stepper_reposition_steps(uniqueStepper, 
                                 uniqueStepper->position + revolution_steps, 
                                 delay);
    }
    else{
        stepper_reposition_steps(uniqueStepper, 
                                 uniqueStepper->position - revolution_steps, 
                                 delay);
    }
}


