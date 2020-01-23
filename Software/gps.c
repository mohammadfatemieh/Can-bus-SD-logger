/*
 * File:   gps.c
 * Author: Hylke
 *
 * Created on March 7, 2019, 5:39 PM
 */

#include <xc.h>
#include <stdint.h>
#include "gps.h"
#include "mcc_generated_files/uart2.h"
#include "softwaretimer.h"
#include "debugprint.h"

static gps_time_t gps_time = {};
static gps_coordinates_t gps_coordinates = {};
static gps_speed_t gps_speed = {};
static uint8_t gps_satellites = 0;
static uint8_t gps_tick = 0;


static char gps_extract_char(char* s, uint8_t index) {
    uint8_t i = 0;
    
    //find the nth ,
    for (i = 0; i < index; i++) {
        while (*s != ',' && *s != '\0') {
            s++;
        }     
        s++;
    }
    
    if (*s == ',') {
        return '?';
    } else {
        return *s;
    }
}

static int32_t gps_extract_value(char* s, uint8_t index, uint8_t res) {
    uint8_t i = 0;
    int32_t value = 0;
    uint8_t neg = 0;
    
    //find the nth ,
    for (i = 0; i < index; i++) {
        while (*s != ',' && *s != '\0') {
            s++;
        }     
        s++;
    }
    
    //Get value before dot
    while (*s != '.' && *s != ',' && *s != '\0') {
        if (*s == '-') {
            neg = 1;
        }
        if (*s >= '0' && *s <= '9') {
            value *= 10;
            value += (uint32_t)*s - '0';
        }
        s++;
    }
    
    // Get value after dot if present
    if (*s == '.' && res != 0) {
        s++;
        while (*s != ',' && res != 0 && *s != '\0') {
            if (*s >= '0' && *s <= '9') {
                value *= 10;
                value += (uint32_t)*s - '0';
            }
            s++;
            res--;
        }
    }
    
    // Finish value by resolution
    while (res != 0) {
        value *= 10;
        res--;
    }
    if (neg) {
        value *= -1;
    }
    
    return value;
}

void gps_handler(void) {
    static char gps_line_buffer[128] = "";
    static uint8_t index = 0;
    static int8_t timeout_timer = SOFTWARETIMER_NONE;
    uint32_t value;
    char ch;
    
    // Init timeout timer if not yet done
    if (timeout_timer == SOFTWARETIMER_NONE) {
        timeout_timer = softwaretimer_create(SOFTWARETIMER_SINGLE_MODE);
    }
    
    // Check if new data is available and read until a $ or newline is found
    // $ character indicates the beginning of a new line.
    // new line indicates the end of a line.
    if (UART2_ReceiveBufferIsEmpty()) {
        // No new data. nothing to check
        return;
    }
    
    // Read until a newline is found
    softwaretimer_start(timeout_timer, 100);
    while (!UART2_ReceiveBufferIsEmpty() && !softwaretimer_get_expired(timeout_timer)) {
        gps_line_buffer[index] = (char)UART2_Read();
        
        // new line was read
        if (gps_line_buffer[index] == '\r') {
            break;
        }
        // Clear the buffer when$ character was read or buffer is full
        if (gps_line_buffer[index] == '$' || index == 127) {
            for (index = 0; index < 128; index++) {
                gps_line_buffer[index] = 0;
            }
            index = 0;
        } else {
            index++;
        }
    }
    softwaretimer_stop(timeout_timer);
    
    // Return when break by empty uart or timeout
    if (gps_line_buffer[index] != '\r') {
        return;
    }
    
    index++;
    gps_line_buffer[index] = '\0';
    
    
    // Data consists of string indicating the type of data and then the data separated by commas
    /*
    $GPRMC,095241.00,A,5306.68774,N,00604.15290,E,0.006,,050617,,,A*7F
    $GPVTG,,T,,M,0.006,N,0.012,K,A*26
    $GPGGA,095241.00,5306.68774,N,00604.15290,E,1,08,0.93,-2.7,M,45.7,M,,*7C
    $GPGSA,A,3,14,19,32,12,15,25,24,17,,,,,1.59,0.93,1.29*03
    $GPGSV,4,1,13,02,09,124,07,06,18,083,29,10,03,262,25,12,89,302,46*78
    $GPGSV,4,2,13,14,23,316,42,15,09,180,39,17,15,039,34,19,32,050,39*7C
    $GPGSV,4,3,13,22,03,352,31,24,62,134,41,25,43,252,43,29,02,197,30*7F
    $GPGSV,4,4,13,32,37,298,39*47
    $GPGLL,5306.68774,N,00604.15290,E,095241.00,A,A*65
     */
    
    // GGA
    if (    gps_line_buffer[0] == 'G' &&
            gps_line_buffer[1] == 'P' &&
            gps_line_buffer[2] == 'G' &&
            gps_line_buffer[3] == 'G' &&
            gps_line_buffer[4] == 'A') {
        
        // Number of satellites
        value = gps_extract_value(gps_line_buffer, 7, 0);
        gps_satellites = value;
        
        //Height
        value = gps_extract_value(gps_line_buffer, 9, 1);
        gps_coordinates.height_m = value;
    }
    
    // RMC
    if (    gps_line_buffer[0] == 'G' &&
            gps_line_buffer[1] == 'P' &&
            gps_line_buffer[2] == 'R' &&
            gps_line_buffer[3] == 'M' &&
            gps_line_buffer[4] == 'C') {
        
        
        
        // Time stamp
        value = gps_extract_value(gps_line_buffer, 1, 0);
        gps_time.hour = value / 10000;
        gps_time.min = (value / 100) % 100;
        gps_time.sec = value % 100;
        
        // Latitude
        value = gps_extract_value(gps_line_buffer, 3, 5);
        gps_coordinates.latitude_degrees = value / 10000000;
        gps_coordinates.latitude_minutes = value % 10000000;
        ch = gps_extract_char(gps_line_buffer, 4);
        if (ch == 'W') {
            gps_coordinates.latitude_degrees *= -1;
        }
        //debugprint_uint(value);
        //debugprint_string("\r\n");
        
        // Longitude
        value = gps_extract_value(gps_line_buffer, 5, 5);
        gps_coordinates.longitude_degrees = value / 10000000;
        gps_coordinates.longitude_minutes = value % 10000000;
        ch = gps_extract_char(gps_line_buffer, 6);
        if (ch == 'S') {
            gps_coordinates.longitude_degrees *= -1;
        }
        //debugprint_uint(value);
        //debugprint_string("\r\n");
        
        // Date
        value = gps_extract_value(gps_line_buffer, 9, 0);
        gps_time.day = value / 10000;
        gps_time.month = (value/ 100) % 100;
        gps_time.year = value % 100;
        
        // Set gps tick if we received a valid timestamp
        if (gps_time.day != 0) {
            gps_tick = 1;
        }
    }
    
    // VTG
    if (    gps_line_buffer[0] == 'G' &&
            gps_line_buffer[1] == 'P' &&
            gps_line_buffer[2] == 'V' &&
            gps_line_buffer[3] == 'T' &&
            gps_line_buffer[4] == 'G') {
        
        // Track in degrees
        value = gps_extract_value(gps_line_buffer, 1, 1);
        gps_speed.direction_degrees = value;
        
        // Speed km
        value = gps_extract_value(gps_line_buffer, 7, 2);
        gps_speed.speed_kmh = value;
    }
    
    // Done reading the buffer.
    // Clear the buffer
    for (index = 0; index < 128; index++) {
        gps_line_buffer[index] = 0;
    }
    index = 0;
}

// Get functions
gps_time_t get_gps_time(void) {
    return gps_time;
}

gps_coordinates_t get_gps_coordinates(void) {
    return gps_coordinates;
}

gps_speed_t get_gps_speed(void) {
    return gps_speed;
}

uint8_t get_gps_satellites(void) {
    return gps_satellites;
}

uint8_t get_gps_tick(void) {
    if (gps_tick) {
        gps_tick = 0;
        return 1;
    } else {
        return 0;
    }
}
