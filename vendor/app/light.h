

#ifndef _LIGHT_H_
#define _LIGHT_H_

#include <tl_common.h>

//#define IIC_SEND_BYTE(X, 1, Y)			i2c_dma_write_byte(X, 1, Y)
//#define IIC_SEND_BUFF			0xB0
//#define IIC_READ_BYTE			0xB0


/************************AW***************************/
#define AW2016_ADDR			0xB0
#define AW2015_ADDR			0xB6
#define AW2015_ADDR_LEN		1
#define AW2015_DATA_LEN		1
#define I2C_CLK_SPEED		400000
#define AW_DIR_CURRENT		255

#define	AW_LED1_R	0x20
#define	AW_LED1_G	0x21
#define	AW_LED1_B	0x22
#define	AW_LED2_R	0x23
#define	AW_LED2_G	0x24
#define	AW_LED2_B	0x25
#define	AW_LED3_R	0x26
#define	AW_LED3_G	0x27
#define	AW_LED3_B	0x28
#define	LIGHT	0xFF

/***********************ETEK************************/
#define ETEK_ADDR			0x90

#define	ETEK_CHANNLE_0	0x16
#define	ETEK_CHANNLE_1	0x17
#define	ETEK_CHANNLE_2	0x18

#define ET_DIR_CURRENT		192

#define	ETEK_LED1_R	0x1A
#define	ETEK_LED1_G	0x1B
#define	ETEK_LED1_B	0x1C
#define	ETEK_LED2_R	0x1D
#define	ETEK_LED2_G	0x1E
#define	ETEK_LED2_B	0x1F
#define	ETEK_LED3_R	0x20
#define	ETEK_LED3_G	0x21
#define	ETEK_LED3_B	0x22

#define	ETEK_RGB3_O	0x25

#define	LIGHT	0xFF

#define LED_R_ON (1<<0)
#define LED_G_ON (1<<2)
#define LED_B_ON (1<<5)


extern u8 ic_flag;
extern u8 et_ic_flag;
extern u8 aw_ic_flag;


typedef enum
{
    LIGHT_MODE_DISABLE      = 0,
    LIGHT_MODE_OFF          = 1,
    LIGHT_MODE_RAINBOW      = 2,//彩虹
    LIGHT_MODE_LEDBREATH    = 3,//呼吸
    LIGHT_MODE_STARLIGHT    = 4,
    LIGHT_MODE_FLYINGRING   = 5,
    LIGHT_MODE_ALLON        = 6,
    LIGHT_MODE_CHASE        = 7,
    LIGHT_MODE_FLOW         = 8,
    LIGHT_MODE_THRCOLOR     = 9,
    LIGHT_MODE_BUFFDELAY    = 10,
    LIGHT_MODE_BOUNCEV1     = 11,
    LIGHT_MODE_BOUNCEV2     = 12,
    LIGHT_MODE_MAXEND       = 13,  

    LIGHT_MODE_RED       = 100,
    LIGHT_MODE_GREEN     = 101,
    LIGHT_MODE_BLUE      = 102,
    LIGHT_MODE_WHITE     = 103,
}light_mode_e;

typedef enum
{
	LIGHT_COLOR_RED   	 = 0,
	LIGHT_COLOR_GREEN    = 1,
	LIGHT_COLOR_BLUE     = 2,
	LIGHT_COLOR_RGB      = 3,
}light_color_e;
/************************************************************/

typedef struct
{
    u8 rRedAddr;
    u8 rGreenAddr;
    u8 rBlueAddr;

    u8 brRedAddr;
    u8 brGreenAddr;
    u8 brBlueAddr;

    u8 r;
    u8 g;
    u8 b;
    u32 delay;

    void (*draw)(void *ptr,u8 r, u8 g, u8 b); // 实现寄存器读写，亮灯
    void (*bright)(void *ptr, u8 brV);
}Led;

typedef struct
{
	Led gLed[12];
	void (*init)(void* ptr);
	void (*doframe)(void *param,void* ptr,u32 timePoint);
}Animator;

typedef struct
{
    u8 animType;
    u32 timeTick;
    Animator *anr;
    void (*init)(u8 animType); // 初始化Animator，实现不同动画
    void (*run)(void);
}Render;

typedef struct
{
	u16 type;
    u16 typeBak;
    u32 cycleMs;
    u8 colorNum;
    u8 rgb[12][3];
}LightParam;

/************************************************************/
void HSVtoRGB(u8 *r, u8 *g, u8 *b, u16 h, u16 s, u16 v);
void rainbow_doframe(void *param,void *ptr,u32 timeTick);

void r_doframe(void *param,void *ptr,u32 timeTick);
void g_doframe(void *param,void *ptr,u32 timeTick);
void b_doframe(void *param,void *ptr,u32 timeTick);
void white_doframe(void *param,void *ptr,u32 timeTick);
void rg_doframe(void *param,void *ptr,u32 timeTick);
void off_doframe(void *param,void *ptr,u32 timeTick);


u8 iic_read_byte(u8 addr, u8 length);
void iic_send_byte(u8 addr, u8 length, u8 data);
void iic_send_buff(u8 addr, u8 addrlen, u8  *dataBuf, u8 datalen);
///************** Functions Declaration *********************/
void light_timer_process(void);
void light_init(void);
void light_reinitialize(void);
#endif

