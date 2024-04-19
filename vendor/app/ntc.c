/********************************************************************************************************
 * @file    ntc.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#include "ntc.h"
#include "app.h"
#include "fan.h"
#include "led.h"
#include "gap.h"
#include "cool.h"
#include "light.h"
#include "flash_env.h"
#include "fixed_time.h"
#include "adc_sample.h"
#include "digital_tube.h"
#include "digital_tube.h"
#include <stack/ble/ble.h>
#include <stack/ble/ble_common.h>

static u32 gStartTick  = 0;
static u32 gStartTick1 = 0;
static u32 ntc_offset = 2;

//u16  g_temp_value = 0;
u8  low_temp_reset = 0;
u8  hot_temp_reset = 0;
u8  led_switch = 1;
u8  g_temp_value = 0;
u16 g_temp_c = 0;
u32 gNtcData = 0;

//void ntc_process(void)
//{
//	if(clock_time_exceed(gStartTick,1000*1000))//1s
//	{
//	   gNtcData = adc_set_sample_and_get_result();
////	   DEBUG_LOG("Read NTC Adc data = %dmV,Hex value=0x%X.\r\n",gNtcData,gNtcData);
//	   gStartTick += 1000*CLOCK_16M_SYS_TIMER_CLK_1MS;
//	}
//}

/**
 * example:1956 = 195.6 kΩ * 10
 * 1956 - > -40℃   22-> 70℃
*/

#define NTC_MIN_TEMP -40
#define NTC_MAX_TEMP 70

static int ntc_res_table[] = {1956,1849,1748,1653,1565,1481,1403,1329, 	
								1260,1194,1133,1075,1021,969,921,875, 	
								832,791,753,716,682,649,619,590, 	
								562,536,511,488,466,445,425,405, 	
								387,370,354,338,324,310,296,284, 	
								272,260,249,239,229,220,211,202, 	
								194,186,179,172,165,158,152,146, 	
								141,135,130,125,120,116,111,107, 	
								103,100,96,92,89,86,83,80, 	
								77,74,71,69,67,64,62,60, 	
								58,56,54,52,50,49,47,45, 	
								44,43,41,40,38,37,36,35, 	
								34,33,32,31,30,29,28,27, 	
								26,25,25,24,23,22,22};

// Past temperature value
void ntc_process(void)
{
	static u16 ntc_count_b = 0;

	if(clock_time_exceed(gStartTick,1000*1000))		// 1s
	{
		adc_base_init(GPIO_NTC);

		gNtcData = adc_set_sample_and_get_result();

		if(gNtcData >= 10 && gNtcData <= 2620)
		{
			g_temp_value = choose_temperture_value(calculate_thermistor_value(gNtcData));
		}
		DEBUG_LOG("Temperature = %d, ADC data = %dmV, ntc_offset=0x%X.\r\n", g_temp_value, gNtcData, ntc_offset);

		if((g_temp_value > 200) && (g_temp_value <= 248)) // -5 - >  251
		{
			if(ntc_count_b >= 300)
			{
				DEBUG_LOG("%s function_disable\n", __FUNCTION__);
				low_temp_reset = 1;
//				TEC_DISABLE();
				function_disable();
				ntc_count_b = 0;
			}
			else
			{
				ntc_count_b ++;
			}
		}
		else
		{
			ntc_count_b = 0;
		}

		gStartTick += 1000*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}

}

void high_tem_process()
{
//	static u32 temp_c;
	static u16 ntc_count_c = 0;
	//static u16 ntc_count_reset = 0;

	if(clock_time_exceed(gStartTick1,1000*1000))		// 1s
	{
		adc_base_init(GPIO_NTC_TYPE);

		g_temp_c = adc_set_sample_and_get_result();

		if(g_temp_c < 81 )	// > 65° ~ < 127°
		{
			if(ntc_count_c >= 120)
			{
				DEBUG_LOG("%s function_disable\n", __FUNCTION__);
				function_disable();
				led_switch = 0;
				ntc_count_c = 0;
			}
			else
			{
				ntc_count_c ++;
			}
		}
		else
		{
			ntc_count_c = 0;
		}
		gStartTick1 += 1000*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}
}

void ntc_module_type_init(void)
{
	enum NTC_TYPE ntc_type = NTC_NULL;

	flash_get(FLASH_NTC_TYPE_ADDR,&ntc_type,sizeof(ntc_type));
	DEBUG_LOG("ntc get type from flash = 0x%x\r\n",ntc_type);

	switch(ntc_type) {
		case NXFT15WF104:
			ntc_offset = 2;
		break;
		case FHCWF22251A:
			ntc_offset = 3;
		break;
		default:
			if(NTC_MODULE_FAC_VERSION) {
				ntc_offset = 3;
				ntc_type = FHCWF22251A;
				flash_set(FLASH_NTC_TYPE_ADDR,&ntc_type,sizeof(ntc_type));
				DEBUG_LOG("ntc type init = %x\r\n",ntc_type);
			}
	}
}

u8 ntc_get_type(void) 
{
	enum NTC_TYPE ntc_type = NTC_NULL;

	flash_get(FLASH_NTC_TYPE_ADDR,&ntc_type,sizeof(ntc_type));

	return ntc_type;
}

u8 ntc_get_temper_offset(void)
{
	return ntc_offset;
}

void ntc_init(void)
{
	ntc_module_type_init();
	adc_init();
	adc_base_init(GPIO_NTC); 			// ADC initial
	DEBUG_LOG("NTC init ok.\r\n\n");
}

/********************************************************************************************************
 *
 * thermistor_value calculate
 *
 * R = 750K; Rt:thermistor; V(mV) = gNtcData.
 *
 * V(mV) = 3440(mV)*Rt/(R + Rt)
 *
 * Rt = V(mV)*R/(3440mV - V(mV))
 *
 *******************************************************************************************************/
u32 calculate_thermistor_value(u32 Ntc_value)
{
	u32 gNtcData_R ;
	u32 three_gNtcData;
	u32 thermistor_value;

	gNtcData_R = Ntc_value * 7500;
	three_gNtcData = 3440 - Ntc_value;
	thermistor_value = gNtcData_R / three_gNtcData;

	DEBUG_LOG("thermistor_value = %d\r\n", thermistor_value);

	return thermistor_value;
}


u32 choose_temperture_value(u32 Rt)
{
	u32 Rt_value = Rt;
	int temp_value = 0;

	if(Rt_value > ntc_res_table[0] || Rt_value < ntc_res_table[sizeof(ntc_res_table)/4-1]) {
		return 0;
	}

	for(int i = 0; i < sizeof(ntc_res_table)/4; i++) {
		if(Rt_value <= ntc_res_table[i] && Rt_value > ntc_res_table[i+1]) {
			temp_value =  (NTC_MIN_TEMP + i);
			break;
		}
	}

	DEBUG_LOG("ETC Temp = %d\r\n", temp_value);

	if(temp_value < 0) {
		return (u8)(temp_value);
	} else {
		return temp_value;
	}

}



















//// Past temperature value
//void ntc_process(void)
//{
//	if(clock_time_exceed(gStartTick,1000*1000))		// 1s
//	{
//		adc_base_init(GPIO_NTC);
//
//		gNtcData = adc_set_sample_and_get_result();
//
//		if(gNtcData >= 10 && gNtcData <= 2620)
//		{
//			g_temp_value = choose_temperture_value(calculate_thermistor_value(gNtcData)) - 2;
//
//			temperture_divide(g_temp_value);
//		}
////		DEBUG_LOG("Temperature = %d, ADC data = %dmV, Hex value=0x%X.\r\n", g_temp_value, gNtcData, gNtcData);
//		gStartTick += 1000*CLOCK_16M_SYS_TIMER_CLK_1MS;
//	}
//
//	if((g_temp_value > 65) && (g_temp_value < 220))	// high temperature protect
//	{
//		// entry low power mode to shut down back clip
//		spi_display_allclose();
//		cpu_sleep_wakeup_32kpad(DEEPSLEEP_MODE, 0, 0);
//	}
//}

