/********************************************************************************************************
 * @file     timer.c
 *
 * @brief    for TLSR chips
 *
 * @author	 john.zhou
 * @date     Jan, 2019
 *
 * @par
 *
 *******************************************************************************************************/
#include "timer.h"
#include "app.h"
#include "led.h"
#include <stack/ble/ble.h>
#include "fan.h"
#include "light.h"
#include "FP_QC_Sink.h"
#include "pd_qc.h"
#include "digital_tube.h"
#include "FP6606_ARM.h"
#include "FP6606_tcpm.h"

//u8 switch_qc = 0;
//static u16  count_qc = 0;

static u32 gStartTick = 0;
u8 light_time_sync_mark = 1;

_attribute_ram_code_ void irq_handler(void)	// BLE interrupt process entrance
{
	irq_blt_sdk_handler();
	/***************GPIO_FAN_FG********************/
	if((reg_irq_src & FLD_IRQ_GPIO_EN)==FLD_IRQ_GPIO_EN)
	{
		reg_irq_src |= FLD_IRQ_GPIO_EN;
		fan_input_triger();
	}
	/***************GPIO_FP_INT********************/
	if((reg_irq_src & FLD_IRQ_GPIO_RISC0_EN)==FLD_IRQ_GPIO_RISC0_EN)
	{
		reg_irq_src |= FLD_IRQ_GPIO_RISC0_EN;

		fp6606_pd.alert_status = 1;
	}

	/***************TIMER1******************/
	if(reg_tmr_sta & FLD_TMR_STA_TMR1)
	{
		reg_tmr_sta |= FLD_TMR_STA_TMR1; //clear irq status

		/**************calculate fan_speed************/
		fan_cal_interval();

		/*****************pd_qc_count*****************/
		pd_qc_count();

	}
	/*********************************************/

}

void timer_process(void)
{
	if(light_time_sync_mark)
	{
		gStartTick = clock_time();
		light_time_sync_mark = 0;
	}
	if(clock_time_exceed(gStartTick,HW_TIMER_BASE_TOMS*1000))
	{
	   light_timer_process();

	   gStartTick += HW_TIMER_BASE_TOMS*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}
}

void timer_init(void)
{
	timer1_set_mode(TIMER_MODE_SYSCLK, 0, HW_TIMER_BASE_TOMS * CLOCK_SYS_CLOCK_1MS);//10ms
	timer_start(TIMER1);

//	timer2_set_mode(TIMER_MODE_TICK, 0, HW_TIMER_BASE_TOMS * CLOCK_SYS_CLOCK_1MS);//10ms
//	timer_start(TIMER2);

	DEBUG_LOG("Hardware timer init ok.\r\n\n");
}
