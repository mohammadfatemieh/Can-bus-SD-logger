// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef CANBUS_H
#define	CANBUS_H

#include <stdint.h>

#define CAN_BUS_SEND_PERIOD_MS  1000

// Initializes the can bus.
void can_bus_init(void);

// Sends and receives messages.
// Should be called once per main loop.
void can_bus_process(void);

#define NODE_ID_MG_BATTERY      0x02
#define NODE_ID_MG_MPPT         0x04
#define NODE_ID_MG_MPPT_TOTAL   10
#define NODE_ID_SLS             0x10
#define NODE_ID_FOIL_CONTROL    0x11


typedef struct {
    uint16_t voltage_mv;
    int16_t current_10ma;
    int16_t discharge_current_10ma;
    int16_t charge_current_10ma;
    uint8_t soc;
    uint16_t time_to_go_min;
    uint32_t bms_state;
    uint8_t temp[4];
    uint16_t cell_voltage_mv[12];
    uint8_t power_level;
}mg_battery_t;

typedef struct {
    int16_t current_in_ma;
    uint16_t voltage_in_mv;
    uint16_t voltage_out_mv;
    int16_t power_in_100mw;
}mg_mppt_t;

typedef struct {
    uint32_t status;
    uint32_t limiting;
    int16_t temp_power_100mdeg;
    int16_t temp_electronics_100mdeg;
    int16_t temp_motor_1_100mdeg;
    int16_t temp_motor_2_100mdeg;
    uint16_t uzk_10mv;
    int16_t motor_current_100ma;
    int16_t input_currect_100ma;
    int16_t rpm;
}sls_t;

typedef struct {
    uint16_t primary_input_position;
    uint16_t primary_output_position;
}foil_control_t;

mg_battery_t get_can_data_mg_battery(void);

mg_mppt_t get_can_data_mg_mppt(uint8_t nr);

sls_t get_can_data_sls(void);

foil_control_t get_can_data_foil_control(void);

#endif	
