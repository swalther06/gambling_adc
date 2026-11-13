#include "fsm.h"
#include "rpi_comms.h"

state_t state_idle_run(fsm_ctxt_t *ctxt) {
    if (ctxt->start_scan) {
        ctxt->to_rpi.reset = false;
        return STATE_FOLLOW;
    }

    return STATE_FOLLOW;


}


state_t state_follow_run(fsm_ctxt_t *ctxt) {
    if (ctxt->from_rpi.left) {
        // drive right motor signal low
        // drive left motor signal high
    } else if (ctxt->from_rpi.right) {
        // drive right motor signal high
        // drive left motor signal low
    } else if (ctxt->from_rpi.center) {
        // drive right motor signal low
        // drive left motor signal low
    }

    if (ctxt->from_rpi.player) {
        ctxt->player_angles[ctxt->num_players++] = ctxt->pan_angle;
        ctxt->to_rpi.reset = true;
        return STATE_ADVANCE;
    } else if (ctxt->from_rpi.skip) {
        ctxt->to_rpi.reset = true;
        return STATE_ADVANCE;
    }

    if (ctxt->pan_angle >= 360.0) {// full rotation
        ctxt->to_rpi.reset = true;
        return STATE_DONE;
    }

    return STATE_FOLLOW;
}


state_t state_advance_run(fsm_ctxt_t *ctxt) {
    if (ctxt->to_rpi.dir) { // clockwise
        if (ctxt->pan_angle > ctxt->player_angles[ctxt->num_players-1] + 10/*MARGIN*/)
            return STATE_FOLLOW;
        
        // drive turret clockwise
    } else { // counterclockwise
        if (ctxt->pan_angle < ctxt->player_angles[ctxt->num_players-1] - 10 /*MARGIN*/)
            return STATE_FOLLOW;

        // drive turret counterclockwise
    }
}


state_t state_done_run(fsm_ctxt_t *ctxt) {
    return STATE_DONE;
}





typedef state_t (*state_fn)(fsm_ctxt_t *);

static const state_fn state_table[STATE_COUNT] = {
    [STATE_IDLE] = state_idle_run,
    [STATE_FOLLOW] = state_follow_run,
    [STATE_ADVANCE] = state_advance_run,
    [STATE_DONE] = state_done_run,
};

static state_t current = STATE_IDLE;

void fsm_step(fsm_ctxt_t *ctxt) {
    current = state_table[current](ctxt);
}





