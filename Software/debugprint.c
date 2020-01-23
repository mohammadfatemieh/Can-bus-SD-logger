/*
 * File:   debugprint.c
 * Author: Hylke
 *
 * Created on February 17, 2019, 4:48 PM
 */

#include "debugprint.h"
#include <stdint.h>
#include "mcc_generated_files/uart1.h"


void debugprint_char(char c) {
    if (!(UART1_TRANSFER_STATUS_TX_FULL & UART1_TransferStatusGet())) {
        UART1_Write(c);
    }
}

void debugprint_string(char *str) {
    // Fill the buffer until \0 char is found
    while (*str != '\0') {
        // Check if more space is free in the buffer
        if (!(UART1_TRANSFER_STATUS_TX_FULL & UART1_TransferStatusGet())) {
            UART1_Write(*str++);
        }
    }
}

void debugprint_string_len(char *str, uint32_t len) {
    // Fill the buffer until len
    while (len != 0) {
        // Check if more space is free in the buffer
        if (!(UART1_TRANSFER_STATUS_TX_FULL & UART1_TransferStatusGet())) {
            UART1_Write(*str++);
            len--;
        }
    }
}

void debugprint_int(int32_t value) {
    char result_str[12];    // minus sign, 10 char value, zero termination
    char *ptr = result_str, *ptr1 = result_str;
    char tmp_char;
    int32_t tmp_value;
    
    do {
        tmp_value = value;
        value /= 10;
        *ptr++ = "9876543210123456789" [9 + (tmp_value - value * 10)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    
    debugprint_string(result_str);
}

void debugprint_uint(uint32_t value) {
    char result_str[11];     // 10 char value, zero termination
    char *ptr = result_str, *ptr1 = result_str;
    char tmp_char;
    uint32_t tmp_value;
    
    do {
        tmp_value = value;
        value /= 10;
        *ptr++ = "9876543210123456789" [9 + (tmp_value - value * 10)];
    } while ( value );

    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    
    debugprint_string(result_str);
}

void debugprint_hex(uint32_t value) {
    char result_str[9];     // 8 char value, zero termination
    char *ptr = result_str, *ptr1 = result_str;
    char tmp_char;
    uint32_t tmp_value;
    uint8_t i = 0;
    
    do {
        tmp_value = value;
        value /= 16;
        *ptr++ = "FEDCBA9876543210123456789ABCDEF" [15 + (tmp_value - value * 16)];
        i++;
    } while ( value );

    if ((i & 0b1) != 0) {
        *ptr++ = '0';
    }
    
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    
    debugprint_string(result_str);
}

