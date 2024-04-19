#ifndef _APP_FAN_H_
#define _APP_FAN_H_


typedef enum
{
	BS_FAN_CTRL_CMD         = 0x00,
    BS_FAN_RPM_CMD          = 0x01,
    BS_FAN_STA_CMD			= 0x02,
    BS_FAN_ABNORMAL_CMD     = 0x03,
	BS_FAN_CMD_MAX
}bs_fan_cmd_t;

int fan_profile_init(void);
#endif
