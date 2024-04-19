//#include "bs_protocol.h"
//#include "drivers.h"
//#include "string.h"
//#include "log.h"
//#include "app_debug.h"
//
//
//static int bs_debug_open(bs_cmd_t *cmd)
//{
//    bs_cmd_t *data = cmd;
//
//    u8 debug_flag = data->param[0];
//
//    debug_log(debug_flag);
//
//    DEBUG_LOG("debug_flag = %d\r\n",debug_flag);
//
//	data->param[0] = 0x01;
//    bs_prot_cmd_send(data);
//
//    return 0;
//}
//
//
//static int default_pack_process (bs_cmd_t *cmd)
//{
//    return 0;
//}
//
//static bs_cmd_node_t bs_debug_cmd_tbl[] = {
//		{0x00, default_pack_process, bs_debug_open},
//};
//
//int debug_profile_init(void)
//{
//	return bs_prot_cmd_tbl_register(BS_CMD_SETID_DEBUG, ARRAY_SIZE(bs_debug_cmd_tbl), NULL, &bs_debug_cmd_tbl[0]);
//}
