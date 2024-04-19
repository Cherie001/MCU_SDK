#include "fan.h"
#include "cool.h"
#include "app_cool.h"
#include "tl_common.h"
#include "bs_protocol.h"
#include <stack/ble/ble.h>

extern u8 tec_time_sync_mark;
u8 gsmart_level = 54;

/**
  * @brief  bs_cool_boot_ctrl.
  * @param  None
  * @retval cmd send result.
  */
static int bs_cool_set_level(bs_cmd_t *cmd)
{
    static u8 voltage_level = BR31_COOL_1;
    bs_cmd_t *data = cmd;
    u8 cool_level = data->param[0];

    DEBUG_LOG("BS APP set Fan level  = %d\r\n",cool_level);

    if(gpio_read(GPIO_TEC_EN) == 0)
    {
    	pwm_start(PWM5_ID);
    	TEC_ENABLE();
    }

    if(cool_level == 0)
    {
		cool_level_set(cool_level);
    }

    if(cool_level == 0xFA)
    {
    	tec_time_sync_mark = 1;
    	v_reference_tec    = 0x64;
		cool_level_set(BR31_COOL_1);
    }
    else
    {
        if(voltage_level > cool_level)
        {
        	tec_time_sync_mark = 1;
            tec_inbox_switch = 1;
        }
        else
    	{
        	cool_level_set(cool_level);
    	}
    }

    voltage_level = cool_level;

//    if((cool_level == 0x64) || (cool_level == 0x5E) || (cool_level == 0x4C) || (cool_level == 0x08) || (cool_level == 0x2A) || (cool_level == 0xFA))
    {
    	cool_level_save(cool_level);
    }

	data->param[0] = 0x01;
	data->head.bit.length = sizeof(bs_cmd_t) + 1;
    bs_prot_cmd_send(data);
    return 0;
}

static int bs_cool_get_level(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

    data->param[0] = cool_level_get();

    DEBUG_LOG("BS APP get fan level =%d.\r\n",data->param[0]);

    bs_prot_cmd_send(data);
    return 0;
}

static int bs_cool_get_smartlevel(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

    data->param[0] = cool_level_get();

    DEBUG_LOG("BS APP get fan level =%d.\r\n",data->param[0]);

    bs_prot_cmd_send(data);
    return 0;
}
/**
  * @brief  default_pack_process.
  * @param  None
  * @retval None
  */
static int default_pack_process (bs_cmd_t *cmd)
{
    return 0;
}

/**
  * @brief  bs_cool_module_tbl.
  * @param  None
  * @retval None
  */
static bs_cmd_node_t bs_cool_module_tbl[] = {
	{BS_COOL_CMD_CTRL, default_pack_process, bs_cool_set_level},    	// 00 Control cool enable and level.           // 08
    {BS_COOL_CMD_REGULATE, default_pack_process, bs_cool_get_level},	// 08
    {BS_COOL_CMD_SMARTLEVEL, default_pack_process, bs_cool_get_smartlevel},	// 10
};

/**
  * @brief  bs_cool_module_init.
  * @param  None
  * @retval None
  */
int cool_profile_init(void)
{
	return bs_prot_cmd_tbl_register(BS_CMD_SETID_COOL, ARRAY_SIZE(bs_cool_module_tbl), NULL, &bs_cool_module_tbl[0]);
}
