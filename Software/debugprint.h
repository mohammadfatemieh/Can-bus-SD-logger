/* 
 * File:                debugprint.h
 * Author:              Hylke
 * Comments:            Simple uart debugging output
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef DEBUGPRINT_H
#define	DEBUGPRINT_H

#include <stdint.h>

// Prints a character onto the debug uart
// Parameters:
//  c            Character to be printed
void debugprint_char(char c);

// Prints a string onto the debug uart
// Parameters:
//  *str            Pointer to the string to send. The string needs to be \0 terminated
void debugprint_string(char *str);

// Prints a string with a specific length onto the debug uart
// Parameters:
//  *str            Pointer to the string to send.
//  len             The length of the string to be send in characters.
void debugprint_string_len(char *str, uint32_t len);

// Prints a signed integer onto the debug uart
// Parameters:
//  value           The integer to be printed
void debugprint_int(int32_t value);

// Prints an unsigned integer onto the debug uart
// Parameters:
//  value           The integer to be printed
void debugprint_uint(uint32_t value);

// Prints a hex integer onto the debug uart
// Parameters:
//  value           The integer to be printed
void debugprint_hex(uint32_t value);

#endif	/* DEBUGPRINT_H */

