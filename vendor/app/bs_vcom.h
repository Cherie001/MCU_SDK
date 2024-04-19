#ifndef _BS_VCOM_H_
#define _BS_VCOM_H_


#include "tl_common.h"
#include "bs_os.h"
/*============================================================================*
 *                      Define to prevent recursive inclusion
 *============================================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "bs_os.h"

/*============================================================================*
 *								Macros
 *============================================================================*/

/*============================================================================*
 *                              Types
 *============================================================================*/
typedef int (*receive)(unsigned char *buffer, int size);
typedef int (*send)(unsigned char *buffer, int size);

//static int vcom_ble_data_send(unsigned char *buffer, int size);
typedef struct _bs_vcom_t{
	bool bind;//1 - binded with protocol else - not binded
	bs_device_t	devtype;
	int vcomfd;
	char devname[BS_NAME_SIZE];
	send tx;
	receive rx;
	struct _bs_vcom_t *next;
}bs_vcom_t;




/*============================================================================*
 *                              Functions
 *============================================================================*/
/**
    * @brief    seek a bind vcom by device type
    * @param    vcom fd
    * @return	0 - mean no vcom match
    			>0 - mean seek success
    			<0 - mean error code of seek operation
    * @retval	integrate
*/
int bs_vcom_seek(bs_device_t type);

/**
    * @brief    bind paired vcom layer and protocol layer
    * @param    vcomfd - vcom fd
    			cb - protocol layer data receive trigger function
    * @return   0 - bind success else - bind failed reason code
    * @retval   integrate error code define
*/
int bs_vcom_bind(int vcomfd, receive cb);

/**
    * @brief    receive data from driver
    * @param	vcomid - fd of vcom layer
    			data - data buffer of receive packet
    			size - buffer size
    * @return	0 - mean process success else mean error code
    * @retval   integrate
*/
int bs_vcom_data_receive(int vcomfd, unsigned char *data, int size);

/**
    * @brief    send data to driver
    * @param	vcomfd - fd of vcom layer
    			data - data buffer of send packet
    			size - buffer size
    * @return	unsend size of data buffer or error code,
    			0 - mean send finish
    			>0 - mean remain some data
    			<0 - mean error occur
    * @retval   integrate
*/
int bs_vcom_data_send(int vcomfd, unsigned char *data, int size);

/**
    * @brief    register a vcom to vcom layer
    * @param
    * @return	0 - register success else - register failed reason code
    * @retval   integrate error code define
*/
int bs_vcom_register(bs_vcom_t *vcom);

/**
    * @brief    unregister a vcom from vcom layer
    * @param
    * @return	0 - unregister success else - unregister failed reason code
    * @retval   integrate error code define
*/
int bs_vcom_unregister(int fd);

#ifdef __cplusplus
}
#endif

#endif
