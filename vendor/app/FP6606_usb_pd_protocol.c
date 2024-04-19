#include "FP6606_usb_pd_policy_engine.h"
//#define PD_PROTOCOL_FUN
#include "global_fun.h"
#include "global_value.h"
#include "FP6606_usb_pd.h"
#include "FP6606_usb_pd_protocol.h"



#define MSG_ID_CLEARED 					0xCE       // random value, must bigger than 7


void usb_pd_prl_transmit_alert_handler(tx_status_t tx_status)
{
    // Increment message ID.
    usb_pd_port_t *dev = usb_pd_pe_get_device();
    dev->msg_id[dev->tx_sop_type]++;

    if (tx_status == TX_STATUS_SUCCESS)
    {
        // GoodCRC received.
        usb_pd_pe_notify(PRL_ALERT_MSG_TX_SUCCESS);
    }
    else if (tx_status == TX_STATUS_DISCARDED)
    {
        // Either we issued a Hard Reset or received a msg.
        usb_pd_pe_notify(PRL_ALERT_MSG_TX_DISCARDED);
    }
    else if (tx_status == TX_STATUS_FAILED)
    {
        // No valid GoodCRC received.
        usb_pd_pe_notify(PRL_ALERT_MSG_TX_FAILED);
    }

}

void usb_pd_prl_receive_alert_handler(void)
{
    unsigned char  msg_id ;
    unsigned short hdr;
    usb_pd_port_t *dev = usb_pd_pe_get_device();

    tcpc_read16(TCPC_REG_RX_HDR, &hdr);
    tcpc_read8(TCPC_REG_RX_BUF_FRAME_TYPE, &dev->rx_msg_sop_frame);

    if(dev->rx_msg_sop_frame == TCPC_TX_CABLE_RESET)
    {
        usb_pd_pe_notify(PRL_ALERT_HARD_RESET_RECEIVED);
        return;
    }

    if (dev->rx_msg_sop_frame == TCPC_TX_SOP)
    {
        if (dev->data_role == USB_PD_HDR_GET_DATA_ROLE(hdr))
        {
            tcpm_error_recovery();
            return;
        }
    }

    dev->rx_ext_flag = USB_PD_HDR_GET_EXTENDED_HEADER(hdr);

    dev->rx_msg_type = USB_PD_HDR_GET_MSG_TYPE(hdr);

    dev->rx_msg_data_len = USB_PD_HDR_GET_DATA_LEN(hdr);

    dev->rx_msg_spec_rev = USB_PD_HDR_GET_SPEC_REV(hdr);

    // Check for Soft Reset.
    if ((dev->rx_msg_data_len == 0) &&
            (dev->rx_msg_type == CTRL_MSG_TYPE_SOFT_RESET))
    {
        // Reset message ID.
        dev->msg_id[dev->rx_msg_sop_frame] = 0;

        dev->stored_msg_id[dev->rx_msg_sop_frame] = MSG_ID_CLEARED;

        dev->non_interruptable_ams = false;

        usb_pd_pe_notify(PRL_ALERT_MSG_RECEIVED);
    }
    else
    {
        msg_id = USB_PD_HDR_GET_MSG_ID(hdr);
        // Verify this is not a retry msg.
        if (msg_id != dev->stored_msg_id[dev->rx_msg_sop_frame])
        {
            // Store message ID.
            dev->stored_msg_id[dev->rx_msg_sop_frame] = msg_id;

            tcpm_read_message(dev->rx_msg_buf, dev->rx_msg_data_len);

            usb_pd_pe_notify(PRL_ALERT_MSG_RECEIVED);
        }
    }

    return;
}


static void usb_pd_prl_tx_msg(u8 *buf, tcpc_transmit_t sop_type)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("usb_pd_prl_tx_msg(u8 *buf, tcpc_transmit_t sop_type)\r\n");

    usb_pd_port_t *dev = usb_pd_pe_get_device();

//    DEBUG_LOG("sop_type = %d     \r\n",sop_type);
    tcpm_transmit(buf, sop_type);

    dev->tx_sop_type = sop_type;
    return;
}

void usb_pd_prl_tx_ctrl_msg(u8 *buf, msg_hdr_ctrl_msg_type_t msg_type, tcpc_transmit_t sop_type)
{

//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("Enter usb_pd_prl_tx_ctrl_msg(u8 *buf, msg_hdr_ctrl_msg_type_t msg_type, tcpc_transmit_t sop_type)\r\n");
    usb_pd_port_t *dev = usb_pd_pe_get_device();
    buf[0] = 2; /* Tx byte cnt */
    buf[1] = USB_PD_HDR_GEN_BYTE0(g_PD_Ver, dev->data_role, msg_type);
    buf[2] = USB_PD_HDR_GEN_BYTE1(0, 0, dev->msg_id[sop_type], dev->power_role);
    if (msg_type == CTRL_MSG_TYPE_SOFT_RESET)
    {
        // Reset message ID counter.
        dev->msg_id[sop_type] = 0;
        dev->stored_msg_id[sop_type] = MSG_ID_CLEARED;
        dev->non_interruptable_ams = false;
    }
//    DEBUG_LOG(" sop_type = %d \r\n",sop_type);

    usb_pd_prl_tx_msg(buf, sop_type);
    return;
}


void usb_pd_prl_tx_data_msg(u8 *buf, msg_hdr_data_msg_type_t msg_type, tcpc_transmit_t sop_type, u32 ndo)
{
    usb_pd_port_t *dev = usb_pd_pe_get_device();

    buf[0] = (ndo << 2) + 2; // Each data object is 4-bytes plus 2-byte header.
    buf[1] = USB_PD_HDR_GEN_BYTE0(g_PD_Ver, dev->data_role, msg_type);
    buf[2] = USB_PD_HDR_GEN_BYTE1(0, ndo, dev->msg_id[sop_type], dev->power_role);
    if(sop_type == TCPC_TX_SOP_P)
    {
        buf[1] = USB_PD_HDR_GEN_BYTE0(g_PD_Ver, 0, msg_type);
        buf[2] = USB_PD_HDR_GEN_BYTE1(0, ndo, dev->msg_id[sop_type], 0);
    }

//    DEBUG_LOG(" sop_type = %d \r\n",sop_type);
    usb_pd_prl_tx_msg(buf, sop_type);
    return;
}
