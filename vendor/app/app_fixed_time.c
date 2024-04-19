#include "app_fixed_time.h"
#include "bs_protocol.h"
#include "fixed_time.h"
#include "drivers.h"
#include "string.h"

static int bs_time_shut_down_set(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

    shut_down_time = data->param[0];

    if(shut_down_time)
    {
    	shut_down_flag = 1;
    }else
    {
        shut_down_flag = 0;
    }

    DEBUG_LOG("system shut down time = %d\r\n",shut_down_time);

	data->param[0] = 0x01;
	data->head.bit.length = sizeof(bs_cmd_t) + 1;
    bs_prot_cmd_send(data);

    return 0;
}

static int bs_time_shut_down_get(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

	data->param[0] = 0x01;

	data->head.bit.length = sizeof(bs_cmd_t) + 1 + sizeof(u8) + sizeof(u16);
    memcpy(data->param+1,&shut_down_time,sizeof(u8));

    memcpy(data->param+2,&gDownTick,sizeof(u16));

    bs_prot_cmd_send(data);

    return 0;
}

static int default_pack_process (bs_cmd_t *cmd)
{
    return 0;
}

static bs_cmd_node_t bs_time_cmd_tbl[] = {
		{0x00, default_pack_process, bs_time_shut_down_set},
		{0x01, default_pack_process, bs_time_shut_down_get}
};

int time_profile_init(void)
{
	return bs_prot_cmd_tbl_register(BS_CMD_SETID_TIME, ARRAY_SIZE(bs_time_cmd_tbl), NULL, &bs_time_cmd_tbl[0]);
}



