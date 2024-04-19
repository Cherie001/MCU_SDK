#ifndef __FP6606_USB_PD_PROTOCOL_H__
#define __FP6606_USB_PD_PROTOCOL_H__
// Must Include
#include "FP6606_common.h"
#include "FP6606_tcpci.h"
#include "FP6606_usb_pd.h"
#include "global_fun.h"

typedef enum
{
    PRL_ALERT_MSG_RECEIVED = 0,
    PRL_ALERT_MSG_TX_SUCCESS,
    PRL_ALERT_MSG_TX_DISCARDED,
    PRL_ALERT_MSG_TX_FAILED,
    PRL_ALERT_NO_RESPONSE_TIMEOUT,     // no response after hard reset
    PRL_ALERT_SENDER_RESPONSE_TIMEOUT,
    PRL_ALERT_VOLTAGE_ALARM_HI,
    PRL_ALERT_VOLTAGE_ALARM_LO,
    PRL_ALERT_HARD_RESET_RECEIVED
} usb_pd_prl_alert_t;



void usb_pd_prl_transmit_alert_handler(tx_status_t tx_status);
void usb_pd_prl_receive_alert_handler(void);













#endif



