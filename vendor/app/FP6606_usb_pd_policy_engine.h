#ifndef __FP6606_USB_PD_POLICY_ENGINE_H__
#define __FP6606_USB_PD_POLICY_ENGINE_H__

#include "FP6606_tcpm.h"
#include "FP6606_tcpci.h"
#include "global_value.h"
#include "FP6606_common.h"
#include "FP6606_usb_pd_protocol.h"


#define MAX_EXT_MSG_LEN 260        // 260-bytes
#define PD_STATE_HISTORY_LEN  8
#define MAX_SOP_NUM 7              // SOP, SOP', SOP", SOP_DBG', SOP_DBG" ,HardReset , Cable Reset
#define PD_STATE_INDEX_MASK   0x7  // bitmask based on history length

typedef enum
{
    // Sink states
    PE_SNK_STARTUP,				// 0
    PE_SNK_DISCOVERY,			// 1
    PE_SNK_WAIT_FOR_CAPS,		// 2
    PE_SNK_EVALUATE_CAPABILITY,	// 3
    PE_SNK_SELECT_CAPABILITY,	// 4
    PE_SNK_TRANSITION_SINK,		// 5
    PE_SNK_READY,				// 6
    PE_SNK_HARD_RESET,			// 7
    PE_SNK_TRANSITION_TO_DEFAULT,// 8
    PE_SNK_GIVE_SINK_CAP,		// 9
    PE_SNK_SEND_SOFT_RESET,		// 10
    PE_SNK_SOFT_RESET,			// 11
    PE_SNK_SEND_NOT_SUPPORTED,	// 12
    PE_SNK_SEND_REJECT,			// 13

    //PE_SNK_CHUNK_RECEIVED,
    PE_INIT_STATES,				// 14
    PE_NUM_STATES				// 15
} usb_pd_pe_state_t;
typedef struct
{
    supply_type_t       srcSupplyType;
    peak_current_t      srcPeakCur;
    unsigned char       pd_version;      // 0918 add
    bool                ExCap;
} snk_getSrcStatus_t;

typedef struct
{
    bool                CapMsg;
    bool                rejectPWR_Swap;
} snk_recvSrcMsg_t;

typedef struct
{
    snk_getSrcStatus_t  snk_getSrcStatus;
    snk_recvSrcMsg_t    snk_recvSrcMag;
    bool                snk_tx_ok;
} snk_cap_t;


typedef enum
{
    TCPC_TX_SOP          = 0,
    TCPC_TX_SOP_P        = 1,
    TCPC_TX_SOP_PP       = 2,
    TCPC_TX_DEBUG_SOP_P  = 3,
    TCPC_TX_DEBUG_SOP_PP = 4,
    TCPC_TX_HARD_RESET   = 5,
    TCPC_TX_CABLE_RESET  = 6,
    TCPC_TX_BIST_MODE2   = 7
} tcpc_transmit_t;



//------------------------------------
// usb pd port
//------------------------------------
typedef struct
{
    usb_pd_pe_state_t	state[PD_STATE_HISTORY_LEN];  // 8 bytes
    usb_pd_pe_state_t   *current_state;               // 4 bytes
    tcpc_transmit_t     tx_sop_type;                  // For incrementing correct msg ID when Tx is successful / 1 bytes
    tcpc_transmit_t 	rx_msg_sop_frame;			  // 5/4  add for decide sop or sop' not supported
    supply_type_t       snk_supply_type;

    unsigned char       state_idx;
    unsigned int        snk_pdo[PD_MAX_PDO_NUM];
    unsigned short		min_voltage;		          // 25mV LSB
    unsigned char       port;
    unsigned char       power_role;
    unsigned char       data_role;
    unsigned char       msg_id[MAX_SOP_NUM];          // For Tx.  Masked off to 3-bits when building msg header
    unsigned char       stored_msg_id[MAX_SOP_NUM];   // For Rx
    unsigned char       rx_msg_buf[MAX_EXT_MSG_LEN];

    unsigned char       rx_msg_data_len;
    unsigned char       rx_msg_type;
    unsigned char       rx_msg_spec_rev;              // 4/20 add / header spec rev

    // counter
    unsigned char       hard_reset_cnt;
    unsigned char		rx_ext_flag;
    unsigned char       snk_pps_req_cnt;    		  // for Ellisys SNK3.E13 / 180206
    unsigned char       object_position;              // Range: 1 - 7

    bool				state_change;
    bool                non_interruptable_ams;
    bool                pd_connected_since_attach;    // PD connected at any point since attachment */
    bool                explicit_contract;
    bool                no_response_timed_out;
    bool                wait_received;
    bool                high_pwr_cable;               // If using cable with 5A support
} usb_pd_port_t;







extern u32   rdo;
extern snk_cap_t       snk_pdo;







void usb_qc_pe_connection_state_change_handler(tcpc_state_t state);
void usb_pd_pe_connection_state_change_handler(tcpc_state_t state);
void usb_pd_prl_reset(void);
void pe_set_state(usb_pd_port_t *dev, usb_pd_pe_state_t new_state);
void usb_pd_pm_evaluate_src_caps(void);
usb_pd_port_t * usb_pd_pe_get_device(void);
void build_rdo(void);
void usb_pd_pe_notify(usb_pd_prl_alert_t prl_alert);
void usb_pd_pe_state_machine(void);
void usb_pd_init(const usb_pd_port_config_t *port_config);


//void usb_pd_prl_tx_data_msg(u8 *buf, msg_hdr_data_msg_type_t msg_type, tcpc_transmit_t sop_type, u32 ndo);







#endif













