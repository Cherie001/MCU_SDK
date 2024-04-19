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
#ifndef _OTA_H
#define _OTA_H

#include "app_config.h"

#if(BLE_REMOTE_OTA_ENABLE)
	void ota_init(void);
#endif

#endif /* APP_H_ */
