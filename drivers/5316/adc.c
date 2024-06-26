/********************************************************************************************************
 * @file     adc.c
 *
 * @brief    This is the ADC driver file for TLSR8232
 *
 * @author   junyuan.zhang ; junwei.lu
 * @date     May 8, 2018
 *
 * @par      Copyright (c) 2018, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *           The information contained herein is confidential property of Telink
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *           Co., Ltd. and the licensee or the terms described here-in. This heading
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 * @par      History:
 * 			 1.initial release(DEC. 26 2018)
 *
 * @version  A001
 *
 *******************************************************************************************************/

#include "adc.h"

#define ADC_SINGLE_END			0


volatile unsigned short adc_cal_value = 0xffff;


//extern u32 adcresult;

unsigned char ADC_Vref = 0;     		//ADC  Vref
unsigned char ADC_VBAT_Scale = 0;		//ADC VBAT scale
unsigned char ADC_Pre_Scale = 1;		//ADC pre scale

GPIO_PinTypeDef ADC_GPIO_tab[10] = {

		GPIO_PA6,GPIO_PA7,
		GPIO_PB0,GPIO_PB1,
		GPIO_PB2,GPIO_PB3,
		GPIO_PB4,GPIO_PB5,
		GPIO_PB6,GPIO_PB7
};

const unsigned char  Vref_tab[4] = {2,3,4,1};
const unsigned char  VBAT_Scale_tab[4] = {1,4,3,2};

/**
 * @brief This function serves to ADC init.
 * @param[in]   none
 * @return none
 */
void adc_init(void ){

	/****** sar adc Reset ********/
	//reset whole digital adc module
	adc_reset();

	/******enable signal of 24M clock to sar adc********/
	adc_clk_en(1);

	/******set adc clk as 4MHz******/
	adc_set_clk_div(5);

	/******set adc L R channel Gain Stage bias current trimming******/
	//adc_set_pga_left_power_on(0);
	//adc_set_pga_right_power_on(0); Hawk Ã»ÓÐÓÒÍ¨µÀPGA
	adc_set_atb(ADC_SEL_ATB_1);  ///resolve adc value jitter at low temperature
	adc_set_left_gain_bias(ADC_GAIN_STAGE_BIAS_PER100);
	//adc_set_right_gain_bias(ADC_GAIN_STAGE_BIAS_PER100);
}

/**
 * @brief This function is used for IO port configuration of ADC IO port voltage sampling.
 * @param[in]   GPIO_PinTypeDef pin
 * @return none
 */
void adc_base_pin_init(GPIO_PinTypeDef pin)
{
	//ADC GPIO Init
	gpio_set_func(pin, AS_GPIO);
	gpio_set_input_en(pin,0);
	gpio_set_output_en(pin,0);
	gpio_write(pin,0);
}

/**
 * @brief This function is used for IO port configuration of ADC supply voltage sampling.
 * @param[in]   GPIO_PinTypeDef pin
 * @return none
 */
void adc_vbat_pin_init(GPIO_PinTypeDef pin)
{
	gpio_set_func(pin, AS_GPIO);
	gpio_set_input_en(pin,0);
	gpio_set_output_en(pin,1);
	gpio_write(pin,1);
}


/**
 * @brief This function is used for ADC configuration of ADC IO voltage sampling.
 * @param[in]   GPIO_PinTypeDef pin
 * @return none
 */

void adc_ntc_init(GPIO_PinTypeDef pin)
{
	unsigned char i;
	unsigned char gpio_num=0;

	//set R_max_mc,R_max_c,R_max_s
	adc_set_misc_rns_capture_state_length(0xf0);						//max_mc
//	adc_set_left_right_capture_state_length(AMIC_ADC_SampleLength[0]);	//max_c	96K
	adc_set_all_set_state_length(0x0a);												//max_s

	//set total length for sampling state machine and channel
	adc_set_chn_en(ADC_MISC_CHN);
	adc_set_max_state_cnt(0x02);

	//set channel Vref
	adc_set_all_vref(ADC_MISC_CHN, ADC_VREF_VBAT_N);
	ADC_Vref = (unsigned char)ADC_VREF_VBAT_N;

	//set Vbat divider select,
	adc_set_vref_vbat_div(ADC_VBAT_DIVIDER_1F2);
	ADC_VBAT_Scale = VBAT_Scale_tab[ADC_VBAT_DIVIDER_1F2];

	//set channel mode and channel
	adc_base_pin_init(pin);		//ADC GPIO Init
	for(i=0;i<11;i++)
	{
		if(pin == ADC_GPIO_tab[i])
		{
			gpio_num = i+1;
			break;
		}
	}
#if ADC_SINGLE_END
	adc_set_all_input_mode(ADC_MISC_CHN, SINGLE_ENDED_MODE);
	adc_set_all_single_end_ain(ADC_MISC_CHN, gpio_num);
#else
	adc_set_all_input_mode(ADC_MISC_CHN, DIFFERENTIAL_MODE);
	adc_set_all_differential_p_n_ain(ADC_MISC_CHN, gpio_num, GND);
#endif

	//set resolution for RNG
	adc_set_all_resolution(ADC_MISC_CHN, RES10);

	//Number of ADC clock cycles in sampling phase
	adc_set_all_tsample_cycle(ADC_MISC_CHN, SAMPLING_CYCLES_6);
	//set Analog input pre-scaling
	adc_set_all_ain_pre_scaler(ADC_PRESCALER_1F8);
	ADC_Pre_Scale = 1<<(unsigned char)ADC_PRESCALER_1F8;

	//set RNG mode
	adc_set_mode(ADC_NORMAL_MODE);
}


/**
 * @brief This function is used for ADC configuration of ADC IO voltage sampling.
 * @param[in]   GPIO_PinTypeDef pin
 * @return none
 */

void adc_base_init(GPIO_PinTypeDef pin)
{
	unsigned char i;
	unsigned char gpio_num=0;

	//set R_max_mc,R_max_c,R_max_s
	adc_set_misc_rns_capture_state_length(0xf0);						//max_mc
//	adc_set_left_right_capture_state_length(AMIC_ADC_SampleLength[0]);	//max_c	96K
	adc_set_all_set_state_length(0x0a);												//max_s

	//set total length for sampling state machine and channel
	adc_set_chn_en(ADC_MISC_CHN);
	adc_set_max_state_cnt(0x02);

//	if (pin == GPIO_PB6)
//	{	//set channel Vref
//		adc_set_all_vref(ADC_MISC_CHN, ADC_VREF_VBAT_N);
//		ADC_Vref = (unsigned char)ADC_VREF_VBAT_N;
//
//		//set Vbat divider select,
//		adc_set_vref_vbat_div(ADC_VBAT_DIVIDER_1F2);
//		ADC_VBAT_Scale = VBAT_Scale_tab[ADC_VBAT_DIVIDER_1F2];
//	}
//	else
	{
		//set channel Vref
		adc_set_all_vref(ADC_MISC_CHN, ADC_VREF_1P2V);
		ADC_Vref = (unsigned char)ADC_VREF_1P2V;

		//set Vbat divider select,
		adc_set_vref_vbat_div(ADC_VBAT_DIVIDER_OFF);
		ADC_VBAT_Scale = VBAT_Scale_tab[ADC_VBAT_DIVIDER_OFF];
	}
	//set channel mode and channel
	adc_base_pin_init(pin);		//ADC GPIO Init
	for(i=0;i<11;i++)
	{
		if(pin == ADC_GPIO_tab[i])
		{
			gpio_num = i+1;
			break;
		}
	}
#if ADC_SINGLE_END
	adc_set_all_input_mode(ADC_MISC_CHN, SINGLE_ENDED_MODE);
	adc_set_all_single_end_ain(ADC_MISC_CHN, gpio_num);
#else
	adc_set_all_input_mode(ADC_MISC_CHN, DIFFERENTIAL_MODE);
	adc_set_all_differential_p_n_ain(ADC_MISC_CHN, gpio_num, GND);
#endif

	//set resolution for RNG
	adc_set_all_resolution(ADC_MISC_CHN, RES14);

	//Number of ADC clock cycles in sampling phase
	adc_set_all_tsample_cycle(ADC_MISC_CHN, SAMPLING_CYCLES_48);
	//set Analog input pre-scaling
	adc_set_all_ain_pre_scaler(ADC_PRESCALER_1);
	ADC_Pre_Scale = 1<<(unsigned char)ADC_PRESCALER_1F8;

	//set RNG mode
	adc_set_mode(ADC_NORMAL_MODE);

}


/**
 * @brief This function is used for ADC configuration of ADC supply voltage sampling.
 * @param[in]   GPIO_PinTypeDef pin
 * @return none
 */

void adc_vbat_init(GPIO_PinTypeDef pin)
{
	unsigned char i;
	unsigned char gpio_no=0;

	//set R_max_mc,R_max_c,R_max_s
	adc_set_misc_rns_capture_state_length(0xf0);		 //max_mc
	adc_set_all_set_state_length(0x0a);									//max_s

	//set total length for sampling state machine and channel
	adc_set_chn_en(ADC_MISC_CHN);
	adc_set_max_state_cnt(0x02);

	//set channel Vref
	adc_set_all_vref(ADC_MISC_CHN, ADC_VREF_1P2V);

	//set Vbat divider select,
	adc_set_vref_vbat_div(ADC_VBAT_DIVIDER_OFF);
	ADC_VBAT_Scale = VBAT_Scale_tab[ADC_VBAT_DIVIDER_OFF];

	//set channel mode and channel
	adc_vbat_pin_init(pin);
	for(i=0;i<10;i++)
	{
		if(pin == ADC_GPIO_tab[i])
		{
			gpio_no = i+1;
			break;
		}
	}

	adc_set_all_input_mode(ADC_MISC_CHN, DIFFERENTIAL_MODE);
	adc_set_all_differential_p_n_ain(ADC_MISC_CHN, gpio_no, GND);

	//set resolution for RNG
	adc_set_all_resolution(ADC_MISC_CHN, RES14);

	//Number of ADC clock cycles in sampling phase
	adc_set_all_tsample_cycle(ADC_MISC_CHN, SAMPLING_CYCLES_6);

	//set Analog input pre-scaling and
	adc_set_all_ain_pre_scaler(ADC_PRESCALER_1F8);

	//set RNG mode
	adc_set_mode(ADC_NORMAL_MODE);

}



/**
 * @brief This function serves to set the channel reference voltage.
 * @param[in]   enum variable of ADC input channel.
 * @param[in]   enum variable of ADC reference voltage.
 * @return none
 */
void adc_set_all_vref(ADC_ChTypeDef ch_n, ADC_RefVolTypeDef v_ref)
{
	if(ch_n & ADC_LEFT_CHN)
	{
		adc_set_left_vref(v_ref);
	}
	if(ch_n & ADC_RIGHT_CHN)
	{
		adc_set_right_vref(v_ref);
	}
	if(ch_n & ADC_MISC_CHN)
	{
		adc_set_misc_vref(v_ref);
	}


	if(v_ref == ADC_VREF_1P2V)
	{
		//Vref buffer bias current trimming: 		150%
		//Vref buffer bias current trimming: 		150%
		//Comparator preamp bias current trimming:  100%
		analog_write(anareg_fa, (analog_read(anareg_fa)&(0xC0)) | 0x3d );
//		analog_write(anareg_fa, 0x3D );

	}
	else
	{
		//Vref buffer bias current trimming: 		100%
		//Vref buffer bias current trimming: 		100%
		//Comparator preamp bias current trimming:  100%
		analog_write(anareg_fa, (analog_read(anareg_fa)&(0xC0)) | 0x15 );
	}
}

/**
 * @brief This function serves to set resolution.
 * @param[in]   enum variable of ADC input channel.
 * @param[in]   enum variable of ADC resolution.
 * @return none
 */
void adc_set_all_resolution(ADC_ChTypeDef ch_n, ADC_ResTypeDef v_res)
{
	if(ch_n & ADC_LEFT_CHN)
	{
		adc_set_left_resolution(v_res);
	}
	if(ch_n & ADC_RIGHT_CHN)
	{
		adc_set_right_resolution(v_res);
	}
	if(ch_n & ADC_MISC_CHN)
	{
		adc_set_misc_resolution(v_res);
	}
}

/**
 * @brief This function serves to set sample_cycle.
 * @param[in]   enum variable of ADC input channel.
 * @param[in]   enum variable of ADC Sampling cycles.
 * @return none
 */
void adc_set_all_tsample_cycle(ADC_ChTypeDef ch_n, ADC_SampCycTypeDef adcST)
{
	if(ch_n & ADC_LEFT_CHN)
	{
		adc_set_left_tsample_cycle(adcST);
	}
	if(ch_n & ADC_RIGHT_CHN)
	{
		adc_set_right_tsample_cycle(adcST);
	}
	if(ch_n & ADC_MISC_CHN)
	{
		adc_set_misc_tsample_cycle(adcST);
	}
}

/**
 * @brief This function serves to set input_mode.
 * @param[in]   enum variable of ADC input channel.
 * @param[in]   enum variable of ADC channel input mode.
 * @return none
 */
void adc_set_all_input_mode(ADC_ChTypeDef ch_n,  ADC_InputModeTypeDef m_input)
{
	if(ch_n & ADC_LEFT_CHN)
	{
		adc_set_left_input_mode(m_input);
	}
	if(ch_n & ADC_RIGHT_CHN)
	{
		adc_set_right_input_mode(m_input);
	}
	if(ch_n & ADC_MISC_CHN)
	{
		adc_set_misc_input_mode(m_input);
	}
}

/**
 * @brief This function serves to set input channel in single_ended_input_mode.
 * @param[in]   enum variable of ADC input channel.
 * @param[in]   enum variable of ADC analog positive input channel.
 * @return none
 */
void adc_set_all_single_end_ain(ADC_ChTypeDef ch_n, ADC_InputPchTypeDef InPCH)
{

	if(ch_n & ADC_LEFT_CHN)
	{
		adc_set_left_n_ain(GND);
		adc_set_left_p_ain(InPCH);
		adc_set_left_input_mode(SINGLE_ENDED_MODE);
	}
	if(ch_n & ADC_RIGHT_CHN)
	{
		adc_set_right_n_ain(GND);
		adc_set_right_p_ain(InPCH);
		adc_set_right_input_mode(SINGLE_ENDED_MODE);
	}
	if(ch_n & ADC_MISC_CHN)
	{
		adc_set_misc_n_ain(GND);
		adc_set_misc_p_ain(InPCH);
		adc_set_misc_input_mode(SINGLE_ENDED_MODE);
	}
}

/**
 * @brief This function serves to set input channel in differential_mode.
 * @param[in]   enum variable of ADC input channel.
 * @param[in]   enum variable of ADC analog positive input channel.
 * @param[in]   enum variable of ADC analog negative input channel.
 * @return none
 */
void adc_set_all_differential_p_n_ain(ADC_ChTypeDef ch_n, ADC_InputPchTypeDef InPCH,ADC_InputNchTypeDef InNCH)
{

	if(ch_n & ADC_LEFT_CHN)
	{
		adc_set_left_n_ain(InNCH);
		adc_set_left_p_ain(InPCH);
		adc_set_left_input_mode(DIFFERENTIAL_MODE);
	}
	if(ch_n & ADC_RIGHT_CHN)
	{
		adc_set_right_n_ain(InNCH);
		adc_set_right_p_ain(InPCH);
		adc_set_right_input_mode(DIFFERENTIAL_MODE);
	}
	if(ch_n & ADC_MISC_CHN)
	{
		adc_set_misc_n_ain(InNCH);
		adc_set_misc_p_ain(InPCH);
		adc_set_misc_input_mode(DIFFERENTIAL_MODE);
	}
}

/**
 * @brief This function serves to set state length.
 * @param[in]   Value of length of "capture" state for RNS & MISC channel.
 * @param[in]   Value of length of "capture" state for L & R channel.
 * @param[in]   Value of length of "capture" state for L & R & MISC channel.
 * @return none
 */
void adc_set_all_set_and_capture_state_length(unsigned short r_max_mc, unsigned short r_max_c,unsigned char r_max_s)
{
	unsigned char data[3]={0};
	if(r_max_mc&0x3ff)    //r_max_mc[9:0]serves to set length of state for RNS and Misc channel.
	{
		data[0] = (unsigned char)r_max_mc;
		data[2] = (unsigned char)(r_max_mc>>2)&0xc0;
	}
	if(r_max_c&0x3ff)     //r_max_c*9:0+ serves to set length of  state for left and right channel.
	{
		data[1] = (unsigned char)r_max_c;
		data[2] |= (unsigned char)(r_max_c>>4)&0x30;
	}
	if(r_max_s)     //r_max_s serves to set length of  state for left, right and Misc channel.
	{
		data[2] |= (unsigned char)(r_max_s&0x0f);
	}

	analog_write(anareg_ef, data[0]);
	analog_write(anareg_f0, data[1]);
	analog_write(anareg_f1, data[2]);
}


/**
 * @brief This function serves to set pre_scaling.
 * @param[in]   enum variable of ADC pre_scaling factor.
 * @return none
 */
void adc_set_all_ain_pre_scaler(ADC_PreScalingTypeDef v_scl)
{


	analog_write(anareg_fa, (analog_read(anareg_fa)&(~FLD_SEL_AIN_SCALE)) | (v_scl<<6) );

	//setting adc_sel_atb ,if stat is 0,clear adc_sel_atb,else set adc_sel_atb[0]if(stat)
	unsigned char tmp;
	if(v_scl)
	{
		//ana_F9<4> must be 1
		tmp = analog_read(0xF9);
		tmp = tmp|0x10;                    //open tmp = tmp|0x10;
		analog_write (0xF9, tmp);
	}
	else
	{
		//ana_F9 <4> <5> must be 0
		tmp = analog_read(0xF9);
		tmp = tmp&0xcf;
		analog_write (0xF9, tmp);
	}

}

#define ADC_SAMPLE_NUM		8 // the value should be x*8

/**
 * @brief This function serves to set adc sampling and get results.
 * @param[in]  none.
 * @return the result of sampling.
 */
volatile signed short dat_buf[ADC_SAMPLE_NUM];
unsigned int adc_set_sample_and_get_result(void)  ////_attribute_ram_code_
{
	unsigned short temp;
	//volatile signed int adc_dat_buf[ADC_SAMPLE_NUM];
	unsigned int adc_vol_mv;
	unsigned short adc_sample[ADC_SAMPLE_NUM] = {0};
	unsigned int adc_result;
	int i,j;

	adc_reset();
	aif_reset();
	adc_power_on(1); ///add by QW.

	unsigned int t0 = clock_time();

	for(i=0;i<ADC_SAMPLE_NUM;i++){   	//dfifo data clear
		dat_buf[i] = 0;
	}

	sleep_us(25);   //wait at least 2 sample cycle(f = 96K, T = 10.4us)
	//dfifo setting will lose in suspend/deep, so we need config it every time
	adc_aif_set_misc_buf((unsigned short *)dat_buf,ADC_SAMPLE_NUM);  //size: ADC_SAMPLE_NUM*4
	adc_aif_set_m_chn_en(1);
	adc_aif_set_use_raw_data_en();

//////////////// get adc sample data and sort these data ////////////////
	for(i=0;i<ADC_SAMPLE_NUM;i++){
		while((!dat_buf[i])&&(!clock_time_exceed(t0,20)));  //wait for new adc sample data,
															   //When the data is not zero or more than 1.5 sampling times (when the data is zero),The default data is already ready.
		t0 = clock_time();

		if(dat_buf[i] & BIT(13)){  //14 bit resolution, BIT(13) is sign bit, 1 means negative voltage in differential_mode
			adc_sample[i] = 0;
		}
		else{
			adc_sample[i] = ((unsigned short)dat_buf[i] & 0x1FFF);  //BIT(12..0) is valid adc result
		}

		//insert sort
		if(i){
			if(adc_sample[i] < adc_sample[i-1]){
				temp = adc_sample[i];
				adc_sample[i] = adc_sample[i-1];
				for(j=i-1;j>=0 && adc_sample[j] > temp;j--){
					adc_sample[j+1] = adc_sample[j];
				}
				adc_sample[j+1] = temp;
			}
		}
	}

	adc_power_on(0);   ///power down sar moudle
	adc_aif_set_m_chn_en(0);		//misc channel data dfifo disable

///// get average value from raw data(abandon some small and big data ), then filter with history data //////
	///if ADC_SAMPLE_NUM is 8, the calculate method below is right. but if the ADC_SAMPLE_NUM is other value???
	unsigned int adc_average = (adc_sample[2] + adc_sample[3] + adc_sample[4] + adc_sample[5])/4;

	adc_result = adc_average;
//	adcresult = adc_average;

	//////////////// adc sample data convert to voltage(mv) ////////////////
	//                         (1180mV Vref, 1/8 scaler)   (BIT<12~0> valid data)
	// 			 =  adc_result   *   1160     * 8        /        0x2000
	//           =  adc_result * 4680 >>12
	//           =  adc_result * 295 >>8

	//////////////// adc sample data convert to voltage(mv) ////////////////
	//                         (1200mV Vref, 1 scaler)   (BIT<13~0> valid data)
	// 			 =  adc_result   *   1210     * 1        /        0x2000
	//           =  adc_result * 1210 >>13
	//           =  adc_result * 605 >> 12

	if((adc_cal_value!=0xffff)&&(adc_cal_value != 0x0000))  //Already calibrated
	{
		adc_vol_mv  = adc_result*1200/1024;       //this used 1000mV calibrated value
	}
	else
	{
		//adc_vol_mv  = (adc_result * 295)>>8;
		adc_vol_mv  = (adc_result * 605)>>12;
	}

	return adc_vol_mv;
}

