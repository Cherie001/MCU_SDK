/********************************************************************************************************
 * @file     ota.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#ifndef _ADV_H
#define _ADV_H

#include "app_config.h"
#include <tl_common.h>

#define	ADV_TIMEOUT_S				5

#define MY_APP_ADV_CHANNEL			BLT_ENABLE_ADV_ALL
#define MY_ADV_INTERVAL_MIN			ADV_INTERVAL_30MS
#define MY_ADV_INTERVAL_MAX			ADV_INTERVAL_35MS
#define	BLE_DEVICE_ADDRESS_TYPE 	BLE_DEVICE_ADDRESS_PUBLIC

extern u8  gAdvEnable;
//extern u32 gAdvTick;
//extern u32 g_StartTick;

void adv_update_status(u8 status);
void adv_process(void);

#endif /* APP_H_ */
