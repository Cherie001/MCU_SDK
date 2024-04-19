#include "app_mac.h"
#include "drivers.h"
#include "bs_protocol.h"

static int bs_mac_val_process(bs_cmd_t *cmd)
{

	u8 mac_public[6];
	u8 mac_random_static[6];
	blc_initMacAddress(CFG_ADR_MAC, mac_public, mac_random_static);

    bs_cmd_t *data = cmd;

    data->head.bit.length = sizeof(bs_cmd_t) + 1 + sizeof(mac_public);

    data->param[0] = 1;


    memcpy(data->param+1,mac_public,sizeof(mac_public));

    bs_prot_cmd_send(data);

    return 0;
}


static int defalut_pack_process(bs_cmd_t *cmd)
{
	return 0;
}

static bs_cmd_node_t mac_cmd_tbl[] = {
		{BS_MAC_CTRL_CMD, defalut_pack_process, bs_mac_val_process},		// 00
};

int mac_profile_init(void)
{
	return bs_prot_cmd_tbl_register(BS_CMD_SETID_MAC, ARRAY_SIZE(mac_cmd_tbl), NULL, &mac_cmd_tbl[0]);
}
