#include "fan.h"
#include "ntc.h"
#include "cool.h"
#include "timer.h"
#include "app_cool.h"
#include <stack/ble/ble.h>

u8 ctl_level 	= 1;	// every time entry smart mode, ensure gfan_level = 45;
//u8 smart_level 	= SMART_LEVEL_LOW_VOLTAGE;
//u8 gfan_level  	= SMART_LEVEL_MID_VOLTAGE;
//u8 gcool_level 	= SMART_LEVEL_MID_VOLTAGE;

extern u8 tec_time_sync_mark;

u8  v_fan = 0x5B, v_tec   = 0x64;
u8  v_reference_fan    = 0x5B;
u8  v_reference_tec    = 0x64;

bool AI_level_process(void)
{
	static u32 gStartTick  = 0;
	static u32 gStartTick1 = 0;
	static u32 gStartTick2 = 0;
	static u8  sys_boot    = 0;			// Power up default is AI_level mode use
//	u8  v_fan = 0x64, v_tec   = 0x64;
//	u8  v_reference_fan    = 0x64;
//	u8  v_reference_tec    = 0x64;

	if((check_fan_ai() == false))	// Normal mode
	{
		sys_boot = 1;
		return false;
	}

	if(tec_time_sync_mark)
	{
		tec_time_sync_mark = 0;
		gStartTick2 = clock_time();
	}

	if(sys_boot == 0)
	{
		static u8 level = BS_FAN_LEVEL_MAX;
		if(clock_time_exceed(gStartTick,100*1000))	// 100ms
		{
			if( level >= BR33_COOL_1)
			{
				cool_level_set(level);
				level--;
			}
			else
			{
				sys_boot = 1;
				tec_time_sync_mark = 1;	// timer_aline = 1�� change normal mode for reset clock
			}
			gStartTick += 100*CLOCK_16M_SYS_TIMER_CLK_1MS;
		}
	}
	else
	{
		if(clock_time_exceed(gStartTick1,30000*1000))	// 30s
		{
			v_fan = v_reference_fan;
			v_tec = v_reference_tec;

			if((gNtcData >= _66_mV) && (gNtcData <= _168_mV))	// temperature: -5��~ 15��
			{
				v_reference_fan = BR33_FAN_1;
				v_reference_tec = BR33_COOL_1;
			}
			else if((gNtcData < _66_mV) && (gNtcData >= _54_mV))	// temperature: 15��~ 20��
			{
				v_reference_fan = BR33_FAN_2;
				v_reference_tec = BR33_COOL_2;
			}
			else
			{
				v_reference_fan = BR33_FAN_3;
				v_reference_tec = BR33_COOL_3;
			}

			gStartTick1 += 30000*CLOCK_16M_SYS_TIMER_CLK_1MS;
		}

		if(clock_time_exceed(gStartTick2,1000*1000))	// 1s
		{
			if(v_fan > v_reference_fan)
			{
				v_fan -= 1;
			}
			else if(v_fan < v_reference_fan)
			{
				v_fan += 1;
			}
			else
			{
				v_fan = v_reference_fan;
			}
			if(v_tec > v_reference_tec)
			{
				v_tec -= 2;
			}
			else if(v_tec < v_reference_tec)
			{
				v_tec += 2;
			}
			else
			{
				v_tec = v_reference_tec;
			}

			fan_level_set(v_fan);
			cool_level_set(v_tec);

			gStartTick2 += 1000*CLOCK_16M_SYS_TIMER_CLK_1MS;
		}
	}
	return true;
}
