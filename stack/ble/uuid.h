/********************************************************************************************************
 * @file     uuid.h 
 *
 * @brief    for TLSR chips
 *
 * @author	 BLE Group
 * @date     Sep. 18, 2015
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

/** @addtogroup  GATT_Common_Module GATT UUID
 *  @{
 */


/** @addtogroup GATT_UUID GATT 16 bit UUID definition
 * @{
 */
#define GATT_UUID_PRIMARY_SERVICE        0x2800     //!< Primary Service
#define GATT_UUID_SECONDARY_SERVICE      0x2801     //!< Secondary Service
#define GATT_UUID_INCLUDE                0x2802     //!< Include
#define GATT_UUID_CHARACTER              0x2803     //!< Characteristic
#define GATT_UUID_CHAR_EXT_PROPS         0x2900     //!< Characteristic Extended Properties
#define GATT_UUID_CHAR_USER_DESC         0x2901     //!< Characteristic User Description
#define GATT_UUID_CLIENT_CHAR_CFG        0x2902     //!< Client Characteristic Configuration
#define GATT_UUID_SERVER_CHAR_CFG        0x2903     //!< Server Characteristic Configuration
#define GATT_UUID_CHAR_PRESENT_FORMAT    0x2904     //!< Characteristic Present Format
#define GATT_UUID_CHAR_AGG_FORMAT        0x2905     //!< Characteristic Aggregate Format
#define GATT_UUID_VALID_RANGE            0x2906     //!< Valid Range
#define GATT_UUID_EXT_REPORT_REF         0x2907     //!< External Report Reference
#define GATT_UUID_REPORT_REF             0x2908     //!< Report Reference

#define GATT_UUID_DEVICE_NAME            0x2a00     //!< Report Reference
#define GATT_UUID_APPEARANCE             0x2a01
#define GATT_UUID_PERI_CONN_PARAM 		 0x2a04
#define GATT_UUID_SERVICE_CHANGE		 0x2a05
#define	GATT_UUID_BATTERY_LEVEL			 0x2A19
#define	GATT_UUID_FIRMWARE_VER			 0x2a26		//!<Firmware Version
/** @} end of group GATT_UUID */


///////////////////////////
// Service UUID
///////////////////////////
/**
 *  @brief  Definition for Services UUID
 */
#define SERVICE_UUID_ALERT_NOTIFICATION                           0x1811
#define SERVICE_UUID_BATTERY                                      0x180F
#define SERVICE_UUID_BLOOD_PRESSURE                               0x1810
#define SERVICE_UUID_CURRENT_TIME                                 0x1805
#define SERVICE_UUID_CYCLING_POWER                                0x1818
#define SERVICE_UUID_CYCLING_SPEED_AND_CADENCE                    0x1816
#define SERVICE_UUID_DEVICE_INFORMATION                           0x180A
#define SERVICE_UUID_GENERIC_ACCESS                               0x1800
#define SERVICE_UUID_GENERIC_ATTRIBUTE                            0x1801
#define SERVICE_UUID_GLUCOSE                                      0x1808
#define SERVICE_UUID_HEALTH_THERMOMETER                           0x1809
#define SERVICE_UUID_HEART_RATE                                   0x180D
#define SERVICE_UUID_HUMAN_INTERFACE_DEVICE                       0x1812
#define SERVICE_UUID_IMMEDIATE_ALERT                              0x1802
#define SERVICE_UUID_LINK_LOSS                                    0x1803
#define SERVICE_UUID_LOCATION_AND_NAVIGATION                      0x1819
#define SERVICE_UUID_NEXT_DST_CHANGE                              0x1807
#define SERVICE_UUID_PHONE_ALERT_STATUS                           0x180E
#define SERVICE_UUID_REFERENCE_TIME_UPDATE                        0x1806
#define SERVICE_UUID_RUNNING_SPEED_AND_CADENCE                    0x1814
#define SERVICE_UUID_SCAN_PARAMETER                               0x1813
#define SERVICE_UUID_TX_POWER                                     0x1804
#define SERVICE_UUID_USER_DATA                                    0x181C
#define SERVICE_UUID_CONTINUOUS_GLUCOSE_MONITORING                0x181F
#define SERVICE_UUID_WEIGHT_SCALE                                 0x181D


////////////////////////////////////
// Telink Service
////////////////////////////////////
#define TELINK_OTA_UUID_SERVICE   			{0x12,0x19,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}		//!< TELINK_OTA service
#define TELINK_SPP_DATA_OTA 				{0x12,0x2B,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00} 		//!< TELINK_SPP data for ota

////////////////////////////////////
// Blackshark Service
////////////////////////////////////
#define BS_UUID_SERVICE   					{0x12,0xA2,0x4D,0x2E,0xFE,0x14,0x48,0x8E,0x93,0xD2,0x17,0x3C,0xA0,0xA0,0x00,0x00}//0000a0a0-3c17-d293-8e48-14fe2e4da212
#define	BS_RX_UUID_CHAR						0xA001
#define BS_TX_UUID_CHAR						0xA002

/** @addtogroup  GATT_UUID_Variables GATT UUID variables
 *  @{
 */
/**
 *  @brief  External variable for GATT UUID which can be used in service definition
 */
/** @} end of group GATT_UUID_Variables */

/** @} end of group GATT_Common_Module */


#define CHARACTERISTIC_UUID_BATTERY_LEVEL                    0x2A19
#define CHARACTERISTIC_UUID_ALERT_LEVEL						 0x2A06

#include "service/hids.h"
#include "service/device_information.h"
