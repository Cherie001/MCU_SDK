/********************************************************************************************************
 * @file     cool.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#include "cool.h"
#include "app.h"
#include "led.h"
#include "fan.h"
#include <stack/ble/ble.h>
#include "flash_env.h"

u8 tec_time_sync_mark = 1;
u8 tec_inbox_switch = 0;

static u32 gStartTick = 0;
static u8 gCoolLevel = 0;


void cool_level_set(u8 level)
{
	if(level == BS_COOL_DISABLE)
	{
		level = BS_COOL_DISABLE;
		pwm_stop(PWM5_ID);
		TEC_DISABLE();
	}
	else
	{
		pwm_set_cycle_and_duty(PWM5_ID, (u16) (1000 * CLOCK_SYS_CLOCK_1US),  (u16) (level*10 * CLOCK_SYS_CLOCK_1US));
//		pwm_start(PWM5_ID);
//		TEC_ENABLE();
	}

//	DEBUG_LOG("Cool level set = %d.\r\n",level);
}

void cool_level_save(u8 level)
{
    flash_set(FLASH_COOL_LEVEL_ADDR,&level,sizeof(level));
    DEBUG_LOG("Cool level save = %d.\r\n",level);
}

u8 cool_level_get(void)
{
	u8 level = BS_COOL_LEVEL_MAX;

	flash_get(FLASH_COOL_LEVEL_ADDR,&level,sizeof(level));
	if((level >= BS_COOL_LEVEL_MAX) && (level != 0xFA))
	{
		level = BR33_COOL_3;
	    cool_level_save(COOL_LEVEL_START_DEFAULT);
	    DEBUG_LOG("Cool level read flash fail.Set default level = %d.\r\n",level);
	}
	else
	{
		if(level == BS_COOL_DISABLE)
		{
			level = BR33_COOL_3;
		    cool_level_save(COOL_LEVEL_START_DEFAULT);
		}
	}
	return level;
}

void cool_process(void)
{
//	static u8 level = BS_COOL_LEVEL_START;
	static u8 level = 100;

	if(tec_inbox_switch)		// Reset the power on the ladder
	{
		tec_inbox_switch = 0;
//		timer_aline = 0;
		level = BS_COOL_LEVEL_MAX;
	}

	if(tec_time_sync_mark)
	{
		tec_time_sync_mark = 0;
		gStartTick = clock_time();
	}


	if(clock_time_exceed(gStartTick,20*1000))//20ms
	{
		if( level >= cool_level_get())
		{
			cool_level_set(level);
			level--;
			DEBUG_LOG("level = %d.\r\n",level);
		}
		else
		{
			cool_level_set(cool_level_get());
		}
	   gStartTick += 20*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}
}

void cool_init(void)
{
	// GPIO_PA7 GPIO_TEC_PWM
	reg_clk_en1 |= FLD_CLK1_PWM_EN;
	pwm_set_clk(CLOCK_SYS_CLOCK_HZ, CLOCK_SYS_CLOCK_HZ);
	gpio_set_func(GPIO_TEC_PWM,  AS_PWM5);
	pwm_set_mode(PWM5_ID, PWM_NORMAL_MODE);

	// GPIO_PB3 COOL_VDD_EN
	gpio_set_func(GPIO_TEC_EN ,AS_GPIO);
	gpio_set_output_en(GPIO_TEC_EN, 1); 		// Enable output
	gpio_set_input_en(GPIO_TEC_EN ,0);		// Disable input
//	gpio_write(GPIO_TEC_EN, 1);
	gpio_write(GPIO_TEC_EN, 0);

	gCoolLevel = cool_level_get();
	cool_level_set(100);

	pwm_start(PWM5_ID);
	TEC_ENABLE();

	DEBUG_LOG("Cool init ok.Level = %d.\r\n",gCoolLevel);
}


