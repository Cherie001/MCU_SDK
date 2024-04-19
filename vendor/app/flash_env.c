/********************************************************************************************************
 * @file     flash.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#include "flash_env.h"
#include "app.h"
#include "led.h"
#include "light.h"
#include <stack/ble/ble.h>
#include "timer.h"
#include "app_light.h"

#define	FLASH_MAGIC		0x55

void user_flash_write_page(u32 sector_addr,u32 cad_len,u8 *ptr)
{
	flash_lock(FLASH_TYPE_GD,0x18);
	if(sector_addr >= CUST_USER_VOLTAGE_ADDR && sector_addr < CUST_USER_DEFINE_ADDR_PRO)
	{
		flash_write_page(sector_addr, cad_len, ptr);
	}
	else
	{
		DEBUG_LOG("Flash write error = 0x%05X.\r\n",sector_addr);
	}
	flash_unlock(FLASH_TYPE_GD);
}

void user_flash_earse_sector(u32 sector_addr)
{
	flash_lock(FLASH_TYPE_GD,0x18);
	if(sector_addr >= CUST_USER_VOLTAGE_ADDR && sector_addr < CUST_USER_DEFINE_ADDR_PRO)
	{
		flash_erase_sector(sector_addr);
	}
	else
	{
		DEBUG_LOG("Flash erase error = 0x%05X.\r\n",sector_addr);
	}
	flash_unlock(FLASH_TYPE_GD);
}

void flash_power(void)
{
	u8 value_0x05 = analog_read(0x05);
	value_0x05 |= 0x20;
	analog_write(0x05, value_0x05);

}

void flash_env_sector_dump(u32 addr)
{
	u8 buf[FLASH_ENV_UNIT_BYTE]={0};
	DEBUG_LOG("Flash dump.............................................................................................................\r\n");

	for(u32 i =0; i < 4096/FLASH_ENV_UNIT_BYTE;i++)
	{
		flash_read_page(addr+i*FLASH_ENV_UNIT_BYTE, FLASH_ENV_UNIT_BYTE, buf);
		DEBUG_LOG("          [0x%04X]",addr+i*FLASH_ENV_UNIT_BYTE);
		DEBUG_LOG_BUF(buf,FLASH_ENV_UNIT_BYTE);
		DEBUG_LOG("\r\n");
	}
}

u32 flash_alloc(u32 sector_addr,u32 data_len)
{
	u8 data;
	u8 buf[FLASH_ENV_UNIT_BYTE]={0};

	flash_read_page(sector_addr,1,&data);
	if(data == 0xFF)
	{
		DEBUG_LOG("Flash not init,Flash find magic addr = 0x%04X.\r\n",sector_addr);
		return sector_addr;
	}

	for(u32 i =0; i < 4096/FLASH_ENV_UNIT_BYTE;i++)
	{
		flash_read_page(sector_addr+i*FLASH_ENV_UNIT_BYTE, FLASH_ENV_UNIT_BYTE, buf);
		for(u32 j =0; j < FLASH_ENV_UNIT_BYTE; j++)
		{
			if(buf[j] == FLASH_MAGIC)
			{
				if((4096-(i*FLASH_ENV_UNIT_BYTE+j+data_len+1)) < data_len)
				{
					DEBUG_LOG("Flash addr = 0x%04X capacity is not enough,Erase sector...\r\n",sector_addr);
					user_flash_earse_sector(sector_addr);
					return sector_addr;
				}
				else
				{
//					if((sector_addr >= CUST_USER_COUNT_ADDR) && (sector_addr <= CUST_USER_VERSION_ADDR))
//					{
//						memset(buf,0,1);
//					}
//					else
					{
						memset(buf,0,FLASH_ENV_UNIT_BYTE);
					}
					user_flash_write_page(sector_addr+i*FLASH_ENV_UNIT_BYTE+j, data_len + 1, buf);
					return sector_addr+i*FLASH_ENV_UNIT_BYTE+j+data_len+1;
				}
			}
		}
	}
	user_flash_earse_sector(sector_addr);
	return sector_addr;
}



u32 flash_ntc_alloc(u32 sector_addr,u32 data_len)
{
	u8 data;
	u8 buf[FLASH_ENV_UNIT_BYTE]={0};

	flash_read_page(sector_addr,1,&data);
	if(data == 0xFF)
	{
		DEBUG_LOG("Flash not init,Flash find magic addr = 0x%04X.\r\n",sector_addr);
		return sector_addr;
	}

	for(u32 i =0; i < 4096/FLASH_ENV_UNIT_BYTE;i++)
	{
		flash_read_page(sector_addr+i*FLASH_ENV_UNIT_BYTE, FLASH_ENV_UNIT_BYTE, buf);
		for(u32 j =0; j < FLASH_ENV_UNIT_BYTE; j++)
		{
			if(buf[j] == 0xFF)
			{
				if((4096-(i*FLASH_ENV_UNIT_BYTE+j+data_len+1)) < data_len)
				{
					DEBUG_LOG("Flash addr = 0x%04X capacity is not enough,Erase sector...\r\n",sector_addr);
					user_flash_earse_sector(sector_addr);
					return sector_addr;
				}
				else
				{
//					return sector_addr+i*FLASH_ENV_UNIT_BYTE+j+data_len+1;
					return sector_addr+i*FLASH_ENV_UNIT_BYTE+j;
				}
			}
		}
	}
	user_flash_earse_sector(sector_addr);
	return sector_addr;
}


u32 flash_find(u32 sector_addr)
{
	u8 data;
	u8 buf[FLASH_ENV_UNIT_BYTE]={0};

	flash_read_page(sector_addr,1,&data);
	if(data == 0xFF)
	{
		DEBUG_LOG("Flash not init,return.\r\n");
		return 0xFFFFFFFF;
	}
	for(u32 i =0; i < 4096/FLASH_ENV_UNIT_BYTE;i++)
	{
		flash_read_page(sector_addr+i*FLASH_ENV_UNIT_BYTE, FLASH_ENV_UNIT_BYTE, buf);
		for(u32 j =0; j < FLASH_ENV_UNIT_BYTE; j++)
		{
			if(buf[j] == FLASH_MAGIC)
			{
				return (sector_addr+i*FLASH_ENV_UNIT_BYTE+j);
			}
		}
	}
	return 0xFFFFFFFF;
}

u8 flash_get(u32 sector_addr,u8 *ptr,u32 len)
{
	u32 addr_data = flash_find(sector_addr);
	if(addr_data == 0xFFFFFFFF)
	{
		return false;
	}

	//DEBUG_LOG("Flash get addr = 0x%04X.\r\n",addr_data);
	flash_read_page(addr_data + 1 , len, ptr);
	return true;
}

u8 flash_set(u32 sector_addr,u8 *ptr,u32 len)
{
	if(256%(len+1) != 0)
	{
		DEBUG_LOG("Flash set len error.len = 0x%04X.\r\n",len);
		return false;
	}

	u8 magic = FLASH_MAGIC;
	u32 addr_data = flash_alloc(sector_addr,len);
	user_flash_write_page(addr_data, 1, &magic);
	user_flash_write_page(addr_data + 1, len, ptr);
	DEBUG_LOG("Flash set addr = 0x%04X.\r\n\n",addr_data+1);
	return true;
}

u8 flash_ntc(u32 sector_addr,u8 *ptr,u32 len)
{
	if(256%(len+1) != 0)
	{
		DEBUG_LOG("Flash set len error.len = 0x%04X.\r\n",len);
		return false;
	}

	u32 addr_data = flash_ntc_alloc(sector_addr,len);
	user_flash_write_page(addr_data, len, ptr);

	DEBUG_LOG("Flash set addr = 0x%04X.\r\n\n",addr_data);
	return true;
}


void flash_env_init(void)
{
	//flash_env_sector_dump(FLASH_ADV_COUNT_ADDR);
	//flash_env_sector_dump(FLASH_FAN_LEVEL_ADDR);
	//flash_env_sector_dump(FLASH_COOL_LEVEL_ADDR);
	//flash_env_sector_dump(FLASH_LIGHT_PARAM_ADDR);
	flash_env_sector_dump(CUST_USER_COUNT_ADDR);
}
