/*
 * File:   main.c
 * Author: Hylke
 *
 * Created on February 17, 2019, 4:46 PM
 */

#include <stdint.h>
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/watchdog.h"

#include "softwaretimer.h"
#include "debugprint.h"
#include "canbus.h"
#include "sd_logger.h"
#include "gps.h"

// Main application
int main(void) {
    int8_t one_sec_timer, led_timer, switch_counter = 0;
    uint32_t time_since_boot_sec = 0;
    
    // initialize the device
    SYSTEM_Initialize();
    
    // Set leds
    IO_LED_R_SetLow();
    IO_LED_G_SetHigh();
    
    debugprint_string("Hello universe!\r\nBecause greeting the world is thinking too small...\r\n");
    
    // Init software
    // Create timers
    softwaretimer_init();
    one_sec_timer = softwaretimer_create(SOFTWARETIMER_CONTINUOUS_MODE);
    softwaretimer_start(one_sec_timer, 1000);
    led_timer = softwaretimer_create(SOFTWARETIMER_SINGLE_MODE);
    // CAN bus
    can_bus_init();
    // SD card
    while (1) {
        // Try each second to init the sd card.
        // If succeeded continue.
        // After 128 seconds the watchdog timer expires and the uc is reset
        if (softwaretimer_get_expired(one_sec_timer)) {
            if (sd_logger_init() == 0) {
                // no errors
                break;
            }
            IO_LED_R_Toggle();
        }
    }
    
    IO_LED_R_SetLow();
    IO_LED_G_SetLow();
    
    while (1) {
        // Kick the dog
        WATCHDOG_TimerClear();
        
        // Receive all the data and then log it.
        can_bus_process();
        gps_handler();
        sd_logger_process();
        
        // Triggers every 1 sec
        if (softwaretimer_get_expired(one_sec_timer) == 1) {
            softwaretimer_start(led_timer, 50);
            IO_LED_G_SetHigh();
            
            time_since_boot_sec++;
            debugprint_string("Time since bootup: ");
            debugprint_uint(time_since_boot_sec);
            debugprint_string("sec\r\n");
            
            if(!IO_SWITCH_GetValue()) {
                switch_counter++;
            } else {
                switch_counter = 0;
            }
        }
        
        // Triggers 50ms after each 1 second trigger. Used to turn the led off
        if (softwaretimer_get_expired(led_timer) == 1) {
            IO_LED_G_SetLow();
        }
        
        // If the switch was held down for 3 sec we go into infinite while loop doing nothing.
        // This enables the user to safely remove the sd card without corrupting it.
        if (switch_counter >= 3) {
            IO_LED_G_SetHigh();
            while(1);
        }
        
        // If the under voltage is triggered we wait until either the us is shut down or the voltage goes back up.
        // No data is written to the sd card to prevent corruption.
        if (!IO_UVP_GetValue()) {
            IO_LED_R_SetHigh();
            debugprint_string("UVP ERROR! Going to sleep.\r\n");
            while (!IO_UVP_GetValue()) {
                WATCHDOG_TimerClear();
            }
            IO_LED_R_SetLow();
        }
        
    }
    return 1; 
}
