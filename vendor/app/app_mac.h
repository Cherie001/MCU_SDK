#ifndef _APP_MAC_H_
#define _APP_MAC_H_


typedef enum
{
	BS_MAC_CTRL_CMD         = 0x00,
	BS_MAC_CMD_MAX
}bs_mac_cmd_t;

int mac_profile_init(void);
#endif
