/********************************************************************************************************
 * @file     app_att.c 
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

#include <tl_common.h>
#include "app.h"
#include "ntc.h"
#include <stack/ble/ble.h>

//// public attribute values
const u16 my_primaryServiceUUID = GATT_UUID_PRIMARY_SERVICE;
const u16 my_characterUUID = GATT_UUID_CHARACTER;

//// GAP attribute values
const u16 my_gapServiceUUID = SERVICE_UUID_GENERIC_ACCESS;
const u16 my_devNameUUID = GATT_UUID_DEVICE_NAME;
u8  my_devName[26] = {
		 'B',
		 'l',
		 'a',
		 'c',
		 'k',
		 ' ',

		 'S',
		 'h',
		 'a',
		 'r',
		 'k',
		 ' ',


//		 'F',
//		 'u',
//		 'n',
//		 'C',
//		 'o',
//		 'o',
//		 'l',
//		 'e',
//		 'r',

		 'M',
		 'a',
		 'g',
		 'C',
		 'o',
		 'o',
		 'l',
		 'e',
		 'r',
		 ' ',

		 '3',
		 'P',
		 'r',
		 'o'
};
const u16 my_appearanceUIID = 0x2a01;
u16 my_appearance = GAP_APPEARE_UNKNOWN;
const u16 my_periConnParamUUID = 0x2a04;
typedef struct
{
  /** Minimum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMin;
  /** Maximum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMax;
  /** Number of LL latency connection events (0x0000 - 0x03e8) */
  u16 latency;
  /** Connection Timeout (0x000A - 0x0C80 * 10 ms) */
  u16 timeout;
} gap_periConnectParams_t;
gap_periConnectParams_t my_periConnParameters = {20, 40, 0, 1000};
static const u8 my_devNameCharVal[5] =
{
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GenericAccess_DeviceName_DP_H), U16_HI(GenericAccess_DeviceName_DP_H),
	U16_LO(GATT_UUID_DEVICE_NAME), U16_HI(GATT_UUID_DEVICE_NAME)
};
static const u8 my_appearanceCharVal[5] =
{
	CHAR_PROP_READ,
	U16_LO(GenericAccess_Appearance_DP_H), U16_HI(GenericAccess_Appearance_DP_H),
	U16_LO(GATT_UUID_APPEARANCE), U16_HI(GATT_UUID_APPEARANCE)
};
static const u8 my_periConnParamCharVal[5] =
{
	CHAR_PROP_READ,
	U16_LO(CONN_PARAM_DP_H), U16_HI(CONN_PARAM_DP_H),
	U16_LO(GATT_UUID_PERI_CONN_PARAM), U16_HI(GATT_UUID_PERI_CONN_PARAM)
};

//// GATT attribute values
const u16 my_gattServiceUUID = SERVICE_UUID_GENERIC_ATTRIBUTE;
const u16 serviceChangeUIID = GATT_UUID_SERVICE_CHANGE;
const u16 clientCharacterCfgUUID = GATT_UUID_CLIENT_CHAR_CFG;
u16 serviceChangeVal[2] = {0};
static u8 serviceChangeCCC[2]={0,0};
static const u8 my_serviceChangeCharVal[5] =
{
	CHAR_PROP_INDICATE,
	U16_LO(GenericAttribute_ServiceChanged_DP_H), U16_HI(GenericAttribute_ServiceChanged_DP_H),
	U16_LO(GATT_UUID_SERVICE_CHANGE), U16_HI(GATT_UUID_SERVICE_CHANGE)
};

//// device Information values
const u16 my_devServiceUUID = SERVICE_UUID_DEVICE_INFORMATION;
const u16 my_PnPUUID = CHARACTERISTIC_UUID_PNP_ID;
const u8	my_PnPtrs [] = {0x02, 0x8a, 0x24, 0x66, 0x82, 0x01, 0x00};
static const u8 my_PnCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(DeviceInformation_pnpID_DP_H), U16_HI(DeviceInformation_pnpID_DP_H),
	U16_LO(CHARACTERISTIC_UUID_PNP_ID), U16_HI(CHARACTERISTIC_UUID_PNP_ID)
};

//// OTA attribute values
u8	 my_OtaData 		= 0x00;
const u8  my_OtaName[] = {'O', 'T', 'A'};
const u8 my_OtaServiceUUID[16]		= TELINK_OTA_UUID_SERVICE;
const u8 my_OtaUUID[16]		= TELINK_SPP_DATA_OTA;

const u16 userdesc_UUID		= GATT_UUID_CHAR_USER_DESC;
static const u8 my_OtaCharVal[19] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(OTA_CMD_OUT_DP_H), U16_HI(OTA_CMD_OUT_DP_H),
	TELINK_SPP_DATA_OTA,
};

//// BLACKSHARK attribute values
u8	BsData 		= 0x00;
const u8 BsServiceUUID[16] = BS_UUID_SERVICE;
u16 BsRxUUID = BS_RX_UUID_CHAR;	// 0xA001
u16 BsTxUUID = BS_TX_UUID_CHAR;// 0xA002

u8 BsRxProp	= CHAR_PROP_WRITE_WITHOUT_RSP;
u8 BsTxProp	= CHAR_PROP_NOTIFY;

u8	BsRxData[ATT_MTU_SIZE - 3];
u8	BsTxData[ATT_MTU_SIZE - 3];
u8	BsTxClientDataCCC[2] 		= {0};
///////////////////////////////////////////////////////////////////////////////////////////////
bool BsTxSendData(u8 *p, int len)
{
	DEBUG_LOG("<---[TX] len = %d.Hex:",len);
	DEBUG_LOG_BUF(p,len);
	DEBUG_LOG("\r\n");
	ble_sts_t ret = bls_att_pushNotifyData(BS_CMD_TX_DP_H,p,len);
	if(BLE_SUCCESS == ret)
	{
		return true;
	}
	else
	{
		DEBUG_LOG("BlackShark Service tx fail.reason = %d.\r\n",ret);
		return false;
	}
}

void BsRxCallBack(rf_packet_att_write_t *p)
{
	u8 *ptr = &(p->value);
	u8 len = p->l2capLen - 3;
	DEBUG_LOG("--->[RX] len = %d,Hex:",len);
	DEBUG_LOG_BUF(ptr,len);
	DEBUG_LOG("\r\n");
	bs_vcom_data_receive(ble_vcom.vcomfd, ptr, len);

	if(1)
	{
		static u8 flag = 1;
		blc_att_setRxMtuSize(50);
		blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, 50);
	}
}

const attribute_t my_Attributes[] = {

	{ATT_END_H - 1, 0,0,0,0,0},	// total num of attribute

	// 0001 - 0007  gap
	{7,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_gapServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_devNameCharVal),(u8*)(&my_characterUUID), 		(u8*)(my_devNameCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_devName), (u8*)(&my_devNameUUID), (u8*)(my_devName), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_appearanceCharVal),(u8*)(&my_characterUUID), 		(u8*)(my_appearanceCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_appearance), (u8*)(&my_appearanceUIID), 	(u8*)(&my_appearance), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_periConnParamCharVal),(u8*)(&my_characterUUID), 		(u8*)(my_periConnParamCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_periConnParameters),(u8*)(&my_periConnParamUUID), 	(u8*)(&my_periConnParameters), 0},

	// 0008 - 000b gatt
	{4,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_gattServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_serviceChangeCharVal),(u8*)(&my_characterUUID), 		(u8*)(my_serviceChangeCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (serviceChangeVal), (u8*)(&serviceChangeUIID), 	(u8*)(&serviceChangeVal), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof (serviceChangeCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(serviceChangeCCC), 0},

	// 000c - 000e  device Information Service
	{3,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_devServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PnCharVal),(u8*)(&my_characterUUID), 		(u8*)(my_PnCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_PnPtrs),(u8*)(&my_PnPUUID), (u8*)(my_PnPtrs), 0},

	////////////////////////////////////// OTA /////////////////////////////////////////////////////
	// 002e - 0031
	{4,ATT_PERMISSIONS_READ, 2,16,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_OtaServiceUUID), 0},

	{0,ATT_PERMISSIONS_READ, 2,sizeof(my_OtaCharVal),(u8*)(&my_characterUUID), 		(u8*)(my_OtaCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,16,sizeof(my_OtaData),(u8*)(&my_OtaUUID),	(&my_OtaData), &otaWrite, &otaRead},			//value
	{0,ATT_PERMISSIONS_READ, 2,sizeof (my_OtaName),(u8*)(&userdesc_UUID), (u8*)(my_OtaName), 0},

	//////////////////////////////////////BLACKSHARK /////////////////////////////////////////////////////
	{6,ATT_PERMISSIONS_READ,2,16,(u8*)(&my_primaryServiceUUID), 	(u8*)(&BsServiceUUID), 0},

	{0,ATT_PERMISSIONS_READ,2,1,(u8*)(&my_characterUUID), 		(u8*)(&BsRxProp), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(BsRxData),(u8*)(&BsRxUUID), (u8*)(BsRxData), &BsRxCallBack},	//value

	{0,ATT_PERMISSIONS_READ,2,1,(u8*)(&my_characterUUID), 		(u8*)(&BsTxProp), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(BsTxData),(u8*)(&BsTxUUID), (u8*)(BsTxData), 0, 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,2,(u8*)&clientCharacterCfgUUID,(u8*)(&BsTxClientDataCCC)},
};

void my_att_init(void)
{
	u8 mac_public[6];
	u8 mac_random_static[6];
	blc_initMacAddress(CFG_ADR_MAC, mac_public, mac_random_static);
//	sprintf(&my_devName[20],"%02X%02X",mac_public[1],mac_public[0]);

	bls_att_setAttributeTable ((u8 *)my_Attributes);
}

