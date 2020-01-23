/* 
 * File:                softwaretimer.h
 * Author:              Hylke
 * Comments:            Uses timer 1 to create programmable software timers
 */

// This is a guard condition so that contents of this file are not included more than once.  
#ifndef SOFTWARETIMER_H
#define	SOFTWARETIMER_H

#include <stdint.h>


#define SOFTWARETIMER_MAX_TIMERS        16
#define SOFTWARETIMER_NONE              -1
#define SOFTWARETIMER_SINGLE_MODE       0
#define SOFTWARETIMER_CONTINUOUS_MODE   1

  
// Initializes the programmable software timers
void softwaretimer_init(void);

// Creates a new timer
// Parameters:
//  mode            Timer mode, single or continuous. In single mode the timer will be freed upon expire.
//                  In continuous the timer will reset and count again upon expire.
// Returns:
//  The timer number created. Returns -1 if no free timer is available.
int8_t softwaretimer_create(uint8_t mode);

// Deletes a timer and frees its recources.
// Parameters:
//  timer_number    The timer to delete. This was returned when creating the timer
// Returns:
//  Returns 0 if successfully deleted. Returns -1 if timer_number is out of range.
int8_t softwaretimer_delete(uint8_t timer_number);

// Start a software timer
// Parameters:
//  timer_number    The timer to start. This was returned when creating the timer
//  ms              time till expire in ms
// Returns:
//  Returns 0 if successfully started. Returns -1 if timer_number is out of range.
int8_t softwaretimer_start(uint8_t timer_number, uint32_t ms);

// Stops a running timer.
// Parameters:
//  timer_number    The timer to stop. This was returned when creating the timer
// Returns:
//  returns 0 if successful stop of the timer.
//  -1 if the timer number was not a running timer or out of range.
int8_t softwaretimer_stop(uint8_t timer_number);

// Returns if a timer has expired and clears the expire mark.
// Parameters:
//  timer_number    The timer to check. This number was return when the timer was started.
// Returns:
//  True if the timer is expired, false is the timer is not expired.
//  -1 if the timer number was not a running timer or out of range.
int8_t softwaretimer_get_expired(uint8_t timer_number);


#endif	/* SOFTWARETIMER_H */

