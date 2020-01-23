/*
 * File:   softwaretimer.c
 * Author: Hylke
 *
 * Created on February 9, 2019, 2:16 PM
 */

#include <xc.h>
#include <stdint.h>
#include "softwaretimer.h"
#include "mcc_generated_files/tmr1.h"


// Timer resources
static volatile struct{
    uint32_t set_value_ms;
    uint32_t time_left_ms;
    uint8_t used : 1;
    uint8_t mode : 1;
    uint8_t running : 1;
    uint8_t expired : 1;
} softwaretimers[SOFTWARETIMER_MAX_TIMERS] = {};


// Timer 1 interrupt. Triggers every 1 ms
void softwaretimer_interrupt_callback(void) {
    uint8_t timer_number;
    
    // Check all timers
    for (timer_number = 0; timer_number < SOFTWARETIMER_MAX_TIMERS; timer_number++) {
        // Skip non used and non running timers
        if (softwaretimers[timer_number].used == 0 || softwaretimers[timer_number].running == 0) {
            continue;
        }
        // Decrease time left
        if (softwaretimers[timer_number].time_left_ms != 0) {
            softwaretimers[timer_number].time_left_ms--;
        }
        // Check if expired
        if (softwaretimers[timer_number].time_left_ms == 0) {
            softwaretimers[timer_number].expired = 1;
            // Restart if continuous mode and stop if single
            if (softwaretimers[timer_number].mode == SOFTWARETIMER_CONTINUOUS_MODE) {
                softwaretimers[timer_number].time_left_ms = softwaretimers[timer_number].set_value_ms;
            } else {
                softwaretimers[timer_number].running = 0;
            }
        }
    }
}

// Initializes the programmable software timers
void softwaretimer_init(void) {
    // Timer 1 is already initialized in the system code
    TMR1_SetInterruptHandler(&softwaretimer_interrupt_callback);
}

// Creates a new timer
// Parameters:
//  mode            Timer mode, single or continuous. In single mode the timer will be freed upon expire.
//                  In continuous the timer will reset and count again upon expire.
// Returns:
//  The timer number created. Returns -1 if no free timer is available.
int8_t softwaretimer_create(uint8_t mode) {
    uint8_t timer_number;
    
    // Check mode boundary
    if (!(SOFTWARETIMER_SINGLE_MODE <= mode && mode <= SOFTWARETIMER_CONTINUOUS_MODE)) {
        return -1;
    }
    
    // Get first free timer;
    for (timer_number = 0; timer_number < SOFTWARETIMER_MAX_TIMERS; timer_number++) {
        if (softwaretimers[timer_number].used == 0) {
            break;
        }
    }
    // Check if free timer is found
    if (timer_number == SOFTWARETIMER_MAX_TIMERS) {
        return -1;
    }
    // Create timer
    softwaretimers[timer_number].mode = mode;
    softwaretimers[timer_number].expired = 0;
    softwaretimers[timer_number].set_value_ms = 0;
    softwaretimers[timer_number].time_left_ms = 0;
    softwaretimers[timer_number].running = 0;
    softwaretimers[timer_number].used = 1;
    // Return timer no
    return timer_number;
}

// Deletes a timer and frees its recources.
// Parameters:
//  timer_number    The timer to delete. This was returned when creating the timer
// Returns:
//  Returns 0 if successfully deleted. Returns -1 if timer_number is out of range.
int8_t softwaretimer_delete(uint8_t timer_number) {
    // Check timer number boundary
    if (timer_number >= SOFTWARETIMER_MAX_TIMERS) {
        return -1;
    }
    // Delete timer
    softwaretimers[timer_number].used = 0;
    return 0;
}

// Start a software timer
// Parameters:
//  timer_number    The timer to start. This was returned when creating the timer
//  ms              time till expire in ms
// Returns:
//  Returns 0 if successfully started. Returns -1 if timer_number is out of range.
int8_t softwaretimer_start(uint8_t timer_number, uint32_t ms) {
    // Check timer number boundary
    if (timer_number >= SOFTWARETIMER_MAX_TIMERS) {
        return -1;
    }
    // Check if timer was created
    if (softwaretimers[timer_number].used == 0) {
        return -1;
    }
    // Start timer
    softwaretimers[timer_number].set_value_ms = ms;
    softwaretimers[timer_number].time_left_ms = ms;
    softwaretimers[timer_number].running = 1;
    return 0;
}

// Stops a running timer.
// Parameters:
//  timer_number    The timer to stop. This was returned when creating the timer
// Returns:
//  returns 0 if successful stop of the timer.
//  -1 if the timer number was not a running timer or out of range.
int8_t softwaretimer_stop(uint8_t timer_number) {
    // Check timer number boundary
    if (timer_number >= SOFTWARETIMER_MAX_TIMERS) {
        return -1;
    }
    // Check if timer was created
    if (softwaretimers[timer_number].used == 0) {
        return -1;
    }
    softwaretimers[timer_number].running = 0;
    return 0;
}

// Returns if a timer has expired and clears the expire mark.
// Parameters:
//  timer_number    The timer to check. This number was return when the timer was started.
// Returns:
//  True if the timer is expired, false is the timer is not expired.
//  -1 if the timer number was not a running timer or out of range.
int8_t softwaretimer_get_expired(uint8_t timer_number) {
    // Check timer number boundary
    if (timer_number >= SOFTWARETIMER_MAX_TIMERS) {
        return -1;
    }
    if (softwaretimers[timer_number].expired == 1) {
        softwaretimers[timer_number].expired = 0;
        return 1;
    } else {
        return 0;
    }
}
