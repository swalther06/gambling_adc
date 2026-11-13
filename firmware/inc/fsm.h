#ifndef FSM_H
#define FSM_H

#include "stdbool.h"
#include "rpi_comms.h"

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
    rpi_out_t to_rpi;

    float pan_angle;
    float player_angles[8]; // is this how we want to store players?
    int num_players;

    bool start_scan;


    // TODO add more
    
} fsm_ctxt_t;


state_t state_idle_run(fsm_ctxt_t *ctxt);
state_t state_follow_run(fsm_ctxt_t *ctxt);
state_t state_advance_run(fsm_ctxt_t *ctxt);
state_t state_done_run(fsm_ctxt_t *ctxt);


#endif
