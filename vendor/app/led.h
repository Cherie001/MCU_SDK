/********************************************************************************************************
 * @file     ota.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#ifndef _LED_H
#define _LED_H

#include "app_config.h"
#include "tl_common.h"

#define LED_ON()	gpio_write(GPIO_LED, 0)
#define LED_OFF()   gpio_write(GPIO_LED, 1)

//#define LED_ON()	i2c_dma_write_byte(0x21,1,0xFF);
//#define LED_OFF()   i2c_dma_write_byte(0x21,1,0x00);

typedef enum{
    LED_MODE_ON,
    LED_MODE_OFF,
    LED_MODE_BLINK,
}LED_MODE;

typedef enum{
    LED_STA_ON,
    LED_STA_OFF,
}LED_STATUS;

typedef struct{
    u8  Mode;
    u32  Ticks;
    u16 OnTimeMs;
    u16 OffTimeMs;
    u8 status;
}LedParam_t;

void led_ble_adv_start(void);
void led_ble_adv_stop(void);
void led_ble_conn(void);
void led_ota_start(void);
void led_ota_fail(void);

void led_init(void);
void aw_ic_init(void);
void led_process(void);
void entry_ota_mode(void);
void aw_enable_config(void);
void led_red_warnning(void);
void aw_disable_config(void);
int light_profile_init(void);

#endif /* APP_H_ */
