#include "fan.h"
#include "cool.h"
#include "string.h"
#include "drivers.h"
#include "bs_protocol.h"
#include "FP6606_tcpm.h"
#include "app_machine.h"

u16 pd_voltage = 0;
u16 qc_voltage = 0;

u8 dev_state_change = 0;

static int bs_machine_switch(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

	data->param[0] = 0x01;

	data->head.bit.length = sizeof(bs_cmd_t) + 1;

	pd_voltage_save(0xBB);

    bs_prot_cmd_send(data);

    return 0;
}

static int pd_change_qc(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

    u8 ret = 0;

    dev_state_change = 1;

    pd_to_qc();

    ret = bs_prot_cmd_send(data);

    return ret;
}

static int read_pd_qc_voltage(bs_cmd_t *cmd)
{
	u8 voltage_get = 0;// voltage_set = 0xBB;
    bs_cmd_t *data = cmd;

//    data->head.bit.length = sizeof(bs_cmd_t) + 1 + sizeof(u16) + sizeof(u16) + sizeof(u8);
    data->head.bit.length = sizeof(bs_cmd_t) + 1 + sizeof(u16) + sizeof(u16);

    data->param[0] = 1;

	flash_get(CUST_USER_PD_VOLTAGE_ADDR,&voltage_get,sizeof(voltage_get));

	if(voltage_get == 0xBB)
	{
		pd_voltage = 400;
		pd_voltage_save(0);
	}

    memcpy(data->param+1,&pd_voltage,sizeof(u16));
    memcpy(data->param+3,&qc_voltage,sizeof(u16));
//    memcpy(data->param+5,&voltage_get,sizeof(u8));

    bs_prot_cmd_send(data);

    return 0;
}

static int default_pack_process (bs_cmd_t *cmd)
{
    return 0;
}

static bs_cmd_node_t bs_machine_cmd_tbl[] = {	// 00 08 10 18
		{0x00, default_pack_process, bs_machine_switch},	// software switch
		{0x01, default_pack_process, pd_change_qc},			// software switch
		{0x02, default_pack_process, read_pd_qc_voltage}	// read PD/QC voltage
};

int machine_profile_init(void)
{
	return bs_prot_cmd_tbl_register(BS_CMD_SETID_MACHINE, ARRAY_SIZE(bs_machine_cmd_tbl), NULL, &bs_machine_cmd_tbl[0]);
}



