/********************************************************************************************************
 * @file     app_config.h
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
#pragma once

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif


#define	SOFTWARE_VERSION_BUILD      "BR33230412V2"

/*Factory version switch for new ntc devices*/
#define NTC_MODULE_FAC_VERSION		0

/* Function Select -----------------------------------------------------------*/
#define BLE_REMOTE_SECURITY_ENABLE      1
#define BLE_REMOTE_OTA_ENABLE			1

/***select flash size***/
#define FLASH_SIZE_OPTION_128K          0
#define FLASH_SIZE_OPTION_512K          1

#define FLASH_SIZE_OPTION               FLASH_SIZE_OPTION_512K

/***firmware check***/
#define FIRMWARES_SIGNATURE_ENABLE      0


/* Matrix Key Configuration --------------------------------------------------*/

#define	GPIO_FP_INT				GPIO_PA0	// FP6606_INT
#define GPIO_NTC     			GPIO_PB6	// NTC_THERM
#define GPIO_NTC_BUCK			GPIO_PB0	// NTC_BUCK
#define GPIO_NTC_TYPE			GPIO_PB1	// NTC_TYPE
#define GPIO_TEC1     			GPIO_PB7	// ADC_TEC1
#define GPIO_FAN_FG				GPIO_PA5	// FAN_FG
#define GPIO_FAN_PWM			GPIO_PB2	// VDD_FAN_PWM
#define GPIO_TEC_PWM			GPIO_PA7	// VDD_TEC_PWM
#define	GPIO_FAN_EN      	    GPIO_PC6	// FAN_TEC_VDD_EN
#define	GPIO_TEC_EN      	    GPIO_PB3	// FAN_TEC_VDD_EN
#define	GPIO_KEY_POWER          GPIO_PC1	// KEY_PWR

//#define GPIO_VBUS     			GPIO_PB3
#define	GPIO_LIGHT_EN           GPIO_PA6	// LED_SHDN


/* System clock initialization -----------------------------------------------*/
#define CLOCK_SYS_CLOCK_HZ      48000000
enum{
	CLOCK_SYS_CLOCK_1S  = CLOCK_SYS_CLOCK_HZ,
	CLOCK_SYS_CLOCK_1MS = (CLOCK_SYS_CLOCK_1S / 1000),
	CLOCK_SYS_CLOCK_1US = (CLOCK_SYS_CLOCK_1S / 1000000),
};


/* WatchDog ------------------------------------------------------------------*/
#define MODULE_WATCHDOG_ENABLE	1
#define WATCHDOG_INIT_TIMEOUT	3000  //Unit:ms

/* ATT Handle define ---------------------------------------------------------*/
typedef enum
{
	ATT_H_START = 0,


	//// Gap ////
	/**********************************************************************************************/
	GenericAccess_PS_H, 					//UUID: 2800, 	VALUE: uuid 1800
	GenericAccess_DeviceName_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	GenericAccess_DeviceName_DP_H,			//UUID: 2A00,   VALUE: device name
	GenericAccess_Appearance_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	GenericAccess_Appearance_DP_H,			//UUID: 2A01,	VALUE: appearance
	CONN_PARAM_CD_H,						//UUID: 2803, 	VALUE:  			Prop: Read
	CONN_PARAM_DP_H,						//UUID: 2A04,   VALUE: connParameter


	//// gatt ////
	/**********************************************************************************************/
	GenericAttribute_PS_H,					//UUID: 2800, 	VALUE: uuid 1801
	GenericAttribute_ServiceChanged_CD_H,	//UUID: 2803, 	VALUE:  			Prop: Indicate
	GenericAttribute_ServiceChanged_DP_H,   //UUID:	2A05,	VALUE: service change
	GenericAttribute_ServiceChanged_CCB_H,	//UUID: 2902,	VALUE: serviceChangeCCC

	//// device information ////
	/**********************************************************************************************/
	DeviceInformation_PS_H,					//UUID: 2800, 	VALUE: uuid 180A
	DeviceInformation_pnpID_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	DeviceInformation_pnpID_DP_H,			//UUID: 2A50,	VALUE: PnPtrs


	//// Ota ////
	/**********************************************************************************************/
	OTA_PS_H, 								//UUID: 2800, 	VALUE: telink ota service uuid
	OTA_CMD_OUT_CD_H,						//UUID: 2803, 	VALUE:  			Prop: read | write_without_rsp
	OTA_CMD_OUT_DP_H,						//UUID: telink ota uuid,  VALUE: otaData
	OTA_CMD_OUT_DESC_H,						//UUID: 2901, 	VALUE: otaName

	/**********************************************************************************************/
	BS_PS_H,
	BS_CMD_RX_CD_H,
	BS_CMD_RX_DP_H,
	BS_CMD_TX_CD_H,
	BS_CMD_TX_DP_H,
	BS_CMD_TX_CCB_H,

	ATT_END_H,

}ATT_HANDLE;



/* Simulate uart debug Interface ---------------------------------------------*/
#define	SIMULATE_UART_EN		1
#define DEBUG_TX_PIN            GPIO_PB4
#define DEBUG_RX_PIN            GPIO_PB5

#if SIMULATE_UART_EN
#define	DEBUG_LOG				printf
#define DEBUG_LOG_BUF           array_printf
#else
#define	DEBUG_LOG
#define DEBUG_LOG_BUF
#endif

/* Debug Interface -----------------------------------------------------------*/
#define  DEBUG_GPIO_ENABLE					0

#if(DEBUG_GPIO_ENABLE)
		#define PB2_FUNC				AS_GPIO //debug gpio chn0 : PB2
		#define PB3_FUNC				AS_GPIO //debug gpio chn1 : PB3
		#define PB4_FUNC				AS_GPIO //debug gpio chn2 : PB4
		#define PB5_FUNC				AS_GPIO //debug gpio chn3 : PB5
		#define PA6_FUNC                AS_GPIO //debug gpio chn4 : PA6

		#define PB2_INPUT_ENABLE					0
		#define PB3_INPUT_ENABLE					0
		#define PB4_INPUT_ENABLE					0
		#define PB5_INPUT_ENABLE					0
		#define PA6_INPUT_ENABLE					0

		#define PB2_OUTPUT_ENABLE					1
		#define PB3_OUTPUT_ENABLE					1
		#define PB4_OUTPUT_ENABLE					1
		#define PB5_OUTPUT_ENABLE					1
		#define PA6_OUTPUT_ENABLE					1


		#define DBG_CHN0_LOW		( *(unsigned char *)0x80058b &= (~(1<<2)) )
		#define DBG_CHN0_HIGH		( *(unsigned char *)0x80058b |= (1<<2) )
		#define DBG_CHN0_TOGGLE		( *(unsigned char *)0x80058b ^= (1<<2) )

		#define DBG_CHN1_LOW		( *(unsigned char *)0x80058b &= (~(1<<3)) )
		#define DBG_CHN1_HIGH		( *(unsigned char *)0x80058b |= (1<<3) )
		#define DBG_CHN1_TOGGLE		( *(unsigned char *)0x80058b ^= (1<<3) )

		#define DBG_CHN2_LOW		( *(unsigned char *)0x80058b &= (~(1<<4)) )
		#define DBG_CHN2_HIGH		( *(unsigned char *)0x80058b |= (1<<4) )
		#define DBG_CHN2_TOGGLE		( *(unsigned char *)0x80058b ^= (1<<4) )

		#define DBG_CHN3_LOW		( *(unsigned char *)0x80058b &= (~(1<<5)) )
		#define DBG_CHN3_HIGH		( *(unsigned char *)0x80058b |= (1<<5) )
		#define DBG_CHN3_TOGGLE		( *(unsigned char *)0x80058b ^= (1<<5) )

		#define DBG_CHN4_LOW		( *(unsigned char *)0x800583 &= (~(1<<6)) )
		#define DBG_CHN4_HIGH		( *(unsigned char *)0x800583 |= (1<<6) )
		#define DBG_CHN4_TOGGLE		( *(unsigned char *)0x800583 ^= (1<<6) )
#else
		#define DBG_CHN0_LOW
		#define DBG_CHN0_HIGH
		#define DBG_CHN0_TOGGLE
		#define DBG_CHN1_LOW
		#define DBG_CHN1_HIGH
		#define DBG_CHN1_TOGGLE
		#define DBG_CHN2_LOW
		#define DBG_CHN2_HIGH
		#define DBG_CHN2_TOGGLE
		#define DBG_CHN3_LOW
		#define DBG_CHN3_HIGH
		#define DBG_CHN3_TOGGLE
		#define DBG_CHN4_LOW
		#define DBG_CHN4_HIGH
		#define DBG_CHN4_TOGGLE
		#define DBG_CHN5_LOW
		#define DBG_CHN5_HIGH
		#define DBG_CHN5_TOGGLE

#endif  //end of DEBUG_GPIO_ENABLE


/////////////////// set default   ////////////////

#include "../common/default_config.h"

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
