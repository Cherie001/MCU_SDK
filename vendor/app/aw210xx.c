/*
 * aw210xx.c
 *
 * Version: V0.3.0
 *
 * Copyright (c) 2021 AWINIC Technology CO., LTD
 *
 * Author:  <support@awinic.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include "log.h"
#include "light.h"
#include "aw210xx.h"
#include "app_config.h"
#include "stack/ble/ble.h"
#include "drivers/5316/clock.h"

#define AW210XX_DRIVER_VERSION		"V0.3.0"

uint8_t light_color_reg_arry[12] = {};
uint8_t light_bright_reg_arry[12] = {};
uint8_t light_order[12] = {1, 4, 3, 2, 8, 6, 5, 7, 9, 12, 10, 11};
/*
 * 1 4 3 2 8 6 5 7 9 12 10 11
 * */

static void aw210xx_soft_rst(void)
{
	iic_send_byte(REG_RESET, 1, 0x00);
	/* delay 2ms at least */
	WaitMs(5);
};

static void aw210xx_set_global_current(uint8_t current)
{
//	aw_i2c_write_one_byte(REG_GCCR, current);
	  iic_send_byte(REG_GCCR, 1, current);
}
static void aw210xx_set_brightness(uint8_t index, uint8_t brightness)
{
//	aw_i2c_write_one_byte(index, brightness);
	  iic_send_byte(index, 1, brightness);
}
static void aw210xx_set_color(uint8_t index, uint8_t color)
{
//	aw_i2c_write_one_byte(index, color);
	  iic_send_byte(index, 1, color);
}
static void aw210xx_update_brightness_to_display(void)
{
//	aw_i2c_write_one_byte(REG_UPDATE, 0x00);
	  iic_send_byte(REG_UPDATE, 1, 0x00);
}

void aw210xx_reg_init(void)
{
	for(uint8_t i = 0; i < 12; i++) {
		light_color_reg_arry[i] = REG_COL0 + (light_order[i] - 1) * 3;
		light_bright_reg_arry[i] = REG_BR0 + (light_order[i] - 1) * 3;
	}
}

int aw210xx_init(void)
{
	uint8_t val = 0;

	aw210xx_reg_init();

	DEBUG_LOG("%s:enter\n", __func__);
	gpio_set_func(GPIO_LIGHT_EN ,AS_GPIO);
	gpio_set_output_en(GPIO_LIGHT_EN, 1);
	gpio_set_input_en(GPIO_LIGHT_EN ,0);

	gpio_write(GPIO_LIGHT_EN, 0);
	WaitMs(5);
	gpio_write(GPIO_LIGHT_EN, 1);

	i2c_set_pin(I2C_GPIO_GROUP_M_A3A4);
	i2c_master_init(0x68, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*400000)) ); // 400KHz
//	aw210xx_chip_hwen();
	aw210xx_soft_rst();

	/* 1. enable chip and enable auto power-saving */
//	aw_i2c_write_one_byte(REG_GCR, 0x01); /* 0x00 0x01 */
	iic_send_byte(REG_GCR, 1, 0x01);

	/* 2. check id */
//	aw_i2c_read_one_byte(REG_RESET, &val); /* 0x7f 0x18 */
	val = iic_read_byte(REG_RESET, 1);
	if (val != AW210XX_CHIPID)
		DEBUG_LOG("%s:read chip id failed. val = %#x\n", __func__, val);

	/* 4. config global current */
	aw210xx_set_global_current(0x3F);/* 0x6e 0xff */

	return 0;
}

void aw210xx_disable(void)
{
	gpio_write(GPIO_LIGHT_EN, 0);
}

void aw210xx_all_on(uint8_t r, uint8_t g, uint8_t b)
{
	int i = 0;

	/* 1.set BR for brightness */
	for (i = 0; i < AW210XX_LED_NUM; i++)
			aw210xx_set_brightness(REG_BR0+i, 0xFF);
	/* 2.set COL for color */
	for (i = 0; i < 12; i++) {
		aw210xx_set_color(light_color_reg_arry[i], r);
		aw210xx_set_color(light_color_reg_arry[i]+1, g);
		aw210xx_set_color(light_color_reg_arry[i]+2, b);
		aw210xx_update_brightness_to_display();
	}

	aw210xx_update_brightness_to_display();
}

void aw210xx_all_breath(uint8_t r, uint8_t g, uint8_t b, uint8_t rise_on, uint8_t fall_off)
{
	/* 1. set color */
	aw210xx_set_color(REG_GCOLR, r);
	aw210xx_set_color(REG_GCOLG, g);
	aw210xx_set_color(REG_GCOLB, b);

	/* 2. set brigthness */
	aw210xx_set_brightness(REG_FADEH, 0xFF);
	aw210xx_set_brightness(REG_FADEL, 0x00);

	/* 3. group	GEn=1/PATEN=1/GCOLDIS=0 */
//	aw_i2c_write_one_byte(REG_GCFG0, 0xff);
//	aw_i2c_write_one_byte(REG_GCFG1, 0x0f);
	iic_send_byte(REG_GCFG0, 1, 0xff);
	iic_send_byte(REG_GCFG1, 1, 0x0f);

	/* 4. set pattern times T1/T2/T3/T4 */
//	aw_i2c_write_one_byte(REG_PATCT0, 0x30 | 0x03);
//	aw_i2c_write_one_byte(REG_PATCT1, 0x30 | 0x03);
	iic_send_byte(REG_PATCT0, 1, rise_on);
	iic_send_byte(REG_PATCT1, 1, fall_off);

	/* 5. set pattern0_breath start/end phase */
//	aw_i2c_write_one_byte(REG_PATCT2, 0x00);
	iic_send_byte(REG_PATCT2, 1, 0x00);

	/* 6. set pattern0_breath loop times -> forever */
//	aw_i2c_write_one_byte(REG_PATCT3, 0x00);
	iic_send_byte(REG_PATCT3, 1, 0x00);

	/* 7. set auto_breath mode */
//	aw_i2c_write_one_byte(REG_PATCFG, 0x07);
	iic_send_byte(REG_PATCFG, 1, 0x07);

	/* 8. start breath */
//	aw_i2c_write_one_byte(REG_PATGO, 0x01);
	iic_send_byte(REG_PATGO, 1, 0x01);
}

// void aw210xx_all_manual(void)
// {
// 	/* 1. set color */
// 	aw210xx_set_color(REG_GCOLR, 0xff);
// 	aw210xx_set_color(REG_GCOLG, 0xff);
// 	aw210xx_set_color(REG_GCOLB, 0xff);

// 	/* 2. set brigthness */
// 	aw210xx_set_brightness(REG_FADEH, 0xff);
// 	aw210xx_set_brightness(REG_FADEL, 0x00);

// 	/* 3. group	GEn=1/PATEN=1/GCOLDIS=0 */
// //	aw_i2c_write_one_byte(REG_GCFG0, 0xff);
// //	aw_i2c_write_one_byte(REG_GCFG1, 0x0f);
// 	iic_send_byte(REG_GCFG0, 1, 0xff);
// 	iic_send_byte(REG_GCFG1, 1, 0x0f);

// 	/* 4. set auto_breath mode */
// //	aw_i2c_write_one_byte(REG_PATCFG, 0x0d);
// 	iic_send_byte(REG_PATCFG, 1, 0x0d);
// 	soft_delay(200000);

// 	/* 5. set auto_breath mode */
// //	aw_i2c_write_one_byte(REG_PATCFG, 0x05);
// 	iic_send_byte(REG_PATCFG, 1, 0x05);
// 	soft_delay(200000);

// }

void aw210xx_all_off(void)
{
	int i = 0;

	/* set BR for brightness */
	for (i = 0; i < AW210XX_LED_NUM; i++)
		aw210xx_set_color(REG_COL0+i, 0x00);
}
void aw210xx_breath_off(void)
{
	for (int i = 0; i < AW210XX_LED_NUM; i++)
		aw210xx_set_brightness(REG_BR0+i, 0xFF);
	aw210xx_update_brightness_to_display();

	 aw210xx_set_color(REG_GCOLR, 0x00);
	 aw210xx_set_color(REG_GCOLG, 0x00);
	 aw210xx_set_color(REG_GCOLB, 0x00);
	 iic_send_byte(REG_PATCFG, 1, 0x04);
	 iic_send_byte(REG_PATGO, 1, 0x00);
	 iic_send_byte(REG_GCFG0, 1, 0x00);
	 iic_send_byte(REG_GCFG1, 1, 0x00);
}
/*********************************************************
 *
 * light effect
 *
 ********************************************************/
void aw210xx_rgb_multi_breath_init(const AW_MULTI_BREATH_DATA_STRUCT *data)
{
	unsigned char i;

	aw210xx_interface.getBrightnessfunc = aw_get_breath_brightness_algo_func(BREATH_ALGO_GAMMA_CORRECTION);
	algo_data.cur_frame = 0;
	algo_data.total_frames = 20;
	algo_data.data_start = 0;
	algo_data.data_end = 0;
	aw210xx_interface.p_algo_data = &algo_data;

	for (i = 0; i < RGB_NUM; i++) {
		colorful_cur_frame[i] = 0;
		colorful_total_frames[i] = 20;
		colorful_cur_color_index[i] = 0;
		colorful_cur_phase[i] = 0;
		colorful_phase_nums[i] = 5;
		breath_cur_phase[i] = 0;
		breath_phase_nums[i] = 5;
		aw210xx_algo_data[i].cur_frame = 0;
		aw210xx_algo_data[i].total_frames = (data[i].time[0] + 19) / 20 + 1;
		aw210xx_algo_data[i].data_start = data[i].fadel;
		aw210xx_algo_data[i].data_end = data[i].fadeh;
		source_color[i].r = 0x00;
		source_color[i].g = 0x00;
		source_color[i].b = 0x00;
		destination_color[i].r = 0x00;
		destination_color[i].g = 0x00;
		destination_color[i].b = 0x00;
	}
}
void aw210xx_update_frame_idx(const AW_MULTI_BREATH_DATA_STRUCT *data)
{
	unsigned char i;
	int update_frame_idx = 0;

	for (i = 0; i < RGB_NUM; i++) {
		update_frame_idx = 1;
		if (loop_end[i] == 1)
			continue;

		source_color[i].r = destination_color[i].r;
		source_color[i].g = destination_color[i].g;
		source_color[i].b = destination_color[i].b;

		aw210xx_algo_data[i].cur_frame++;
		if (aw210xx_algo_data[i].cur_frame >= aw210xx_algo_data[i].total_frames) {
			aw210xx_algo_data[i].cur_frame = 0;
			breath_cur_phase[i]++;
			if (breath_cur_phase[i] >= breath_phase_nums[i]) {
				colorful_cur_color_index[i]++;
				breath_cur_phase[i] = 1;
				if (colorful_cur_color_index[i] >= data[i].color_nums) {
					colorful_cur_color_index[i] = 0;
					if (0 == data[i].repeat_nums)
						breath_cur_loop[i] = 0;
					else if (breath_cur_loop[i] >= (data[i].repeat_nums - 1))
						update_frame_idx = 0;
					else
						breath_cur_loop[i]++;
				}
			}

			if (update_frame_idx) {
				aw210xx_algo_data[i].total_frames =
					(data[i].time[breath_cur_phase[i]])/20 + 1;
				if (breath_cur_phase[i] == 1) {
					aw210xx_algo_data[i].data_start = data[i].fadel;
					aw210xx_algo_data[i].data_end = data[i].fadeh;
				} else if (breath_cur_phase[i] == 2) {
					aw210xx_algo_data[i].data_start = data[i].fadeh;
					aw210xx_algo_data[i].data_end = data[i].fadeh;
				} else if (breath_cur_phase[i] == 3) {
					aw210xx_algo_data[i].data_start = data[i].fadeh;
					aw210xx_algo_data[i].data_end = data[i].fadel;
				} else {
					aw210xx_algo_data[i].data_start = data[i].fadel;
					aw210xx_algo_data[i].data_end = data[i].fadel;
				}
				/* breath_cur_phase[i]++; */
			} else {
				aw210xx_algo_data[i].cur_frame = 0;
				aw210xx_algo_data[i].total_frames = 1;
				aw210xx_algo_data[i].data_start = 0;
				aw210xx_algo_data[i].data_end = 0;
				loop_end[i] = 1;
			}
		}
		destination_color[i].r = data[i].rgb_color_list[colorful_cur_color_index[i]].r;
		destination_color[i].g = data[i].rgb_color_list[colorful_cur_color_index[i]].g;
		destination_color[i].b = data[i].rgb_color_list[colorful_cur_color_index[i]].b;
	}
}

void aw210xx_frame_display(void)
{
	unsigned char i = 0;
	unsigned char brightness = 0;

	for (i = 0; i < RGB_NUM; i++) {
		aw210xx_interface.p_color_1 = &source_color[i];
		aw210xx_interface.p_color_2 = &destination_color[i];
		aw_set_colorful_rgb_data(i, dim_data, &aw210xx_interface);
		brightness = aw210xx_interface.getBrightnessfunc(&aw210xx_algo_data[i]);

		if (breath_cur_phase[i] == 0)
			brightness = 0;
		aw_set_rgb_brightness(i, fade_data, brightness);
	}
}
void aw210xx_update(void)
{
	unsigned char i = 0;

	for (i = 0; i < RGB_NUM; i++) {
		aw210xx_col_data[aw210xx_reg_map[i * 6 + 0] - 0x4a] = dim_data[i * 3 + 0];
		aw210xx_br_data[aw210xx_reg_map[i * 6 + 1] - 0x01] = fade_data[i * 3 + 0];
		aw210xx_col_data[aw210xx_reg_map[i * 6 + 2] - 0x4a] = dim_data[i * 3 + 1];
		aw210xx_br_data[aw210xx_reg_map[i * 6 + 3] - 0x01] = fade_data[i * 3 + 1];
		aw210xx_col_data[aw210xx_reg_map[i * 6 + 4] - 0x4a] = dim_data[i * 3 + 2];
		aw210xx_br_data[aw210xx_reg_map[i * 6 + 5] - 0x01] = fade_data[i * 3 + 2];
	}
//	aw_i2c_writes(REG_BR0, aw210xx_br_data, LED_NUM);
//	aw_i2c_writes(REG_COL0, aw210xx_col_data, LED_NUM);
//	aw_i2c_write_one_byte(REG_UPDATE, 0x00);
	iic_send_buff(REG_BR0, 1, aw210xx_br_data, LED_NUM);
	iic_send_buff(REG_COL0, 1, aw210xx_col_data, LED_NUM);
	iic_send_byte(REG_UPDATE, 1, 0x00);
}

void aw210xx_rgb_multi_breath(const AW_MULTI_BREATH_DATA_STRUCT *data)
{
	aw210xx_rgb_multi_breath_init(data);

	aw210xx_frame_display();
	aw210xx_update();

	while (1) {
//		soft_delay(50000);
		aw210xx_update_frame_idx(data);
		aw210xx_frame_display();
		aw210xx_update();
	}
}


/*********************************************************
 *
 * call interface
 *
 ********************************************************/
void aw210xx_play(void)
{
	int ret = 0;

	DEBUG_LOG("%s enter, driver version: %s\n",
		   __func__,
		   AW210XX_DRIVER_VERSION);

	/* init */
	ret = aw210xx_init();
	if (ret < 0)
		DEBUG_LOG("%s enter, init failed, %s\n", __func__);

	/* base fuction*/
	/* aw210xx_all_breath(); */

	/*light effect*/
	aw210xx_all_on(0x0, 0xFF, 0x0);  //green
//	aw210xx_rgb_multi_breath(aw210xx_rgb_data);
}
extern void awinic_single_enter(void)
{
	aw210xx_play();
}
