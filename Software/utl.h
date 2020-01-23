#ifndef UTL_H
#define UTL_H

#include <stdint.h>

/**
 *     <b>Function prototype:</b><br>   char *utl_uint32_to_string(UINT32 value, char *str, UINT8 radix)
 * <br>
 * <br><b>Description:</b><br>          Converts an unsigned integer to a null terminated string
 * <br>
 * <br><b>Precondition:</b><br>         None
 * <br>
 * <br><b>Inputs:</b><br>               UINT32 value:   The value to convert
 * <br>                                 char *str:      Pointer to a string buffer
 * <br>                                 UINT8 radix:    The radix to use for the conversion (10->dec, 16->hex)
 * <br>
 * <br><b>Outputs:</b><br>              Pointer to the string buffer
 * <br>
 * <br><b>Example:</b><br>              utl_uint32_to_string(456, temp_str, 10);    //Convert to string
 */
char *utl_uint32_to_string(uint32_t value, char *str, uint8_t radix);

/**
 *     <b>Function prototype:</b><br>   char *utl_int32_to_string(INT32 value, char *str, UINT8 radix)
 * <br>
 * <br><b>Description:</b><br>          Converts an integer to a null terminated string
 * <br>
 * <br><b>Precondition:</b><br>         None
 * <br>
 * <br><b>Inputs:</b><br>               INT32 value:    The value to convert
 * <br>                                 char *str:      Pointer to a string buffer
 * <br>                                 UINT8 radix:    The radix to use for the conversion (10->dec, 16->hex)
 * <br>
 * <br><b>Outputs:</b><br>              Pointer to the string buffer
 * <br>
 * <br><b>Example:</b><br>              utl_int32_to_string(-456, temp_str, 10);    //Convert to string
 */
char *utl_int32_to_string(int32_t value, char *str, uint8_t radix);

/**
 *     <b>Function prototype:</b><br>   char *utl_uint32_to_string_len(UINT32 value, char *str, UINT8 radix)
 * <br>
 * <br><b>Description:</b><br>          Converts an unsigned integer to a null terminated string
 * <br>                                 Adds 0 padding to get to the given length
 * <br>
 * <br><b>Precondition:</b><br>         None
 * <br>
 * <br><b>Inputs:</b><br>               UINT32 value:   The value to convert
 * <br>                                 char *str:      Pointer to a string buffer
 * <br>                                 UINT8 radix:    The radix to use for the conversion (10->dec, 16->hex)
 * <br>                                 UINT8 len:      The length of the string. 0 will be added to get to the length
 * <br>
 * <br><b>Outputs:</b><br>              Pointer to the string buffer
 * <br>
 * <br><b>Example:</b><br>              utl_uint32_to_string_len(456, temp_str, 16, 4);    //Convert to hex string
 */
char *utl_uint32_to_string_len(uint32_t value, char *str, uint8_t radix, uint8_t len);

/**
 * Function prototype:  
 * Description:         
 */
/**
 *     <b>Function prototype:</b><br>   char *utl_float_to_string(float value, char *str)
 * <br>
 * <br><b>Description:</b><br>          Converts an float to a null terminated string
 * <br>                                 Adds 0 padding to get the fixed length of precision
 * <br>
 * <br><b>Precondition:</b><br>         None
 * <br>
 * <br><b>Inputs:</b><br>               float value:    The value to convert
 * <br>                                 char *str:      Pointer to a string buffer
 * <br>
 * <br><b>Outputs:</b><br>              Pointer to the string buffer
 * <br>
 * <br><b>Example:</b><br>              utl_float_to_string_len(45.056, temp_str);    //Convert to string
 */
char *utl_float_to_string(float value, char *str);

/**
 *     <b>Function prototype:</b><br>   UINT32 utl_string_to_uint32(char *str, UINT8 radix)
 * <br>
 * <br><b>Description:</b><br>          Converts a null terminated string to an unsigned integer
 * <br>
 * <br><b>Precondition:</b><br>         None
 * <br>
 * <br><b>Inputs:</b><br>               char *str:      Pointer to al null terminated string
 * <br>                                 UINT8 radix:    Radix to be used for conversion
 * <br>
 * <br><b>Outputs:</b><br>              The value after conversion
 * <br>
 * <br><b>Example:</b><br>              dummy_value = utl_string_to_uint32("01234", 10);    // Convert dummy string to a dummy value
 */
uint32_t utl_string_to_uint32(char *str, uint8_t radix);

/**
 *     <b>Function prototype:</b><br>   INT32 utl_string_to_int32(char *str, UINT8 radix)
 * <br>
 * <br><b>Description:</b><br>          Converts a null terminated string to an integer
 * <br>
 * <br><b>Precondition:</b><br>         None
 * <br>
 * <br><b>Inputs:</b><br>               char *str:      Pointer to al null terminated string
 * <br>                                 UINT8 radix:    Radix to be used for conversion
 * <br>
 * <br><b>Outputs:</b><br>              The value after conversion
 * <br>
 * <br><b>Example:</b><br>              dummy_value = utl_string_to_int32("-01234", 10);    // Convert dummy string to a dummy value
 */
int32_t utl_string_to_int32(char *str, uint8_t radix);

/**
 *     <b>Function prototype:</b><br>   UINT16 utl_calc_crc(UINT8 *pdata, UINT32 ui_size)
 * <br>
 * <br><b>Description:</b><br>          Calculates the CRC 16 CCITT of a byte array buffer.
 * <br>
 * <br><b>Precondition:</b><br>         None
 * <br>
 * <br><b>Inputs:</b><br>               UINT8 *pdata:   Pointer to a byte buffer.
 * <br>                                 UINT32 ui_size: Size of the array
 * <br>
 * <br><b>Outputs:</b><br>              The crc
 * <br>
 * <br><b>Example:</b><br>              crc = utl_calc_crc(eeprom_data.raw, EEPROM_DATA_SIZE-2);    //Get crc
 */
uint16_t utl_calc_crc(uint8_t *pdata, uint32_t ui_size);


#endif
