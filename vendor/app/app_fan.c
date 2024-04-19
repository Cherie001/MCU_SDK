#include "fan.h"
#include "cool.h"
#include "app_fan.h"
#include "tl_common.h"
#include "bs_protocol.h"
#include "stack/ble/ble.h"


static int bs_fan_set_level(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;
    u8 fan_level = data->param[0];

    DEBUG_LOG("BS APP set Fan level  = %d\r\n",fan_level);

    if(gpio_read(GPIO_FAN_EN) == 0)
    {
    	pwm_start(PWM2_ID);
    	FAN_ENABLE();
    }

    if(fan_level == 0)
    {
    	fan_level_set(fan_level);
    }
    else if(fan_level == 0xFA)
    {
    	v_reference_fan    = 0x5B;
    	fan_level_set(BR33_FAN_1);
    }
    else
    {
    	fan_level_set(fan_level);
    }

//    if((fan_level == 0x64) || (fan_level == 0x54) || (fan_level == 0x50) || (fan_level == 0x4B) || (fan_level == 0x40) || (fan_level == 0xFA))
    {
    	fan_level_save(fan_level);
    }
	data->param[0] = 0x01;
	data->head.bit.length = sizeof(bs_cmd_t) + 1;
    bs_prot_cmd_send(data);
    return 0;
}

static int bs_fan_get_speed(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

    data->head.bit.length = sizeof(bs_cmd_t) + sizeof(gFanSpeed);
    memcpy(data->param, &gFanSpeed, sizeof(gFanSpeed));

    DEBUG_LOG("BS APP get Fan speed  = %d\r\n",gFanSpeed);

    bs_prot_cmd_send(data);
    return 0;
}

static int bs_fan_get_level(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

    data->param[0] = fan_level_get();

    DEBUG_LOG("BS APP get fan level =%d.\r\n",data->param[0]);

    bs_prot_cmd_send(data);
    return 0;
}

static int bs_fan_set_ovp(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

	data->param[0] = 0x01;
    data->head.bit.length = sizeof(bs_cmd_t) + sizeof(u16);

    memcpy(data->param, &v_reference_fan, sizeof(v_reference_fan));
    memcpy(data->param+1, &v_fan, sizeof(v_fan));

    bs_prot_cmd_send(data);
    return 0;
}

static int default_pack_process (bs_cmd_t *cmd)
{
    return 0;
}

static bs_cmd_node_t bs_fan_profile_tbl[] = {
	{BS_FAN_CTRL_CMD, default_pack_process, bs_fan_set_level},         // 00 Control fan enable and level.
    {BS_FAN_RPM_CMD, default_pack_process, bs_fan_get_speed},          // 08
    {BS_FAN_STA_CMD, default_pack_process, bs_fan_get_level},          // 10
    {BS_FAN_ABNORMAL_CMD, default_pack_process, bs_fan_set_ovp},       // 18
};


int fan_profile_init(void)
{
	return bs_prot_cmd_tbl_register(BS_CMD_SETID_FAN, ARRAY_SIZE(bs_fan_profile_tbl), NULL, &bs_fan_profile_tbl[0]);
}
