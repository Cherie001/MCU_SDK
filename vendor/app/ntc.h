/********************************************************************************************************
 * @file     ota.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#ifndef _NTC_H
#define _NTC_H

#include "app_config.h"
#include "tl_common.h"

#define 	_613_mV		613		// 15°
#define 	_512_mV		512		// 20°
#define 	_1300_mV	1300	// -5°

#define 	_570_mV		570		// 17°
#define 	_460_mV		460		// 22°
#define 	_1300_mV	1300	// -3°

#define 	_66_mV		66		// 15°
#define 	_54_mV		54		// 20°
#define 	_168_mV	    168	    // -8°

/*
** old ntc  =  NXFT15WF104
** new ntc  =  FHCWF22251A
*/
enum NTC_TYPE{
    NTC_NULL    = 0x0,
    NXFT15WF104 = 0x15,
    FHCWF22251A = 0x22,
};


extern u32 gNtcData;
//extern u16  g_temp_value;
extern u8  led_switch;
extern u8  g_temp_value;
extern u16 g_temp_c;

void ntc_init(void);
void ntc_process(void);
void high_tem_process();
s8  send_temp_data(void);
u32 choose_temperture_value(u32 Rt);
u32 calculate_thermistor_value(u32 Ntc_value);
u8 ntc_get_type(void);
u8 ntc_get_temper_offset(void);

#endif /* APP_H_ */
