/*============================================================================*
 *                      Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _BS_PROTOCOL_H_
#define _BS_PROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include "bs_os.h"

#include "tl_common.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/



/*============================================================================*
 *                              Types
 *============================================================================*/
#pragma pack(1)
typedef union{
	struct{
		u8 length:7;
		u8 dir:1;
		u8 setid:5;
		u8 reserve:3;
		u8 seqid:3;
		u8 command:5;
		u8 alignbyte;
	}bit;
	u32 value;
}bs_cmd_head_t;
#pragma pack()

#pragma pack(1)
typedef struct{
	bs_cmd_head_t head;
	u8 param[0];
}bs_cmd_t;
#pragma pack()

typedef enum{
	BS_CMD_SETID_COMPOSITE = 0,
	BS_CMD_SETID_LED       = 1,
	BS_CMD_SETID_FAN       = 2,
	BS_CMD_SETID_DONGLE    = 3,
    BS_CMD_SETID_FT        = 4,
    BS_CMD_SETID_COOL      = 5,
    BS_CMD_SETID_ADC       = 6,
    BS_CMD_SETID_PROC      = 7,
    BS_CMD_SETID_TIME      = 8,
    BS_CMD_SETID_MAC       = 9,
    BS_CMD_SETID_DEBUG     = 10,
    BS_CMD_SETID_MACHINE   = 11,
	BS_CMD_SETID_UNDEFINE  = 31,//5-bit setid, max-id forbid to use
}bs_cmd_setid_t;

typedef int (*process_t)(bs_cmd_t *cmd);
typedef void (*notify_t)(void);

typedef struct{
	u8 command;
	process_t pack;
	process_t unpack;//WARNING:cmd buffer will free after recv called
}bs_cmd_node_t;

typedef struct{
	bool used;//command vaild flag
	bs_cmd_setid_t setid;//command distinguish id
	u8 count;//command count of table
	notify_t notify;//notify only used in multi-thread condition
	bs_cmd_node_t *node;//command nodes ptr
}bs_cmd_table_t;

/*============================================================================*
 *                              Functions
 *============================================================================*/
/**
    * @brief    register a set command table to global table
    * @param    setid - command distinguish id
				count - count of command node
				notify - notify function of a set command table
				node - command node pointer
    * @return   0 - register success else - register failed reason code
    * @retval   integrate error code define
*/
int bs_prot_cmd_tbl_register(bs_cmd_setid_t setid, u8 count, notify_t notify, bs_cmd_node_t *node);

/**
    * @brief    unregister a set command table from global table
    * @param    setid - command distinguish id
    * @return   0 - unregister success else - unregister failed reason code
    * @retval   integrate error code define
*/
int bs_prot_cmd_tbl_unregister(bs_cmd_setid_t setid);

/**
    * @brief    send a unpack command to protocol layer
    * @param    cmd - unpacked command buffer
    * @return   0 - send success else - send failed reason code
    * @retval   integrate error code define
*/
int bs_prot_cmd_send(bs_cmd_t *cmd);

#if BS_MUL_THREAD
/**
    * @brief    receive a unpack command by setid from protocol layer
    * @param    cmd - unpacked command buffer
    * @return   0 - send success else - send failed reason code
    * @retval   integrate error code define
*/
int bs_prot_cmd_receive(bs_cmd_t *cmd);
#endif

/**
    * @brief    initialize the protocol layer
    * @param    NULL
    * @return   0 - init success else - init failed reason code
    * @retval   integrate error code define
*/
int bs_prot_init(void);

#ifdef __cplusplus
}
#endif

#endif
