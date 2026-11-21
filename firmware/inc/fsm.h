#ifndef FSM_H
#define FSM_H

#include "rpi_comms.h"
#include "stepper.h"
#include "xmc_ccu4.h"
#include "cycfg_pins.h"

#define ADVANCE_STEP_MARGIN 10

// states
typedef enum {
    STATE_IDLE,
    STATE_FOLLOW,
    STATE_ADVANCE,
    STATE_DONE,
    STATE_COUNT // just number of states
    // TODO add more     
} state_t;


// holds current state of everything -> only instantiate 1 copy
// and pass by pointer
typedef struct {
    rpi_inp_t from_rpi;
    bool rpi_rd_valid;
    rpi_out_t to_rpi;

    bool left;
    bool right;
    bool center;
    stepper turret;

    int64_t start_step_pos;
    int64_t advance_start;
    int64_t player_angles[16]; // is this how we want to store players? yes probs
    int num_players;

    bool start_scan;


    // TODO add more
    
} fsm_ctxt_t;


state_t state_idle_run(fsm_ctxt_t *ctxt);
state_t state_follow_run(fsm_ctxt_t *ctxt);
state_t state_advance_run(fsm_ctxt_t *ctxt);
state_t state_done_run(fsm_ctxt_t *ctxt);

void fsm_step(fsm_ctxt_t *ctxt);

void context_init(fsm_ctxt_t *ctxt);

void set_stepper_timer(uint16_t rpm);


#endif
