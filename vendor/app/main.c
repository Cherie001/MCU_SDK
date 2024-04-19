/********************************************************************************************************
 * @file     main.c
 *
 * @brief    for TLSR chips
 *
 * @author	 BLE Group
 * @date     May. 12, 2018
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *			 The information contained herein is confidential and proprietary property of Telink 
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in. 
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this 
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/
#include "app.h"
#include "log.h"
#include "adv.h"
#include "aw210xx.h"
#include "drivers.h"
#include <tl_common.h>
#include "stack/ble/ble.h"
#include "../common/user_config.h"
#include "../common/blt_fw_sign.h"

// 214564

int main(void)
{
	Telink_hd_init();

	Telink_Ble_init();

    irq_enable();

//    open_led();
    aw210xx_play();

    FP6606_IC_init();

	DEBUG_LOG("Now main loop running.\r\n");

	while (1)
	{
	#if (MODULE_WATCHDOG_ENABLE)
		wd_clear(); //clear watch dog
	#endif

	    main_loop();

//		blt_sdk_main_loop();
	}
}
