/********************************************************************************************************
 * @file     cool.h
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#ifndef _COOL_H
#define _COOL_H

#include "app_config.h"
#include "tl_common.h"


#define		TEC_DISABLE()   gpio_write(GPIO_TEC_EN, 0)
#define		TEC_ENABLE()    gpio_write(GPIO_TEC_EN, 1)

#define		BR31_COOL_0				0
#define		BR31_COOL_1				0x64
#define		BR31_COOL_2				0x5E
#define		BR31_COOL_3				0x4C
#define		BR31_COOL_4				0x2A
#define		BR31_COOL_5				0x09	// 0x08

#define		BR33_COOL_1				0x59	//0x64
#define		BR33_COOL_2				0x31	// 59 - 31 = 40
#define		BR33_COOL_3				0x09	// 31 - 09 = 40

#define 	COOL_LEVEL_START_DEFAULT		0xFA	// AI level

extern u8  timer_aline;
extern u8  smart_level;

//extern u8  gfan_level;
//extern u8  gcool_level;
extern u8  tec_inbox_switch;

enum
{
	BS_COOL_DISABLE = 0,
    BS_COOL_LEVEL_START = 85,
    BS_COOL_LEVEL_MAX = 101,
};
void cool_level_set(u8 level);
void cool_level_save(u8 level);
u8 cool_level_get(void);
void cool_process(void);
void cool_init(void);

#endif /* APP_H_ */
