#include "app.h"
#include "drivers.h"
#include <tl_common.h>
#include "fixed_time.h"
#include "digital_tube.h"
#include <stack/ble/ble.h>
#include "../common/user_config.h"
#include "../common/blt_fw_sign.h"

u8  shut_down_flag = 0;		// enable/disable back_clip time function; 0: disable 1: enable
u8  shut_down_time = 0;

u16 gDownTick = 0;
static u32 gStartTick = 0;

// Past temperature value
void shut_down_process(void)
{
	static u8 shut_down_time_value = 0;

	if(shut_down_time_value != shut_down_time)
	{
		shut_down_time_value = shut_down_time;
		gDownTick = 0;
	}

	if(clock_time_exceed(gStartTick,1000*1000))//1s
	{
		if(shut_down_flag)
		{
			if(gDownTick >= (shut_down_time * TIME_SCAALE))
			{
				DEBUG_LOG("shut_down_flag = %d\r\n", shut_down_flag);
				// enter low power mode
				gDownTick = 0;
				shut_down_flag = 0;
//				cpu_sleep_wakeup(DEEPSLEEP_MODE, 0, 0);
				function_disable();
			}
			else
			{
				gDownTick ++;
			}
		}
		gStartTick += 1000*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}
}

