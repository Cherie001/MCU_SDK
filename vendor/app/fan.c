/********************************************************************************************************
 * @file     fan.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#include "fan.h"
#include "app.h"
#include "led.h"
#include "cool.h"
#include "timer.h"
#include "flash_env.h"
#include <tl_common.h>
#include "app_config.h"
#include "adc_sample.h"
#include <stack/ble/ble.h>

u8  voltage_level;
u32 gFanSpeed = 0;

static u32 gFanGpioCnt  = 0;
static u32 gFanTimerCnt = 0;

static u32 gStartTick   = 0;


void get_voltage_level(void)
{
//	voltage_level = fan_level_get();
}

bool check_fan_ai(void)
{
	u8 level = BS_FAN_LEVEL_MAX;

	flash_get(FLASH_COOL_LEVEL_ADDR,&level,sizeof(level));
	if (level > BS_FAN_LEVEL_MAX)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void fan_level_set(u8 level)
{
	if(level == BS_FAN_DISABLE)
	{
		level = BS_FAN_DISABLE;
		pwm_stop(PWM2_ID);
		FAN_DISABLE();
		DEBUG_LOG("OVP and BUCK is fail......\r\n");
	}
	else
	{
		pwm_set_cycle_and_duty(PWM2_ID, (u16) (100 * CLOCK_SYS_CLOCK_1US),  (u16) (level*1 * CLOCK_SYS_CLOCK_1US));
//		pwm_start(PWM2_ID);
//		FAN_ENABLE();
	}

	DEBUG_LOG("Fan level set = %d.\r\n",level);
}

void fan_level_save(u8 level)
{
	flash_set(FLASH_FAN_LEVEL_ADDR,&level,sizeof(level));
	DEBUG_LOG("fan level save = %d.\r\n",level);
}


u8 fan_level_get(void)
{
	u8 level = BS_FAN_LEVEL_MAX;

	flash_get(FLASH_FAN_LEVEL_ADDR,&level,sizeof(level));
	if((level >= BS_FAN_LEVEL_MAX) && (level != FAN_LEVEL_START_DEFAULT))		// "level != 0xFA" is ensure smart_level can be catch by APP
	{
		level = BR33_FAN_3;

		fan_level_save(BR33_FAN_3);

		DEBUG_LOG("Fan level read flash fail.Set default level = %d.\r\n",level);
	}
	else
	{
		if(level == BS_FAN_DISABLE)
		{
			level = BR33_FAN_3;
			fan_level_save(BR33_FAN_3);
		}
	}
	return level;
}

void fan_input_triger(void)
{
	gFanGpioCnt++;
}

void fan_cal_interval(void)
{
	gFanTimerCnt++;
	if(gFanTimerCnt >= (FAN_CAL_TIME_OUT_S*1000/HW_TIMER_BASE_TOMS))
	{
		gFanSpeed = gFanGpioCnt*10;
		gFanGpioCnt = 0;
		gFanTimerCnt = 0;
	}
}

void fan_process(void)
{
	static u8 fan_set = 2;
	if(clock_time_exceed(gStartTick,1000*1000))//1s
	{
//	   DEBUG_LOG("Fan speed = %d.\r\n",gFanSpeed);
	   gStartTick += 1000*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}
	if(fan_set)
	{
		fan_set --;
		fan_level_set(fan_level_get());
	}
}

void fan_init(void)
{
	// GPIO_PA5 FAN_FG(calculate fan speed)
	gpio_set_func(GPIO_FAN_FG, AS_GPIO);
	gpio_set_input_en(GPIO_FAN_FG, 1);				// Enable input
	gpio_set_output_en(GPIO_FAN_FG, 0);				// Disable output
	gpio_setup_up_down_resistor(GPIO_FAN_FG, PM_PIN_PULLUP_10K);
	gpio_set_interrupt(GPIO_FAN_FG,GPIO_Pol_rising);

	// GPIO_PC6 FAN_VDD_EN
	gpio_set_func(GPIO_FAN_EN ,AS_GPIO);
	gpio_set_output_en(GPIO_FAN_EN, 1); 		// Enable output
	gpio_set_input_en(GPIO_FAN_EN ,0);		// Disable input
	gpio_write(GPIO_FAN_EN, 0);
//	gpio_write(GPIO_FAN_EN, 1);

	// GPIO_PB2 FAN_PWM
	reg_clk_en1 |= FLD_CLK1_PWM_EN;
	pwm_set_clk(CLOCK_SYS_CLOCK_HZ, CLOCK_SYS_CLOCK_HZ);
	gpio_set_func(GPIO_FAN_PWM,  AS_PWM2);
	pwm_set_mode(PWM2_ID, PWM_NORMAL_MODE);

	voltage_level = fan_level_get();
	fan_level_set(voltage_level);

	pwm_start(PWM2_ID);
	FAN_ENABLE();

	DEBUG_LOG("fan init ok.Level = %d.\r\n\n",voltage_level);
}
