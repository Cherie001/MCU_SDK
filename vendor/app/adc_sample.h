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
#ifndef _ADC_SAMPLE_H
#define _ADC_SAMPLE_H

extern u16 tec_sample_voltage;
extern u32 ntc_sample_voltage;
extern u32 vbus_sample_voltage;


void adc_sample_ntc(void);
void adc_sample_tec(void);
void adc_sample_vbus(void);

#endif
