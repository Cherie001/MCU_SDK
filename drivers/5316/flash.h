/********************************************************************************************************
 * @file     flash.h
 *
 * @brief    This is the header file for TLSR8232
 *
 * @author	 junwei.lu
 * @date     May 8, 2018
 *
 * @par      Copyright (c) 2018, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *           The information contained herein is confidential property of Telink
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *           Co., Ltd. and the licensee or the terms described here-in. This heading
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/

#pragma once

#include "compiler.h"
#include "../../common/config/user_config.h"

#if (FLASH_SIZE_OPTION == FLASH_SIZE_OPTION_512K)
	#ifndef		CFG_ADR_MAC
	#define		CFG_ADR_MAC						0x76000
	#endif

	#ifndef		CUST_CAP_INFO_ADDR
	#define		CUST_CAP_INFO_ADDR				0x77000
	#endif

	#ifndef		CUST_TP_INFO_ADDR
	#define		CUST_TP_INFO_ADDR				0x77040
	#endif

	#ifndef		CUST_FIRMWARE_SIGNKEY_ADDR
	#define		CUST_FIRMWARE_SIGNKEY_ADDR		0x77180
	#endif

	#ifndef		CUST_USER_VOLTAGE_ADDR
	#define		CUST_USER_VOLTAGE_ADDR			0x44000
	#endif

	#ifndef		CUST_USER_COUNT_ADDR
	#define		CUST_USER_COUNT_ADDR			0x45000
	#endif

	#ifndef		CUST_USER_PD_VOLTAGE_ADDR
	#define		CUST_USER_PD_VOLTAGE_ADDR		0x46000
	#endif

	#ifndef		CUST_USER_VERSION_ADDR
	#define		CUST_USER_VERSION_ADDR			0x6F000
	#endif

	#ifndef		CUST_USER_DEFINE_ADDR
	#define		CUST_USER_DEFINE_ADDR			0x70000
	#endif

	#ifndef		CUST_USER_DEFINE_ADDR_PRO
	#define		CUST_USER_DEFINE_ADDR_PRO		0x75000
	#endif

#else

	#ifndef		CUST_USER_DEFINE_ADDR
	#define		CUST_USER_DEFINE_ADDR			0x74000
	#endif

	#ifndef		CUST_USER_LEDCOMMAND_ADDR
	#define		CUST_USER_LEDCOMMAND_ADDR		0x75000
	#endif

	#ifndef		CFG_ADR_MAC
	#define		CFG_ADR_MAC						0x1F000
	#endif

	#ifndef		CUST_CAP_INFO_ADDR
	#define		CUST_CAP_INFO_ADDR				0x1E000
	#endif

	#ifndef		CUST_TP_INFO_ADDR
	#define		CUST_TP_INFO_ADDR				0x1E040
	#endif

	#ifndef		CUST_FIRMWARE_SIGNKEY_ADDR
	#define		CUST_FIRMWARE_SIGNKEY_ADDR		0x1E180
	#endif

#endif


enum{
	FLASH_WRITE_STATUS_CMD	=	0x01,
	FLASH_WRITE_CMD			=	0x02,
	FLASH_READ_CMD			=	0x03,
	FLASH_WRITE_DISABLE_CMD = 	0x04,
	FLASH_READ_STATUS_CMD	=	0x05,
	FLASH_WRITE_ENABLE_CMD 	= 	0x06,
	FLASH_SECT_ERASE_CMD	=	0x20,
	FLASH_READ_UID_CMD		=	0x4B,	//Flash Type = GD/PUYA
	FLASH_32KBLK_ERASE_CMD	=	0x52,
	FLASH_CHIP_ERASE_CMD	=	0x60,   //or 0xc7
	FLASH_PAGE_ERASE_CMD	=	0x81,   //caution: only P25Q40L support this function
	FLASH_64KBLK_ERASE_CMD	=	0xD8,
	FLASH_POWER_DOWN		=	0xB9,
	FLASH_POWER_DOWN_RELEASE=	0xAB,
	FLASH_GET_JEDEC_ID		=	0x9F,
};

typedef enum{
	FLASH_TYPE_GD = 0 ,
	FLASH_TYPE_XTX,
	FLASH_TYPE_PUYA
}Flash_TypeDef;

/**
 * @brief This function serves to erase a sector.
 * @param[in]   addr the start address of the sector needs to erase.
 * @return none
 */
_attribute_ram_code_ void flash_erase_sector(unsigned long addr);
/**
 * @brief This function writes the buffer's content to a page.
 * @param[in]   addr the start address of the page
 * @param[in]   len the length(in byte) of content needs to write into the page
 * @param[in]   buf the start address of the content needs to write into
 * @return none
 */
_attribute_ram_code_ void flash_write_page(unsigned long addr, unsigned long len, unsigned char *buf);
/**
 * @brief This function reads the content from a page to the buf.
 * @param[in]   addr the start address of the page
 * @param[in]   len the length(in byte) of content needs to read out from the page
 * @param[out]  buf the start address of the buffer
 * @return none
 */
_attribute_ram_code_ void flash_read_page(unsigned long addr, unsigned long len, unsigned char *buf);


_attribute_ram_code_ void flash_read_mid(unsigned char *buf);
_attribute_ram_code_ void flash_read_uid(unsigned char idcmd,unsigned char *buf);
_attribute_ram_code_ int flash_read_mid_uid_with_check( unsigned int *flash_mid ,unsigned char *flash_uid);

_attribute_ram_code_ void flash_unlock(Flash_TypeDef type);
_attribute_ram_code_ void flash_lock(Flash_TypeDef type , unsigned short data);

#if 0
/**
 * @brief This function reads the status of flash.
 * @param[in]  none
 * @return none
 */
_attribute_ram_code_ unsigned char flash_read_status(void);
/**
 * @brief This function write the status of flash.
 * @param[in]  the value of status
 * @return status
 */
_attribute_ram_code_ unsigned char flash_write_status(unsigned char data);
/**
 * @brief This function serves to erase a block(32k).
 * @param[in]   addr the start address of the block needs to erase.
 * @return none
 */
_attribute_ram_code_ void flash_erase_32kblock(unsigned int addr);

/**
 * @brief This function serves to erase a block(64k).
 * @param[in]   addr the start address of the block needs to erase.
 * @return none
 */
_attribute_ram_code_ void flash_erase_64kblock(unsigned int addr);

/***********************************
 * @brief  	Deep Power Down mode to put the device in the lowest consumption mode
 * 			it can be used as an extra software protection mechanism,while the device
 * 			is not in active use,since in the mode,  all write,Program and Erase commands
 * 			are ignored,except the Release from Deep Power-Down and Read Device ID(RDI)
 * 			command.This release the device from this mode
 * @param[in] none
 * @return none.
 */
_attribute_ram_code_ void flash_deep_powerdown(void);

/***********************************
 * @brief		The Release from Power-Down or High Performance Mode/Device ID command is a
 * 				Multi-purpose command.it can be used to release the device from the power-Down
 * 				State or High Performance Mode or obtain the devices electronic identification
 * 				(ID)number.Release from Power-Down will take the time duration of tRES1 before
 * 				the device will resume normal operation and other command are accepted.The CS#
 * 				pin must remain high during the tRES1(8us) time duration.
 * @param[in] none
 * @return none.
 */
_attribute_ram_code_ void flash_release_deep_powerdown(void);
/**
 * @brief This function serves to erase a page(256 bytes).
 * @param[in]   addr the start address of the page needs to erase.
 * @return none
 */
_attribute_ram_code_ void flash_erase_page(unsigned int addr);

/**
 * @brief This function serves to erase a page(256 bytes).
 * @param[in]   addr the start address of the page needs to erase.
 * @return none
 */
_attribute_ram_code_ void flash_erase_chip(void);
/***********************************
 * @brief  read flash ID
 * @param[in] the buffer address to store the flash ID.
 * @return none.
 */
_attribute_ram_code_ unsigned int flash_get_jedec_id();

#endif
