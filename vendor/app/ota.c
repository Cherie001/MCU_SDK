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
#include "ota.h"
#include "app.h"
#include "led.h"
#include "light.h"
#include <stack/ble/ble.h>
#include "timer.h"
#include "app_config.h"

#if(BLE_REMOTE_OTA_ENABLE)

extern u8 led_switch;
extern LightParam _lightParam;

void entry_ota_mode(void)
{
	DEBUG_LOG("Enter OTA mode.\r\n");
//	_lightParam.cycleMs = 500;
//	_lightParam.colorNum = LIGHT_COLOR_GREEN;
//	_lightParam.type = LIGHT_MODE_LEDBREATH;

	led_switch = 0;
	aw210xx_all_breath(0, 0xFF, 0xFF, 0x40, 0x40);
	bls_ota_setTimeout(180 * 1000 * 1000); //set OTA timeout  90 seconds
	timer_stop(TIMER1);

	DEBUG_LOG("OTA start...\r\n");
}

void LED_show_ota_result(int result)
{
	if(result == OTA_SUCCESS)
	{  //OTA success
		DEBUG_LOG("OTA success.\r\n");
	}
	else
	{  //OTA fail
		DEBUG_LOG("OTA fail!!!\r\n");
	}
	timer_start(TIMER1);
}

void ota_init(void)
{
	bls_ota_clearNewFwDataArea(); //must
	bls_ota_registerStartCmdCb(entry_ota_mode);
	bls_ota_registerResultIndicateCb(LED_show_ota_result);
}

#endif
