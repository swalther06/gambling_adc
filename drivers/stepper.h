#include <stdint.h>
#include <xmc_gpio.h>


#ifndef STEPPER
#define STEPPER

// Stepper rotation direction 
// Which DIR pin level gives CW vs CCW depends on coil wiring.
typedef enum : uint8_t{
    STEPPER_DIR_FWD,
    STEPPER_DIR_REV 
} StepperDirection;


// Stepper driver module (opaque handle)
// Usage:
//   1. Create:  stepper *motor = stepper_factory(dirport, dirpin, 
//                                                stepport, steppin, 
//                                                sleepport, sleeppin,
//                                                total_steps, gear_ratio);
//   returns NULL if no slots are left or gear_ratio <= 0 or total_steps <= 0 
//   2. Init:    stepper_init(motor);   //configures GPIO and wakes driver
//   3. Move:    stepper_reposition(motor, target, delay);
//
// Wiring (driver pins):
//   DIR:   level sets rotation direction; which level is FWD/REV depends on coil wiring
//   STEP:  one step per rising edge (low -> high)
//   SLEEP: high = driver awake, coils energized; low = asleep, motor turns freely
//
// stepper is an opaque handle. its contents are private to stepper.c.
// Use the stepper_* functions to control it and read its state.
typedef struct stepper stepper;


//input:    GPIO port/pin for the driver's DIR, STEP, and SLEEP lines
//          number of steps in one revolution of stepper motor (total_steps cannot <= 0, if <=0 will return null pointer)
//          gear ratio where the ratio is ratio:1 (gear_ratio cannot <= 0, if <=0 will return null pointer)
//modifies: adds stepper object to internal array of steppers
//          position set to 0 and direction set to FWD by default. 
//returns:  stepper object pointer or NULL if no slots remain 
stepper *stepper_factory(XMC_GPIO_PORT_t *const dirport, const uint8_t dirpin,
                         XMC_GPIO_PORT_t *const stepport, const uint8_t steppin,
                         XMC_GPIO_PORT_t *const sleepport, const uint8_t sleeppin,
                         const uint16_t total_steps, const float gear_ratio);


//input: specific stepper struct pointer
//modifies: configures gpio for step, direction, and sleep line. Configures I/O and intializes output. Resets position. 
//returns: nothing
void stepper_init(stepper *const uniqueStepper);

//input:    specific stepper struct pointer
//modifies: nothing
//returns:  last set direction
StepperDirection stepper_get_direction(const stepper *const uniqueStepper);

//input:    specific stepper struct pointer
//modifies: nothing
//returns:  current absolute position in steps, relative to where stepper_init was called
int64_t stepper_get_position_steps(const stepper *const uniqueStepper);

//input:    specific stepper struct pointer 
//modifies: nothing
//returns:  current absolute position in degrees (gearbox adjusted), rounded to nearest integer, relative to where stepper_init was called
int64_t stepper_get_position_degrees(const stepper *const uniqueStepper);

//input: stepper pointer, absolute position to move to in degrees (gear box adjusted), increment delay in ms
//modifies: moves stepper to desired position (gear box adjusted) at desired increment delay, updates position and direction
//returns: nothing
//NOTE: the function does not return until move finishes i.e the processor is occupied during duration of movement
void stepper_reposition(stepper *const uniqueStepper, const int64_t wanted_pos, const uint32_t delay);

//input: stepper pointer, direction of spin, stepper increment delay in ms 
//modifies: moves stepper one revolution (gear box adjusted) in desired direction at desired increment delay, updates position and direction
//returns: nothing
//NOTE: the function does not return until move finishes i.e the processor is occupied during duration of movement
void stepper_revolution(stepper *const uniqueStepper, const StepperDirection dir, const uint32_t delay); 

#endif