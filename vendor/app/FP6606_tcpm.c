#include "app.h"
#include "FP6606.h"
#include <tl_common.h>
#include "global_fun.h"
#include "app_config.h"
#include "app_machine.h"
#include "FP6606_tcpm.h"
#include "global_value.h"
#include "FP6606_tcpci.h"
#include "FP6606_usb_pd.h"
#include "FP6606_common.h"
#include "stack/ble/ble.h"
#include "Fp6606_usb_pd_protocol.h"
#include "FP6606_usb_pd_policy_engine.h"



#define T_CC_DEBOUNCE_MS     150    // 100 - 200 ms
#define T_PD_DEBOUNCE_MS     11     // 10  - 14 ms    // fp660 cc sample rate + cc debounce time .
#define T_ERROR_RECOVERY_MS  500    // 25  - ?? ms
#define SOP_DET_MASK(sop,sop_p,sop_p_p,dbg_sop,dbg_sop_p,hrd_rst,cab_rst)    ((cab_rst << 6) | (hrd_rst << 5) | (dbg_sop_p << 4) | (dbg_sop << 3) | (sop_p_p << 2) | (sop_p << 1) | sop)


static tcpc_device_t  tcpc_dev[NUM_TCPC_DEVICES];

unsigned char RX_DET_MASK = 0x00;

static void pe_notify_conn_state(void);

extern u8 power_ic;
extern u16 count_qc;
extern u8 dev_state_change;
extern usb_pd_port_t         pd[NUM_TCPC_DEVICES];

// PD mode change QC mode
void pd_to_qc(void)
{
//	usb_pd_port_t *dev = &tcpc_dev[0];
    usb_pd_port_t *dev = &pd[0];

	// FP6606 and IIC init
//    FP6606_IC_init();

	i2c_set_pin(I2C_GPIO_GROUP_M_A3A4);
	i2c_master_init(0x60, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*400000)) ); // 400KHz

    // open protocol IC flag
	power_ic = 1;
	count_qc = 0;

    pe_set_state(dev, PE_SNK_SEND_SOFT_RESET);

//    pe_switch_qc_state();
}

void pd_voltage_check()
{
	tcpc_device_t *dev = &tcpc_dev[0];

	u16 rv_0x7071;

	tcpc_read16(0x70, &rv_0x7071);

	pd_voltage = rv_0x7071;

	if((dev->connect_mode == PD_CONNECT_MODE) && (rv_0x7071 > 240))
	{
//		switch_qc = 1;
		DEBUG_LOG("PD: rv_0x7071 = %d \r\n", rv_0x7071 * 25);
	}
}


unsigned char tcpc_reg_role_ctrl_set(bool drp, tcpc_role_rp_val_t rp_val, tcpc_role_cc_t cc1, tcpc_role_cc_t cc2)
{
	u8 data = (((drp) ? TCPC_ROLE_CTRL_DRP : 0) |
	           (rp_val) << TCPC_ROLE_CTRL_RP_VALUE_SHIFT |
	           (cc2) << TCPC_ROLE_CTRL_CC2_SHIFT |
	           (cc1));

    return data;
}

void GPIO_FP_INT_PA0(void)
{
	if(gpio_read(GPIO_FP_INT))
    {
		DEBUG_LOG("GPIO_FP_INT high.\r\n");
    }
    else
    {
		DEBUG_LOG("GPIO_FP_INT low.\r\n");
    }

}



void tcpm_transmit(u8 *buf, tcpc_transmit_t sop_type)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("tcpm_transmit\r\n");

    u8 m_retry_cnt = N_PD_2_0_RETRY_COUNT;	// 3

    if (buf)
    {
        tcpc_write_block(TCPC_REG_TX_BYTE_CNT, buf, buf[0] + 1);

//		DEBUG_LOG("buf[0] = 0x%x.\r\n", buf[0]); 	// 0x02
//		DEBUG_LOG("buf[1] = 0x%x.\r\n", buf[1]); 	// 0x8D		//
//		DEBUG_LOG("buf[2] = 0x%x.\r\n", buf[2]);	// 0x02
//		DEBUG_LOG("buf[3] = 0x%x.\r\n", buf[3]);	// 0x0E
//		DEBUG_LOG("buf[4] = 0x%x.\r\n", buf[4]);	// 0x21
    }
    // Start transmit.
    if((sop_type == TCPC_TX_HARD_RESET) || (sop_type == TCPC_TX_CABLE_RESET) || (sop_type == TCPC_TX_CABLE_RESET) || (sop_type == TCPC_TX_BIST_MODE2))
    {
        tcpc_write8(TCPC_REG_TRANSMIT, TCPC_REG_TRANSMIT_NO_RETRY_SET(sop_type));
    }
    else
    {
        (g_PD_Ver == PD_3_0)?(m_retry_cnt = N_PD_3_0_RETRY_COUNT):(m_retry_cnt = N_PD_2_0_RETRY_COUNT);
        tcpc_write8(TCPC_REG_TRANSMIT, TCPC_REG_TRANSMIT_SET(m_retry_cnt, sop_type));
    }

    return;
}


void pe_switch_qc_state(void)
{
	tcpc_device_t *dev = &tcpc_dev[0];

	dev->connect_mode = QC_CONNECT_MODE;
	pe_notify_conn_state();
}

bool tcpm_is_vbus_present(void)
{
    unsigned char pwr_status;

    tcpc_read8(TCPC_REG_POWER_STATUS, &pwr_status);

    return(pwr_status & TCPC_PWR_STATUS_VBUS_PRESENT) ? true : false;
}

void tcpm_error_recovery(void)
{
    tcpc_device_t *dev = &tcpc_dev[0];

    if (dev->state == TCPC_STATE_ERROR_RECOVERY)
    {
        return;
    }
    tcpm_set_state(dev, TCPC_STATE_ERROR_RECOVERY);

    tcpc_modify8(FP6606_REG_NMOS_SW_CTRL, NMOS_SNK_ON, 0); //old
//    tcpc_modify8(FP6606_REG_NMOS_SW_CTRL, NMOS_SNK_ON, 0x80);	//wellsian

    tcpc_write8(TCPC_REG_POWER_CTRL,
                (TCPC_PWR_CTRL_AUTO_DISCHARGE_DISCONNECT | TCPC_PWR_CTRL_DEFAULTS));

    // Remove CC1 & CC2 terminations.
    tcpc_write8(TCPC_REG_ROLE_CTRL,
                tcpc_reg_role_ctrl_set(false, dev->rp_val, CC_OPEN, CC_OPEN));
    return;
}


tcpc_device_t* tcpm_get_device(void)
{
    return &tcpc_dev[0];
}

void tcpm_enable_pd_receive(void)
{
    tcpc_write8(0x80, 0x10);
    tcpc_write8(0x81, 0x30);
    tcpc_modify8(TCPC_REG_TCPC_CTRL, TCPC_CTRL_BIST_TEST_MODE, 0);
    tcpc_write8(TCPC_REG_RX_DETECT, RX_DET_MASK);
    return;
}

static void alert_fault_status_handler(void)
{
    unsigned char status;

    tcpc_read8(TCPC_REG_FAULT_STATUS, &status);

    if (status & TCPC_AUTO_DIS_FAIL_STATUS)
    {
        // Stop VBUS discharge , FP6606 discharge by external circuit.
        tcpc_write8(FP6606_REG_SYS_CTRL_1, FP6606_INT_VBUSDIS_DISABLE);
    }

    // Clear status.
    tcpc_write8(TCPC_REG_FAULT_STATUS, status);

    return;
}

static void alert_power_status_handler(tcpc_device_t *dev)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("Enter alert_power_status_handler(tcpc_device_t *dev)\r\n");

    unsigned int  cc1, cc2;
    unsigned char pwr_status;
    // Read CC status.
    cc1 = TCPC_CC1_STATE(dev->cc_status);
    cc2 = TCPC_CC2_STATE(dev->cc_status);

    // Read power status.
    tcpc_read8(TCPC_REG_POWER_STATUS, &pwr_status);

    DEBUG_LOG("0x1E: pwr_status value = 0x%x, dev->state = %d\r\n", pwr_status, dev->state);

    if (pwr_status & TCPC_PWR_STATUS_VBUS_PRESENT)
    {
        dev->vbus_present = true;

        if (dev->state == TCPC_STATE_WAITING_FOR_VBUS_SNK)
        {
            // Debug Accessory if SNK.Rp on both CC1 and CC2.
            ((cc1 != CC_SNK_STATE_OPEN) && (cc2 != CC_SNK_STATE_OPEN))?(tcpm_set_state(dev, TCPC_STATE_DEBUG_ACC_SNK)):(tcpm_set_state(dev, TCPC_STATE_ATTACHED_SNK));
            DEBUG_LOG("tcpm_set_state 1 \r\n");
        }
    }
    else /* VBUS below threshold  not in the process Hard Reset */
    {
        if(dev->msg_hdrst)
        {
            dev->msg_hdrst = 0;
            return;
        }

        dev->vbus_present = false;
        if ((dev->state == TCPC_STATE_ATTACHED_SNK) ||\
                (dev->state == TCPC_STATE_DEBUG_ACC_SNK))
        {
            tcpm_set_state(dev, TCPC_STATE_UNATTACHED_SNK);

            DEBUG_LOG("tcpm_set_state 2 \r\n");
        }
    }

    return;
}

void tcpm_enable_vbus_detect(void)
{
    // Enable VBUS detect. (not active until voltage monitoring is enabled)
    tcpc_write8(TCPC_REG_COMMAND, TCPC_CMD_ENABLE_VBUS_DETECT);
    // Enable Monitor
    tcpc_modify8(TCPC_REG_POWER_CTRL, TCPC_PWR_CTRL_VBUS_VOLTAGE_MONITOR, 0);

    return;
}

static void timeout_cc_debounce(void)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("Enter timeout_cc_debounce(void)\r\n");

    unsigned int cc1, cc2;
    tcpc_device_t *dev = &tcpc_dev[0];

    cc1 = TCPC_CC1_STATE(dev->cc_status);
    cc2 = TCPC_CC2_STATE(dev->cc_status);

    DEBUG_LOG("dev->cc_status = 0x%x, dev->state = 0x%x \r\n",dev->cc_status, dev->state);

    if (dev->state == TCPC_STATE_ATTACH_WAIT_SNK)
    {
        if (dev->vbus_present)
        {
            if ((cc1 != CC_SNK_STATE_OPEN) &&
                    (cc2 != CC_SNK_STATE_OPEN))
            {
                tcpm_set_state(dev, TCPC_STATE_DEBUG_ACC_SNK);
            }
            else
            {
                tcpm_set_state(dev, TCPC_STATE_ATTACHED_SNK);
            }
        }
        else
        {
            tcpm_set_state(dev, TCPC_STATE_UNATTACHED_SNK);
        }
    }
    return;
}

static void timeout_pd_debounce(void)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("Enter timeout_pd_debounce(void)\r\n");
    unsigned int cc1, cc2;
    tcpc_device_t *dev = &tcpc_dev[0];

    cc1 = TCPC_CC1_STATE(dev->cc_status);
    cc2 = TCPC_CC2_STATE(dev->cc_status);
    DEBUG_LOG("cc1 = %d, cc2 = %d, \r\n",cc1, cc2);

    if ((cc1 == CC_STATE_OPEN) && (cc2 == CC_STATE_OPEN))
    {
        if ((dev->state == TCPC_STATE_ATTACHED_SNK) || (dev->state == TCPC_STATE_DEBUG_ACC_SNK))
        {
            return;
        }
        tcpm_set_state(dev, TCPC_STATE_UNATTACHED_SNK);
    }
    return;
}

static void alert_cc_status_handler(tcpc_device_t *dev)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("Enter alert_cc_status_handler(tcpc_device_t *dev)\r\n");
    u8 cc_status;
    u32 cc1, cc2;
    // Read CC status.
    tcpc_read8(TCPC_REG_CC_STATUS, &cc_status);
    if (!(cc_status & CC_STATUS_LOOKING4CONNECTION))
    {
        cc1 = TCPC_CC1_STATE(cc_status);
        cc2 = TCPC_CC2_STATE(cc_status);

//        DEBUG_LOG("cc1 = %d, cc2 = %d, 0x1D: cc_status = 0x%x \r\n",cc1, cc2, cc_status);
        switch (dev->state)
        {
            case TCPC_STATE_UNATTACHED_SNK:
                if (cc_status & CC_STATUS_CONNECT_RESULT) // Rd
                {
                    if ((cc1 != CC_SNK_STATE_OPEN) || \
                            (cc2 != CC_SNK_STATE_OPEN))
                    {
                        // Enable VBUS detection.
                        tcpm_enable_vbus_detect();
                        tcpm_set_state(dev, TCPC_STATE_ATTACH_WAIT_SNK);
                        timer_start1(TCPC_MODE, T_CC_DEBOUNCE_MS, timeout_cc_debounce);
                        DEBUG_LOG("timer_start1 01 \r\n");
                    }
                }
                break;

            case TCPC_STATE_ATTACHED_SNK:
            case TCPC_STATE_DEBUG_ACC_SNK:
                break;

            default:
                break;
        }

        // If open state on CC1 and CC2.
        if ((cc1 == CC_STATE_OPEN) && (cc2 == CC_STATE_OPEN))
        {
            if (dev->state != TCPC_STATE_UNATTACHED_SNK)
            {
                // Debounce.
                timer_start1(TCPC_MODE, T_PD_DEBOUNCE_MS, timeout_pd_debounce);
                DEBUG_LOG("timer_start1 02 \r\n");
            }
        }
    }

    // Save CC status.
    dev->cc_status = cc_status;
    return;
}



static void tcpm_alert_handler(void)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("Enter tcpm_alert_handler(void)\r\n");

//	u8 rv_0x1F;
	u16 alert ;
    u16 clear_bits;
    tcpc_device_t *dev = &tcpc_dev[0];
    // Read alerts.
    tcpc_read16(TCPC_REG_ALERT, &alert);

//    rv_0x1F = iic_read_byte(0x1F, 1);

    clear_bits = alert & ~(TCPC_ALERT_RX_STATUS | TCPC_ALERT_FAULT | TCPC_ALERT_VOLT_ALARM_HI | TCPC_ALERT_VOLT_ALARM_LO | FP6606_ALERT_IRQ_STATUS );

    WaitMs(5);
//    DEBUG_LOG("0x1110: alert = 0x%x. clear_bits = 0x%x.  0x1F rv_0x1F = 0x%x\r\n",alert, clear_bits, rv_0x1F);
//    GPIO_FP_INT_PA0();
//    DEBUG_LOG("alert = 0x%x.\r\n",alert);

    if (clear_bits)
    {
        tcpc_write16(TCPC_REG_ALERT, clear_bits);
    }
    if (alert & FP6606_ALERT_IRQ_STATUS)
    {
        FP6606_vendorISR();
        // Clear alert.
        tcpc_write16(TCPC_REG_ALERT, FP6606_ALERT_IRQ_STATUS);
    }

    if (alert & TCPC_ALERT_FAULT)  // TCPC_ALERT_FAULT = 1 << 9
    {
//    	tcpc_read8(0x1F, &rv_0x1F);
//        DEBUG_LOG("rv_0x1F = 0x%x.\r\n",rv_0x1F);

        alert_fault_status_handler();
        // Clear alert.
        tcpc_write16(TCPC_REG_ALERT, TCPC_ALERT_FAULT);
    }

    if (alert & TCPC_ALERT_VOLT_ALARM_LO)
    {
        // Clear alarm threshold.
        tcpc_write16(TCPC_REG_VBUS_VOLTAGE_ALARM_LO_CFG, 0);
        // Clear alert.
        tcpc_write16(TCPC_REG_ALERT, TCPC_ALERT_VOLT_ALARM_LO);
    }

    if (alert & TCPC_ALERT_VOLT_ALARM_HI)
    {
        // Clear alarm threshold.
        tcpc_write16(TCPC_REG_VBUS_VOLTAGE_ALARM_HI_CFG, 0x3FF);
        // Clear alert.
        tcpc_write16(TCPC_REG_ALERT, TCPC_ALERT_VOLT_ALARM_HI);
    }

    if (alert & TCPC_ALERT_TX_SUCCESS)
    {
        usb_pd_prl_transmit_alert_handler(TX_STATUS_SUCCESS);
    }
    else if (alert & TCPC_ALERT_TX_DISCARDED)
    {
        usb_pd_prl_transmit_alert_handler(TX_STATUS_DISCARDED);
    }
    else if (alert & TCPC_ALERT_TX_FAILED)
    {
        usb_pd_prl_transmit_alert_handler(TX_STATUS_FAILED);
    }

    if (alert & TCPC_ALERT_RX_HARD_RESET)
    {
        dev->msg_hdrst = 1;
        usb_pd_pe_notify(PRL_ALERT_HARD_RESET_RECEIVED);
    }

    if (alert & TCPC_ALERT_RX_STATUS)
    {
        usb_pd_prl_receive_alert_handler();
        tcpc_write16(TCPC_REG_ALERT, TCPC_ALERT_RX_STATUS);
    }

    if (alert & TCPC_ALERT_POWER_STATUS)
    {
        alert_power_status_handler(dev);
    }

    if (alert & TCPC_ALERT_CC_STATUS)
    {
        alert_cc_status_handler(dev);
    }

    return;
}

void tcpm_read_message(unsigned char *buf, unsigned char len)
{
    unsigned char byte_cnt;
    unsigned char local_buf[OBJ_MAX];

    // Read Rx Byte Cnt.
    tcpc_read8(TCPC_REG_RX_BYTE_CNT, &byte_cnt);
    if (byte_cnt > 3)
    {
        tcpc_read_block(TCPC_REG_RX_BYTE_CNT, local_buf, byte_cnt + 1);

        // Copy message to buffer. (Subtract 3-bytes for frame type and header)
        memcpy(buf, &local_buf[4], (byte_cnt - 3));
    }

    return;
}

static void pe_notify_conn_state(void)
{
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("pe_notify_conn_state(void) \r\n");
	tcpc_device_t *dev = &tcpc_dev[0];

	unsigned short alert ;
	if(dev->connect_mode == QC_CONNECT_MODE)
	{
//	    DEBUG_LOG("        \r\n");
//	    DEBUG_LOG("usb_qc_pe_connection_state_change_handler(dev->state) \r\n");
		dev->state = 3;
		usb_qc_pe_connection_state_change_handler(dev->state);
	}
	else
	{
//	    DEBUG_LOG("        \r\n");
//	    DEBUG_LOG("usb_pd_pe_connection_state_change_handler(dev->state) \r\n");
	    tcpc_read16(TCPC_REG_ALERT, &alert);
//	    DEBUG_LOG("0x1110: alert = 0x%x. dev->state = 0x%x\r\n",alert, dev->state);
		usb_pd_pe_connection_state_change_handler(dev->state);
	}
}

void tcpm_set_state(tcpc_device_t *dev, tcpc_state_t new_state)
{
    dev->last_state = dev->state;
    dev->state = new_state;
    dev->state_change = true;
//	printf(" set - typc_state: %s \r\n",tcpc_state_string[dev->state]);
    return;
}

static void timeout_error_recovery(void)
{
    tcpc_device_t *dev = &tcpc_dev[0];
    // clear Alert
    tcpm_alert_handler();
    tcpm_set_state(dev, TCPC_STATE_UNATTACHED_SNK);
    return;
}
void tcpm_connection_state_machine(void)
{
    u32 cc1 = 0, cc2 = 0 ;

    tcpc_device_t *dev = &tcpc_dev[0];
    snk_cap_t  *dev_snkPdo = &snk_pdo;
//    DEBUG_LOG("        \r\n");
//    DEBUG_LOG("ENTER tcpm_connection_state_machine(void)0 \r\n");

    if (!dev->state_change)
        return;

    DEBUG_LOG("        \r\n");
    DEBUG_LOG("ENTER tcpm_connection_state_machine(void)0 \r\n");

    cc1 = TCPC_CC1_STATE(dev->cc_status);
    cc2 = TCPC_CC2_STATE(dev->cc_status);

//    DEBUG_LOG("dev->cc_status = 0x%x, dev->state = 0x%x \r\n",dev->cc_status, dev->state);

    //return;
    switch (dev->state)
    {
        case TCPC_STATE_UNATTACHED_SNK:

            timer_cancel(TCPC_MODE);
            memset(dev_snkPdo , 0x00 , sizeof(snk_cap_t));
            g_PD_Ver = PD_3_0;
            dev->rp_val = RP_HIGH_CURRENT;

#if USE_QC_SNK_TASK
            if(dev_state_change)
            {
            	dev->connect_mode = QC_CONNECT_MODE;
            	dev_state_change = 0;
                DEBUG_LOG("dev_state_change = %d\r\n", dev_state_change);
            }
            else
			{
            	dev->connect_mode = PD_CONNECT_MODE;
			}
			FP6606_QCSnk_Init();
            tcpc_write8(FP6606_REG_QC_SNK_CTRL_2, 0x00);
            tcpc_write8(FP6606_REG_QC_SNK_STATUS, 0xFF);
#endif

//            tcpc_modify8(FP6606_REG_NMOS_SW_CTRL, NMOS_SNK_ON, 0); //old
            tcpc_modify8(FP6606_REG_NMOS_SW_CTRL, NMOS_SNK_ON, 0x80);	//wellsian

            tcpc_write16(TCPC_REG_VBUS_STOP_DISCHARGE_THRESH, 0);

            tcpc_modify8(TCPC_REG_POWER_CTRL, 0, TCPC_PWR_CTRL_FORCE_DISCHARGE);

            tcpc_modify8(TCPC_REG_CONFIG_STD_OUTPUT, 0, TCPC_AUDIO_ACC_CONNECT);

            tcpc_write8(TCPC_REG_ROLE_CTRL, tcpc_reg_role_ctrl_set(false, dev->rp_val, CC_RD, CC_RD));

            tcpc_write8(TCPC_REG_POWER_CTRL, TCPC_PWR_CTRL_DEFAULTS);

            tcpc_write16(TCPC_REG_VBUS_SINK_DISCONNECT_THRESH, SET_MV(4000));

            tcpc_write8(TCPC_REG_TCPC_CTRL, 0x00);

			pe_notify_conn_state();

            tcpc_modify8(FP6606_REG_TX_CTRL, FP6606_CC1_DIS, 0);

//            tcpc_modify8(FP6606_REG_DBG_FUN, FP6606_CC2_DIS, 0);							//2021/12/9 mark
            tcpc_modify8(FP6606_REG_DBG_FUN, FP6606_CC2_DIS | FP6606_ALERT_DIS_DIS, 0);		//2021/12/9	add

            tcpc_write8(TCPC_REG_COMMAND, TCPC_CMD_SRC_LOOK4CONNECTION);

            break;

        case TCPC_STATE_ATTACHED_SNK:
        case TCPC_STATE_DEBUG_ACC_SNK:
            timer_cancel(TCPC_MODE);
            if (cc1 > cc2)
            {
                dev->plug_polarity = PLUG_UNFLIPPED;
            }
            else
            {
                dev->plug_polarity = PLUG_FLIPPED;
            }
            tcpc_write8(TCPC_REG_TCPC_CTRL,
                        (dev->plug_polarity == PLUG_FLIPPED) ? TCPC_CTRL_PLUG_ORIENTATION : 0);

            if (dev->state == TCPC_STATE_ATTACHED_SNK)
            {
                tcpc_modify8(FP6606_REG_NMOS_SW_CTRL, 0, NMOS_SNK_ON | 0x80);
            }
            tcpc_write8(TCPC_REG_MSG_HDR_INFO, TCPC_REG_MSG_HDR_INFO_SET(0, PD_DATA_ROLE_UFP, g_PD_Ver, PD_PWR_ROLE_SNK));

            /*tcpc_write8(TCPC_REG_POWER_CTRL, (TCPC_PWR_CTRL_AUTO_DISCHARGE_DISCONNECT | TCPC_PWR_CTRL_ENABLE_BLEED_DISCHARGE | TCPC_PWR_CTRL_DISABLE_VOLTAGE_ALARM));*/
            tcpc_write8(TCPC_REG_POWER_CTRL, 0);

            tcpc_write8(FP6606_REG_RX_CTRL , 0x04);

			pe_notify_conn_state();
            break;

        case TCPC_STATE_ERROR_RECOVERY:
            timer_start1(TCPC_MODE, T_ERROR_RECOVERY_MS, timeout_error_recovery);
            break;
        default:
            // Do nothing.
            break;
    }
    dev->state_change = false;
    return;
}


void tcpm_connection_task(void)
{
    tcpm_connection_state_machine();
    return;
}


static void tcpm_reset(void)
{
    tcpc_modify8(FP6606_REG_CC_CTRL, 0,FP6606_GLOBAL_SW_RESET); 	// 0x94, 0, (1 << 5)
//    WaitMs(100);
}

static char tcpm_port_init(const tcpc_config_t *config)
{
    tcpc_device_t *dev = &tcpc_dev[0];
    u8 pwr_status;
    tcpm_reset(); //sw reset
    dev->role = config->role;
    dev->pd_ver = g_PD_Ver;
    dev->port = 0;
    dev->rp_val = config->rp_val;
    dev->state = TCPC_STATE_DISABLED;
    dev->vbus_present = false;

    tcpm_set_state(dev,TCPC_STATE_UNATTACHED_SNK);
    RX_DET_MASK = SOP_DET_MASK(g_sop_flag,g_sop_p_flag,g_sop_p_p_flag,g_dbg_sop_p_flag,g_dbg_sop_p_p_flag,g_hrd_rst_flag,g_cable_rst_flag);
    do
    {
        tcpc_read8(TCPC_REG_POWER_STATUS, &pwr_status);	// 0x1E
//    	DEBUG_LOG("pwr_status value = %d.\r\n",pwr_status);

    }
    while (pwr_status & TCPC_PWR_STATUS_TCPC_INIT_STATUS);

    FP6606_init();
    return 0;
}

char tcpm_init(const tcpc_config_t *config)
{
    char ret;
    ret = tcpm_port_init(&config[0]);
    return ret;
}

bool tcpm_alert_event(void)
{
//	u8 rv_0x10, rv_0x11, rv_0x1F;

//    DEBUG_LOG("        \r\n");
//	DEBUG_LOG("ENTER tcpm_alert_event(void) \r\n");
//	tcpc_read8(0x10, &rv_0x10);
//	tcpc_read8(0x11, &rv_0x11);
//	tcpc_read8(0x1F, &rv_0x1F);
//	DEBUG_LOG("**rv_0x10 = 0x%x, rv_0x11 = 0x%x, rv_0x1F = 0x%x.\r\n", rv_0x10, rv_0x11, rv_0x1F);

//	GPIO_FP_INT_PA0();

    while(!gpio_read(GPIO_FP_INT))
    {
        tcpm_alert_handler();
    }

    return true;
}
