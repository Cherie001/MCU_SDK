#include "FP6606_usb_pd_policy_engine.h"
#include "Fp6606_usb_pd_protocol.h"
#include "FP6606.h"
#include "FP6606_usb_pd.h"
#include "global_value.h"
#include "global_fun.h"
#include "FP6606_ARM.h"	//wellsian
#include <tl_common.h>

typedef void (*state_entry_fptr)(usb_pd_port_t *dev);


/* PD Counter */
#define N_HARD_RESET_COUNT              2
#define MSG_ID_CLEARED 					0xCE       // random value, must bigger than 7


#define T_NO_RESPONSE_MS                5000   // 4.5 - 5.5 s
#define T_TYPEC_SINK_WAIT_CAP_MS        350    // 310 - 620 ms
#define T_PS_TRANSITION_MS              500    // 450 - 550 ms
#define T_SINK_REQUEST_MS               200    // 100 - ? ms
#define T_PPS_REQUEST_MS                9000   //  9 s
#define T_SENDER_RESPONSE_MS            28     //  24 - 30 ms


#define dNUM_SNK_APDO                   1      // number of APDO in sink cap


static bool           vbus_timed_out;
static unsigned short m_vbus_onoff_Timer = 0;

usb_pd_port_t         pd[NUM_TCPC_DEVICES];
snk_cap_t             snk_pdo;
unsigned char         buf[OBJ_MAX];



static void pe_snk_startup_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("ENTER pe_snk_startup_entry. \r\n");
    tcpc_device_t *typec_dev = tcpm_get_device();

    usb_pd_prl_reset();
    dev->power_role = PD_PWR_ROLE_SNK;
    dev->non_interruptable_ams = false;
    typec_dev->msg_hdrst = false;

    pe_set_state(dev, PE_SNK_DISCOVERY);
    return;
}

static void pe_snk_discovery_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("ENTER pe_snk_discovery_entry. \r\n");
    if (dev->no_response_timed_out && \
            dev->pd_connected_since_attach && \
            (dev->hard_reset_cnt > N_HARD_RESET_COUNT))
    {
        tcpm_error_recovery();
    }
    else
    {
        vbus_timed_out = false;
        m_vbus_onoff_Timer = 1000;            // 1s
        while (!tcpm_is_vbus_present() && (vbus_timed_out == false))
        {
            if(m_vbus_onoff_Timer == 0)
            {
                vbus_timed_out = true;
            }
        }
        tcpc_modify8(TCPC_REG_POWER_CTRL, 0, TCPC_PWR_CTRL_AUTO_DISCHARGE_DISCONNECT); // 1 << 4
        pe_set_state(dev, PE_SNK_WAIT_FOR_CAPS);
    }
    return;
}

static void timeout_sink_wait_cap(void)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("ENTER timeout_sink_wait_cap. \r\n");
    usb_pd_port_t *dev = &pd[0];

//	DEBUG_LOG("dev->hard_reset_cnt = %d\r\n",dev->hard_reset_cnt);

//    if (dev->hard_reset_cnt <= N_HARD_RESET_COUNT)
//    {
//        pe_set_state(dev, PE_SNK_HARD_RESET);
//    }
//	else
//	fp_voltage_comfirm();
    {
#if USE_QC_SNK_TASK
		pe_set_state(dev, PE_INIT_STATES);
		// go to QC Snk Mode
		DEBUG_LOG("go to QC Snk Mode\r\n");
		pe_switch_qc_state();
#endif
    }
    return;
}

static void pe_snk_wait_for_caps_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("ENTER pe_snk_wait_for_caps_entry. \r\n");
    timer_start1(PD_MODE, T_TYPEC_SINK_WAIT_CAP_MS, timeout_sink_wait_cap);
    return;
}

static void pe_snk_evaluate_capability_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("ENTER pe_snk_evaluate_capability_entry. \r\n");
    timer_cancel(PD_NO_RESPONSE);

    dev->hard_reset_cnt = 0;

    dev->non_interruptable_ams = true;

    usb_pd_pm_evaluate_src_caps();	// read pd or pps number

    pe_set_state(dev, PE_SNK_SELECT_CAPABILITY);

    return;
}


void usb_pd_pe_init(usb_pd_port_config_t *config)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("ENTER usb_pd_pe_init. \r\n");
    usb_pd_port_t *dev = &pd[0];
    unsigned int i;

    pd[0].state_idx = 0;

    for (i = 0; i < PD_STATE_HISTORY_LEN; i++)
    {
        pd[0].state[i] = (usb_pd_pe_state_t)0xEE;
    }

    dev->current_state = &dev->state[0];
    dev->port = 0;
    dev->state_change = false;
    dev->pd_connected_since_attach = false;
    dev->explicit_contract = false;
    dev->no_response_timed_out = false;
    dev->hard_reset_cnt = 0;
    dev->high_pwr_cable = false;

    return;
}


static void usb_pd_pe_tx_data_msg(msg_hdr_data_msg_type_t msg_type, tcpc_transmit_t sop_type)
{
    usb_pd_port_t *dev = &pd[0];
    snk_cap_t  *dev_snkPdo = &snk_pdo;
    usb_pd_port_config_t *config = usb_pd_pm_get_config();
    u32 *pdo;
    u8  *payload_ptr = &buf[3];
    u8  ndo = 0;
    u8  pdo_idx;

//    DEBUG_LOG("msg_type = %d\r\n", msg_type);
    if (msg_type == DATA_MSG_TYPE_SNK_CAPS)
    {
        pdo = dev->snk_pdo;
        //-----------------------------------------
        // send APDO or not by deciding PD Version
        //-----------------------------------------
        if(dev_snkPdo->snk_getSrcStatus.pd_version == 2)
        {
            ndo = config->num_snk_pdos;
        }
        else                            // PD2
        {
            ndo = config->num_snk_pdos - dNUM_SNK_APDO;
        }

        for (pdo_idx = 0; pdo_idx < ndo; pdo_idx++)
        {
            *payload_ptr++ = (u8)(pdo[pdo_idx] & 0xFF);              // buf[0]
            *payload_ptr++ = (u8)((pdo[pdo_idx] & 0xFF00) >> 8);     // buf[1]
            *payload_ptr++ = (u8)((pdo[pdo_idx] & 0xFF0000) >> 16);  // buf[2]
            *payload_ptr++ = (u8)((pdo[pdo_idx] & 0xFF000000) >> 24);// buf[3]
        }
    }

    else if (msg_type == DATA_MSG_TYPE_REQUEST)
    {
        ndo = 1;

        *payload_ptr++ = (u8)(rdo & 0xFF);
        *payload_ptr++ = (u8)((rdo & 0xFF00) >> 8);
        *payload_ptr++ = (u8)((rdo & 0xFF0000) >> 16);
        *payload_ptr++ = (u8)((rdo & 0xFF000000) >> 24);
    }

    if (ndo > 0)
    {
//    	DEBUG_LOG("ndo > 0\r\n");
        usb_pd_prl_tx_data_msg(buf, msg_type, sop_type, ndo);
    }

    return;
}

static void pe_snk_select_capability_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("ENTER pe_snk_select_capability_entry 04\r\n");
    build_rdo();
    usb_pd_pe_tx_data_msg(DATA_MSG_TYPE_REQUEST, TCPC_TX_SOP);
//    DEBUG_LOG("Break pe_snk_select_capability_entry. \r\n");

    return;
}

static void timeout_ps_transition(void)
{
    usb_pd_port_t *dev = &pd[0];

    if (dev->hard_reset_cnt <= N_HARD_RESET_COUNT)
    {
        pe_set_state(dev, PE_SNK_HARD_RESET);
    }
    return;
}

static void pe_snk_transition_sink_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("ENTER pe_snk_transition_sink_entry 05\r\n");
    timer_start1(PD_MODE, T_PS_TRANSITION_MS, timeout_ps_transition);
    return;
}


static void timeout_sink_request(void)
{
    pe_set_state(&pd[0], PE_SNK_SELECT_CAPABILITY);
    return;
}

static void pe_snk_ready_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG(" \r\n");
//    DEBUG_LOG("ENTER pe_snk_ready_entry 06\r\n");
    dev->pd_connected_since_attach = true;
    dev->explicit_contract = true;
    if (dev->snk_pps_req_cnt >= 1)
    {
        //------------------------------------------
        // SNK send requests pps apdo periodically
        //------------------------------------------
        timer_start1(PPS_MODE, T_PPS_REQUEST_MS, timeout_sink_request);    //180601 add
    }
    if (dev->wait_received)
    {
        dev->wait_received = false;
        // Start SinkRequest timer.
        timer_start1(PD_MODE, T_SINK_REQUEST_MS, timeout_sink_request);
    }
    pd_voltage_check();

    return;
}

static void pe_snk_hard_reset_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG(" \r\n");
//    DEBUG_LOG("ENTER pe_snk_hard_reset_entry 07\r\n");
    tcpc_modify8(TCPC_REG_POWER_CTRL, TCPC_PWR_CTRL_AUTO_DISCHARGE_DISCONNECT, 0);

    tcpc_modify8(FP6606_REG_NMOS_SW_CTRL, NMOS_SNK_ON, 0x80);	//wellsian

    dev->hard_reset_cnt++;
	DEBUG_LOG("dev->hard_reset_cnt = %d\r\n",dev->hard_reset_cnt);

    tcpm_transmit(NULL, TCPC_TX_HARD_RESET);

    return;
}

static void timeout_no_response(void)
{
    usb_pd_port_t *dev = &pd[0];

    dev->no_response_timed_out = true;

    if (dev->hard_reset_cnt > N_HARD_RESET_COUNT)
    {
        tcpm_error_recovery();
    }
    return;
}

static void timer_start_no_response(usb_pd_port_t *dev)
{
    dev->no_response_timed_out = false;

    timer_start1(PD_NO_RESPONSE, T_NO_RESPONSE_MS, timeout_no_response);
    return;
}

static void pe_snk_transition_to_default_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG(" \r\n");
//    DEBUG_LOG("ENTER pe_snk_transition_to_default_entry 08\r\n");
    tcpc_write8(TCPC_REG_MSG_HDR_INFO, TCPC_REG_MSG_HDR_INFO_SET(0, PD_DATA_ROLE_UFP, g_PD_Ver, PD_PWR_ROLE_SRC));
    dev->data_role = PD_DATA_ROLE_UFP;

    tcpc_modify8(TCPC_REG_POWER_CTRL, TCPC_PWR_CTRL_ENABLE_VCONN, 0);

    dev->explicit_contract = false;

    timer_start_no_response(dev);

    pe_set_state(dev, PE_SNK_STARTUP);

    return;
}

static void pe_snk_give_sink_cap_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG(" \r\n");
//    DEBUG_LOG("ENTER pe_snk_give_sink_cap_entry.\r\n");
    // Request Sink Caps from policy manager.
    usb_pd_pe_tx_data_msg(DATA_MSG_TYPE_SNK_CAPS, TCPC_TX_SOP);
    return;
}

static void pe_snk_send_soft_reset_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG(" \r\n");
//    DEBUG_LOG("ENTER pe_snk_send_soft_reset_entry.\r\n");
    usb_pd_prl_tx_ctrl_msg(buf, CTRL_MSG_TYPE_SOFT_RESET, TCPC_TX_SOP);
    return;
}

static void pe_snk_soft_reset_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG(" \r\n");
//    DEBUG_LOG("ENTER pe_snk_soft_reset_entry.\r\n");
    usb_pd_prl_tx_ctrl_msg(buf, CTRL_MSG_TYPE_ACCEPT, TCPC_TX_SOP);
    return;
}

static void pe_snk_send_not_supported_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG(" \r\n");
//    DEBUG_LOG("ENTER pe_snk_send_not_supported_entry.\r\n");
    usb_pd_prl_tx_ctrl_msg(buf, CTRL_MSG_TYPE_NOT_SUPPORTED, dev->rx_msg_sop_frame);   // 0516 modified

    return;
}

static void pe_snk_send_reject_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG(" \r\n");
//    DEBUG_LOG("ENTER pe_snk_send_reject_entry.\r\n");
    usb_pd_prl_tx_ctrl_msg(buf, CTRL_MSG_TYPE_REJECT, TCPC_TX_SOP);
    return;
}

static void pe_init_state_entry(usb_pd_port_t *dev)
{
//    DEBUG_LOG(" \r\n");
//    DEBUG_LOG("ENTER pe_init_state_entry.\r\n");
    return;
}

static const state_entry_fptr pe_state_entry[PE_NUM_STATES] =
{
    pe_snk_startup_entry,                     /* PE_SNK_STARTUP                   */
    pe_snk_discovery_entry,                   /* PE_SNK_DISCOVERY                 */
    pe_snk_wait_for_caps_entry,               /* PE_SNK_WAIT_FOR_CAPS             */
    pe_snk_evaluate_capability_entry,         /* PE_SNK_EVALUATE_CAPABILITY       */
    pe_snk_select_capability_entry,           /* PE_SNK_SELECT_CAPABILITY         */
    pe_snk_transition_sink_entry,             /* PE_SNK_TRANSITION_SINK           */
    pe_snk_ready_entry,                       /* PE_SNK_READY                     */
    pe_snk_hard_reset_entry,                  /* PE_SNK_HARD_RESET                */	// 7
    pe_snk_transition_to_default_entry,       /* PE_SNK_TRANSITION_TO_DEFAULT     */ 	// 8
    pe_snk_give_sink_cap_entry,               /* PE_SNK_GIVE_SINK_CAP             */
    pe_snk_send_soft_reset_entry,             /* PE_SNK_SEND_SOFT_RESET           */
    pe_snk_soft_reset_entry,                  /* PE_SNK_SOFT_RESET                */
    pe_snk_send_not_supported_entry,          /* PE_SNK_SEND_NOT_SUPPORTED        */
    pe_snk_send_reject_entry,                 /* PE_SNK_SEND_REJECT               */
    pe_init_state_entry                      /*  PE_INIT_STATES                     */

};

usb_pd_port_t * usb_pd_pe_get_device(void)
{
    return &pd[0];
}

void usb_pd_prl_reset(void)
{
    unsigned char i;
    usb_pd_port_t *dev = usb_pd_pe_get_device();
    // Reset all msg IDs.
    for (i = 0; i < MAX_SOP_NUM; i++)
    {
        dev->msg_id[i] = 0;
        dev->stored_msg_id[i] = MSG_ID_CLEARED;
    }

    dev->non_interruptable_ams = false;
    tcpm_enable_pd_receive();
    return;
}

void pe_set_state(usb_pd_port_t *dev, usb_pd_pe_state_t new_state)
{
    dev->state[dev->state_idx] = new_state;
    dev->current_state = &dev->state[dev->state_idx];
    dev->state_idx++;
    dev->state_idx &= PD_STATE_INDEX_MASK;
    dev->state_change = true;


    DEBUG_LOG("****new_state = 0x%x*** \r\n", new_state);
//    DEBUG_LOG("dev->current_state = %d\r\n", dev->current_state[dev->state_idx]);
//	printf(" set - current_state: %s \r\n",pd_state_string[*dev->current_state]);	// wellsian debug
    return;
}


void usb_pd_pe_connection_state_change_handler(tcpc_state_t state)
{
    usb_pd_port_t *dev = &pd[0];

    switch (state)
    {
        case TCPC_STATE_UNATTACHED_SNK:
            dev->pd_connected_since_attach = false;
            dev->explicit_contract = false;
            dev->no_response_timed_out = false;
            dev->wait_received = false;
            dev->hard_reset_cnt = 0;
            dev->high_pwr_cable = false;
            dev->non_interruptable_ams = false;
            dev->snk_pps_req_cnt = 0;
            tcpc_modify8(FP6606_REG_DRP_FINISH, DRP_FINISH, 0);
            timer_cancel(ALL_MODE);
            tcpc_write8(TCPC_REG_RX_DETECT, 0);
            pe_set_state(dev, PE_INIT_STATES);
            break;

        case TCPC_STATE_ATTACHED_SNK:
            tcpc_modify8(FP6606_REG_DRP_FINISH, 0, DRP_FINISH);
            dev->data_role = PD_DATA_ROLE_UFP;
            pe_set_state(dev, PE_SNK_STARTUP);
            break;

        default:
            break;
    }

    return;
}


void usb_pd_pe_state_machine(void)
{

    usb_pd_port_t *dev = &pd[0];
//    DEBUG_LOG("dev->state_change = 0x%x.\r\n", dev->state_change);

    if (!dev->state_change)
        return;

//    DEBUG_LOG(" \r\n");
//    DEBUG_LOG("ENTER usb_pd_pe_state_machine.\r\n");

    while (dev->state_change)
    {

        dev->state_change = false;

        // Use branch table to execute "actions on entry" for the current state.
        if (*dev->current_state < PE_NUM_STATES)
        {
            pe_state_entry[*dev->current_state](dev);
        }
    }

    return;
}

static void timeout_sender_response(void)
{
    usb_pd_port_t *dev = &pd[0];

    switch (*dev->current_state)
    {
        case PE_SNK_SELECT_CAPABILITY:
            pe_set_state(dev, PE_SNK_HARD_RESET);
            break;

        default :
            break;

    }
    return;
}

static void check_PDVersion(usb_pd_port_t *dev)
{
    snk_cap_t  *dev_snkPdo = &snk_pdo;
    unsigned char msg_hdr_temp;
    if ( g_PD_Ver >= dev->rx_msg_spec_rev)
    {
        g_PD_Ver = dev->rx_msg_spec_rev;
        tcpc_read8(TCPC_REG_MSG_HDR_INFO, &msg_hdr_temp);
        msg_hdr_temp = (msg_hdr_temp & ~(0x06)) | (dev->rx_msg_spec_rev << 1);
        tcpc_write8(TCPC_REG_MSG_HDR_INFO, msg_hdr_temp);
        dev_snkPdo->snk_getSrcStatus.pd_version = dev->rx_msg_spec_rev;
    }
    return;
}

static void usb_pd_pe_unhandled_rx_msg(usb_pd_port_t *dev)
{
    if (*dev->current_state == PE_SNK_TRANSITION_SINK)
    {
        pe_set_state(dev, PE_SNK_HARD_RESET);
    }
    else if (*dev->current_state == PE_SNK_READY)
    {
        (g_PD_Ver == PD_3_0)?(pe_set_state(dev, PE_SNK_SEND_NOT_SUPPORTED)):(pe_set_state(dev, PE_SNK_SEND_REJECT));
    }
    else if (dev->non_interruptable_ams)
    {
        pe_set_state(dev, PE_SNK_SEND_SOFT_RESET);
    }

    return;
}

static void usb_pd_pe_data_msg_rx_handler(usb_pd_port_t *dev)
{
    switch (dev->rx_msg_type)
    {
        case DATA_MSG_TYPE_SRC_CAPS:
            check_PDVersion(dev);
            if (*dev->current_state == PE_SNK_WAIT_FOR_CAPS)
            {
                // Cancel SinkWaitCap timer.
                timer_cancel(PD_MODE);
                pe_set_state(dev, PE_SNK_EVALUATE_CAPABILITY);
            }
            else if (*dev->current_state == PE_SNK_READY)
            {
                pe_set_state(dev, PE_SNK_EVALUATE_CAPABILITY);
            }
            else
            {
                // Hard reset for this particular protocol error.
                pe_set_state(dev, PE_SNK_HARD_RESET);
            }
            break;

        case DATA_MSG_TYPE_REQUEST:
            break;

        case DATA_MSG_TYPE_BIST:
            if (*dev->current_state == PE_SNK_READY)
            {
                // simple code will send not support;
                    pe_set_state(dev, PE_SNK_SEND_NOT_SUPPORTED);
            }

            break;

        case DATA_MSG_TYPE_BATT_STATUS:
        case DATA_MSG_TYPE_ALERT:
            timer_cancel(PD_MODE);
            pe_set_state(dev, PE_SNK_SEND_NOT_SUPPORTED);
            break;

        case DATA_MSG_TYPE_VENDOR:
            if((dev->rx_msg_sop_frame != TCPC_TX_SOP_P) && (dev->rx_msg_sop_frame != TCPC_TX_SOP_PP))
            {
                if(*(dev->current_state) == PE_SNK_READY)
                {
                    (g_PD_Ver == PD_3_0)?(pe_set_state(dev, PE_SNK_SEND_NOT_SUPPORTED)):(pe_set_state(dev, PE_SNK_SEND_REJECT));
                }
            }
            break;


        default:
            pe_set_state(dev, PE_SNK_SEND_SOFT_RESET);
            break;
    }

    if (!dev->state_change)
    {
        usb_pd_pe_unhandled_rx_msg(dev);
    }

    return;
}

static void usb_pd_pe_ctrl_msg_rx_handler(usb_pd_port_t *dev)
{
    bool pinged = false;
    bool m_unexpect_msg = false;

    switch (dev->rx_msg_type)
    {
        case CTRL_MSG_TYPE_GOTO_MIN:
            if (*dev->current_state == PE_SNK_READY)
            {
                pe_set_state(dev, PE_SNK_TRANSITION_SINK);
            }
            break;

        case CTRL_MSG_TYPE_ACCEPT:
            if (*dev->current_state == PE_SNK_SEND_SOFT_RESET)
            {
                // Stop sender response timer.
                timer_cancel(PD_MODE);
                pe_set_state(dev, PE_SNK_WAIT_FOR_CAPS);
            }
            else if (*dev->current_state == PE_SNK_SELECT_CAPABILITY)
            {
                // Stop sender response timer.
                timer_cancel(PD_MODE);
                pe_set_state(dev, PE_SNK_TRANSITION_SINK);
            }
            break;

        case CTRL_MSG_TYPE_REJECT:
            if (*dev->current_state == PE_SNK_SELECT_CAPABILITY)
            {
                (!dev->explicit_contract)? pe_set_state(dev, PE_SNK_WAIT_FOR_CAPS): pe_set_state(dev, PE_SNK_READY);
            }
            break;

        case CTRL_MSG_TYPE_PING:
            pinged = true;
            break;

        case CTRL_MSG_TYPE_PS_RDY:
            if (*dev->current_state == PE_SNK_TRANSITION_SINK)
            {
                // Cancel PSTransition timer.
                timer_cancel(PD_MODE);
                pe_set_state(dev, PE_SNK_READY);
            }
            break;

        case CTRL_MSG_TYPE_GET_SNK_CAP:
            if (*dev->current_state == PE_SNK_READY)
            {
                pe_set_state(dev, PE_SNK_GIVE_SINK_CAP);
            }
            break;

        case CTRL_MSG_TYPE_WAIT:
            if (*dev->current_state == PE_SNK_SELECT_CAPABILITY)
            {
                if (!dev->explicit_contract)
                {
                    pe_set_state(dev, PE_SNK_WAIT_FOR_CAPS);
                }
                else
                {
                    dev->wait_received = true;
                    pe_set_state(dev, PE_SNK_READY);
                }
            }
            break;

        case CTRL_MSG_TYPE_SOFT_RESET:
            pe_set_state(dev, PE_SNK_SOFT_RESET);
            break;

        case CTRL_MSG_TYPE_NOT_SUPPORTED:
            if (*dev->current_state == PE_SNK_READY)
            {
                //pe_set_state(dev, PE_SNK_NOT_SUPPORTED_RECEIVED);
                pe_set_state(dev, PE_SNK_READY);
            }
            else
            {
                timer_cancel(PD_MODE);
                if(dev->pd_connected_since_attach)
                {
                    pe_set_state(dev, PE_SNK_READY);
                }
            }
            break;

        default:
            usb_pd_pe_unhandled_rx_msg(dev);
            break;
    }

    if(m_unexpect_msg)
    {
        pe_set_state(dev, PE_SNK_SEND_SOFT_RESET);
    }

    if (!dev->state_change && !pinged)
    {
        usb_pd_pe_unhandled_rx_msg(dev);
    }

    return;
}

static void usb_pd_pe_data_ext_msg_rx_handler(usb_pd_port_t *dev)
{
    switch (dev->rx_msg_type)
    {
        case EXT_MSG_TYPE_SRC_CAPS_EXT:
        case EXT_MSG_TYPE_GET_BATT_CAP:
        case EXT_MSG_TYPE_GET_BATT_STATUS:
        case EXT_MSG_TYPE_GET_MANUF_INFO:
        case EXT_MSG_TYPE_SECURITY_REQUEST:
        case EXT_MSG_TYPE_CHUNK:
            pe_set_state(dev, PE_SNK_SEND_NOT_SUPPORTED);
            break;

        default:
            pe_set_state(dev, PE_SNK_SEND_SOFT_RESET);
            break;
    }

    if (!dev->state_change)
    {
        usb_pd_pe_unhandled_rx_msg(dev);
    }

    return;
}

void usb_pd_pe_notify(usb_pd_prl_alert_t prl_alert)
{
    usb_pd_port_t *dev = &pd[0];
    switch (prl_alert)
    {
        case PRL_ALERT_HARD_RESET_RECEIVED:
            timer_cancel(ALL_MODE);
						tcpc_modify8(TCPC_REG_POWER_CTRL, TCPC_PWR_CTRL_AUTO_DISCHARGE_DISCONNECT, 0);
            tcpc_modify8(FP6606_REG_NMOS_SW_CTRL, NMOS_SNK_ON, 0);
						tcpc_modify8(FP6606_REG_NMOS_SW_CTRL, NMOS_SNK_ON, 0x80);	//wellsian
            pe_set_state(dev, PE_SNK_TRANSITION_TO_DEFAULT);

            DEBUG_LOG("prl_alert01 value = %d\r\n",prl_alert);
            break;

        case PRL_ALERT_MSG_TX_SUCCESS:   /* GoodCRC Recieved */
            switch (*dev->current_state)
            {
                case PE_SNK_HARD_RESET:
                    DEBUG_LOG("prl_alert02 value = %d\r\n",prl_alert);
                    pe_set_state(dev, PE_SNK_TRANSITION_TO_DEFAULT);
                    break;

                case PE_SNK_SEND_SOFT_RESET:
                case PE_SNK_SELECT_CAPABILITY:
                    timer_start1(PD_MODE, T_SENDER_RESPONSE_MS, timeout_sender_response);
                    break;

                case PE_SNK_SOFT_RESET:
                    pe_set_state(dev, PE_SNK_WAIT_FOR_CAPS);
                    break;

                case PE_SNK_SEND_REJECT:
                case PE_SNK_GIVE_SINK_CAP:
                    pe_set_state(dev, PE_SNK_READY);
                    break;
                case PE_SNK_SEND_NOT_SUPPORTED:
                    dev->msg_id[TCPC_TX_SOP] = 0;
                    pe_set_state(dev, PE_SNK_READY);
                    break;
                default:
                    break;
            }
            break;

        case PRL_ALERT_MSG_TX_FAILED: /* No GoodCRC response */
            if (*dev->current_state == PE_SNK_SEND_SOFT_RESET)
            {
                pe_set_state(dev, PE_SNK_HARD_RESET);
            }
            else if(*dev->current_state != PE_INIT_STATES)
            {
                pe_set_state(dev, PE_SNK_SEND_SOFT_RESET);
            }

            break;

        case PRL_ALERT_MSG_RECEIVED:
            timer_cancel(PD_MODE);
            if(g_PD_Ver == PD_3_0)
            {
                if (dev->rx_msg_data_len)
                {
                    (dev->rx_ext_flag)?(usb_pd_pe_data_ext_msg_rx_handler(dev)):(usb_pd_pe_data_msg_rx_handler(dev));
                }
                else
                {
                    usb_pd_pe_ctrl_msg_rx_handler(dev);
                }
            }
            else
            {
                (dev->rx_msg_data_len)?(usb_pd_pe_data_msg_rx_handler(dev)):(usb_pd_pe_ctrl_msg_rx_handler(dev));
            }
            break;
    }

    return;
}





















