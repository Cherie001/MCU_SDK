#include "app_version.h"
#include "bs_protocol.h"
#include "drivers.h"
#include "string.h"

static int bs_ftpp_get_sw_version(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

    u8 ret = 0;

    memcpy(data->param, SOFTWARE_VERSION_BUILD, strlen(SOFTWARE_VERSION_BUILD));

    sprintf(data->param + strlen(SOFTWARE_VERSION_BUILD),"%02X",read_reg_ver());

    DEBUG_LOG("BS APP get version = %s.\r\n",data->param);

    data->head.bit.length = sizeof(bs_cmd_t) + strlen(SOFTWARE_VERSION_BUILD) + 2;

    ret = bs_prot_cmd_send(data);

    return ret;
}

static int default_pack_process (bs_cmd_t *cmd)
{
    return 0;
}

static bs_cmd_node_t bs_ftpp_cmd_tbl[] = {
		{BS_FTPP_GET_SW_VERSION, default_pack_process, bs_ftpp_get_sw_version}
};

int version_profile_init(void)
{
	return bs_prot_cmd_tbl_register(BS_CMD_SETID_FT, ARRAY_SIZE(bs_ftpp_cmd_tbl), NULL, &bs_ftpp_cmd_tbl[0]);
}



