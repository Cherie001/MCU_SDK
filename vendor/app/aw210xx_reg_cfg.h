#include "aw_lamp_interface.h"

#define AW21024	0
#define AW21036	1

/******************************************************
 *
 * Register List
 *
 *****************************************************/
#define REG_GCR					0x00
#define REG_BR0					0x01
#define REG_BR1					0x02
#define REG_BR2					0x03
#define REG_BR3					0x04
#define REG_BR4					0x05
#define REG_BR5					0x06
#define REG_BR6					0x07
#define REG_BR7					0x08
#define REG_BR8					0x09
#define REG_BR9					0x0A
#define REG_BR10				0x0B
#define REG_BR11				0x0C
#define REG_BR12				0x0D
#define REG_BR13				0x0E
#define REG_BR14				0x0F
#define REG_BR15				0x10
#define REG_BR16				0x11
#define REG_BR17				0x12
#define REG_BR18				0x13
#define REG_BR19				0x14
#define REG_BR20				0x15
#define REG_BR21				0x16
#define REG_BR22				0x17
#define REG_BR23				0x18
#define REG_BR24				0x19
#define REG_BR25				0x1A
#define REG_BR26				0x1B
#define REG_BR27				0x1C
#define REG_BR28				0x1D
#define REG_BR29				0x1E
#define REG_BR30				0x1F
#define REG_BR31				0x20
#define REG_BR32				0x21
#define REG_BR33				0x22
#define REG_BR34				0x23
#define REG_BR35				0x24
#define REG_UPDATE				0x49
#define REG_COL0				0x4a
#define REG_COL1				0x4b
#define REG_COL2				0x4c
#define REG_COL3				0x4d
#define REG_COL4				0x4e
#define REG_COL5				0x4f
#define REG_COL6				0x50
#define REG_COL7				0x51
#define REG_COL8				0x52
#define REG_COL9				0x53
#define REG_COL10				0x54
#define REG_COL11				0x55
#define REG_COL12				0x56
#define REG_COL13				0x57
#define REG_COL14				0x58
#define REG_COL15				0x59
#define REG_COL16				0x5a
#define REG_COL17				0x5b
#define REG_COL18				0x5c
#define REG_COL19				0x5d
#define REG_COL20				0x5e
#define REG_COL21				0x5f
#define REG_COL22				0x60
#define REG_COL23				0x61
#define REG_COL24				0x62
#define REG_COL25				0x63
#define REG_COL26				0x64
#define REG_COL27				0x65
#define REG_COL28				0x66
#define REG_COL29				0x67
#define REG_COL30				0x68
#define REG_COL31				0x69
#define REG_COL32				0x6a
#define REG_COL33				0x6b
#define REG_COL34				0x6c
#define REG_COL35				0x6d
#define REG_GCCR				0x6e
#define REG_PHCR				0x70
#define REG_OSDCR				0x71
#define REG_OSST0				0x72
#define REG_OSST1				0x73
#define REG_OSST2				0x74
#define REG_0SST3				0x75
#define REG_OSST4				0x76
#define REG_OTCR				0x77
#define REG_SSCR				0x78
#define REG_UVCR				0x79
#define REG_GCR2				0x7A
#define REG_GCR4				0x7C
#define REG_VER					0x7E
#define REG_RESET				0x7F
#define REG_WBR					0x90
#define REG_WBG					0x91
#define REG_WBB					0x92
#define REG_PATCFG				0xA0
#define REG_PATGO				0xA1
#define REG_PATCT0				0xA2
#define REG_PATCT1				0xA3
#define REG_PATCT2				0xA4
#define REG_PATCT3				0xA5
#define REG_FADEH				0xA6
#define REG_FADEL				0xA7
#define REG_GCOLR				0xA8
#define REG_GCOLG				0xA9
#define REG_GCOLB				0xAA
#define REG_GCFG0				0xAB
#define REG_GCFG1				0xAC

/******************************************************
 *
 * Register Write/Read Access
 *
 *****************************************************/
#define REG_NONE_ACCESS			0
#define REG_RD_ACCESS			1
#define REG_WR_ACCESS			0
#define AW210XX_REG_MAX			0x100

/*********************************************************
 *
 * chip info
 *
 ********************************************************/
#define AW210XX_CHIPID				0x18

#define AW21024_DEVICE_ADDR			(0x30 << 1) /* AD0 and AD1 both GND, addr = 0x30 */
#define AW21036_DEVICE_ADDR			(0x34 << 1) /* AD0——>GND, addr = 0x38 */
#if AW21024
#define AW210XX_DEVICE_ADDR			AW21024_DEVICE_ADDR
#else
#define AW210XX_DEVICE_ADDR			AW21036_DEVICE_ADDR
#endif

#define AW21024_LED_NUM				24
#define AW21036_LED_NUM				36
#if AW21024
#define AW210XX_LED_NUM				AW21024_LED_NUM
#else
#define AW210XX_LED_NUM				AW21036_LED_NUM
#endif

#if AW21036
#define LED_NUM 36
#elif AW21024
#define LED_NUM 24
#endif

static const int aw210xx_reg_map[] = {
#if AW21036
	0x4a, 0x01,
	0x4b, 0x02,
	0x4c, 0x03,

	0x4d, 0x04,
	0x4e, 0x05,
	0x4f, 0x06,

	0x50, 0x07,
	0x51, 0x08,
	0x52, 0x09,

	0x53, 0x0a,
	0x54, 0x0b,
	0x55, 0x0c,

	0x56, 0x0d,
	0x57, 0x0e,
	0x58, 0x0f,

	0x59, 0x10,
	0x5a, 0x11,
	0x5b, 0x12,

	0x5c, 0x13,
	0x5d, 0x14,
	0x5e, 0x15,

	0x5f, 0x16,
	0x60, 0x17,
	0x61, 0x18,

	0x62, 0x19,
	0x63, 0x1a,
	0x64, 0x1b,

	0x65, 0x1c,
	0x66, 0x1d,
	0x67, 0x1e,

	0x68, 0x1f,
	0x69, 0x20,
	0x6a, 0x21,

	0x6b, 0x22,
	0x6c, 0x23,
	0x6d, 0x24,
#endif
#if AW21024
	0x4a, 0x01,
	0x4b, 0x02,
	0x4c, 0x03,

	0x4d, 0x04,
	0x4e, 0x05,
	0x4f, 0x06,

	0x50, 0x07,
	0x51, 0x08,
	0x52, 0x09,

	0x53, 0x0a,
	0x54, 0x0b,
	0x55, 0x0c,

	0x56, 0x0d,
	0x57, 0x0e,
	0x58, 0x0f,

	0x59, 0x10,
	0x5a, 0x11,
	0x5b, 0x12,

	0x5c, 0x13,
	0x5d, 0x14,
	0x5e, 0x15,

	0x5f, 0x16,
	0x60, 0x17,
	0x61, 0x18,
#endif
};

/*********************************************************
 *
 * effect data
 *
 ********************************************************/
/* breath */
#if 1
static const AW_COLOR_STRUCT rgb_color_list[] = {
	{  0,   0, 255},
	{255,   0,   0},
	{  0, 255,   0},
};
static const AW_MULTI_BREATH_DATA_STRUCT aw210xx_rgb_data[] = {
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{0, 500, 50, 500, 50}, 0, 255, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
};
#endif
/* horse race lamp */
#if 0
static const AW_COLOR_STRUCT rgb_color_list[] = {
	{  125,  0, 0},
	{  0,  125, 0},
	{  0,  0, 125},
};
static const AW_MULTI_BREATH_DATA_STRUCT aw210xx_rgb_data[] = {
	{{  0, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{ 50, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{100, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{150, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{200, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{250, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{300, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{350, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{400, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{450, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{500, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
	{{550, 100, 50, 100, 50}, 2, 120, 0, sizeof(rgb_color_list)/sizeof(AW_COLOR_STRUCT), rgb_color_list},
};
#endif
