#ifndef __FP6606_TCPM_H__
#define __FP6606_TCPM_H__


#include <tl_common.h>
#include "FP6606_common.h"


#define NUM_TCPC_DEVICES   1

#define LSB_MV  25
#define SET_MV(mv)        (mv/LSB_MV)

/**
  * @brief  GPIO Bit SET and Bit RESET enumeration
  */
typedef enum
{
  GPIO_PIN_RESET = 0,
  GPIO_PIN_SET
}GPIO_PinState;



/* Sink with Accessory Support is NOT supported by the state machine */

typedef enum
{
    TCPC_STATE_UNATTACHED_SNK = 0,
    TCPC_STATE_ATTACH_WAIT_SNK,			//1
    TCPC_STATE_WAITING_FOR_VBUS_SNK,	//2
    TCPC_STATE_ATTACHED_SNK,			//3
    TCPC_STATE_DEBUG_ACC_SNK,			//4
    TCPC_STATE_ERROR_RECOVERY,			//5
    TCPC_STATE_DISABLED  				//6 /* no CC terminations */
} tcpc_state_t;

typedef enum
{
    PLUG_UNFLIPPED = 0,            /* USB-PD comm on CC1 */
    PLUG_FLIPPED,                  /* USB-PD comm on CC2 */
} plug_polarity_t;

typedef enum
{
	PD_CONNECT_MODE = 0,
	QC_CONNECT_MODE
}tcpc_connect_mode_e;


typedef struct
{
    unsigned short    	 flags;
    u8 	   				 pd_ver;
    u8      			 port;
    tcpc_state_t     	 state;
    tcpc_state_t     	 last_state;    /* for debug */
    bool              	 state_change;
    bool              	 vbus_present;
    tc_role_t         	 role;
    tcpc_role_rp_val_t	 rp_val;
    u8      			 cc_status;
    plug_polarity_t   	 plug_polarity;
    bool              	 msg_hdrst;
    u8     				 revision;
	u8     				 connect_mode;

} tcpc_device_t;











char tcpm_init(const tcpc_config_t *config);

tcpc_device_t* tcpm_get_device(void);
void tcpm_enable_pd_receive(void);
void tcpm_error_recovery(void);
void tcpm_set_state(tcpc_device_t *dev, tcpc_state_t new_state);
bool tcpm_is_vbus_present(void);
void pe_switch_qc_state(void);
void tcpm_connection_task(void);
void GPIO_FP_INT_PA0(void);
void pd_voltage_check();
void pd_to_qc(void);











#endif //__TCPM_H__
