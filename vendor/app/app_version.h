#ifndef _APP_VERSION_H_
#define _APP_VERSION_H_


#define REG_VER_BASE_ADDR			0x80007D
#define read_reg_ver()			    (*(volatile unsigned char*) (REG_VER_BASE_ADDR))

typedef enum
{
    BS_FTPP_ECHO_ACESS_DET      = 0x00,
    BS_FTPP_BUT_COOL_STATE      = 0x01,
    BS_FTPP_HALL_SWTICH_STATE   = 0x02,
    BS_FTPP_GET_MAC_ADDR        = 0x03,
    BS_FTPP_GET_VENDOR_INFO     = 0x04,
    BS_FTPP_GET_FACTORY_INFO    = 0x05,
    BS_FTPP_GET_SW_VERSION      = 0x06,
    BS_FTPP_QUERY_BAT_PER       = 0x07,
    BS_FTPP_REPORT_BAT_PER      = 0x08,
    BS_FTPP_SET_FACTORY_TEST    = 0x09,
}bs_ftpp_cmd_t;

int version_profile_init(void);

#endif

