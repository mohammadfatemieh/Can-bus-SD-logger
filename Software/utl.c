#include "utl.h"
#include <stdint.h>

static const char hex_chars[] = "0123456789ABCDEF";

/**
 * Function prototype:  char *utl_uint32_to_string(UINT32 value, char *str, UINT8 radix)
 * Description:         Converts an unsigned integer to a null terminated string
 */
char *utl_uint32_to_string(uint32_t value, char *str, uint8_t radix) {
    char temp[16];
    uint32_t rest, index;
    char *ptr;
    int8_t i;

    ptr=str;                                // Save string ptr
    if (radix < 2 || radix > 16) {          // Wrong radix
        return ptr;
    }
    index = 0;                              // Do conversion
    do {
        rest = value % radix;               // Rest is char LSB first
        temp[index++] = hex_chars[rest];
        value = value / radix;
    } while (value != 0);

    index--;
    for (i = index ; i >= 0 ; i--) {        // Swap LSB MSB
        *str++ = temp[i];
    }
    *str = '\0';
    return ptr;
}

/**
 * Function prototype:  char *utl_int32_to_string(INT32 value, char *str, UINT8 radix)
 * Description:         Converts an integer to a null terminated string
 */
char *utl_int32_to_string(int32_t value, char *str, uint8_t radix) {
    char temp[16];
    uint32_t rest, index;
    char *ptr;
    int8_t i;

    ptr=str;                                // Save string ptr
    if (radix < 2 || radix > 16) {          // Wrong radix
        return ptr;
    }
    if (value < 0) {                        // Negative number 
        value = -value;
        *str++ = '-';
    }

    index = 0;                              // Do conversion
    do {
        rest = value % radix;               // Rest is char LSB first
        temp[index++] = hex_chars[rest];
        value = value / radix;
    } while (value != 0);

    index--;
    for (i = index ; i >= 0 ; i--) {        // Swap LSB MSB
        *str++ = temp[i];
    }
    *str = '\0';
    return ptr;
}

/**
 * Function prototype:  char *utl_uint32_to_string_len(UINT32 value, char *str, UINT8 radix, UINT8 len)
 * Description:         Converts an unsigned integer to a null terminated string
 *                      Adds 0 padding to get to the given length
 */
char *utl_uint32_to_string_len(uint32_t value, char *str, uint8_t radix, uint8_t len) {
    char temp[16];
    uint32_t rest, index;
    char *ptr;
    int8_t i;

    ptr=str;                                // Save string ptr
    if (radix < 2 || radix > 16) {          // Wrong radix
        return ptr;
    }
    index = 0;                              // Do conversion
    do {
        rest = value % radix;               // Rest is char LSB first
        temp[index++] = hex_chars[rest];
        value = value / radix;
    } while (value != 0);
    
    while (index<len) {
        temp[index++] = '0';
    }
    index--;
    for (i = index ; i >= 0 ; i--) {        // Swap LSB MSB
        *str++ = temp[i];
    }
    *str = '\0';
    return ptr;
}

/**
 * Function prototype:  char *utl_float_to_string(float value, char *str, UINT8 radix, UINT8 precision)
 * Description:         Converts an float to a null terminated string
 */
char *utl_float_to_string(float value, char *str) {
    int32_t value_significant = value;
    int32_t value_precision = (int32_t)(value * 1000) % 1000;
    char temp[16];
    uint32_t rest, index;
    char *ptr;
    int8_t i;
    

    ptr=str;                                // Save string ptr
    if (value < 0) {                        // Negative number 
        value_significant = -value_significant;
        value_precision = -value_precision;
        *str++ = '-';
    }

    index = 0;                              // Do conversion
    do {
        rest = value_precision % 10;               // Rest is char LSB first
        temp[index++] = hex_chars[rest];
        value_precision = value_precision / 10;
    } while (value_precision != 0);
    while (index < 3) {
        temp[index++] = '0';
    }
    temp[index++] = '.';
    do {
        rest = value_significant % 10;               // Rest is char LSB first
        temp[index++] = hex_chars[rest];
        value_significant = value_significant / 10;
    } while (value_significant != 0);
    
    index--;
    for (i = index ; i >= 0 ; i--) {        // Swap LSB MSB
        *str++ = temp[i];
    }
    *str = '\0';
    return ptr;
}

/**
 * Function prototype:  UINT32 utl_string_to_uint32(char *str, UINT8 radix)
 * Description:         Converts a null terminated string to an unsigned integer
 */
uint32_t utl_string_to_uint32(char *str, uint8_t radix) {
    char temp;
    uint32_t value = 0;
    
    if (radix < 2 || radix > 16) {          // Wrong radix
        return 0;
    }
    while (*str != '\0') {
        temp = *str;
        // Convert char to int
        if      ('0'<=temp && temp<='9')    temp -= '0';
        else if ('a'<=temp && temp<='f')    temp = temp - 'a' + 10;
        else if ('A'<=temp && temp<='F')    temp = temp - 'A' + 10;
        else                                temp = 0;
        
        if (temp > radix) temp = 0;
        // Add to value
        value *= radix;
        value += temp;
        str++;
    }
    return value;
}

/**
 * Function prototype:  INT32 utl_string_to_int32(char *str, UINT8 radix)
 * Description:         Converts a null terminated string to an integer
 */
int32_t utl_string_to_int32(char *str, uint8_t radix) {
    char temp;
    int32_t value = 0;
    uint8_t negative = 0;
    
    if (radix < 2 || radix > 10) {          // Wrong radix
        return 0;
    }
    if (*str == '-') {
        negative = 1;
        str++;
    }
    
    while (*str != '\0') {
        temp = *str;
        // Convert char to int
        if ('0'<=temp && temp<='9') temp -= '0';
        else temp = 0;
        if (temp > radix) temp = 0;
        // Add to value
        value *= radix;
        value += temp;
        str++;
    }
    if (negative) value *= -1;
    return value;
}

#define CRC_POLY_CRC16_CCITT 0x1021 // X^16 + X^12 + X^5 + 1

/**
 *     <b>Function prototype:</b><br>   static void hash_crc_16ccitt(WORD *crc,BYTE data)
 * <br>
 * <br><b>Description:</b><br>          Calculate CRC 16 CCITT
 * <br>
 * <br><b>Precondition:</b><br>         None
 * <br>
 * <br><b>Inputs:</b><br>               WORD *crc:  The crc to add this byte to
 * <br>                                 BYTE data:  byte to add on CRC
 * <br>
 * <br><b>Outputs:</b><br>              None
 * <br>
 * <br><b>Example:</b><br>              hash_crc_16ccitt(&wCrc,*pdata);
 */
static void hash_crc_16ccitt(uint16_t *crc,uint8_t data) {
    uint8_t cnt;

    *crc = ((uint16_t)data << 8) ^ *crc;
    for (cnt = 0; cnt < 8; cnt++) {
        if (*crc & 0x8000) {
            *crc = (*crc << 1) ^ CRC_POLY_CRC16_CCITT;
        } else {
            *crc <<= 1;
        }
    }
}

/**
 * Function prototype:  UINT16 utl_calc_crc(UINT8 *pdata, UINT32 ui_size)
 * Description:         Calculates the CRC 16 CCITT of a byte array buffer.
 */
uint16_t utl_calc_crc(uint8_t *pdata, uint32_t ui_size) {
   int n=0;
   uint16_t wCrc = 0x1D0F;

   for (n=0; n<ui_size ; n++) {
      hash_crc_16ccitt(&wCrc,*pdata);
      pdata++;
   }
   return wCrc;
}
