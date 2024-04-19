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
#ifndef _FAN_H
#define _FAN_H

#include "app_config.h"
#include "tl_common.h"

#define		BACKCLIP_VERSION_FLAG			0xAA

#define		FAC_TEST						0x01

#define		BR31_FAN_0				0x64
#define		BR31_FAN_1				0x64
#define		BR31_FAN_2				0x54
#define		BR31_FAN_3				0x50
#define		BR31_FAN_4				0x4B
#define		BR31_FAN_5				0x2C	// 0x40

#define		BR33_FAN_1				0x4B	// 0x5B	//
#define		BR33_FAN_2				0x3C	// 4B - 3C = 15
#define		BR33_FAN_3				0x2C	// 3C - 2C = 16

#define FAN_DISABLE()  gpio_write(GPIO_FAN_EN, 0)
#define	FAN_ENABLE()   gpio_write(GPIO_FAN_EN, 1)

#define FAN_CAL_TIME_OUT_S			3
#define FAN_LEVEL_START_DEFAULT		0xFA	// AI level

extern u8 smart_level;
extern u32 gFanSpeed;
extern u8  ctl_level;
extern u8  voltage_level;

extern u8  v_fan;
extern u8  v_tec;
extern u8  v_reference_fan;
extern u8  v_reference_tec;


u8 low_level_voltage_value;
u8 high_level_voltage_value;
u8 smart_level_mid_voltage_value;
u8 smart_level_low_voltage_value;
u8 smart_level_high_voltage_value;

typedef enum
{
    BS_FAN_DISABLE = 0,
    BS_FAN_LEVEL_MAX = 101,
}bs_fan_level_t;

bool check_fan_ai(void);
u8 fan_level_get(void);
void fan_level_set(u8 level);
void fan_level_save(u8 level);
void fan_select_circuit(u8 fan_data);
void fan_process(void);
void backclip_voltage_init(void);
void fan_cal_interval(void);
void fan_input_triger(void);
void fan_init(void);
void voltage_level_judge(void);
bool AI_level_process(void);

#endif /* APP_H_ */
