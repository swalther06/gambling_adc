#include "fsm.h"
#include "rpi_comms.h"
#include "SEGGER_RTT.h"

state_t state_idle_run(fsm_ctxt_t *ctxt) {
    if (ctxt->start_scan) {
        ctxt->start_scan = 0;
        ctxt->to_rpi.reset = false;
        return STATE_FOLLOW;
    }

    ctxt->start_step_pos = stepper_get_pos(&ctxt->turret);
    return STATE_IDLE;


}


state_t state_follow_run(fsm_ctxt_t *ctxt) {
    if (ctxt->rpi_rd_valid) {
        ctxt->left = ctxt->from_rpi.left;
        ctxt->right = ctxt->from_rpi.right;
        ctxt->center = ctxt->from_rpi.center;
    }

    int64_t step_pos = stepper_get_pos(&ctxt->turret);
    int64_t steps_turned = step_pos - ctxt->start_step_pos;

    if (steps_turned >= REVOLUTION_STEPS || steps_turned <= -REVOLUTION_STEPS) {// full rotation
        ctxt->start_step_pos = step_pos;
        ctxt->to_rpi.reset = true;
        return STATE_DONE;
    }

    if (ctxt->left) {
        stepper_rot_ccw(&ctxt->turret);
    } else if (ctxt->right) {
        stepper_rot_cw(&ctxt->turret);
    } else if (!ctxt->center) {
        if (ctxt->to_rpi.dir) // clockwise
            stepper_rot_cw(&ctxt->turret);
        else
            stepper_rot_ccw(&ctxt->turret);
    }

    if (ctxt->from_rpi.player) {
        ctxt->player_angles[ctxt->num_players++] = step_pos;
        ctxt->advance_start = step_pos;
        ctxt->to_rpi.reset = true;
        return STATE_ADVANCE;
    } else if (ctxt->from_rpi.skip) {
        ctxt->advance_start = step_pos;
        ctxt->to_rpi.reset = true;
        return STATE_ADVANCE;
    }

    return STATE_FOLLOW;
}


state_t state_advance_run(fsm_ctxt_t *ctxt) {
    if (!ctxt->rpi_rd_valid)
        return STATE_ADVANCE;

    int64_t steps_turned = stepper_get_pos(&ctxt->turret) - ctxt->advance_start;

    if (steps_turned < ADVANCE_STEP_MARGIN && steps_turned > -ADVANCE_STEP_MARGIN) {
        if (ctxt->to_rpi.dir) { // clockwise
            stepper_rot_cw(&ctxt->turret);
        } else {
            stepper_rot_ccw(&ctxt->turret);
        }

        return STATE_ADVANCE;
    }
    
    if (ctxt->from_rpi.player || ctxt->from_rpi.skip)
        return STATE_ADVANCE;

    ctxt->to_rpi.reset = false;
    return STATE_FOLLOW;
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


void context_init(fsm_ctxt_t *ctxt) {
    ctxt->turret = stepper_factory(DIR_PORT, 
                                    DIR_PIN, 
                                    STEP_PORT, 
                                    STEP_PIN, 
                                    SLEEP_PORT, 
                                    SLEEP_PIN);
    stepper_init(&ctxt->turret);
    ctxt->start_scan = 1;
}





