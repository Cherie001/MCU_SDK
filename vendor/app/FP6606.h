#ifndef __FP6606_H__
#define __FP6606_H__
//#include "stm32f7xx_hal.h"
//#include "stdbool.h"
#include "FP6606_common.h"

// Vendor define register
#define FP6606_ALERT_IRQ_STATUS  0x8000


typedef enum
{
    // System
    FP6606_REG_SYS_CTRL_1       = 0x80,
    FP6606_REG_TX_CTRL          = 0x81,
    FP6606_REG_DBG_FUN          = 0x82,
    FP6606_REG_SYS_WATCH_1      = 0x83,     // QC Mode / DRP Mode
    FP6606_REG_SYS_WATCH_2      = 0x84,     // QC Mode / Bus Idel / PD Mode
    FP6606_REG_NMOS_SW_CTRL     = 0x85,
    FP6606_REG_RX_CTRL          = 0x86,
    FP6606_REG_SYS_CTRL_2       = 0x87,
    // QC Snk Ctrl
    FP6606_REG_QC_SNK_CTRL_1    = 0x88,
    FP6606_REG_QC_SNK_CTRL_2    = 0x89,
    FP6606_REG_QC_SNK_CTRL_3    = 0x8A,
    FP6606_REG_QC_SNK_STATUS    = 0x8B,

    // Systme
    FP6606_REG_INT_STATUS       = 0x90,
    FP6606_REG_INT_STATUS_BYTE1 = 0x91,
    FP6606_REG_INT_MASK         = 0x92,
    FP6606_REG_CC_CTRL          = 0x94,

    FP6606_BMC_SETTING_1        = 0x95,
    FP6606_BMC_SETTING_2        = 0x96,
    // QC
    FP6606_REG_QC_FUNL_CTRL_0   = 0x9C,
    FP6606_REG_QC_FUNL_CTRL_1   = 0x9D,
    FP6606_REG_QC_FUNL_CTRL_2   = 0x9E,
    FP6606_REG_QC_FUNL_CTRL_3   = 0x9F,
    FP6606_REG_QC_FUNL_CTRL_4   = 0xA0,

    // OTHER
    FP6606_REG_I2C_ADDR         = 0xA1,
    FP6606_REG_ADP_TYPE         = 0xA2,
    FP6606_REG_B_ADP_TYPE       = 0xA3,
    FP6606_REG_SCP_VENDER_ID_H  = 0xA4,
    FP6606_REG_SCP_VENDER_ID_L  = 0xA5,
    FP6606_REG_MODULE_ID_H      = 0xA6,
    FP6606_REG_MODULE_ID_L      = 0xA7,
    FP6606_REG_SERIAL_NO_H      = 0xA8,
    FP6606_REG_SERIAL_NO_L      = 0xA9,
    FP6606_REG_CHIP_ID          = 0xAA,
    FP6606_REG_HW_VER           = 0xAB,
    FP6606_REG_FW_VER_H         = 0xAC,
    FP6606_REG_FW_VER_L         = 0xAD,
    FP6606_REG_ADP_B_TYPE1      = 0xAE,
    FP6606_REG_FACTORY          = 0xAF,

    // BMC STATUS & PROTOCOL SETTING FOR DEBUG
    FP6606_REG_BMC_RX_STATUS    = 0xB1,
    FP6606_REG_BMC_RX_MASK      = 0xB2,
    FP6606_REG_BMC_PROTOCOL_SETTING_1 = 0xB3,
    FP6606_REG_BMC_PROTOCOL_SETTING_2 = 0xB4,
    FP6606_REG_BMC_CRC_DATA_0   = 0xB5,
    FP6606_REG_BMC_CRC_DATA_1   = 0xB6,
    FP6606_REG_BMC_CRC_DATA_2   = 0xB7,
    FP6606_REG_BMC_CRC_DATA_3   = 0xB8,

    // ADC
    FP6606_REG_ADC_CTR          = 0xC0,
    FP6606_REG_ADC_VOLT         = 0xC1,
    FP6606_REG_ADC_CURRENT      = 0xC3,
    FP6606_REG_ADC_TEMP         = 0xC4,

    FP6606_REG_OUTPUT_EN        = 0xC8,
    FP6606_REG_OUTPUT_VAL       = 0xC9,
    FP6606_REG_INPUT_VAL        = 0xCA,
    FP6606_REG_DRP_FINISH       = 0xCB,
    // Current Deck
    FP6606_REG_CURRENT_DECK_CTRL = 0xD0,
    FP6606_REG_MCU_IT_CNT        = 0xD1,
    FP6606_REG_MCU_IT_CNT_2      = 0xD2,
    FP6606_REG_HW_IT_CNT         = 0xD3,

    // SCP CTRL SYS
    FP6606_REG_SCP_ACCESS        = 0xD8,
    FP6606_REG_SCP_ADDRESS       = 0xD9,
    FP6606_REG_SCP_LEN           = 0xDA,
    FP6606_REG_SCP_ABNORMAL      = 0xDC,
    FP6606_REG_FITI_HW_VER       = 0xDF,

    FP6606_REG_END         = 0xFF
} FP6606_reg_t;

typedef enum                // 0x80
{
    FP6606_ADC_FORCE_ON                         = (1 << 7),
    FP6606_BUS_IDEL_GATING_DISABLE              = (1 << 6),
    FP6606_FORCE_4D8M_ON                        = (1 << 5),
    FP6606_PD_CLK_GATING_ENABLE                 = (1 << 4),
    FP6606_HIGH_VOL_SUPPORT                     = (1 << 3),
    FP6606_OTSD1_EN_BIT                         = (1 << 2),
    FP6606_INT_VCONNDIS_DISABLE                 = (1 << 1),
    FP6606_INT_VBUSDIS_DISABLE                  = (1 << 0),
} FP6606_sys_t;




typedef enum
{
    NMOS_SNK_ON                 = (1 << 1),
}FP6606_NMOS_t;

typedef enum            // 0xCB
{
    DRP_FINISH                          = (1 << 5),
}FP6606_drp_finish_t;

typedef enum        // 0x81
{
    FP6606_CC1_DIS                  = (1 << 7),
    FP6606_DRP_CLK_DIS              = (1 << 6),
    FP6606_VBUS_DET_EN              = (1 << 5),
    FP6606_VCONN_DET_EN             = (1 << 4),
    FP6606_AUTO_BMC_DIS             = (1 << 3),
    FP6606_BMC_TX_CARRIER_MODE      = (1 << 2),
    FP6606_QC_SRC_RST               = (1 << 1),
    FP6606_BMC_TX_FASTROLE_SWAP     = (1 << 0)
} FP6606_bmc_tx_t;

typedef enum        // 0x82
{
    FP6606_CC2_DIS                  = (1 << 7),
    FP6606_ALERT_DIS_DIS            = (1 << 6),
    FP6606_DBG_MEAS_EN              = (1 << 5),
    FP6606_DBG_VOLT_LVL_EN          = (1 << 4),
    FP6606_FORCE_CC2_MEAS           = (1 << 1),
    FP6606_FORCE_CC1_MEAS           = (1 << 0)
} FP6606_dbg_t;


typedef enum        // 0x94
{
    FP6606_PD_TXRX_RESET                = (1 << 6),
    FP6606_GLOBAL_SW_RESET              = (1 << 5),
    FP6606_AUTO_DRP_SAMPLE              = (1 << 4),
} FP6606_reset_t;

typedef enum        // 0x90
{
    FP6606_OV_OCCUR           = (1 << 12),
    FP6606_SCP_ABNORMAL       = (1 << 11),
    FP6606_ADC_CUR_UPD        = (1 << 10),
    FP6606_VCONN_INVCUR       = (1 << 9),
    FP6606_QC_ST_CHG          = (1 << 8),
    FP6606_SCP_W_CMD          = (1 << 7),
    FP6606_SCP_R_CMD          = (1 << 6),
    FP6606_SCP_POR            = (1 << 5),
    FP6606_ADC_VOL_UPD        = (1 << 4),
    FP6606_INT_VCONN_OC       = (1 << 3),
    FP6606_INT_OTSD1_STAT     = (1 << 2),
    FP6606_PD_ENGINE_CHG      = (1 << 1),
    FP6606_INT_FAST_ROLE_SWAP = (1 << 0),

    FP6606_INT_MASK_ALL       = 0x07FF
} FP6606_int_t;



















void FP6606_vendorISR(void);
void FP6606_init(void);
char tcpc_read16(unsigned char reg, unsigned short *data);














#endif

