#ifndef _AW_LAMP_INTERFACE_H_
#define _AW_LAMP_INTERFACE_H_

#include "aw_breath_algorithm.h"

typedef struct{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} AW_COLOR_STRUCT;

typedef struct {
	uint32_t time[5];
	uint32_t repeat_nums;
	uint8_t  fadeh;
	uint8_t  fadel;
	uint8_t color_nums;
	const AW_COLOR_STRUCT *rgb_color_list;

} AW_MULTI_BREATH_DATA_STRUCT;

typedef struct{
	GetBrightnessFuncPtr getBrightnessfunc;
	ALGO_DATA_STRUCT *p_algo_data;
	AW_COLOR_STRUCT *p_color_1;
	AW_COLOR_STRUCT *p_color_2;
} AW_COLORFUL_INTERFACE_STRUCT;

extern void aw_set_colorful_rgb_data(uint8_t rgb_idx, uint8_t *dim_reg,
	AW_COLORFUL_INTERFACE_STRUCT *p_colorful_interface);
extern void aw_set_rgb_brightness(unsigned char rgb_idx,
	unsigned char *fade_reg, unsigned char brightness);

extern unsigned char aw_get_real_dim(unsigned char led_dim);
#endif
