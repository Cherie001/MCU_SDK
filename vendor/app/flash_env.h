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
#ifndef _FLASH_ENV_H
#define _FLASH_ENV_H

#include "app_config.h"
#include "tl_common.h"

#define FLASH_ENV_UNIT_BYTE			64

#define	 FLASH_ADV_COUNT_ADDR	(CUST_USER_DEFINE_ADDR)
#define	 FLASH_FAN_LEVEL_ADDR	(FLASH_ADV_COUNT_ADDR+0x1000)
#define	 FLASH_COOL_LEVEL_ADDR	(FLASH_FAN_LEVEL_ADDR+0x1000)
#define	 FLASH_LIGHT_PARAM_ADDR	(FLASH_COOL_LEVEL_ADDR+0x1000)
#define	 FLASH_NTC_TYPE_ADDR    (FLASH_LIGHT_PARAM_ADDR+0x1000)

void flash_power(void);
void flash_env_init(void);
u8 flash_get(u32 sector_addr,u8 *ptr,u32 len);
u8 flash_set(u32 sector_addr,u8 *ptr,u32 len);
u8 flash_ntc(u32 sector_addr,u8 *ptr,u32 len);


#endif /* APP_H_ */
