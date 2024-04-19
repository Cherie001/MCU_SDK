#include "bs_vcom.h"
#include "app_config.h"

static bs_vcom_t *g_vcom_head = NULL;
static int g_vcom_fd = 0;


int bs_vcom_register(bs_vcom_t *vcom) //
{
	int rc = 0;
	bs_vcom_t *node = g_vcom_head;
	bs_vcom_t *prev = NULL;

	if(NULL == vcom){
		rc = -EINVAL;
//		BS_LOG_ERROR("vcom ptr is NULL.\n");
		goto out;
	}

	if(NULL == g_vcom_head){
		g_vcom_head = vcom;
	}else{
		while(node){
			if(node->devtype == vcom->devtype){
				rc = -EEXIST;
				goto out;
			}
			prev = node;
			node = node->next;
		}
		prev->next = vcom;
	}
	vcom->next = NULL;
	vcom->rx = NULL;
	vcom->bind = FALSE;
	vcom->vcomfd = ++g_vcom_fd;
	rc = g_vcom_fd;

out:
	return rc;
}

int bs_vcom_unregister(int vcomfd)
{
	//to be continue
	return 0;
}

int bs_vcom_seek(bs_device_t type)
{
	bs_vcom_t *node = g_vcom_head;

	while(node){
		if(node->devtype == type)
			break;
		node = node->next;
	}

	return node?node->vcomfd:0;
}

int bs_vcom_bind(int vcomfd, receive cb)
{
	int rc = 0;
	bs_vcom_t *node = g_vcom_head;

	while(node){
		if(node->vcomfd == vcomfd)
			break;
		node = node->next;
	}

	if(node && (node->bind == FALSE)){
		node->bind = TRUE;
		node->rx = cb;
	}else{
		rc = -EIO;
	}

	return rc;
}

int bs_vcom_data_receive(int vcomfd, unsigned char *data, int size)
{
	int rc;
	bs_vcom_t *node = g_vcom_head;

	while(node){
		if(node->vcomfd == vcomfd)
			break;
		node = node->next;
	}

	if(node && node->rx){
		rc = node->rx(data, size);
	}else{
		rc = -EEXIST;
	}
	return rc;
}

int bs_vcom_data_send(int vcomfd, unsigned char *data, int size)
{
	int rc;
	bs_vcom_t *node = g_vcom_head;

	while(node){
		if(node->vcomfd == vcomfd)
			break;
		node = node->next;
	}

	if(node && node->tx){
		rc = node->tx(data, size);
	}else{
		rc = -EEXIST;
	}
	return rc;
}

