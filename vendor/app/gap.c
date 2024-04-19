/********************************************************************************************************
 * @file     app.c
 *
 * @brief    for TLSR chips
 *
 * @author	 BLE Group
 * @date     May. 12, 2018
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *			 The information contained herein is confidential and proprietary property of Telink 
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in. 
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this 
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/
#include "gap.h"
#include "ota.h"
#include "app.h"
#include "led.h"
#include "adv.h"
#include "drivers.h"
#include "tl_common.h"
#include <stack/ble/ble.h>
#include "../common/blt_common.h"

MYFIFO_INIT(blt_rxfifo, 64, 8);
MYFIFO_INIT(blt_txfifo, 40, 16);

u8 bond_flag = 0;
void write_vetsion_to_flash(void)
{
	u8 Firmware_version[12] = {0};
	u8 flash_version[12] = {0};

    memcpy(Firmware_version, SOFTWARE_VERSION_BUILD, strlen(SOFTWARE_VERSION_BUILD));

//	flash_write_page (CUST_USER_VERSION_ADDR, 12, Firmware_version);	// Write Firmware_version into flash

    if(flash_get(CUST_USER_VERSION_ADDR, flash_version, 12))
    {
    	if(memcmp(Firmware_version, flash_version, 12))
    	{
    		// Firmware_version = flash_version
    	}
    	else
    	{
        	user_flash_write_page(CUST_USER_VERSION_ADDR, 12, Firmware_version);
    	}
    }
    else
	{
    	user_flash_write_page(CUST_USER_VERSION_ADDR, 12, Firmware_version);
	}
}

/*----------------------------------------------------------------------------*/
/*------------- CallBack function of BLE                      ----------------*/
/*----------------------------------------------------------------------------*/
void bluetooth_terminate(u8 e,u8 *p, int n) //*p is terminate reason
{
	DEBUG_LOG("Bluetooth terminate,reason = 0x%02x\r\n",*p);

	adv_update_status(1);
	led_ble_adv_start();
	if(1)
	{
		blc_att_setRxMtuSize(23);
		blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, 23);
	}

	switch(*p)
	{
		case HCI_ERR_CONN_TIMEOUT:
			DEBUG_LOG("Bluetooth terminate,HCI_ERR_CONN_TIMEOUT\r\n");
			break;
		case HCI_ERR_REMOTE_USER_TERM_CONN:
			DEBUG_LOG("Bluetooth terminate,HCI_ERR_REMOTE_USER_TERM_CONN\r\n");
			break;
		case HCI_ERR_CONN_TERM_MIC_FAILURE:
			DEBUG_LOG("Bluetooth terminate,HCI_ERR_CONN_TERM_MIC_FAILURE\r\n");
			break;
		default:
			break;
	}
}

void bluetooth_connect(u8 e, u8 *p, int n)
{
	DEBUG_LOG("Bluetooth connected.\r\n");
	adv_update_status(0);
	led_ble_conn();
	bls_l2cap_requestConnParamUpdate (24, 24, 0, 300);  //interval=30ms latency=0 timeout=3s

	write_vetsion_to_flash();
	bond_flag = 1;
}

void bluetooth_pair(u8 e, u8 *p, int n)
{
	DEBUG_LOG("Bluetooth pair,status = 0x%02x\r\n",*p);
}

void gap_init(void)
{
	/*-- BLE stack initialization --------------------------------------------*/
	u8 mac_public[6];
	u8 mac_random_static[6];
	blc_initMacAddress(CFG_ADR_MAC, mac_public, mac_random_static);

	DEBUG_LOG("Local MAC address = 0x[%02X][%02X][%02X][%02X][%02X][%02X]\r\n",mac_public[5],mac_public[4],mac_public[3],mac_public[2],mac_public[1],mac_public[0]);

	/*-- BLE Controller initialization ---------------------------------------*/
	blc_ll_initBasicMCU(mac_public);//mandatory
	blc_ll_initAdvertising_module(mac_public);//adv module: mandatory for BLE slave,
	blc_ll_initSlaveRole_module();//slave module: mandatory for BLE slave,

	/*-- BLE Host initialization ---------------------------------------------*/
	//GATT initialization]
	my_att_init();
	//L2CAP initialization
	blc_l2cap_register_handler(blc_l2cap_packet_receive);
	/*-- BLE SMP initialization ----------------------------------------------*/
  #if (BLE_REMOTE_SECURITY_ENABLE)
	blc_smp_param_setBondingDeviceMaxNumber(4);  	//default is SMP_BONDING_DEVICE_MAX_NUM, can not bigger that this value
													//and this func must call before bls_smp_enableParing
	bls_smp_enableParing (SMP_PARING_CONN_TRRIGER );
  #else
	bls_smp_enableParing (SMP_PARING_DISABLE_TRRIGER);
  #endif

	rf_set_power_level_index (RF_POWER_0dBm);//OK

	adv_init();
	//ble event call back
	bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, &bluetooth_connect);
	bls_app_registerEventCallback (BLT_EV_FLAG_TERMINATE, &bluetooth_terminate);
	bls_app_registerEventCallback (BLT_EV_FLAG_PAIRING_END, &bluetooth_pair);

   #if(BLE_REMOTE_OTA_ENABLE)
	ota_init();
   #endif
}



