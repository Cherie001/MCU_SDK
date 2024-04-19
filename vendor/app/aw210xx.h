#ifndef __LEDS_AW210XX_H__
#define __LEDS_AW210XX_H__

#include "aw_lamp_interface.h"
#include "aw210xx_reg_cfg.h"

#define I2C_RETRY_TIMES			0x03U
#define AW_RESET_VALUE			0x55AAU

#define	AWINIC_UART_DEBUG
#ifdef	AWINIC_UART_DEBUG
#define	AWLOGD(format, arg...)	printf(format, ##arg)
#else
#define	AWLOGD(format, arg...)	do {} while (0)
#endif

typedef enum{
	I2C_OK			= 0x00U,
	I2C_ERROR		= 0x01U,
	I2C_BUSY		= 0x02U,
	I2C_TIMEOUT		= 0x03U
} AW_I2C_StatusTypeDef;

//extern I2C_HandleTypeDef hi2c1;
/* base fuction */
extern void aw210xx_chip_hwen(void);
extern int	aw210xx_init(void);
extern void aw210xx_all_on(uint8_t r, uint8_t g, uint8_t b);
extern void aw210xx_all_breath(uint8_t r, uint8_t g, uint8_t b, uint8_t rise_on, uint8_t fall_off);
extern void aw210xx_all_manual(void);
extern void aw210xx_group(void);
extern void aw210xx_clear(void);
extern void aw210xx_clear_breath(void);

/*********************************************************
 *
 * algorithm variable
 *
 ********************************************************/

static unsigned char aw210xx_br_data[LED_NUM] = {0};
static unsigned char aw210xx_col_data[LED_NUM] = {0};
static unsigned char dim_data[LED_NUM];
static unsigned char fade_data[LED_NUM];
static AW_COLORFUL_INTERFACE_STRUCT aw210xx_interface;

#define RGB_NUM		(LED_NUM/3)
static unsigned char loop_end[RGB_NUM] = {0};
static AW_COLOR_STRUCT source_color[RGB_NUM];
static AW_COLOR_STRUCT destination_color[RGB_NUM];
static ALGO_DATA_STRUCT algo_data;
static ALGO_DATA_STRUCT aw210xx_algo_data[RGB_NUM];
static ALGO_DATA_STRUCT aw210xx_algo_color[RGB_NUM];
static unsigned char colorful_phase_nums[RGB_NUM] = {0};
static unsigned char colorful_cur_phase[RGB_NUM] = {0};
static unsigned char breath_cur_phase[RGB_NUM] = {0};
static unsigned char breath_cur_loop[RGB_NUM] = {0};
static unsigned char breath_phase_nums[RGB_NUM] = {0};
static unsigned char colorful_total_frames[RGB_NUM] = {0};
static unsigned char colorful_cur_frame[RGB_NUM] = {0};
static unsigned char colorful_cur_color_index[RGB_NUM] = {0};

/* call interface */
void aw210xx_play(void);
void aw210xx_disable(void);

#endif
