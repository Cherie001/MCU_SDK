#include "fan.h"
#include "drivers.h"
#include "app_config.h"
#include <tl_common.h>
#include "adc_sample.h"
#include "stack/ble/ble.h"

#define ENABLE		1
#define DISABLE		0

extern u8 flag_judge_voltage;

u16 tec_sample_voltage;
u32 ntc_sample_voltage;
u32 vbus_sample_voltage = 0;


#if DISABLE
static u32 gTecTick = 0;
static u32 gNtcTick = 0;
static u32 gVbusTick = 0;

void adc_sample_process(void)
{
	if(clock_time_exceed(gTecTick,1000*1000))	// 1s
	{
		adc_base_init(GPIO_NTC); 			// ADC initial
		ntc_sample_voltage = adc_set_sample_and_get_result();
		DEBUG_LOG("...........ntc_sample_voltage value = %dmV.\r\n", ntc_sample_voltage);

		gStartTick += 1000*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}

	if(clock_time_exceed(gNtcTick,2000*1000))	// 2s
	{
		adc_base_init(GPIO_TEC); 			// ADC initial
		tec_sample_voltage = adc_set_sample_and_get_result();
		DEBUG_LOG("...........tec_sample_voltage value = %dmV.\r\n", tec_sample_voltage);

		gStartTick += 2000*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}
	if(clock_time_exceed(gVbusTick,3000*1000))	// 3s
	{
//		adc_base_init(GPIO_VBUS); 			// ADC initial
		vbus_sample_voltage = adc_set_sample_and_get_result();
		DEBUG_LOG("...........vbus_sample_voltage value = %dmV.\r\n", vbus_sample_voltage);

		gStartTick += 3000*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}
}
#else

void adc_sample_ntc(void)
{
	adc_base_init(GPIO_NTC); 			// ADC initial
	ntc_sample_voltage = adc_set_sample_and_get_result();
	DEBUG_LOG("...........ntc_sample_voltage value = %dmV.\r\n", ntc_sample_voltage);
}

void adc_sample_tec(void)
{
	adc_base_init(GPIO_NTC_TYPE); 			// ADC initial
	tec_sample_voltage = adc_set_sample_and_get_result();
	DEBUG_LOG("...........tec_sample_voltage value = %dmV.\r\n", tec_sample_voltage);
}

void adc_sample_vbus(void)
{
//	static u8 times = 0;
//	u16 vbus_voltage_value = 0;
//	static u16 vbus_voltage = 0;

	adc_base_init(GPIO_NTC_BUCK); 			// ADC initial
	vbus_sample_voltage = adc_set_sample_and_get_result();
}













#endif
