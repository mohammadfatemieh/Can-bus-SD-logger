/*
 * File:   canbus.c
 * Author: Hylke
 *
 * Created on February 19, 2019, 7:15 PM
 */


#include "canbus.h"
#include "mcc_generated_files/can1.h"
#include "mcc_generated_files/can_types.h"
#include "softwaretimer.h"
#include "debugprint.h"

static mg_battery_t mg_battery = {};
static mg_mppt_t mg_mppt[NODE_ID_MG_MPPT_TOTAL] = {};
static sls_t sls = {};
static foil_control_t foil_control = {};

static void can_bus_receive_messages(void) {
    uCAN_MSG rx_msg;
    uint16_t cob_id, index, function_code;
    uint8_t sub_index, node_id;
    union {
        uint32_t uint32;
        double double32;
    }double_uint32_conversion;
    
    if (CAN1_messagesInBuffer() > 0){
        CAN1_receive(&rx_msg);
                
        // Debug data
        /*
        debugprint_string("R can ");
        debugprint_hex(rx_msg.frame.id);
        debugprint_string(" ");
        debugprint_hex(rx_msg.frame.dlc);
        debugprint_string("  ");
        debugprint_hex(rx_msg.frame.data0);
        debugprint_string(" ");
        debugprint_hex(rx_msg.frame.data1);
        debugprint_string(" ");
        debugprint_hex(rx_msg.frame.data2);
        debugprint_string(" ");
        debugprint_hex(rx_msg.frame.data3);
        debugprint_string("  ");
        debugprint_hex(rx_msg.frame.data4);
        debugprint_string(" ");
        debugprint_hex(rx_msg.frame.data5);
        debugprint_string(" ");
        debugprint_hex(rx_msg.frame.data6);
        debugprint_string(" ");
        debugprint_hex(rx_msg.frame.data7);
        debugprint_string("\r\n");
        */
        
        cob_id = rx_msg.frame.id;
        function_code = cob_id & ~0x007F;
        node_id = cob_id & 0x7F;
        index = rx_msg.frame.data2 << 8 | rx_msg.frame.data1;
        sub_index = rx_msg.frame.data3;
        
        debugprint_string("CAN\r\n");
        
        // MG battery
        if (node_id == NODE_ID_MG_BATTERY && function_code == 0x200) {
            // Power level
            mg_battery.power_level = rx_msg.frame.data0;
        }
        else if (node_id == NODE_ID_MG_BATTERY && function_code == 0x300 && index == 0x2005 && sub_index == 0x01) {
            // Battery voltage
            mg_battery.voltage_mv = (uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4;
        }
        else if (node_id == NODE_ID_MG_BATTERY && function_code == 0x300 && index == 0x2005 && sub_index == 0x02) {
            // Battery current
            mg_battery.current_10ma = (int16_t)((uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4);
        }
        else if (node_id == NODE_ID_MG_BATTERY && function_code == 0x300 && index == 0x2005 && sub_index == 0x03) {
            // discharge current
            mg_battery.discharge_current_10ma = (int16_t)((uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4);
        }
        else if (node_id == NODE_ID_MG_BATTERY && function_code == 0x300 && index == 0x2005 && sub_index == 0x04) {
            // charge current
            mg_battery.charge_current_10ma = (int16_t)((uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4);
        }
        else if (node_id == NODE_ID_MG_BATTERY && function_code == 0x300 && index == 0x2005 && sub_index == 0x05) {
            // soc
            mg_battery.soc = rx_msg.frame.data4;
        }
        else if (node_id == NODE_ID_MG_BATTERY && function_code == 0x300 && index == 0x2005 && sub_index == 0x06) {
            // time to go
            mg_battery.time_to_go_min = (uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4;
        }
        else if (node_id == NODE_ID_MG_BATTERY && function_code == 0x400 && index == 0x2005 && sub_index == 0x0E) {
            // BMS state
            mg_battery.bms_state = (uint32_t)rx_msg.frame.data7 << 24 | (uint32_t)rx_msg.frame.data6 << 16 | (uint32_t)rx_msg.frame.data5 << 8 | (uint32_t)rx_msg.frame.data4;
        }
        else if (node_id == NODE_ID_MG_BATTERY && function_code == 0x400 && index == 0x2005 && sub_index == 0x0F) {
            // Temperature
            mg_battery.temp[0] = rx_msg.frame.data4;
            mg_battery.temp[1] = rx_msg.frame.data5;
            mg_battery.temp[2] = rx_msg.frame.data6;
            mg_battery.temp[3] = rx_msg.frame.data7;
        }
        else if (node_id == NODE_ID_MG_BATTERY && function_code == 0x480 && index == 0x2000) {
            if (1 <= sub_index && sub_index <= 12) {
                mg_battery.cell_voltage_mv[sub_index-1] = (uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4;
            }
        }
        
        // MG MPPT
        else if (NODE_ID_MG_MPPT <= node_id && node_id <= (NODE_ID_MG_MPPT + NODE_ID_MG_MPPT_TOTAL) && function_code == 0x180) {
            // Current in
            double_uint32_conversion.uint32 = (uint32_t)rx_msg.frame.data3 << 24 | (uint32_t)rx_msg.frame.data2 << 16 | (uint32_t)rx_msg.frame.data1 << 8 | (uint16_t)rx_msg.frame.data0;
            mg_mppt[node_id - 0x04].current_in_ma = double_uint32_conversion.double32;
            // voltage in
            double_uint32_conversion.uint32 = (uint32_t)rx_msg.frame.data7 << 24 | (uint32_t)rx_msg.frame.data6 << 16 | (uint32_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4;
            mg_mppt[node_id - 0x04].voltage_in_mv = double_uint32_conversion.double32 * 1000;
        }
        else if (NODE_ID_MG_MPPT <= node_id && node_id <= (NODE_ID_MG_MPPT + NODE_ID_MG_MPPT_TOTAL) && function_code == 0x280) {
            // voltage out
            double_uint32_conversion.uint32 = (uint32_t)rx_msg.frame.data3 << 24 | (uint32_t)rx_msg.frame.data2 << 16 | (uint32_t)rx_msg.frame.data1 << 8 | (uint16_t)rx_msg.frame.data0;
            mg_mppt[node_id - 0x04].voltage_out_mv = double_uint32_conversion.double32 * 1000;
            // power in
            double_uint32_conversion.uint32 = (uint32_t)rx_msg.frame.data7 << 24 | (uint32_t)rx_msg.frame.data6 << 16 | (uint32_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4;
            mg_mppt[node_id - 0x04].power_in_100mw = double_uint32_conversion.double32 / 100;
        }
        
        // SLS motor controller
        else if (node_id == NODE_ID_SLS && function_code == 0x180 && index == 0x2000 && sub_index == 0x01) {
            // status
            sls.status = (uint32_t)rx_msg.frame.data7 << 24 | (uint32_t)rx_msg.frame.data6 << 16 | (uint32_t)rx_msg.frame.data5 << 8 | (uint32_t)rx_msg.frame.data4;
        }
        else if (node_id == NODE_ID_SLS && function_code == 0x180 && index == 0x2001 && sub_index == 0x01) {
            // output limiting
            sls.limiting = (uint32_t)rx_msg.frame.data7 << 24 | (uint32_t)rx_msg.frame.data6 << 16 | (uint32_t)rx_msg.frame.data5 << 8 | (uint32_t)rx_msg.frame.data4;
        }
        else if (node_id == NODE_ID_SLS && function_code == 0x280 && index == 0x2000 && sub_index == 0x01) {
            // power temperature
            sls.temp_power_100mdeg = (int16_t)((uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4);
        }
        else if (node_id == NODE_ID_SLS && function_code == 0x280 && index == 0x2000 && sub_index == 0x02) {
            // electronic temperature
            sls.temp_electronics_100mdeg = (int16_t)((uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4);
        }
        else if (node_id == NODE_ID_SLS && function_code == 0x280 && index == 0x2001 && sub_index == 0x01) {
            // motor 1 temperature
            sls.temp_motor_1_100mdeg = (int16_t)((uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4);
        }
        else if (node_id == NODE_ID_SLS && function_code == 0x280 && index == 0x2001 && sub_index == 0x02) {
            // motor 2 temperature
            sls.temp_motor_2_100mdeg = (int16_t)((uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4);
        }
        else if (node_id == NODE_ID_SLS && function_code == 0x380 && index == 0x2000 && sub_index == 0x01) {
            // uzk
            sls.uzk_10mv = (uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4;
        }
        else if (node_id == NODE_ID_SLS && function_code == 0x380 && index == 0x2001 && sub_index == 0x01) {
            // motor current
            sls.motor_current_100ma = (int16_t)((uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4);
        }
        else if (node_id == NODE_ID_SLS && function_code == 0x380 && index == 0x2002 && sub_index == 0x01) {
            // input current
            sls.input_currect_100ma = (int16_t)((uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4);
        }
        else if (node_id == NODE_ID_SLS && function_code == 0x380 && index == 0x2003 && sub_index == 0x01) {
            // rpm
            sls.rpm = (uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4;
        }
        
        // Foil control
        else if (node_id == NODE_ID_FOIL_CONTROL && function_code == 0x280 && index == 0x2000 && sub_index == 0x01) {
            // Primary input position
            foil_control.primary_input_position = (uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4;
        }
        else if (node_id == NODE_ID_FOIL_CONTROL && function_code == 0x280 && index == 0x2001 && sub_index == 0x01) {
            // Primary output position
            foil_control.primary_output_position = (uint16_t)rx_msg.frame.data5 << 8 | (uint16_t)rx_msg.frame.data4;
        }
    }
}

void can_bus_init(void) {
    // Enable the CAN bus
    CAN1_TransmitEnable();
    CAN1_ReceiveEnable();
    
    
}

// Needs to be called in the main loop
void can_bus_process(void) {
    // Read can bus message
    can_bus_receive_messages();
    
    /*
    uCAN_MSG tx_msg;
    
    tx_msg.frame.msgtype = CAN_MSG_DATA;
    tx_msg.frame.idType = CAN_FRAME_STD;
    tx_msg.frame.dlc = 8;
    tx_msg.frame.id = 0x402;
    tx_msg.frame.data0 = 0x01;
    tx_msg.frame.data1 = 0x02;
    tx_msg.frame.data2 = 0x03;
    tx_msg.frame.data3 = 0x04;
    tx_msg.frame.data4 = 0x05;
    tx_msg.frame.data5 = 0x06;
    tx_msg.frame.data6 = 0x07;
    tx_msg.frame.data7 = 0x08;
    
    CAN1_transmit(CAN_PRIORITY_HIGH, &tx_msg);
     * */
}

mg_battery_t get_can_data_mg_battery(void) {
    return mg_battery;
}

mg_mppt_t get_can_data_mg_mppt(uint8_t nr) {
    if (nr < NODE_ID_MG_MPPT_TOTAL) {
        return mg_mppt[nr];
    } else  {
        return mg_mppt[0];
    }
}

sls_t get_can_data_sls(void) {
    return sls;
}

foil_control_t get_can_data_foil_control(void) {
    return foil_control;
}
