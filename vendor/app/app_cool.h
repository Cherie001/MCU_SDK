#ifndef _APP_COOL_H_
#define _APP_COOL_H_

extern u8 gsmart_level;

typedef enum
{
	BS_COOL_CMD_CTRL        = 0x00,
    BS_COOL_CMD_REGULATE    = 0x01,
    BS_COOL_CMD_SMARTLEVEL  = 0x02,
	BS_COOL_CMD_MAX
}bs_cool_cmd_t;

typedef enum
{
	BS_COOL_0V_STATE        = 0x00,
    BS_COOL_1V9_STATE       = 0x01,
    BS_COOL_2V5_STATE       = 0x02,
	BS_COOL_STATE_MAX
}bs_cool_work_state_t;

int cool_profile_init(void);

#endif
