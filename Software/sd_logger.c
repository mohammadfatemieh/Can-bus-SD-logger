/*
 * File:   sd_logger.c
 * Author: Hylke
 *
 * Created on March 29, 2019, 7:33 PM
 */

#include <xc.h>
#include <stdint.h>
#include "sd_logger.h"
#include "mla_fileio/fileio.h"
#include "mla_fileio/sd_spi.h"
#include "debugprint.h"
#include "softwaretimer.h"
#include <string.h>
#include "utl.h"
#include "gps.h"
#include "mcc_generated_files/pin_manager.h"
#include "canbus.h"

// ********************************************************
// * FILE IO AND SD CARD
// ********************************************************

void sd_logger_SdSpiConfigurePins (void);
inline void sd_logger_SdSpiSetCs(uint8_t a);
inline bool sd_logger_SdSpiGetCd(void);
inline bool sd_logger_SdSpiGetWp(void);

// The sdCardMediaParameters structure defines user-implemented functions needed by the SD-SPI fileio driver.
// The driver will call these when necessary.  For the SD-SPI driver, the user must provide
// parameters/functions to define which SPI module to use, Set/Clear the chip select pin,
// get the status of the card detect and write protect pins, and configure the CS, CD, and WP
// pins as inputs/outputs (as appropriate).
// For this demo, these functions are implemented in system.c, since the functionality will change
// depending on which demo board/microcontroller you're using.
// This structure must be maintained as long as the user wishes to access the specified drive.
FILEIO_SD_DRIVE_CONFIG sdCardMediaParameters =
{
    1,                                  // Use SPI module 2
    sd_logger_SdSpiSetCs,                    // User-specified function to set/clear the Chip Select pin.
    sd_logger_SdSpiGetCd,                    // User-specified function to get the status of the Card Detect pin.
    sd_logger_SdSpiGetWp,                    // User-specified function to get the status of the Write Protect pin.
    sd_logger_SdSpiConfigurePins             // User-specified function to configure the pins' TRIS bits.
};

// The gSDDrive structure allows the user to specify which set of driver functions should be used by the
// FILEIO library to interface to the drive.
// This structure must be maintained as long as the user wishes to access the specified drive.
const FILEIO_DRIVE_CONFIG gSdDrive =
{
    (FILEIO_DRIVER_IOInitialize)FILEIO_SD_IOInitialize,                      // Function to initialize the I/O pins used by the driver.
    (FILEIO_DRIVER_MediaDetect)FILEIO_SD_MediaDetect,                       // Function to detect that the media is inserted.
    (FILEIO_DRIVER_MediaInitialize)FILEIO_SD_MediaInitialize,               // Function to initialize the media.
    (FILEIO_DRIVER_MediaDeinitialize)FILEIO_SD_MediaDeinitialize,           // Function to de-initialize the media.
    (FILEIO_DRIVER_SectorRead)FILEIO_SD_SectorRead,                         // Function to read a sector from the media.
    (FILEIO_DRIVER_SectorWrite)FILEIO_SD_SectorWrite,                       // Function to write a sector to the media.
    (FILEIO_DRIVER_WriteProtectStateGet)FILEIO_SD_WriteProtectStateGet,     // Function to determine if the media is write-protected.
};


void sd_logger_SdSpiConfigurePins (void)
{
    //pin init done in mcc
}

inline void sd_logger_SdSpiSetCs(uint8_t a)
{
    if (a) {
        IO_SD_CS_SetHigh();
    } else {
        IO_SD_CS_SetLow();
    }
}

inline bool sd_logger_SdSpiGetCd(void)
{
    //return (!PORTCbits.RC2) ? true : false;
    if (!IO_SD_DETECT_GetValue()) {
        return true;
    } else {
        return false;
    }
    //return true;
}

inline bool sd_logger_SdSpiGetWp(void)
{
    //return (PORTGbits.RG1) ? true : false;
    if (IO_SD_PROTECT_GetValue()) {
        return true;
    } else {
        return false;
    }
    //return false;
}

void GetTimestamp (FILEIO_TIMESTAMP * timeStamp)
{
    static uint16_t counter = 0;
    
    counter++;
    
    timeStamp->timeMs = 0;
    timeStamp->time.bitfield.hours = (counter / 1800) % 24;
    timeStamp->time.bitfield.minutes = (counter / 30) % 60;
    timeStamp->time.bitfield.secondsDiv2 = counter % 30;

    timeStamp->date.bitfield.day = 1;
    timeStamp->date.bitfield.month = 1;
    // Years in the FAT file system go from 1980-2108.
    timeStamp->date.bitfield.year = 20;
}

static int8_t sd_logger_fileio_init(void) {
    FILEIO_ERROR_TYPE error;
    // Initialize the library
    if (!FILEIO_Initialize()) {
        debugprint_string("Failed to init FILEIO\r\n");
        return -1;
    }
    
    FILEIO_RegisterTimestampGet (GetTimestamp);
    
    if (FILEIO_MediaDetect(&gSdDrive, &sdCardMediaParameters) != true) {
        debugprint_string("No media detected\r\n");
        return -1;
    } else {
        debugprint_string("Media detected\r\n");
    }
    if (FILEIO_SD_WriteProtectStateGet(&sdCardMediaParameters) == true) {
        debugprint_string("Media write protected\r\n");
        return -1;
    }
    error = FILEIO_DriveMount('A', &gSdDrive, &sdCardMediaParameters);
    if (error == FILEIO_ERROR_NONE) {
        debugprint_string("Successfully mounted the drive\r\n");
        return 0;
    } else {
        debugprint_string("Error mounting drive\r\n");
        debugprint_uint(error);
        return -1;
    }
    return -1;
}


// ********************************************************
// * LOGGING
// ********************************************************

static uint8_t timer_sd_logger = SOFTWARETIMER_NONE;
static uint8_t sd_logger_file_number = 0;
static uint8_t sd_logger_file_new = 0;


static void sd_logger_find_free_file_number(void) {
    uint8_t i;
    char file_name[13];
    char temp[8];
    FILEIO_OBJECT file;
    
    // find next free number in filename
    for (i=0; i<254; i++) {
        strcpy(file_name, "LOG");
        utl_uint32_to_string(i, temp, 10);
        strcat(file_name, temp);
        strcat(file_name, ".CSV");
        // Try to open file
        if (FILEIO_Open(&file, file_name, FILEIO_OPEN_READ) != FILEIO_RESULT_SUCCESS) {
            // Could not open file. Means the file is not yet there and we can use this number.
            break;
        }
        FILEIO_Close (&file);
    }
    
    sd_logger_file_number = i;
}

static void sd_logger_write_to_file(char *buffer, uint16_t buffer_length) {
    FILEIO_OBJECT file;
    char file_name[13];
    static uint8_t write_errors = 0;
    char temp_string[16] = "";
    
    // Write to file
    strcpy(file_name, "LOG");
    utl_uint32_to_string(sd_logger_file_number, temp_string, 10);
    strcat(file_name, temp_string);
    strcat(file_name, ".CSV");
    
    if (FILEIO_Open(&file, file_name, FILEIO_OPEN_WRITE | FILEIO_OPEN_APPEND | FILEIO_OPEN_CREATE) != FILEIO_RESULT_SUCCESS) {
        write_errors++;
        IO_LED_R_SetLow();
        if (write_errors > 16) {
            asm("reset");
        }
        return;
    }else {
        write_errors = 0;
    }
    FILEIO_Write (buffer, 1, buffer_length, &file);
    FILEIO_Close (&file);
}

int8_t sd_logger_init(void) {
    // Init sd card until success
    int8_t res = sd_logger_fileio_init();
    if (res == -1) {
        return -1;
    } 
    // Successfully init filesystem
    else {
        sd_logger_find_free_file_number();
        timer_sd_logger = softwaretimer_create(SOFTWARETIMER_CONTINUOUS_MODE);
        sd_logger_file_new = 1;
        softwaretimer_start(timer_sd_logger, 1000);
        
        debugprint_string("Using logfile ");
        debugprint_uint(sd_logger_file_number);
        debugprint_string("\r\n");
        
        return 0;
    }
}

void sd_logger_process(void) {
    //static uint8_t counter;
    char log_string[512] = "";
    char temp_string[16] = "";
    static uint16_t times_written_counter = 0;
    uint8_t i;
    
    gps_time_t gps_time;
    gps_coordinates_t gps_coordinates;
    gps_speed_t gps_speed;
    mg_battery_t mg_battery;
    mg_mppt_t mg_mppt;
    sls_t sls;
    foil_control_t foil_control;
    
    if (softwaretimer_get_expired(timer_sd_logger) == 1) {
        // Create new file when written for one hour
        times_written_counter++;
        if (times_written_counter == 3601) {
            sd_logger_find_free_file_number();
            sd_logger_file_new = 1;
            
            debugprint_string("Using logfile ");
            debugprint_uint(sd_logger_file_number);
            debugprint_string("\r\n");
            
            times_written_counter = 1;
        }
        
        if (sd_logger_file_new == 1) {
            // Logger
            strcat(log_string, "Log counter;");
            sd_logger_write_to_file(log_string, strlen(log_string));
            strcpy(log_string, "");
            // GPS
            strcat(log_string, "Day;Month;Year;Hour;Min;Sec;Latitude deg;Latitude min;Longitude deg;Longitude min;Direction;Speed;");
            sd_logger_write_to_file(log_string, strlen(log_string));
            strcpy(log_string, "");
            // Batt
            strcat(log_string, "Batt voltage;Batt current; Batt discharge current;Batt charge current;Batt soc;Batt time to go;Batt bms state;");
            for (i = 0; i < 4; i++) {
                strcat(log_string, "Batt temp ");
                utl_uint32_to_string(i, temp_string, 10);
                strcat(log_string, temp_string);
                strcat(log_string, ";");
            }
            for (i = 0; i < 12; i++) {
                strcat(log_string, "Batt cell ");
                utl_uint32_to_string(i+1, temp_string, 10);
                strcat(log_string, temp_string);
                strcat(log_string, " voltage;");
            }
            strcat(log_string, "Power level;");
            sd_logger_write_to_file(log_string, strlen(log_string));
            strcpy(log_string, "");
            // MPPT
            for (i = 0; i < NODE_ID_MG_MPPT_TOTAL; i++) {
                strcat(log_string, "MPPT ");
                utl_uint32_to_string(i + 1, temp_string, 10);
                strcat(log_string, temp_string);
                strcat(log_string, " A in;");
                strcat(log_string, "MPPT ");
                utl_uint32_to_string(i + 1, temp_string, 10);
                strcat(log_string, temp_string);
                strcat(log_string, " V in;");
                strcat(log_string, "MPPT ");
                utl_uint32_to_string(i + 1, temp_string, 10);
                strcat(log_string, temp_string);
                strcat(log_string, " V out;");
                strcat(log_string, "MPPT ");
                utl_uint32_to_string(i + 1, temp_string, 10);
                strcat(log_string, temp_string);
                strcat(log_string, " P in;");
            }
            sd_logger_write_to_file(log_string, strlen(log_string));
            strcpy(log_string, "");
            // sls
            strcat(log_string, "SLS status;SLS limiting;SLS temp power;SLS temp elec;SLS temp motor 1;SLS temp motor 2;SLS UZK;SLS motor current;SLS input current;RPM;");
            sd_logger_write_to_file(log_string, strlen(log_string));
            strcpy(log_string, "");
            
            // Foil control
            strcat(log_string, "Foil input 1 pos;Foil output 1 pos;");
            sd_logger_write_to_file(log_string, strlen(log_string));
            strcpy(log_string, "");
            
            // End of line
            strcat(log_string, "\r\n");
            sd_logger_write_to_file(log_string, strlen(log_string));
            strcpy(log_string, "");
            
            sd_logger_file_new = 0;
        }

        // Create log data
        // Logger info
        utl_uint32_to_string(times_written_counter, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        sd_logger_write_to_file(log_string, strlen(log_string));
        strcpy(log_string, "");
        
        // GPS
        gps_time = get_gps_time();
        gps_coordinates = get_gps_coordinates();
        gps_speed = get_gps_speed();
        // Time and date
        utl_uint32_to_string(gps_time.day, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(gps_time.month, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(gps_time.year, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(gps_time.hour, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(gps_time.min, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(gps_time.sec, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        // GPS location
        utl_int32_to_string(gps_coordinates.latitude_degrees, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(gps_coordinates.latitude_minutes, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_int32_to_string(gps_coordinates.longitude_degrees, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(gps_coordinates.longitude_minutes, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        // GPS additional data: direction, speed, height, satellites
        utl_uint32_to_string(gps_speed.direction_degrees, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(gps_speed.speed_kmh, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        sd_logger_write_to_file(log_string, strlen(log_string));
        strcpy(log_string, "");
        
        // MG battery
        mg_battery = get_can_data_mg_battery();
        utl_uint32_to_string(mg_battery.voltage_mv, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_int32_to_string(mg_battery.current_10ma, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_int32_to_string(mg_battery.discharge_current_10ma, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_int32_to_string(mg_battery.charge_current_10ma, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(mg_battery.soc, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(mg_battery.time_to_go_min, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(mg_battery.bms_state, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        for (i = 0; i < 4; i++) {
            utl_uint32_to_string(mg_battery.temp[i], temp_string, 10);
            strcat(log_string, temp_string);
            strcat(log_string, ";");
        }
        for (i = 0; i < 12; i++) {
            utl_uint32_to_string(mg_battery.cell_voltage_mv[i], temp_string, 10);
            strcat(log_string, temp_string);
            strcat(log_string, ";");
        }
        utl_uint32_to_string(mg_battery.power_level, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        sd_logger_write_to_file(log_string, strlen(log_string));
        strcpy(log_string, "");
        
        // MG mppt
        for (i = 0; i < NODE_ID_MG_MPPT_TOTAL; i++) {
            mg_mppt = get_can_data_mg_mppt(i);
            utl_int32_to_string(mg_mppt.current_in_ma, temp_string, 10);
            strcat(log_string, temp_string);
            strcat(log_string, ";");
            utl_uint32_to_string(mg_mppt.voltage_in_mv, temp_string, 10);
            strcat(log_string, temp_string);
            strcat(log_string, ";");
            utl_uint32_to_string(mg_mppt.voltage_out_mv, temp_string, 10);
            strcat(log_string, temp_string);
            strcat(log_string, ";");
            utl_int32_to_string(mg_mppt.power_in_100mw, temp_string, 10);
            strcat(log_string, temp_string);
            strcat(log_string, ";");
        }
        sd_logger_write_to_file(log_string, strlen(log_string));
        strcpy(log_string, "");
        
        // SLS
        sls = get_can_data_sls();
        utl_uint32_to_string(sls.status, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(sls.limiting, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_int32_to_string(sls.temp_power_100mdeg, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_int32_to_string(sls.temp_electronics_100mdeg, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_int32_to_string(sls.temp_motor_1_100mdeg, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_int32_to_string(sls.temp_motor_2_100mdeg, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(sls.uzk_10mv, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_int32_to_string(sls.motor_current_100ma, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_int32_to_string(sls.input_currect_100ma, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_int32_to_string(sls.rpm, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        sd_logger_write_to_file(log_string, strlen(log_string));
        strcpy(log_string, "");
        
        // Foil control
        foil_control = get_can_data_foil_control();
        utl_uint32_to_string(foil_control.primary_input_position, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        utl_uint32_to_string(foil_control.primary_output_position, temp_string, 10);
        strcat(log_string, temp_string);
        strcat(log_string, ";");
        sd_logger_write_to_file(log_string, strlen(log_string));
        strcpy(log_string, "");
        
        // New line
        strcat(log_string, "\r\n");
        sd_logger_write_to_file(log_string, strlen(log_string));
        strcpy(log_string, "");
        
        debugprint_string("Written to SD card\r\n");
    }
}
