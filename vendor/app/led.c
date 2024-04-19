/********************************************************************************************************
 * @file     led.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#include "led.h"
#include "app.h"
#include "light.h"
#include <stack/ble/ble.h>
#include "aw210xx.h"

extern LightParam _lightParam;
extern u8 light_time_sync_mark;

static u32 gStartTick = 0;
static u32 gTimerMs = 0;
static LedParam_t gLedParam;


void aw_disable_config(void)
{
	aw210xx_disable();
}

void led_process(void)
{
	if(gLedParam.Mode == LED_MODE_BLINK)
	{
		if(light_time_sync_mark) {
			gStartTick = clock_time();
			light_time_sync_mark = 0;
		}

		if((clock_time_exceed(gStartTick,gTimerMs*1000))&&(gLedParam.Ticks > 1))//1ms process
		{
			if(gLedParam.status == LED_STA_ON)
			{
				gLedParam.status = LED_STA_OFF;
				aw210xx_all_off();
				gTimerMs = gLedParam.OffTimeMs;
			}
			else if(gLedParam.status == LED_STA_OFF)
			{
				gLedParam.status = LED_STA_ON;
				aw210xx_all_on(0x0,0xFF,0x0); //green
				gTimerMs = gLedParam.OnTimeMs;
			}

			if(gLedParam.Ticks != 0xFFFF)
			{
				gLedParam.Ticks --;
			}
			gStartTick += gTimerMs*CLOCK_16M_SYS_TIMER_CLK_1MS;
		}
	}
}

void Led_Auto(LedParam_t* led_para)
{
	switch (led_para->Mode)
	{
	case LED_MODE_ON:
	case LED_MODE_OFF:
		//aw_disable_config();
		gLedParam.Mode = LED_MODE_OFF;
		break;
	case LED_MODE_BLINK:
		//aw_enable_config();
		gLedParam.OnTimeMs = led_para->OnTimeMs;
		gLedParam.OffTimeMs = led_para->OffTimeMs;
		gLedParam.Ticks = led_para->Ticks;
		gTimerMs = led_para->OnTimeMs;
		gLedParam.status = LED_STA_ON;
		gLedParam.Ticks = led_para->Ticks*2;
		gLedParam.Mode = LED_MODE_BLINK;
		break;
	default:
		break;
	}

	light_reinitialize();

	gStartTick = clock_time();
}

void led_ble_adv_start(void)
{
	LedParam_t led_para;
    led_para.Mode = LED_MODE_BLINK;
    led_para.OnTimeMs = 400;
    led_para.OffTimeMs = 400;
    led_para.Ticks = 0xFFFF;
    Led_Auto(&led_para);
}

void led_ble_adv_stop(void)
{
	LedParam_t led_para;
    led_para.Mode = LED_MODE_OFF;
    Led_Auto(&led_para);
}


void led_ble_conn(void)
{
	LedParam_t led_para;
    led_para.Mode = LED_MODE_ON;
    Led_Auto(&led_para);
}

void led_ota_start(void)
{
	LedParam_t led_para;
    led_para.Mode = LED_MODE_BLINK;
    led_para.OnTimeMs = 800;
    led_para.OffTimeMs = 800;
    led_para.Ticks = 0xFFFF;
    Led_Auto(&led_para);
}



