/********************************************************************************************************
 * @file     app.h
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#ifndef _APP_H
#define _APP_H

#include "bs_vcom.h"
#define POWER_KEY_STATUS()	gpio_read(GPIO_KEY_POWER)

typedef enum{
    POWER_KEY_OFF = 0,
    POWER_KEY_ON  = 1,
}POWER_LEY_STATUS;

extern bs_vcom_t ble_vcom;
extern u8 power_ic;
extern u8 switch_qc;

void function_disable();
void open_led(void);
void Telink_hd_init(void);
void Telink_Ble_init(void);
void FP6606_IC_init(void);
void check_power_key_off(void);
void peripher_init(void);
void profile_init(void);
bool BsTxSendData(u8 *p, int len);
void main_loop(void);
u8 qc_voltage_check(void);
#endif /* GAP_H_ */
