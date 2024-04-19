#ifndef __USB_PD_H__
#define __USB_PD_H__


// All values in this header are defined by the USB PD spec.
#define PD_3_0   2
#define PD_2_0   1

#define PD_PWR_ROLE_SNK   0
#define PD_PWR_ROLE_SRC   1

#define PD_DATA_ROLE_UFP  0

#define N_PD_2_0_RETRY_COUNT    3
#define N_PD_3_0_RETRY_COUNT    2

/* Control msg when number of data objects is zero */
typedef enum
{
    CTRL_MSG_TYPE_GOOD_CRC        = 0x01,
    CTRL_MSG_TYPE_GOTO_MIN        = 0x02, /* SOP only */
    CTRL_MSG_TYPE_ACCEPT          = 0x03,
    CTRL_MSG_TYPE_REJECT          = 0x04, /* SOP only */
    CTRL_MSG_TYPE_PING            = 0x05, /* SOP only */
    CTRL_MSG_TYPE_PS_RDY          = 0x06, /* SOP only */
    CTRL_MSG_TYPE_GET_SRC_CAP     = 0x07, /* SOP only */
    CTRL_MSG_TYPE_GET_SNK_CAP     = 0x08, /* SOP only */
    CTRL_MSG_TYPE_DR_SWAP         = 0x09, /* SOP only */
    CTRL_MSG_TYPE_PR_SWAP         = 0x0A, /* SOP only */
    CTRL_MSG_TYPE_VCONN_SWAP      = 0x0B, /* SOP only */
    CTRL_MSG_TYPE_WAIT            = 0x0C, /* SOP only */
    CTRL_MSG_UNKNOW               = 0x0E,
    CTRL_MSG_TYPE_SOFT_RESET      = 0x0D,
    CTRL_MSG_TYPE_NOT_SUPPORTED   = 0x10,
    CTRL_MSG_TYPE_GET_SRC_CAP_EXT = 0x11, /* SOP only */
    CTRL_MSG_TYPE_GET_STATUS      = 0x12, /* SOP only */
    CTRL_MSG_TYPE_FR_SWAP         = 0x13, /* SOP only */
    CTRL_MSG_TYPE_GET_PPS_STATUS  = 0x14, /* SOP only */
    CTRL_MSG_TYPE_GET_COUNTRY_CODES = 0x15
} msg_hdr_ctrl_msg_type_t;

/* Data msg when number of data objects is non-zero */
typedef enum
{
    DATA_MSG_TYPE_SRC_CAPS    = 0x01,  /* SOP only */
    DATA_MSG_TYPE_REQUEST     = 0x02,  /* SOP only */
    DATA_MSG_TYPE_BIST        = 0x03,
    DATA_MSG_TYPE_SNK_CAPS    = 0x04,  /* SOP only */
    DATA_MSG_TYPE_BATT_STATUS = 0x05,  /* SOP only */
    DATA_MSG_TYPE_ALERT       = 0x06,  /* SOP only */
    DATA_MSG_TYPE_VENDOR      = 0x0F
} msg_hdr_data_msg_type_t;

typedef enum
{
    EXT_MSG_TYPE_SRC_CAPS_EXT       = 0x01,   /* SOP only */
    EXT_MSG_TYPE_STATUS             = 0x02,   /* SOP only */
    EXT_MSG_TYPE_GET_BATT_CAP       = 0x03,   /* SOP only */
    EXT_MSG_TYPE_GET_BATT_STATUS    = 0x04,   /* SOP only */
    EXT_MSG_TYPE_BATT_CAPABILITIES  = 0x05,   /* SOP only */
    EXT_MSG_TYPE_GET_MANUF_INFO     = 0x06,
    EXT_MSG_TYPE_MANUF_INFO         = 0x07,
    EXT_MSG_TYPE_SECURITY_REQUEST   = 0x08,
    EXT_MSG_TYPE_SECURITY_RESPONSE  = 0x09,
    EXT_MSG_TYPE_FW_UPDATE_REQUEST  = 0x0A,
    EXT_MSG_TYPE_FW_UPDATE_RESPONSE = 0x0B,
    EXT_MSG_TYPE_PPS_STATUS         = 0x0C,
    EXT_MSG_TYPE_CHUNK              = 0x1F
} ext_msg_hdr_msg_type_t;




/* Data role field is reserved for SOP'/SOP" */
#define USB_PD_HDR_GEN_BYTE0(ver, data_role, msg_type)  ( ((ver) << 6) | ((data_role) << 5) | (msg_type) )
#define USB_PD_HDR_GEN_BYTE1(ext, num_data_obj, msg_id, power_role)  ( ((ext) << 7) | ((num_data_obj) << 4) | ((msg_id & 0x07) << 1) | (power_role) )

/* Data role field is reserved for SOP'/SOP" */
#define USB_PD_HDR_GEN_BYTE0(ver, data_role, msg_type)  ( ((ver) << 6) | ((data_role) << 5) | (msg_type) )
#define USB_PD_HDR_GEN_BYTE1(ext, num_data_obj, msg_id, power_role)  ( ((ext) << 7) | ((num_data_obj) << 4) | ((msg_id & 0x07) << 1) | (power_role) )



#define USB_PD_HDR_GET_DATA_LEN(header)  ((((header) >> 12) & 0x07) << 2)  /* number of data objects x 4-bytes per object */
#define USB_PD_HDR_GET_MSG_ID(header)    (((header) >> 9) & 0x07)
#define USB_PD_HDR_GET_DATA_ROLE(header) (((header) >> 5) & 0x01)
#define USB_PD_HDR_GET_EXTENDED_HEADER(header) (((header) >> 15) & 0x01)
#define USB_PD_HDR_GET_MSG_TYPE(header)  ((header) & 0x1F)
#define USB_PD_HDR_GET_SPEC_REV(header) (((header) >> 6) & 0x03)

























#endif
