#ifndef _FP6606_TCPCI_H_
#define _FP6606_TCPCI_H_

#include "FP6606_common.h"
#include "FP6606_usb_pd_policy_engine.h"

#define CC_STATE_OPEN 0

typedef enum
{
    TCPC_REG_VENDOR_ID                   = 0x00,
    TCPC_REG_PRODUCT_ID                  = 0x02,
    TCPC_REG_DEVICE_ID                   = 0x04,
    TCPC_REG_USB_TYPEC_REV               = 0x06,
    TCPC_REG_PD_REV_VER                  = 0x08,
    TCPC_REG_PD_INTERFACE_REV            = 0x0a,
    TCPC_REG_ALERT                       = 0x10,
    TCPC_REG_ALERT_MASK                  = 0x12,
    TCPC_REG_POWER_STATUS_MASK           = 0x14,
    TCPC_REG_FAULT_STATUS_MASK           = 0x15,
    TCPC_REG_CONFIG_STD_OUTPUT           = 0x18,
    TCPC_REG_TCPC_CTRL                   = 0x19,
    TCPC_REG_ROLE_CTRL                   = 0x1a,
    TCPC_REG_FAULT_CTRL                  = 0x1b,
    TCPC_REG_POWER_CTRL                  = 0x1c,
    TCPC_REG_CC_STATUS                   = 0x1d,
    TCPC_REG_POWER_STATUS                = 0x1e,
    TCPC_REG_FAULT_STATUS                = 0x1f,
    TCPC_REG_COMMAND                     = 0x23,
    TCPC_REG_DEV_CAP_1_L                 = 0x24,
    TCPC_REG_DEV_CAP_1_H                 = 0x25,
    TCPC_REG_DEV_CAP_2                   = 0x26,
    TCPC_REG_STD_INPUT_CAP               = 0x28,
    TCPC_REG_STD_OUTPUT_CAP              = 0x29,
    TCPC_REG_MSG_HDR_INFO                = 0x2e,
    TCPC_REG_RX_DETECT                   = 0x2f,
    TCPC_REG_RX_DETECT_SOP_HRST_MASK     = 0x21,
    TCPC_REG_RX_BYTE_CNT                 = 0x30,
    TCPC_REG_RX_BUF_FRAME_TYPE           = 0x31,
    TCPC_REG_RX_HDR                      = 0x32,
    TCPC_REG_RX_DATA                     = 0x34, /* through 0x4f */
    TCPC_REG_TRANSMIT                    = 0x50,
    TCPC_REG_TX_BYTE_CNT                 = 0x51,
    TCPC_REG_TX_HDR                      = 0x52,
    TCPC_REG_TX_DATA                     = 0x54, /* through 0x6f */
    TCPC_REG_VBUS_VOLTAGE                = 0x70,
    TCPC_REG_VBUS_SINK_DISCONNECT_THRESH = 0x72,
    TCPC_REG_VBUS_STOP_DISCHARGE_THRESH  = 0x74,
    TCPC_REG_VBUS_VOLTAGE_ALARM_HI_CFG   = 0x76,
    TCPC_REG_VBUS_VOLTAGE_ALARM_LO_CFG   = 0x78,
} tcpc_reg_t;

typedef enum
{
    TCPC_ALERT_VBUS_DISCONNECT = 2048,//(1 << 11),
    TCPC_ALERT_RX_BUF_OVERFLOW = 1024,//(1 << 10),
    TCPC_ALERT_FAULT           = 512,//(1 << 9),
    TCPC_ALERT_VOLT_ALARM_LO   = 256,//(1 << 8),
    TCPC_ALERT_VOLT_ALARM_HI   = 128,//(1 << 7),
    TCPC_ALERT_TX_SUCCESS      = 64,//(1 << 6),
    TCPC_ALERT_TX_DISCARDED    = 32,//(1 << 5),
    TCPC_ALERT_TX_FAILED       = 16,//(1 << 4),
    TCPC_ALERT_RX_HARD_RESET   = 8,//(1 << 3),
    TCPC_ALERT_RX_STATUS       = 4,//(1 << 2),
    TCPC_ALERT_POWER_STATUS    = 2,//(1 << 1),
    TCPC_ALERT_CC_STATUS       = 1,//(1 << 0),
    TCPC_ALERT_MASK_ALL        = 0x8fff
} tcpc_alert_bits_t;

typedef enum
{
    TCPC_FORCEOFF_VBUS_STATUS   = 64,	//(1 << 6),
    TCPC_AUTO_DIS_FAIL_STATUS   = 32,	//(1 << 5),
    TCPC_FORCE_DIS_FAIL_STATUS  = 16,	//(1 << 4),
    TCPC_VBUS_OCP_FAULT_STATUS  = 8,	//(1 << 3),
    TCPC_VBUS_OVP_FAULT_STATUS  = 4,	//(1 << 2),
    TCPC_VCONN_OCP_FAULT_STATUS = 2,	//(1 << 1),
    TCPC_I2C_INT_ERROR_STATUS   = 1		//(1 << 0)
} fault_status_t;


typedef enum
{
    TCPC_PWR_CTRL_VBUS_VOLTAGE_MONITOR      = 64,//(1 << 6),  /* 1 = disabled */
    TCPC_PWR_CTRL_DISABLE_VOLTAGE_ALARM     = 32,//(1 << 5),  /* 1 = disabled */
    TCPC_PWR_CTRL_AUTO_DISCHARGE_DISCONNECT = 16,//(1 << 4),
    TCPC_PWR_CTRL_ENABLE_BLEED_DISCHARGE    = 8,//(1 << 3),
    TCPC_PWR_CTRL_FORCE_DISCHARGE           = 4,//(1 << 2),
    TCPC_PWR_CTRL_VCONN_SUPPORTED           = 2,//(1 << 1),
    TCPC_PWR_CTRL_ENABLE_VCONN              = 1//(1 << 0)
} tcpc_pwr_ctrl_t;

typedef enum
{
    TCPC_DBG_ACC_CONNECT = 64,//(1 << 6),
    TCPC_AUDIO_ACC_CONNECT = 32//(1 << 5)
}tcpc_strd_output_t;

typedef enum
{
    CC_STATUS_CONNECT_RESULT     = 16,//(1 << 4),
    CC_STATUS_LOOKING4CONNECTION = 32,//(1 << 5),
    CC_STATUS_CC2_STATE_MASK     = 0xC,
    CC_STATUS_CC2_STATE_SHIFT    = 2,
    CC_STATUS_CC1_STATE_MASK     = 0x3
} tcpc_cc_status_t;

typedef enum
{
    TCPC_ROLE_CTRL_DRP            = 64,//(1 << 6),
    TCPC_ROLE_CTRL_RP_VALUE_SHIFT = 4,
    TCPC_ROLE_CTRL_CC2_SHIFT      = 2
} tcpc_role_ctrl_t;

typedef enum
{
    TCPC_CMD_WAKE_I2C            = 0x11,
    TCPC_CMD_DISABLE_VBUS_DETECT = 0x22,
    TCPC_CMD_ENABLE_VBUS_DETECT  = 0x33,
    TCPC_CMD_DISABLE_SNK_VBUS    = 0x44,
    TCPC_CMD_SNK_VBUS            = 0x55,
    TCPC_CMD_DISABLE_SRC_VBUS    = 0x66,
    TCPC_CMD_SRC_VBUS_DEFAULT    = 0x77,
    TCPC_CMD_SRC_VBUS_HI_VOLTAGE = 0x88,
    TCPC_CMD_SRC_LOOK4CONNECTION = 0x99,
    TCPC_CMD_RX_ONE_MORE         = 0xAA,
    TCPC_CMD_I2C_IDLE            = 0xFF
} tcpc_command_t;

typedef enum
{
    TCPC_CTRL_PLUG_ORIENTATION = 1,//(1 << 0),
    TCPC_CTRL_BIST_TEST_MODE   = 2//(1 << 1)
} tcpc_ctrl_t;

typedef enum
{
    CC_RA = 0,
    CC_RP,
    CC_RD,
    CC_OPEN   /* Disconnect or don't care for DRP */
} tcpc_role_cc_t;

typedef enum
{
    CC_SNK_STATE_OPEN = 0,
    CC_SNK_STATE_DEFAULT,
    CC_SNK_STATE_POWER15,
    CC_SNK_STATE_POWER30
} tcpc_cc_snk_state_t;

typedef enum
{
    TCPC_PWR_STATUS_DEBUG_ACC_CONNECTED  = 128,//(1 << 7),
    TCPC_PWR_STATUS_TCPC_INIT_STATUS     = 64,//(1 << 6),
    TCPC_PWR_STATUS_SOURCING_HI_VOLTAGE  = 32,//(1 << 5),
    TCPC_PWR_STATUS_SOURCING_VBUS        = 16,//(1 << 4),
    TCPC_PWR_STATUS_VBUS_PRES_DETECT_EN  = 8,//(1 << 3),
    TCPC_PWR_STATUS_VBUS_PRESENT         = 4,//(1 << 2),
    TCPC_PWR_STATUS_VCONN_PRESENT        = 2,//(1 << 1),
    TCPC_PWR_STATUS_SINKING_VBUS         = 1//(1 << 0)
} tcpc_pwr_status_t;

//typedef enum
//{
//    TCPC_TX_SOP          = 0,
//    TCPC_TX_SOP_P        = 1,
//    TCPC_TX_SOP_PP       = 2,
//    TCPC_TX_DEBUG_SOP_P  = 3,
//    TCPC_TX_DEBUG_SOP_PP = 4,
//    TCPC_TX_HARD_RESET   = 5,
//    TCPC_TX_CABLE_RESET  = 6,
//    TCPC_TX_BIST_MODE2   = 7
//} tcpc_transmit_t;


#define TCPC_CC1_STATE(cc_status)  ((unsigned int)(cc_status) & CC_STATUS_CC1_STATE_MASK)
#define TCPC_CC2_STATE(cc_status)  (((unsigned int)(cc_status) & CC_STATUS_CC2_STATE_MASK) >> CC_STATUS_CC2_STATE_SHIFT)
#define TCPC_PWR_CTRL_DEFAULTS  (TCPC_PWR_CTRL_VBUS_VOLTAGE_MONITOR | TCPC_PWR_CTRL_DISABLE_VOLTAGE_ALARM) /* per TCPC spec */
#define TCPC_REG_MSG_HDR_INFO_SET(cable_plug, data_role, ver, power_role)  ( ((cable_plug) << 4) | ((data_role) << 3) | ((ver) << 1) | power_role )




/* Retry cnt is ignored for hard reset, cable reset, and BIST */
#define TCPC_REG_TRANSMIT_SET(retry_cnt ,sop_msg) ( ((retry_cnt) << 4) | (sop_msg) )
#define TCPC_REG_TRANSMIT_NO_RETRY_SET(sop_msg)  (sop_msg)










































#endif

