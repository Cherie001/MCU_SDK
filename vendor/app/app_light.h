#ifndef _APP_LIGHT_H_
#define _APP_LIGHT_H_

#include "tl_common.h"

typedef enum
{
	LIGHT_MODE_FAC_CMD = 0x00,
	LIGHT_MODE_SET_CMD,
	LIGHT_MODE_GET_CMD,
	LIGHT_MODE_PARAM_GET_CMD,
	LIGHT_MODE_PARAM_SET_CMD,
	LIGHT_CMD_MAX,
}bs_light_cmd_t;

#pragma pack(1)
typedef struct
{
	u16 Mode;
    u16 Timeout;
    u16 Speed;
    u8 ColorCount;
    u8 rgb[12][3];
}bs_light_param_t;

#pragma pack()

void bs_light_save_flash(void);
void bs_light_read_flash(void);
void bs_light_update(void);

#endif

