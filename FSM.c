/******************************************************************************
* File Name:   main.c
*
* Description: The FSM
*
* Notes: The GameContext struct is the global variables (a better way).
*           If you have any globals, add them there. It is passed along via a
*           pointer to the one declared in main.
*
******************************************************************************/

#include "cybsp.h"
#include "cy_utils.h"
#include "xmc_gpio.h"
#include "nhd_lib.h"
#include "lidar07.h"

typedef struct {
    bool is_holdem;
    uint8_t num_players;
} GameContext;

// the states from our FSM diagram
typedef enum {
    STATE_CONFIG,
    STATE_SCAN,
    STATE_READY,
    STATE_INIT_DEAL,
    STATE_IDLE,
    STATE_NEXT_HAND,
    STATE_FLOP,
    STATE_TURN,
    STATE_RIVER,
    STATE_DEAL_X_FLOP,
    STATE_DEAL_X_TURN,
    STATE_DEAL_X_RIVER,
    STATE_EJECT_DECK,
    STATE_COUNT // just used for the declaration of the state table as size
} State;

// Forward declarations for all state functions
State state_config_run(GameContext *ctx);
State state_scan_run(GameContext *ctx);
State state_ready_run(GameContext *ctx);
State state_init_deal_run(GameContext *ctx);
State state_idle_run(GameContext *ctx);
State state_next_hand_run(GameContext *ctx);
State state_flop_run(GameContext *ctx);
State state_turn_run(GameContext *ctx);
State state_river_run(GameContext *ctx);
State state_deal_x_flop_run(GameContext *ctx);
State state_deal_x_turn_run(GameContext *ctx);
State state_deal_x_river_run(GameContext *ctx);
State state_eject_deck_run(GameContext *ctx);



typedef State (*StateFunc)(GameContext *ctx);

static const StateFunc state_table[STATE_COUNT] = {
    [STATE_CONFIG]          = state_config_run,
    [STATE_SCAN]            = state_scan_run,
    [STATE_READY]           = state_ready_run,
    [STATE_INIT_DEAL]       = state_init_deal_run,
    [STATE_IDLE]            = state_idle_run,
    [STATE_NEXT_HAND]       = state_next_hand_run,
    [STATE_FLOP]            = state_flop_run,
    [STATE_TURN]            = state_turn_run,
    [STATE_RIVER]           = state_river_run,
    [STATE_DEAL_X_FLOP]     = state_deal_x_flop_run,
    [STATE_DEAL_X_TURN]     = state_deal_x_turn_run,
    [STATE_DEAL_X_RIVER]    = state_deal_x_river_run,
    [STATE_EJECT_DECK]      = state_eject_deck_run,
};


//!--------------------------------------------------------------
//! Below is where you write the code for each of the states.
//!--------------------------------------------------------------


State state_config_run(GameContext *ctx) {
    // domenu(ctx); // my function that printed the menu and set the globals
    return STATE_SCAN;
}

State state_scan_run(GameContext *ctx) {
    // implement logic for scanning
    // read sensor data, store in data structure
}

State state_ready_run(GameContext *ctx) {
    // implement logic for the ready state
    // print "load deck, and press middle button to cont" or something like that
}

State state_init_deal_run(GameContext *ctx) {
    // if misdeal, go back to STATE_READY
    // otherwise print "press middle button for flop" and do whats needed for that
    // also implement running it x times (have menu for chosing deal turn or run x times)
}

State state_idle_run(GameContext *ctx) {
    // misdeal button press goes to STATE_INIT_DEAL
    // next hand button goes to STATE_NEXT_HAND
    // run it x times goes to STATE_DEAL_X_FLOP
}

State state_next_hand_run(GameContext *ctx) {
    // increment dealer index
    // proceed to STATE_EJECT_DECK 
}

State state_eject_deck_run(GameContext *ctx) {
    // eject the deck
    // move to STATE_READY
}


// ! the 6 states below could have logic for misdeals if our thing is bad.
State state_flop_run(GameContext *ctx) {
    // deal either turn or deal turn x times
    // or if next hand button is pressed go to STATE_NEXT_HAND
}

State state_turn_run(GameContext *ctx) {
    //deal rither river or deal river x times
    // or if next hand button is pressed go to STATE_NEXT_HAND
}

State state_river_run(GameContext *ctx) {
    // go to next hand state on button push
}

State state_deal_x_flop_run(GameContext *ctx) {
    // continue button press goes to STATE_DEAL_X_TURN
}

State state_deal_x_turn_run(GameContext *ctx) {
    // continue button press goes to STATE_DEAL_X_RIVER
}

State state_deal_x_river_run(GameContext *ctx) {
    // continue button press goes to STATE_NEXT_HAND
}


int main(){
    cybsp_init();
    GameContext ctx = { .is_holdem = true, .num_players = 4 };

    State current_state = STATE_CONFIG;
    //State last_state = STATE_COUNT; // useful to detect on-entry logic

    while (1) {
        // Optional: Entry action when switching into a new state
        // if (current_state != last_state) {
        //     on_state_enter(current_state, &ctx);
        //     last_state = current_state;
        // }
        // Run the current state logic
        if (state_table[current_state] != NULL) {
            current_state = state_table[current_state](&ctx);
        }
    }
}
/* [] END OF FILE */
    