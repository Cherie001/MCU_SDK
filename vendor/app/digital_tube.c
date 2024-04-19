/********************************************************************************************************
 * @file    digital_tube.c
 *
 * @brief    for TLSR chips
 *
 * @author
 * @date     D, 2020
 *
 * @par
 *
 *******************************************************************************************************/
#include "gap.h"
#include "app.h"
#include "light.h"
#include <tl_common.h>
#include "digital_tube.h"
#include "stack/ble/ble.h"
#include "../common/user_config.h"
#include "../common/blt_fw_sign.h"


#define UNIT	256
#define SPI_CS_PIN				GPIO_PC2//SPI CS pin

static u32 gStartTick = 0;

u8 tube_test_flag = 0;
u8 spi_display_data[3] = {0x40, 0xDA, 0xBE};

// temperature value:   0     1     2     3     4     5     6     7     8     9 (¡æ)
s8 spi_temp_num[10] = {0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xF6};

/**********************************************************************************/
void spi_init(void)
{
	spi_master_init((u8)(CLOCK_SYS_CLOCK_HZ/(2*500000)-1),SPI_MODE0);//div_clock. spi_clk = sys_clk/((div_clk+1)*2),mode select

	spi_master_set_pin(SPI_GPIO_GROUP_C2C3C4C5);//master mode: spi pin set

	spi_write_buff(0x00, 1, 0, 0, SPI_CS_PIN);		// 0x00
	spi_write_buff(0x22, 1, 0, 0, SPI_CS_PIN);		// 0x44 -> 0x22

	DEBUG_LOG("Hardware spi init ok.\r\n");
}
/**********************************************************************************/

/**********************************************************************************/
void temperture_divide(u8 value)
{
	u8 temperature = value;

	static u8 temperature_upto_app[4] = {0x84, 0x06, 0x01, 0x00};

	if(temperature >= 0 && temperature < 50)
	{
		spi_display_data[0] = 0x40;
		spi_display_data[1] = spi_temp_num[temperature/10];
		spi_display_data[2] = spi_temp_num[temperature%10];
	}
	else if(temperature >= 247 && temperature <= 255)	// -1 ~ -9¡æ
	{
		spi_display_data[0] = 0x40;
		spi_display_data[1] = 0x02;
		spi_display_data[2] = spi_temp_num[(UNIT - temperature)%10];
	}
	else if(temperature <= 246 && temperature >= 238)	// -10 ~ -18¡æ
	{
		spi_display_data[0] = 0x42;
		spi_display_data[1] = spi_temp_num[(UNIT - temperature)/10];
		spi_display_data[2] = spi_temp_num[(UNIT - temperature)%10];
	}
	else
	{
		spi_display_data[0] = 0;
		spi_display_data[1] = 0;
		spi_display_data[2] = 0;
	}
	if(tube_test_flag)
	{
		spi_display_alldisplay();
	}
	else
	{
		spi_display_temp();
		DEBUG_LOG("temperature value = %d.\r\n",temperature);

	}
	if(clock_time_exceed(gStartTick,1000*1000))		// 2s
	{
		temperature_upto_app[3] = temperature;
		if(bond_flag)
		{
			BsTxSendData(temperature_upto_app, sizeof(temperature_upto_app));
		}
		gStartTick += 1000*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}
}

void spi_display_temp(void)
{
	spi_write_buff(0x03, 1, spi_display_data[0], 1, SPI_CS_PIN);
	spi_write_buff(0x43, 1, spi_display_data[1], 1, SPI_CS_PIN);
	spi_write_buff(0x23, 1, spi_display_data[2], 1, SPI_CS_PIN);

	spi_write_buff(0xF1, 1, 0, 0, SPI_CS_PIN);
}

void spi_display_alldisplay(void)
{
	spi_write_buff(0x03, 1, 0xFF, 1, SPI_CS_PIN);
	spi_write_buff(0x43, 1, 0xFF, 1, SPI_CS_PIN);
	spi_write_buff(0x23, 1, 0xFF, 1, SPI_CS_PIN);

	spi_write_buff(0xF1, 1, 0, 0, SPI_CS_PIN);
}

void spi_display_allclose(void)
{
	spi_write_buff(0x03, 1, 0x00, 1, SPI_CS_PIN);
	spi_write_buff(0x43, 1, 0x00, 1, SPI_CS_PIN);
	spi_write_buff(0x23, 1, 0x00, 1, SPI_CS_PIN);

	spi_write_buff(0x01, 1, 0, 0, SPI_CS_PIN);
}
