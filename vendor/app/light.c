#include <tl_common.h>
#include "drivers.h"
#include "stack/ble/ble.h"
#include "../common/user_config.h"
#include "light.h"
#include "timer.h"
#include "bs_protocol.h"
#include "app_light.h"
#include "ntc.h"

volatile LightParam _lightParam;
Render   _render;
Animator _anim_light;

extern u8 gLightSta;
extern bs_light_param_t _bsLightParam;
extern u8 light_color_reg_arry[12];
extern u8 light_bright_reg_arry[12];

u8 iic_read_byte(u8 addr, u8 length)
{
	u8 data;
	data = i2c_dma_read_byte(addr, length);
	return data;
}

void iic_send_byte(u8 addr, u8 length, u8 data)
{
	i2c_dma_write_byte(addr, length, data);
}

void iic_send_buff(u8 addr, u8 addrlen, u8  *dataBuf, u8 datalen)
{
	i2c_dma_write_buff(addr, addrlen, dataBuf, datalen);
}

static void draw(void *ptr, u8 r, u8 g, u8 b)
{
	Led *this =(Led*) ptr;

	iic_send_byte(this->rRedAddr,  1,r);
	iic_send_byte(this->rGreenAddr,1,g);
	iic_send_byte(this->rBlueAddr, 1,b);
}

static void bright(void *ptr, u8 brV)
{
	Led *this =(Led*) ptr;

	iic_send_byte(this->brRedAddr,  1, brV);
	iic_send_byte(this->brGreenAddr,  1, brV);
	iic_send_byte(this->brBlueAddr,  1, brV);
	iic_send_byte(0x49, 1, 0x00);
	
}

static void light_base_init(void *ptr)
{
	Animator *this = ptr;

	DEBUG_LOG("light_base_init.\r\n");

	for(int i = 0; i < 12; i++) {
		this->gLed[i].rRedAddr   = light_color_reg_arry[i];
		this->gLed[i].rGreenAddr = light_color_reg_arry[i]+1;
		this->gLed[i].rBlueAddr  = light_color_reg_arry[i]+2;
		this->gLed[i].brRedAddr  = light_bright_reg_arry[i];
		this->gLed[i].brGreenAddr= light_bright_reg_arry[i]+1;
		this->gLed[i].brBlueAddr = light_bright_reg_arry[i]+2;
		this->gLed[i].draw = draw;
		this->gLed[i].bright = bright;
	}

	aw210xx_breath_off();

}

void HSVtoRGB(u8 *r, u8 *g, u8 *b, u16 h, u16 s, u16 v)
{
	// R,G,B from 0-255, H from 0-360, S,V from 0-100
	u32 i;
	u32 RGB_min, RGB_max;
	u32 RGB_Adj;

	//	RGB_max = v*2.55f;
	RGB_max = v*2.55f;
	RGB_min = RGB_max*(100 - s) / 100.0f;

	i = h / 60;
	u32 difs = h % 60; // factorial part of h
	// RGB adjustment amount by hue
	RGB_Adj = (RGB_max - RGB_min)*difs / 60.0f;

	switch (i) {
	case 0:
		*r = RGB_max;
		*g = (RGB_min + RGB_Adj);
		*b = RGB_min;
		break;
	case 1:
		*r = (RGB_max - RGB_Adj);
		*g = RGB_max;
		*b = RGB_min;
		break;
	case 2:
		*r = RGB_min;
		*g = RGB_max;
		*b = (RGB_min + RGB_Adj);
		break;
	case 3:
		*r = RGB_min;
		*g = (RGB_max - RGB_Adj);
		*b = RGB_max;
		break;
	case 4:
		*r = (RGB_min + RGB_Adj);
		*g = RGB_min;
		*b = RGB_max;
		break;
	default:		// case 5:
		*r = RGB_max;
		*g = RGB_min;
		*b = (RGB_max - RGB_Adj);
		break;
	}
}
/*******************************************************************/
/*********************LED_RGB MODULE****************************/
/*******************************************************************/
void r_doframe(void *param,void *ptr,u32 timeTick)
{
	Animator *this = ptr;

	for(int i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]),255,0,0);
	}
}
void g_doframe(void *param,void *ptr,u32 timeTick)
{
	Animator *this = ptr;

	for(int i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]),0,255,0);
	}
}
void b_doframe(void *param,void *ptr,u32 timeTick)
{
	Animator *this = ptr;

	for(int i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]),0,0,255);
	}
}

void white_doframe(void *param,void *ptr,u32 timeTick)
{
	Animator *this = ptr;

	for(int i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]),255,255,255);
	}
}

void rg_doframe(void *param,void *ptr,u32 timeTick)
{
	Animator *this = ptr;

	for(int i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]),255,255,0);
	}
}

/*******************************************************************/
/*********************LED_DISABLE MODULE****************************/
/*******************************************************************/
void off_doframe(void *param,void *ptr,u32 timeTick)
{
	Animator *this = ptr;

	for(int i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]),0,0,0);
	}
}
/*******************************************************************/
/*********************LED_RAINBOW MODULE****************************/
/*******************************************************************/
void rainbow_doframe(void *param,void *ptr,u32 timeTick)
{
	LightParam *plightParam =  (LightParam*)param;
	Animator *this = ptr;
	u8 r,g,b;
	u16 Rh;
	u32 cTick = plightParam->cycleMs/HW_TIMER_BASE_TOMS;

	// if(cTick%360 != 0) {
	// 	if(cTick%360 < 900) {
	// 		cTick = cTick/360 +900;
	// 	}
	// }

	for(int i = 0; i < 12; i++) {
		if(cTick > 360 && cTick%360 == 0) {
			Rh = (30*i+((timeTick/(cTick/360))%360))%360;
		} else {
			Rh = cTick <= 360 ? (30*i+((360/cTick)*(timeTick%cTick)))%360 : (30*i+(360/(cTick%360)*((timeTick/(cTick/(cTick%360)))%(cTick%360))))%360;
		}
		HSVtoRGB(&r,&g,&b,Rh,100,100);
		this->gLed[i].draw(&(this->gLed[i]),r,g,b);
//		DEBUG_LOG("rain r0 = %d, g0 = %d, b0 = %d.\r\n", r, g, b);
	}

}
/*******************************************************************/
/*******************************************************************/
/*********************BS_LED_MODE_LEDBREATH*************************/
/*******************************************************************/

void light_breath_init()
{
	u8 r, g, b;
	u8 rise_on, fall_off;

	if(_lightParam.colorNum == 1) {
		r = _lightParam.rgb[0][0];
		g = _lightParam.rgb[0][1];
		b = _lightParam.rgb[0][2];

		fall_off = (_lightParam.cycleMs >> 8) & 0xFF;
		rise_on = _lightParam.cycleMs & 0xFF;
	} else {
		r = 0xFF;
		g = 0xFF;
		b = 0xFF;
		rise_on = 0x63;
		fall_off = 0x63;
	}

	aw210xx_all_breath(r, g, b, rise_on, fall_off);

	DEBUG_LOG("%s  r = %x g = %x b = %x rise_on = %x fall_off = %x\r\n", __func__, r, g, b, rise_on, fall_off);
}

void bs_breath_doframe(void)
{
	return;
}



/*******************************************************************/
/*******************************************************************/
/*************************LED_STARLIGHT MODULE*****************************/
/*******************************************************************/

/*
1. 红色：255,0,0——web颜色值：#ff0000

2. 橙红：255,51,0——web颜色值：#ff3300

3. 橙色：255,102,0——web颜色值：#ff6600

4. 橙黄：255,153,0——web颜色值：#ff9900

5. 黄色：255,255,0——web颜色值：#ffff00

6. 黄绿：153,255,0——web颜色值：#99ff00

7. 绿色：0,255,0——web颜色值：#00ff00

8. 蓝绿：0,255,255——web颜色值：#00ffff

9. 蓝色：0,0,255——web颜色值：#0000ff

10. 蓝紫：102,0,255——web颜色值：#6600ff

11. 紫色：255,0,255——web颜色值：#ff00ff

12. 紫红：255,0,102——web颜色值：ff0066
*/

#define STARLIGHT_EFFECT_NUMBER	16

static u8 light_arry[12] = {0};
static u8 color_arry[12] = {0};
static u8 light_color[12][3] = {{0xFF, 0x0, 0x0}, {0xFF, 0x33, 0x0}, {0xFF, 0x66, 0x0}, {0xFF, 0x99, 0x0}, {0xFF, 0xFF, 0x0}, {0x99, 0xFF, 0x0},
								{0x0, 0xFF, 0x0}, {0x0, 0xFF, 0xFF}, {0x0, 0x0, 0xFF}, {0x66, 0x0, 0xFF}, {0xFF, 0x0, 0xFF}, {0xFF, 0x0, 0x66}};

void starlight_color(void *ptr, u8 LedNum)
{
	Animator *this = ptr;

	this->gLed[light_arry[LedNum]].draw(&(this->gLed[light_arry[LedNum]]), light_color[color_arry[LedNum]][0], light_color[color_arry[LedNum]][1], light_color[color_arry[LedNum]][2]);
}

u8 starlight_bright(void *ptr, u32 timeTick, u32 CycleTick, u8 LedNum)
{
	Animator *this = ptr;

	if((timeTick >= 0)&&(timeTick <= CycleTick*5/16))
	{
		u8 step = 255/((CycleTick*5)/16);
		this->gLed[LedNum].bright(&(this->gLed[LedNum]),step * timeTick);
	}
	else if(timeTick <= CycleTick*8/16)
    {
		this->gLed[LedNum].bright(&(this->gLed[LedNum]),255);
    }
	else if(timeTick <= CycleTick*13/16)
    {
		u8 step = 255/((CycleTick*5)/16);
		this->gLed[LedNum].bright(&(this->gLed[LedNum]),255-(step*(timeTick-CycleTick*8/16)));
    }
	else if(timeTick <= CycleTick*16/16)
	{
		this->gLed[LedNum].bright(&(this->gLed[LedNum]),0);
		return 1;
	}

	return 0;
}

static u8 NextLedFlag = 1;
static u8 starlight_auto_colours = 0;

void starlight_init(void *ptr)
{
	Animator *this = ptr;

	light_base_init(this);

	NextLedFlag = 1;
	u8 r, g, b;

	if(_lightParam.colorNum == 1) {
		starlight_auto_colours = 0;
		r = _lightParam.rgb[0][0];
		g = _lightParam.rgb[0][1];
		b = _lightParam.rgb[0][2];
	} else {
		starlight_auto_colours = 1;
		r = 0;
		g = 0;
		b = 0;
	}

	DEBUG_LOG("%s: r = %x g = %x b = %x\n", __func__, r, g, b);

	for(u8 i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]), r, g, b);
		this->gLed[i].bright(&(this->gLed[i]),0);
	}
	
	DEBUG_LOG("StarLight Color R:%x G:%x B:%x\n", this->gLed[0].r, this->gLed[0].g, this->gLed[0].b);

}

void getRandNum(u8 *a,u8 n,u8 min,u8 max)   //获取[min-max]之间不重复的n个随机数保存到数组a中
{
	u8 i,j,t,m = 0,flag;   
//	srand(time(NULL));     //随机数种子函数
	for(i=0; i<n; i++)           //循环n次得到n个随机数
	{
		while(1)    
		{
			flag = 0;                //进入while(1)，标志位置0
			t = rand()%(max - min + 1) + min;   //rand()%5得到 [0-4) 之间的随机数，因此rand()%(max - min + 1)可以得到[0 - (max - min + 1) )之间的随机数。再加上min 就能得到 [min - max]之间的随机数
			for(j=0; j<m; j++)   //第一次m = 0,不执行循环语句
			{
				if(a[j] == t)          //新生成的随机数只要和数组中的元素重复
				{
					flag = 1;          // flag 置1
					break;            //一旦找到一个重复的，直接跳出for循环
				}
			}
			if(flag == 0)           //第一次flag = 0
			{
				a[m++] = t;        //生成的随机数和数组中已有的元素不重复时，保存到数组中。
				break;               //跳出while循环，继续获得后面的随机数
			}
		}
	}
}

void starlight_doframe(void *param, void *ptr, u32 timeTick)
{
	LightParam *plightParam =  (LightParam*)param;

	static u8 LedNum = 12;
	
	if(LedNum == 12) {
		getRandNum(&light_arry, 12, 0, 11);
		getRandNum(&color_arry, 12, 0, 11);
		LedNum = 0;
	}

	if(NextLedFlag && starlight_auto_colours) {
		NextLedFlag = 0;
		starlight_color(ptr, LedNum);
//		DEBUG_LOG("Current light number %d = %d\r\n",LedNum, light_arry[LedNum]);
	}

	u32 cTick = plightParam->cycleMs/HW_TIMER_BASE_TOMS;

	cTick = cTick < STARLIGHT_EFFECT_NUMBER ? STARLIGHT_EFFECT_NUMBER : cTick;

	if(starlight_bright(ptr, timeTick%cTick, cTick, light_arry[LedNum])) {
		LedNum++;
		NextLedFlag = 1;
	}

}

/*******************************************************************/
/*******************************************************************/
/*************************LED_FLYINGRING MODULE*****************************/
/*******************************************************************/

#define FLYING_EFFECT_NUMBER	26

u8 flying_effect_arry1[] = {0x0, 0xFF, 0xFF, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0x0, 0x0, 0x0};
u8 flying_effect_arry2[] = {0xFF, 0x0, 0xFF, 0xFF, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xFF, 0xFF};
u8 flying_effect_arry3[] = {0xFF, 0xFF, 0x0, 0xFF, 0xFF, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xFF};
u8 flying_effect_arry4[] = {0xFF, 0xFF, 0xFF, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0xFF};

void flyingring_init(void *ptr)
{
	Animator *this = ptr;

	light_base_init(this);

	u8 r, g, b;

	if(_lightParam.colorNum == 1) {
		r = _lightParam.rgb[0][0];
		g = _lightParam.rgb[0][1];
		b = _lightParam.rgb[0][2];
	} else {
		r = 0xFF;
		g = 0xFF;
		b = 0xFF;
	}

	DEBUG_LOG("%s: r = %x g = %x b = %x\n", __func__, r, g, b);

	for(u8 i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]), r, g, b);
		this->gLed[i].bright(&(this->gLed[i]),0);
	}
	
	DEBUG_LOG("flyingring Color R:%x G:%x B:%x\n", this->gLed[0].r, this->gLed[0].g, this->gLed[0].b);
}

u8 flyingring_bright(void *ptr, u32 timeTick, u32 CycleTickSW)
{
	Animator *this = ptr;
	static u8 light_cnt = 0;

	light_cnt++;

	if(light_cnt >= FLYING_EFFECT_NUMBER) {
		light_cnt = 0;
	}

	//DEBUG_LOG("%s %d \n", __func__, light_cnt);

	for(u8 i = 0; i < 3; i++) {
		this->gLed[0 + i*4].bright(&(this->gLed[0 + i*4]), flying_effect_arry1[light_cnt]);
		this->gLed[1 + i*4].bright(&(this->gLed[1 + i*4]), flying_effect_arry2[light_cnt]);
		this->gLed[2 + i*4].bright(&(this->gLed[2 + i*4]), flying_effect_arry3[light_cnt]);
		this->gLed[3 + i*4].bright(&(this->gLed[3 + i*4]), flying_effect_arry4[light_cnt]);
	}

}

void flyingring_doframe(void *param, void *ptr, u32 timeTick)
{
	LightParam *plightParam =  (LightParam*)param;

	u32 cTick = plightParam->cycleMs/HW_TIMER_BASE_TOMS;

	//DEBUG_LOG("%s cTick:%d %d timeTick:%d\n", __func__, cTick, sizeof(flying_effect_arry), timeTick);

	if(cTick < (FLYING_EFFECT_NUMBER * 2)) {
		cTick = 1;
	} else {
		cTick = cTick / FLYING_EFFECT_NUMBER;
	}

	if(timeTick%cTick == 0) {
		flyingring_bright(ptr, timeTick%cTick, cTick);
	}

}

/*******************************************************************/
/*******************************************************************/
/*************************LED_ALLON MODULE*****************************/
/*******************************************************************/

static u8 init_once = 0;

void allon_init(void *ptr)
{
	Animator *this = ptr;

	DEBUG_LOG("%s:enter\n", __func__);

	light_base_init(this);

	for(u8 i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]), 0, 0, 0);
		this->gLed[i].bright(&(this->gLed[i]),0xFF);
	}

	init_once = 0;
}

void allon_doframe(void *param, void *ptr, u32 timeTick)
{
	if(init_once) return;
	init_once++;

	LightParam *plightParam =  (LightParam*)param;
	Animator *this = ptr;

	switch(plightParam->colorNum) {
		case 1:
			for(u8 i = 0; i < 12; i++) {
				this->gLed[i].draw(&(this->gLed[i]), plightParam->rgb[0][0], plightParam->rgb[0][1], plightParam->rgb[0][2]);
			}
		break;
		case 2:
			for(u8 i = 0; i < 6; i++) {
				this->gLed[i].draw(&(this->gLed[i]), plightParam->rgb[0][0], plightParam->rgb[0][1], plightParam->rgb[0][2]);
			}
			for(u8 i = 6; i < 12; i++) {
				this->gLed[i].draw(&(this->gLed[i]), plightParam->rgb[1][0], plightParam->rgb[1][1], plightParam->rgb[1][2]);
			}
		break;
		case 3:
			for(u8 i = 0; i < 4; i++) {
				this->gLed[i].draw(&(this->gLed[i]), plightParam->rgb[0][0], plightParam->rgb[0][1], plightParam->rgb[0][2]);
			}
			for(u8 i = 4; i < 8; i++) {
				this->gLed[i].draw(&(this->gLed[i]), plightParam->rgb[1][0], plightParam->rgb[1][1], plightParam->rgb[1][2]);
			}
			for(u8 i = 8; i < 12; i++) {
				this->gLed[i].draw(&(this->gLed[i]), plightParam->rgb[2][0], plightParam->rgb[2][1], plightParam->rgb[2][2]);
			}
		break;
		default:
			for(u8 i = 0; i < 12; i++) {
				this->gLed[i].draw(&(this->gLed[i]), 0xFF, 0xFF, 0xFF);
			}
		break;
	}
}


/*******************************************************************/
/*******************************************************************/
/*************************LED_CHASE MODULE*****************************/
/*******************************************************************/

#define FLOW_CHASE_EFFECT_FRE	12
#define DOUB_CHASE_EFFECT_FRE	6

static u8 current_fre = 0;
static u8 chase_backcolor[] = {0x11, 0x22, 0x33};
static u8 chase_maincolor[] = {0xFF, 0xFF, 0x0};

void flow_chase_init(void *ptr)
{
	Animator *this = ptr;

	DEBUG_LOG("%s:enter\n", __func__);

	light_base_init(this);

	for(u8 i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]), 0, 0, 0);
		this->gLed[i].bright(&(this->gLed[i]),0xFF);
	}

	if(_lightParam.colorNum == 2) {
		for(u8 i = 0; i < 3; i++) {
			chase_maincolor[i] = _lightParam.rgb[0][i];
			chase_backcolor[i] = _lightParam.rgb[1][i];
		}
	} else {
		chase_maincolor[0] = 0xFF;
		chase_maincolor[0] = 0xD7;
		chase_maincolor[0] = 0x00;
		chase_backcolor[0] = 0xFF;
		chase_backcolor[1] = 0x0;
		chase_backcolor[2] = 0x0;
	}
	
	DEBUG_LOG("main r = %x g = %x b = %x\n", chase_maincolor[0], chase_maincolor[1], chase_maincolor[2]);
	DEBUG_LOG("back r = %x g = %x b = %x\n", chase_backcolor[0], chase_backcolor[1], chase_backcolor[2]);
}

u8 flow_chase_bright(void *ptr, u32 timeTick, u32 CycleTickSW)
{
	Animator *this = ptr;
	static u8 light_cnt = 0;

	light_cnt++;

	if(light_cnt >= current_fre) {
		light_cnt = 0;
	}

	//DEBUG_LOG("%s %d \n", __func__, light_cnt);

	if(current_fre == DOUB_CHASE_EFFECT_FRE) {
		for(u8 i = 0; i < DOUB_CHASE_EFFECT_FRE; i++) {
			if(i == light_cnt) {
				this->gLed[i].draw(&(this->gLed[i]), chase_maincolor[0], chase_maincolor[1], chase_maincolor[2]);
				this->gLed[i+6].draw(&(this->gLed[i+6]), chase_maincolor[0], chase_maincolor[1], chase_maincolor[2]);
			} else {
				this->gLed[i].draw(&(this->gLed[i]), chase_backcolor[0], chase_backcolor[1], chase_backcolor[2]);
				this->gLed[i+6].draw(&(this->gLed[i+6]), chase_backcolor[0], chase_backcolor[1], chase_backcolor[2]);
			}
		}
	} else {
		for(u8 i = 0; i < FLOW_CHASE_EFFECT_FRE; i++) {
			if(i == light_cnt) {
				this->gLed[i].draw(&(this->gLed[i]), chase_maincolor[0], chase_maincolor[1], chase_maincolor[2]);
			} else {
				this->gLed[i].draw(&(this->gLed[i]), chase_backcolor[0], chase_backcolor[1], chase_backcolor[2]);
			}
		}
	}
}

void flow_chase_doframe(void *param, void *ptr, u32 timeTick)
{

	LightParam *plightParam =  (LightParam*)param;
	Animator *this = ptr;

	current_fre = plightParam->type == LIGHT_MODE_CHASE ? DOUB_CHASE_EFFECT_FRE : FLOW_CHASE_EFFECT_FRE;

	u32 cTick = plightParam->cycleMs/HW_TIMER_BASE_TOMS;

	//DEBUG_LOG("%s cTick:%d %d timeTick:%d\n", __func__, cTick, sizeof(flying_effect_arry), timeTick);

	if(cTick < (current_fre * 2)) {
		cTick = 1;
	} else {
		cTick = cTick / current_fre;
	}

	if(timeTick%cTick == 0) {
		flow_chase_bright(ptr, timeTick%cTick, cTick);
	}
}

/*******************************************************************/
/*******************************************************************/
/*************************LIGHT_MODE_THRCOLOR MODULE*****************************/
/*******************************************************************/

#define THRCOLOR_FRE	12

static u8 thrcolor_backcolor[] = {0x11, 0x22, 0x33};
static u8 thrcolor_maincolor1[] = {0xFF, 0xFF, 0x0};
static u8 thrcolor_maincolor2[] = {0xFF, 0x0, 0x0};


void thrcolor_init(void *ptr)
{
	Animator *this = ptr;

	DEBUG_LOG("%s:enter\n", __func__);

	light_base_init(this);

	for(u8 i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]), 0, 0, 0);
		this->gLed[i].bright(&(this->gLed[i]),0xFF);
	}

	if(_lightParam.colorNum == 3) {
		for(u8 i = 0; i < 3; i++) {
			thrcolor_maincolor1[i] = _lightParam.rgb[0][i];
			thrcolor_maincolor2[i] = _lightParam.rgb[1][i];
			thrcolor_backcolor[i] = _lightParam.rgb[2][i];
		}
	} else {
		thrcolor_maincolor1[0] = 0xFF;
		thrcolor_maincolor1[1] = 0xD7;
		thrcolor_maincolor1[2] = 0x0;
		thrcolor_maincolor2[0] = 0x0;
		thrcolor_maincolor2[1] = 0x0;
		thrcolor_maincolor2[2] = 0xFF;
		thrcolor_backcolor[0] = 0xFF;
		thrcolor_backcolor[1] = 0x0;
		thrcolor_backcolor[2] = 0x0;
	}
	
	DEBUG_LOG("main1 r = %x g = %x b = %x\n", thrcolor_maincolor1[0], thrcolor_maincolor1[1], thrcolor_maincolor1[2]);
	DEBUG_LOG("main2 r = %x g = %x b = %x\n", thrcolor_maincolor2[0], thrcolor_maincolor2[1], thrcolor_maincolor2[2]);
	DEBUG_LOG("back r = %x g = %x b = %x\n", thrcolor_backcolor[0], thrcolor_backcolor[1], thrcolor_backcolor[2]);
}

u8 thrcolor_bright(void *ptr, u32 timeTick, u32 CycleTickSW)
{
	Animator *this = ptr;
	static u8 light_cnt = 0;

	light_cnt++;

	if(light_cnt >= THRCOLOR_FRE) {
		light_cnt = 0;
	}

	//DEBUG_LOG("%s %d \n", __func__, light_cnt);

	for(u8 i = 0; i < THRCOLOR_FRE; i++) {
		this->gLed[i].draw(&(this->gLed[i]), thrcolor_backcolor[0], thrcolor_backcolor[1], thrcolor_backcolor[2]);
	}

	this->gLed[THRCOLOR_FRE-1-light_cnt].draw(&(this->gLed[THRCOLOR_FRE-1-light_cnt]), thrcolor_maincolor2[0], thrcolor_maincolor2[1], thrcolor_maincolor2[2]);
	this->gLed[light_cnt].draw(&(this->gLed[light_cnt]), thrcolor_maincolor1[0], thrcolor_maincolor1[1], thrcolor_maincolor1[2]);

}

void thrcolor_doframe(void *param, void *ptr, u32 timeTick)
{

	LightParam *plightParam =  (LightParam*)param;
	Animator *this = ptr;

	u32 cTick = plightParam->cycleMs/HW_TIMER_BASE_TOMS;

	//DEBUG_LOG("%s cTick:%d %d timeTick:%d\n", __func__, cTick, sizeof(flying_effect_arry), timeTick);

	if(cTick < (THRCOLOR_FRE * 2)) {
		cTick = 1;
	} else {
		cTick = cTick / THRCOLOR_FRE;
	}

	if(timeTick%cTick == 0) {
		thrcolor_bright(ptr, timeTick%cTick, cTick);
	}
}


/*******************************************************************/
/*******************************************************************/
/*************************LIGHT_BUFFDELAY MODULE*****************************/
/*******************************************************************/

#define BUFFDELAY_FRE	18

static u8 buffdelay_maincolor1[] = {0xFF, 0xFF, 0x0};
static u8 buffdelay_maincolor2[] = {0x0, 0x0, 0xFF};

static u8 buffdelay_auto_colours;

u8 buffdelay_bright(void *ptr, u32 timeTick, u32 CycleTickSW)
{
	Animator *this = ptr;
	static u8 light_cnt = 0;

	light_cnt++;

	if(light_cnt >= BUFFDELAY_FRE) {
		light_cnt = 0;
	}

	//DEBUG_LOG("%s %d \n", __func__, light_cnt);
	if(light_cnt < 12) {
		this->gLed[light_cnt].draw(&(this->gLed[light_cnt]), buffdelay_maincolor1[0], buffdelay_maincolor1[1], buffdelay_maincolor1[2]);
	} else if(light_cnt == 12){
		for(u8 i = 0; i < 12; i++) {
			this->gLed[i].draw(&(this->gLed[i]), 0, 0, 0);
		}
	} else if(light_cnt < 14) {
		return 0;
	} else if(light_cnt == 14) {
		for(u8 i = 0; i < 12; i++) {
			this->gLed[i].draw(&(this->gLed[i]), buffdelay_maincolor2[0], buffdelay_maincolor2[1], buffdelay_maincolor2[2]);
		}
	} else if(light_cnt < 16) {
		return 0;
	} else if(light_cnt == 16) {
		for(u8 i = 0; i < 12; i++) {
			this->gLed[i].draw(&(this->gLed[i]), 0, 0, 0);
		}
	} else {
		return 1;
	}

	return 0;
}

void buffdelay_init(void *ptr)
{
	Animator *this = ptr;

	DEBUG_LOG("%s:enter\n", __func__);

	light_base_init(this);

	for(u8 i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]), 0, 0, 0);
		this->gLed[i].bright(&(this->gLed[i]),0xFF);
	}

	if(_lightParam.colorNum == 2) {
		buffdelay_auto_colours = 0;
		for(u8 i = 0; i < 3; i++) {
			buffdelay_maincolor1[i] = _lightParam.rgb[0][i];
			buffdelay_maincolor2[i] = _lightParam.rgb[1][i];
		}
	} else {
		buffdelay_auto_colours = 1;
	}
	
	DEBUG_LOG("main1 r = %x g = %x b = %x\n", buffdelay_maincolor1[0], buffdelay_maincolor1[1], buffdelay_maincolor1[2]);
	DEBUG_LOG("main2 r = %x g = %x b = %x\n", buffdelay_maincolor2[0], buffdelay_maincolor2[1], buffdelay_maincolor2[2]);
}

void buffdelay_doframe(void *param, void *ptr, u32 timeTick)
{

	LightParam *plightParam =  (LightParam*)param;
	Animator *this = ptr;

	static u8 buff_color_cnt = 0;
	static u8 current_round_done = 0;

	if(buffdelay_auto_colours) {
		for(u8 i = 0; i < 3; i++) {
			buffdelay_maincolor1[i] = light_color[buff_color_cnt][i];

			if(buff_color_cnt <= 2) {
				buffdelay_maincolor2[i] = light_color[10+buff_color_cnt][i];
			} else {
				buffdelay_maincolor2[i] = light_color[buff_color_cnt - 3][i];
			}
		}
		if(current_round_done) {
			current_round_done = 0;
			buff_color_cnt++;
			if(buff_color_cnt >= 12) {
				buff_color_cnt = 0;
			}
		}
	}

	u32 cTick = plightParam->cycleMs/HW_TIMER_BASE_TOMS;

	//DEBUG_LOG("%s cTick:%d %d timeTick:%d\n", __func__, cTick, sizeof(flying_effect_arry), timeTick);

	if(cTick < (BUFFDELAY_FRE * 2)) {
		cTick = 1;
	} else {
		cTick = cTick / BUFFDELAY_FRE;
	}

	if(timeTick%cTick == 0) {
		current_round_done = buffdelay_bright(ptr, timeTick%cTick, cTick);
	}
}

/*****************************************/

#define BOUNCEV1_FRE 12

/*
1. 红色：255,0,0——web颜色值：#ff0000

2. 橙红：255,51,0——web颜色值：#ff3300

3. 橙色：255,102,0——web颜色值：#ff6600

4. 橙黄：255,153,0——web颜色值：#ff9900

5. 黄色：255,255,0——web颜色值：#ffff00

6. 黄绿：153,255,0——web颜色值：#99ff00

7. 绿色：0,255,0——web颜色值：#00ff00

8. 蓝绿：0,255,255——web颜色值：#00ffff

9. 蓝色：0,0,255——web颜色值：#0000ff

10. 蓝紫：102,0,255——web颜色值：#6600ff

11. 紫色：255,0,255——web颜色值：#ff00ff

12. 紫红：255,0,102——web颜色值：ff0066
*/

// static u8 buffer_new1_light_color[9][3] = {{0xFF, 0x0, 0x0},  
// 											{0xFF, 0x7F, 0x0}, 
// 											{0x80, 0xFF, 0x0}, 
// 											{0x0, 0xFF, 0x0}, 
// 											{0x0, 0xFF, 0x7F},
// 											{0x0, 0x80, 0xFF},
// 											{0x0, 0x0, 0xFF},
// 											{0x7F, 0x0, 0xFF},
// 											{0xFF, 0x0, 0x80}};

const u8 bounceV1_light_color_default[6][3] = {{0xFF, 0x0, 0x0},  
											{0x0, 0xFF, 0x0}, 
											{0x0, 0x0, 0xFF},
											{0xFF, 0xFF, 0x0}, //yellow
											{0x0, 0xFF, 0xFF},	//cyan
											{0xFF, 0x0, 0xFF}};	//purple

static u8 bounceV1_light_color[9][3] = {0};

static u8 bounceV1_maincolor[] = {0xFF, 0xFF, 0x0};

static u8 direction_lock = 0;
static u8 buff_color_cnt_max = 0;

u8 bounceV1_bright(void *ptr, u32 timeTick, u32 CycleTickSW)
{
	Animator *this = ptr;
	static u8 light_cnt = 0;
	u8 current_light = 0;
	static u8 light_direction = 0;

	light_cnt++;

	if(light_cnt >= BOUNCEV1_FRE) {
		light_cnt = 0;
		if(!direction_lock) {
			light_direction = ~light_direction;
		}
	}

	current_light = light_direction ? (11 - light_cnt) : light_cnt;

	//DEBUG_LOG("%s %d \n", __func__, light_cnt);
	if(light_cnt < 12) {
		this->gLed[current_light].draw(&(this->gLed[current_light]), bounceV1_maincolor[0], bounceV1_maincolor[1], bounceV1_maincolor[2]);
	} 

	if(light_cnt == 11) {
		return 1;
	}

	return 0;
}

void bounceV1_init(void *ptr)
{
	Animator *this = ptr;

	DEBUG_LOG("%s:enter\n", __func__);

	light_base_init(this);

	direction_lock = 0;
	buff_color_cnt_max = sizeof(bounceV1_light_color)/sizeof(bounceV1_light_color[0]);

	if(_lightParam.colorNum == 6) {
		for(int i = 0; i < 6; i++) {
			bounceV1_light_color[i][0] = _lightParam.rgb[i][0];
			bounceV1_light_color[i][1] = _lightParam.rgb[i][1];
			bounceV1_light_color[i][2] = _lightParam.rgb[i][2];
		}
	} else {
		for(int i = 0; i < 6; i++) {
			bounceV1_light_color[i][0] = bounceV1_light_color_default[i][0];
			bounceV1_light_color[i][1] = bounceV1_light_color_default[i][1];
			bounceV1_light_color[i][2] = bounceV1_light_color_default[i][2];
		}
		if(_lightParam.colorNum == 2) {
			direction_lock = 1;
			buff_color_cnt_max = 7;
		}
	}


	for(u8 i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]), 0, 0, 0);
		this->gLed[i].bright(&(this->gLed[i]),0xFF);
	}
}

void bounceV1_doframe(void *param, void *ptr, u32 timeTick)
{

	LightParam *plightParam =  (LightParam*)param;
	Animator *this = ptr;

	static u8 buff_color_cnt = 0;
	static u8 current_round_done = 0;

	if(current_round_done) {
		for(u8 i = 0; i < 3; i++) {
			bounceV1_maincolor[i] = bounceV1_light_color[buff_color_cnt][i];
		}

		buff_color_cnt++;

		if(buff_color_cnt >= buff_color_cnt_max) {
			buff_color_cnt = 0;
		}
		current_round_done = 0;
	}

	u32 cTick = plightParam->cycleMs/HW_TIMER_BASE_TOMS;

	if(plightParam->colorNum == 0) { 
		cTick = 300/HW_TIMER_BASE_TOMS;
	}

	//DEBUG_LOG("%s cTick:%d %d timeTick:%d\n", __func__, cTick, sizeof(flying_effect_arry), timeTick);

	if(cTick < (BOUNCEV1_FRE * 2)) {
		cTick = 1;
	} else {
		cTick = cTick / BOUNCEV1_FRE;
	}

	if(timeTick%cTick == 0) {
		current_round_done = bounceV1_bright(ptr, timeTick%cTick, cTick);
	}
}


/************************************************************/
#define BOUNCEV2_FRE 28

const u8 bounceV2_light_color_default[5][3] = {{0xFF, 0x0, 0x0}, {0xFF, 0xFF, 0x0}, {0x0, 0xFF, 0x0}, {0x0, 0xFF, 0xFF}, {0x0, 0x0, 0xFF}};
static u8 bounceV2_light_color[5][3] = {0};

u8 bounceV2_bright(void *ptr, u32 timeTick, u32 CycleTickSW)
{
	Animator *this = ptr;
	static u8 light_cnt = 0;

	if(light_cnt >= BOUNCEV2_FRE) {
		light_cnt = 0;
	}

	if(light_cnt <= 7) {
		if(light_cnt >= 1) {
			this->gLed[light_cnt-1].draw(&(this->gLed[light_cnt-1]), 0x0, 0x0, 0x0);
		}
		for(u8 i = 0; i < 5; i++) {
			this->gLed[light_cnt+i].draw(&(this->gLed[light_cnt+i]), bounceV2_light_color[i][0], bounceV2_light_color[i][1], bounceV2_light_color[i][2]);
		} 
	} else if(light_cnt <= 10) {
		this->gLed[light_cnt-1].draw(&(this->gLed[light_cnt-1]), 0x0, 0x0, 0x0);
		for(u8 i = 0; i < 5 - (light_cnt - 7); i++) {
			this->gLed[light_cnt+i].draw(&(this->gLed[light_cnt+i]), bounceV2_light_color[i][0], bounceV2_light_color[i][1], bounceV2_light_color[i][2]);
		}
	} else if(light_cnt == 11) {

	}else if(light_cnt <= 14) {
		for(u8 i = 0; i < 3 + (light_cnt - 12); i++) {
			this->gLed[9-(light_cnt-12)+i].draw(&(this->gLed[9-(light_cnt-12)+i]), bounceV2_light_color[i][0], bounceV2_light_color[i][1], bounceV2_light_color[i][2]);
		}
	} else if(light_cnt <= 21) {
		for(u8 i = 0; i < 5 ; i++) {
			this->gLed[6-(light_cnt-15)+i].draw(&(this->gLed[6-(light_cnt-15)+i]), bounceV2_light_color[i][0], bounceV2_light_color[i][1], bounceV2_light_color[i][2]);
		}
		this->gLed[11-(light_cnt-15)].draw(&(this->gLed[11-(light_cnt-15)]), 0x0, 0x0, 0x0);
	} else if(light_cnt <= 24) {
		for(u8 i = 0; i < 5 - (light_cnt - 21) ; i++) {
			this->gLed[i].draw(&(this->gLed[i]), bounceV2_light_color[i][0], bounceV2_light_color[i][1], bounceV2_light_color[i][2]);
		}
		this->gLed[4-(light_cnt-22)].draw(&(this->gLed[4-(light_cnt-22)]), 0x0, 0x0, 0x0);
	} else if(light_cnt == 25) {

	} else if(light_cnt <= 27) {
		for(u8 i = 0; i < 2 + (light_cnt - 25) ; i++) {
			this->gLed[i].draw(&(this->gLed[i]), bounceV2_light_color[i][0], bounceV2_light_color[i][1], bounceV2_light_color[i][2]);
		}
	} 

	light_cnt++;

	return 0;
}

void bounceV2_init(void *ptr)
{
	Animator *this = ptr;

	DEBUG_LOG("%s:enter\n", __func__);

	if(_lightParam.colorNum == 1) {
		for(int i = 0; i < sizeof(bounceV2_light_color)/sizeof(bounceV2_light_color[0]); i++) {
			bounceV2_light_color[i][0] = _lightParam.rgb[0][0];
			bounceV2_light_color[i][1] = _lightParam.rgb[0][1];
			bounceV2_light_color[i][2] = _lightParam.rgb[0][2];
		}
	} else if(_lightParam.colorNum == 5) {
		for(int i = 0; i < sizeof(bounceV2_light_color)/sizeof(bounceV2_light_color[0]); i++) {
			bounceV2_light_color[i][0] = _lightParam.rgb[i][0];
			bounceV2_light_color[i][1] = _lightParam.rgb[i][1];
			bounceV2_light_color[i][2] = _lightParam.rgb[i][2];
		}
	} else {
		for(int i = 0; i < sizeof(bounceV2_light_color)/sizeof(bounceV2_light_color[0]); i++) {
			bounceV2_light_color[i][0] = bounceV2_light_color_default[i][0];
			bounceV2_light_color[i][1] = bounceV2_light_color_default[i][1];
			bounceV2_light_color[i][2] = bounceV2_light_color_default[i][2];
		}
	}

	DEBUG_LOG("setting light num = %d  %d\n", _lightParam.colorNum, sizeof(bounceV2_light_color)/sizeof(bounceV2_light_color[0]));

	light_base_init(this);

	for(u8 i = 0; i < 12; i++) {
		this->gLed[i].draw(&(this->gLed[i]), 0, 0, 0);
		this->gLed[i].bright(&(this->gLed[i]),0xFF);
	}
}

void bounceV2_doframe(void *param, void *ptr, u32 timeTick)
{

	LightParam *plightParam =  (LightParam*)param;
	Animator *this = ptr;

	u32 cTick = plightParam->cycleMs/HW_TIMER_BASE_TOMS;

	if(plightParam->colorNum == 0) { 
		cTick = 1500/HW_TIMER_BASE_TOMS;
	}

	//DEBUG_LOG("%s cTick:%d %d timeTick:%d\n", __func__, cTick, sizeof(flying_effect_arry), timeTick);

	if(cTick < (BOUNCEV2_FRE * 2)) {
		cTick = 1;
	} else {
		cTick = cTick / BOUNCEV2_FRE;
	}

	if(timeTick%cTick == 0) {
		bounceV2_bright(ptr, timeTick%cTick, cTick);
	}
}

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

void render_run(void)
{
	_render.anr->doframe((void *)&_lightParam,_render.anr,_render.timeTick);
}

void render_init(u8 animType)	/*choose LED mode**/
{
	_render.animType = animType;
	_render.anr=&_anim_light;
	_render.anr->init = light_base_init;
	switch(animType)
	{
	case LIGHT_MODE_OFF:
		_render.anr->doframe = off_doframe;
		break;
	case LIGHT_MODE_RAINBOW:
		_render.anr->doframe = rainbow_doframe;
		break;
	case LIGHT_MODE_LEDBREATH:
		_render.anr->init = light_breath_init;
		_render.anr->doframe = bs_breath_doframe;
		break;
	case LIGHT_MODE_STARLIGHT:
		_render.anr->init = starlight_init;
		_render.anr->doframe = starlight_doframe;
		break;
	case LIGHT_MODE_FLYINGRING:
		_render.anr->init = flyingring_init;
		_render.anr->doframe = flyingring_doframe;
		break;
	case LIGHT_MODE_ALLON:
		_render.anr->init = allon_init;
		_render.anr->doframe = allon_doframe;
		break;
	case LIGHT_MODE_CHASE:
		_render.anr->init = flow_chase_init;
		_render.anr->doframe = flow_chase_doframe;
		break;
	case LIGHT_MODE_FLOW:
		_render.anr->init = flow_chase_init;
		_render.anr->doframe = flow_chase_doframe;
		break;
	case LIGHT_MODE_THRCOLOR:
		_render.anr->init = thrcolor_init;
		_render.anr->doframe = thrcolor_doframe;
		break;
	case LIGHT_MODE_BUFFDELAY:
		_render.anr->init = buffdelay_init;
		_render.anr->doframe = buffdelay_doframe;
		break;
	case LIGHT_MODE_BOUNCEV1:
		_render.anr->init = bounceV1_init;
		_render.anr->doframe = bounceV1_doframe;
		break;
	case LIGHT_MODE_BOUNCEV2:
		_render.anr->init = bounceV2_init;
		_render.anr->doframe = bounceV2_doframe;
		break;


	case LIGHT_MODE_RED:
		_render.anr->doframe = r_doframe;
		break;
	case LIGHT_MODE_GREEN:
		_render.anr->doframe = g_doframe;
		break;
	case LIGHT_MODE_BLUE:
		_render.anr->doframe = b_doframe;
		break;
	case LIGHT_MODE_WHITE:
		_render.anr->doframe = white_doframe;
		break;
	default:
		_render.anr->doframe = rainbow_doframe;
		_render.animType = LIGHT_MODE_RAINBOW;
		break;
	}

	_render.anr->init(_render.anr);
}

void light_reinitialize(void)
{
	_lightParam.typeBak = LIGHT_MODE_DISABLE;
}

void light_timer_process(void)
{
	if ((_render.animType == NULL) || (_lightParam.type != _render.animType) || (_lightParam.typeBak == LIGHT_MODE_DISABLE))
	{
		_lightParam.typeBak = _lightParam.type;
		DEBUG_LOG("Light set_lightParam.type = %d,_render.animType = %d \r\n",_lightParam.type,_render.animType);
		_render.init(_lightParam.type);
	} 
	_render.run();
	_render.timeTick ++;
}


void i2c_led_init(void)
{
	i2c_master_init(0x68, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*I2C_CLK_SPEED)) ); // 400KHz
}

void light_default_init(void) 
{
	_lightParam.cycleMs = 3600;
	_lightParam.type = LIGHT_MODE_RAINBOW;
}

void light_init(void)
{
	i2c_led_init();

	_render.anr = NULL;
	_render.timeTick = 0;
	_render.animType = LIGHT_MODE_DISABLE;
	_render.init = render_init;
	_render.run = render_run;

	light_default_init();

	 bs_light_read_flash();
	 bs_light_update();

	DEBUG_LOG("Light init ok.Set type = %d.\r\n",_lightParam.type);
}
