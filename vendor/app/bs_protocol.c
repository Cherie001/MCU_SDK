#include "bs_protocol.h"
#include "bs_vcom.h"
#include "app_config.h"
#include "bs_os.h"
#include "log.h"


static bs_cmd_table_t g_cmd_tbl[BS_CMD_SETID_UNDEFINE] = {0};

static int g_vcom_fd = 0;

static process_t get_cmd_tbl_pack_func(u8 setid, u8 command)
{
	int index;
	bs_cmd_node_t *node;
	process_t pack = NULL;

	if(g_cmd_tbl[setid].used){
		node = g_cmd_tbl[setid].node;
		for(index = 0; index < g_cmd_tbl[setid].count; index++,node++){
			if(node && (node->command == command)){
				pack = node->pack;
				break;
			}
		}
	}else{
		DEBUG_LOG("setid (%u) cmd (%u) table is unused\n.", setid, command);
	}

	return pack;
}

static process_t get_cmd_tbl_unpack_func(u8 setid, u8 command)
{
	int index;
	bs_cmd_node_t *node;
	process_t unpack = NULL;

	if(g_cmd_tbl[setid].used){
		node = g_cmd_tbl[setid].node;
		for(index = 0; index < g_cmd_tbl[setid].count; index++,node++){
			if(node && (node->command == command)){
				unpack = node->unpack;
				break;
			}
		}
	}else{
		DEBUG_LOG("setid (%u) cmd (%u) table is unused\n.", setid, command);
	}

	return unpack;
}

#if BS_MUL_THREAD
static int bs_prot_assign_comnand(uint8_t *buffer, int size)
{
	//to be continue
	return 0;
}
#else
static int bs_prot_assign_comnand(u8 *buffer, int size)
{
	int ret = 0;
	bs_cmd_t *cmd = (bs_cmd_t *)buffer;
	process_t unpack = NULL;

	if(cmd){
		if(size == cmd->head.bit.length){//command size vaild check
			unpack = get_cmd_tbl_unpack_func(cmd->head.bit.setid, cmd->head.bit.command);
			if(unpack && (0 == unpack(cmd))){
			}else{

				ret = -EIO;
			}
		}else{
			ret = -EFAULT;
		}
	}else{
		ret = -EINVAL;
	}

	return ret;
}
#endif

int bs_prot_init(void)
{
	int rc, fd = 0;
	fd = bs_vcom_seek(BS_DEV_TYPE);
	if(fd <= 0){
		rc = -EIO;
	}else{
		rc = bs_vcom_bind(fd, bs_prot_assign_comnand);
		if(0 == rc){
			g_vcom_fd = fd;
		}else{
		}
	}

	return rc;
}

int bs_prot_cmd_tbl_register(bs_cmd_setid_t setid, u8 count, notify_t notify, bs_cmd_node_t *node)
{

	if(setid >= BS_CMD_SETID_UNDEFINE || count == 0 || node == NULL){
		return -EINVAL;
	}

	if(g_cmd_tbl[setid].used){
		return -EEXIST;
	}

	g_cmd_tbl[setid].setid = setid;
	g_cmd_tbl[setid].count = count;
	g_cmd_tbl[setid].notify = notify;
	g_cmd_tbl[setid].node = node;
	g_cmd_tbl[setid].used = TRUE;

	return 0;
}

int bs_prot_cmd_tbl_unregister(bs_cmd_setid_t setid)
{
	//to be continue
	return 0;
}

/****************************
WARNING...

BS_MUL_THREAD is an option of architecture for send/recv
If this marco is enabled, we use async notify architecture for send/recv.

Recv Follow:
when a command is coming, protocol thread call unpack to process
the command and insert the unpacked command buffer in data list,
last, notify set-command thread that a new command is waiting
for process. Then, set-command thread will call bs_prot_cmd_recv
to copy the unpacked command buffer from data list.

Send Follow:
when we want to send a command, first call bs_prot_cmd_send to alloc a
command buffer and insert it in data list and wakeup protocol thread
to do follow-up work. the protocol thread will call pack function to
convert unpack command buffer to final packed command and send it out.

Else this marco is disable, we use sync architecture for send/recv.

Recv Follow:
when a command is coming, protocol thread call unpack function which is
also the notify of set-command driver layer to process the command, so
there is no need to use data list or notify function and bs_prot_cmd_recv
is useless. However the danger is all callback function binded together
if someone make a mistake, the whole call-chain will crash.

Send Follow:
when we want to send a command, set-command driver also use bs_prot_cmd_send,
But we'll not use data list to pass and cache command buffer, instead, the
set-command driver call pack function to convert unpacked command buffer to
final packed command and send it out directlly.

Now you should understand the differnts behind the Marco of BS_MUL_THREAD.
****************************/
#if BS_MUL_THREAD
int bs_prot_cmd_send(bs_cmd_t *cmd)
{
	//to be continue
	return 0;
}

int bs_prot_cmd_receive(bs_cmd_t *cmd)
{
	//to be continue
	return 0;
}

#else
int bs_prot_cmd_send(bs_cmd_t *cmd)
{
	int ret;
	process_t pack;

	if(cmd == NULL){
		ret = -EINVAL;
		goto out;
	}

	pack = get_cmd_tbl_pack_func(cmd->head.bit.setid, cmd->head.bit.command);
	if(pack && (0 == pack(cmd))){
	}else{
		ret = -EIO;
		goto out;
	}

	cmd->head.bit.dir = BS_CMD_DIR;

	ret = BsTxSendData(cmd, cmd->head.bit.length);
out:
	return ret;
}

#endif
