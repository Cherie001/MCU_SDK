/********************************************************************************************************
 * @file     adv.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#include "adv.h"
#include "app.h"
#include "led.h"
#include <stack/ble/ble.h>
#include "flash_env.h"

own_addr_type_t app_own_address_type = OWN_ADDRESS_PUBLIC;

static u32 gStartTick = 0;
extern u8 light_time_sync_mark;

u8 gAdvEnable = 0;
static u32 gAdvTick = 0;

/* ADV Packet, SCAN Response Packet define */
u8 tbl_advData[31] = {
	 0x02, 0x01, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported

	 0x1B, 0x09,

	 'B',
	 'l',
	 'a',
	 'c',
	 'k',
	 ' ',

	 'S',
	 'h',
	 'a',
	 'r',
	 'k',
	 ' ',

//	 'F',
//	 'u',
//	 'n',
//	 'C',
//	 'o',
//	 'o',
//	 'l',
//	 'e',
//	 'r',

	 'M',
	 'a',
	 'g',
	 'C',
	 'o',
	 'o',
	 'l',
	 'e',
	 'r',
	 ' ',

	 '3',
	 'P',
	 'r',
	 'o'
};

u8 tbl_scanRsp [31] = {

	0x02, 0x01, 0x06,

	0x1B,
	0xFF,
	0x8F, 0x03,						//mi sig
	0x4D,							//M acsii

	0x21,							//Allocation ID
	0x00,							//CheckSum = tbl_scanRsp[9]

	0xA0,							//Reserve
	0x02,							//status
	0x06,							//SubId.
	0x00,0x00,						//Reserve

	0x00,							//MAC LAP
	0x00,							//MAC LAP
	0x00,							//MAC LAP

	0x00,0x00,0x00,0x00,0x00,0x00,	//Reserve

	0x00,							//adv tick
	0x00,0x00,0x00,0x00,0x00,0x00	//Reserve
};

void adv_process(void)
{
	if(light_time_sync_mark) {
		gStartTick = clock_time();
//		light_time_sync_mark = 0;
	}

//	DEBUG_LOG(".......................................................Read NTC data data = %d.\r\n",data);
	if(clock_time_exceed(gStartTick,1000*1000))//1s
	{
	   if(gAdvEnable&&(gAdvTick >= ADV_TIMEOUT_S))
	   {
//		   bls_ll_setAdvEnable(0);
		   led_ble_adv_stop();
		   gAdvEnable = 0;
		   	DEBUG_LOG("........................................................\r\n");
	   }
	   else
	   {
		   gAdvTick ++;
	   }

	   gStartTick += 1000*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}
}

void adv_update_status(u8 status)
{
	DEBUG_LOG("Adv status update = %d.\r\n",status);
	gAdvEnable = status;
	light_time_sync_mark = 1;
	gAdvTick = 0;
	gStartTick = clock_time();
}

void adv_init(void)
{
	u8 check_sum = 0;
	u8 ble_tick  = 0x01;

	u8 mac_public[6];
	u8 mac_random_static[6];
	blc_initMacAddress(CFG_ADR_MAC, mac_public, mac_random_static);

	if(flash_get(FLASH_ADV_COUNT_ADDR,&ble_tick,sizeof(ble_tick)) == false)
	{
		ble_tick =  0;
	}
	ble_tick ++;
	DEBUG_LOG("Read mi auto pair tick = %d.\r\n",ble_tick);
	flash_set(FLASH_ADV_COUNT_ADDR,&ble_tick,sizeof(ble_tick));

	tbl_scanRsp[18] = mac_public[4];
	tbl_scanRsp[19] = mac_public[5];
	tbl_scanRsp[20] = mac_public[3];
	tbl_scanRsp[21] = mac_public[0];
	tbl_scanRsp[22] = mac_public[1];
	tbl_scanRsp[23] = mac_public[2];
	for(u8 i = 0; i < 8; i++)
	{
		check_sum = check_sum + tbl_scanRsp[10 + i];
	}
	tbl_scanRsp[9] = check_sum;
	tbl_scanRsp[24] = ble_tick;
	/*-- USER application initialization -------------------------------------*/
	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));

	/* Configure ADV packet */
	#if(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_PUBLIC)
		app_own_address_type = OWN_ADDRESS_PUBLIC;
	#elif(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_RANDOM_STATIC)
		app_own_address_type = OWN_ADDRESS_RANDOM;
		blc_ll_setRandomAddr(mac_random_static);
	#endif

	u8 status = bls_ll_setAdvParam(  MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
									 ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
									 0,  NULL,
									 MY_APP_ADV_CHANNEL,
									 ADV_FP_NONE);
	//debug: ADV setting err
	if(status != BLE_SUCCESS)
	{
		DEBUG_LOG("Adv set param fail.reboot....\r\n");
		write_reg8(0x8000, 0x11);
	    while(1);
	}

	bls_ll_setAdvEnable(1);  //adv enable
	adv_update_status(1);
	led_ble_adv_start();
	DEBUG_LOG("Adv start.\r\n\n");
}

