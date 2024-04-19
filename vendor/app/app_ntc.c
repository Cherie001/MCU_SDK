#include "ntc.h"
#include "gap.h"
#include "drivers.h"
#include "app_ntc.h"
#include "adc_sample.h"
#include "bs_protocol.h"
#include "digital_tube.h"

extern u8  hot_temp_reset;

static u8 fac_test_flag = 0;

static int bs_adc_ntc_fac_get_val_process(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

    data->head.bit.length = sizeof(bs_cmd_t) + 1 + sizeof(u16) + sizeof(u16) + sizeof(u8);

    data->param[0] = 1;

    static u8 record01_temp = 0;
    static u8 record01_flag = 0;
    static u8 record02_temp = 0;

    if(fac_test_flag) {
        if(!record01_flag) {
            record01_temp = g_temp_value;
            record01_flag = 1;
            if(record01_temp < 246 && record01_temp > 40 || record01_temp == 0) {  //-10 ~ 40 !0
                record01_temp = 0xFF;
            } else {
                if(record01_temp > 127) {
                    record01_temp-=128;
                } else {
                    record01_temp+=128;
                }
            }
            data->param[1] = record01_temp;
            DEBUG_LOG("BS first fac get NTC = 0x%X.\r\n",data->param[1]);
        } else {
    //        record01_flag = 0;
            record02_temp = g_temp_value;
            if(record01_temp == 0xFF) {
                record02_temp = 0xFF;
            } else {
                if(record02_temp > 127) {
                    record02_temp-=128;
                } else {
                    record02_temp+=128;
                }
            }
            data->param[1] = record02_temp;
            DEBUG_LOG("BS second fac get NTC = 0x%X.\r\n",data->param[1]);
        }

        DEBUG_LOG("BS fac get NTC = 0x%X.\r\n",g_temp_value);
    } else {
        data->param[1] = g_temp_value;
        DEBUG_LOG("BS get NTC = 0x%X.\r\n",g_temp_value);
    }

    data->param[2] = 0;

//    memcpy(data->param+1,&gNtcData,sizeof(u16));
//    memcpy(data->param+1,&g_temp_value,sizeof(u16));
    memcpy(data->param+3,&g_temp_c,sizeof(u16));
    memcpy(data->param+5,&hot_temp_reset,sizeof(u8));

    bs_prot_cmd_send(data);

    tube_test_flag = 0;

    return 0;
}

static int bs_adc_ntc_app_get_val_process(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

    data->head.bit.length = sizeof(bs_cmd_t) + 1 + sizeof(u16) + sizeof(u16) + sizeof(u8);

    data->param[0] = 1;

    DEBUG_LOG("BS APP get NTC = 0x%X.\r\n",g_temp_value);

    data->param[1] = g_temp_value;
    data->param[2] = 0;

//    memcpy(data->param+1,&gNtcData,sizeof(u16));
//    memcpy(data->param+1,&g_temp_value,sizeof(u16));
    memcpy(data->param+3,&g_temp_c,sizeof(u16));
    memcpy(data->param+5,&hot_temp_reset,sizeof(u8));

    bs_prot_cmd_send(data);

    tube_test_flag = 0;

    return 0;
}

static int bs_adc_ntc_tubetest_process(bs_cmd_t *cmd)
{
	bs_cmd_t *data = cmd;

	tube_test_flag = data->param[0];

	data->param[0] = 0x01;

	bs_prot_cmd_send(data);
    return 0;
}

static int bs_adc_ntc_disable(bs_cmd_t *cmd)
{
	bs_cmd_t *data = cmd;

	bond_flag = data->param[0];

	data->param[0] = 0x01;

    fac_test_flag = 1;

	bs_prot_cmd_send(data);

    return 0;
}

static int bs_adc_ntc_read(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

    data->head.bit.length = sizeof(bs_cmd_t) + 1 + sizeof(u16) + sizeof(u16);

    data->param[0] = 1;

    adc_sample_tec();
    adc_sample_vbus();

    memcpy(data->param+1,&tec_sample_voltage,sizeof(u16));
    memcpy(data->param+3,&vbus_sample_voltage,sizeof(u16));

    bs_prot_cmd_send(data);

    return 0;
}

static int debug_bs_adc_ntc_type_read(bs_cmd_t *cmd)
{
    bs_cmd_t *data = cmd;

    data->head.bit.length = sizeof(bs_cmd_t) + 1 + sizeof(u8) + sizeof(u8);

    data->param[0] = 1;
    data->param[1] = ntc_get_type();
    data->param[2] = ntc_get_temper_offset();

    DEBUG_LOG("BS APP get NTC Type = 0x%x current offset = %d\r\n",data->param[1], data->param[2]);
    bs_prot_cmd_send(data);

    return 0;
}

static int defalut_pack_process(bs_cmd_t *cmd)
{
	return 0;
}

static bs_cmd_node_t adc_cmd_tbl[] = {
		{ADC_NTC_VAL, defalut_pack_process, bs_adc_ntc_fac_get_val_process},		// 00
		{ADC_NTC_TUBE, defalut_pack_process, bs_adc_ntc_tubetest_process},	// 08
		{ADC_NTC_DISABLE, defalut_pack_process, bs_adc_ntc_disable},		// 10
		{ADC_NTC_READ, defalut_pack_process, bs_adc_ntc_read},				// 18


        /* for debug*/
        {ADC_NTC_TYPE_READ, defalut_pack_process, debug_bs_adc_ntc_type_read}, // 20

};

int ntc_profile_init(void)
{
	return bs_prot_cmd_tbl_register(BS_CMD_SETID_ADC, ARRAY_SIZE(adc_cmd_tbl), NULL, &adc_cmd_tbl[0]);
}
