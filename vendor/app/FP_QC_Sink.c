#include "app.h"
#include "light.h"
#include "FP6606.h"
#include "FP_QC_Sink.h"
#include "FP6606_ARM.h"
#include "Global_fun.h"
#include "app_machine.h"
#include "Global_value.h"
#include "FP6606_tcpci.h"
#include "FP6606_common.h"
#include "stack/ble/ble.h"
#include "FP6606_usb_pd_policy_engine.h"

#define LSB_MV               25
#define SET_VOLT(mv)         mv/LSB_MV

#define VOLT_5V              4000
#define VOLT_9V              8000
#define VOLT_12V             11000
#define VOLT_20V             19000

#define T_CONFIG_MS          10		// 5
#define T_EN_MS              100
#define T_SNK_DETECT_MS      2000	// 2000
#define T_DETECT_MS          200	// 200
#define T_DONE_MS            10
#define T_RESULT_MS          10		// 10


#define T_MEAS_VBUS_MS       1000		// 1000
#define T_WAIT_NEXT_STATE_MS 400

static void QCSnk_Config(void);
static void QCSnk_SDPEnable(void);
static void QCSnk_SDPDetect(void);

QC_Sink_t      QC_Snk;
QC_JOB_TYPE    m_QCJob_Content;
QC_DETECT_STEP QC_Detec_Job = DETECT_SDP;

//extern FP6606_PD_QC_timer FP6606_pd_qc;

/*---------------------------------------------------------------
| Name: QCSnk_SDPEnable
| Description: QC task - start SDP Enable
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
static void QCSnk_SDPEnable(void)
{
    tcpc_write8(FP6606_REG_QC_SNK_STATUS, 0xFF);    // 180815 add / clear 0x8B
    tcpc_write8(FP6606_REG_QC_SNK_CTRL_1, SDP_DETECT_EN);
    WaitMs(4);
    tcpc_write8(FP6606_REG_QC_SNK_CTRL_1, SDP_DETECT_EN);
}


/*---------------------------------------------------------------
| Name: QCSnk_SDPDetect
| Description: QC task - start SDP detect
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
static void QCSnk_SDPDetect(void)
{
    unsigned char mode = 0;

    tcpc_read8(FP6606_REG_QC_SNK_STATUS, &mode);

    (mode & SDP_MODE_DETECT)?(QC_Snk.mode = QCSNK_SDP_MODE):(0);
}

/*---------------------------------------------------------------
| Name: QCSnk_DCPDetect
| Description: QC task - start DCP or CDP Enable
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
static void QCSnk_DCPEnable(void)
{
    tcpc_write8(FP6606_REG_QC_SNK_STATUS, 0xFF);    // 180815 add / clear 0x8B

    tcpc_write8(FP6606_REG_QC_SNK_CTRL_1, DCP_DETECT_EN);
    WaitMs(4);
    tcpc_write8(FP6606_REG_QC_SNK_CTRL_1, DCP_DETECT_EN);
}

/*---------------------------------------------------------------
| Name: QCSnk_DCPDetect
| Description: QC task - start DCP or CDP detect
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
static void QCSnk_DCPDetect(void)
{
    unsigned char mode = 0;

    tcpc_read8(FP6606_REG_QC_SNK_STATUS, &mode);

    (mode & DCP_MODE_DETECT)?(QC_Snk.mode = QCSNK_DCP_MODE):(QC_Snk.mode = QCSNK_CDP_MODE);
}

/*---------------------------------------------------------------
| Name: QCSnk_SNKEnable
| Description: QC task - start detect enable
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
static void QCSnk_SNKEnable(void)
{
    tcpc_write8(FP6606_REG_QC_SNK_STATUS, 0xFF);    // 180815 add / clear 0x8B

    tcpc_write8(FP6606_REG_QC_SNK_CTRL_1, SNK_DETECT_EN);
    WaitMs(4);
    tcpc_write8(FP6606_REG_QC_SNK_CTRL_1, SNK_DETECT_EN);
}

/*---------------------------------------------------------------
| Name: QCSnk_SNKDetect
| Description: QC task detect snk mode
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
static void QCSnk_SNKDetect(void)
{
    unsigned char mode = 0;

    tcpc_read8(FP6606_REG_QC_SNK_STATUS, &mode);

    (mode & SNK_MODE_DETECT)?(QC_Snk.mode = QCSNK_SNK_MODE):(0);

}


//---------------------------------------------------------------
// Name: QCSnk_Config
// Description: QC task - config call back function
// Input: N/A
// Output: N/A
// Return: N/A
// Usage Note: N/A
//---------------------------------------------------------------
static void QCSnk_Config(void)
{
    if(QC_Detec_Job == DETECT_SDP)
    {
        m_QCJob_Content.qc_start_enable = QCSnk_SDPEnable;
        m_QCJob_Content.qc_detect_fun = QCSnk_SDPDetect;
    }
    else if((QC_Detec_Job == DETECT_DCP) || (QC_Detec_Job == DETECT_CDP))
    {
        m_QCJob_Content.qc_start_enable = QCSnk_DCPEnable;
        m_QCJob_Content.qc_detect_fun = QCSnk_DCPDetect;
    }
    else if(QC_Detec_Job == DETECT_QC_SNK)
    {
        m_QCJob_Content.qc_start_enable = QCSnk_SNKEnable;
        m_QCJob_Content.qc_detect_fun = QCSnk_SNKDetect;
    }
    else
    {
        //DEBUG("Done\n");
    }
}

/*---------------------------------------------------------------
| Name: QCSnk_Result
| Description: QC snk get detect result
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
static void QCSnk_Result(void)
{
    if(QC_Snk.mode == NONE_MODE)
    {
        (QC_Detec_Job < DETECT_DONE)?(QC_Detec_Job++):(QC_Detec_Job = DETECT_SDP);
    }
    else if(QC_Snk.mode == QCSNK_SDP_MODE)
    {
        QC_Detec_Job = DETECT_SDP;
    }
    else if(QC_Snk.mode == QCSNK_CDP_MODE)
    {
        QC_Detec_Job = DETECT_CDP;
    }
    else if(QC_Snk.mode == QCSNK_DCP_MODE)
    {
        QC_Detec_Job = DETECT_QC_SNK;
    }
    else if(QC_Snk.mode == QCSNK_SNK_MODE)
    {
        //QCSnk_notifyEvent(QC_Snk.mode);
        //QCSnk_2_0_select9V(0);
    }
}

static void QC_set_charge_state(qc_snk_charger_state_t new_state)
{
    QC_Snk.charge_state = new_state;
//    if(new_state >= CHARGER_DETECT)CHARGER_TEST_END
    if(new_state >= CHARGER_TEST_END)
    {
    	QC_Snk.charge_state_change = false;
    }
    else
    {
    	QC_Snk.charge_state_change = true;
    }

//	printf(" QC_charger_state: %s \r\n",QC_charger_state_string[QC_Snk.charge_state]);

    return;
}

/*---------------------------------------------------------------
| Name: QCSnk_notifyEvent
| Description:
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
static void QCSnk_notifyEvent(QC_SNK_MODE mode)
{
    switch(mode)
    {
        case QCSNK_SNK_MODE:
            QC_set_charge_state(CHARGER_STARTUP);
            break;
        default:

            break;
    }
    return;
}
static void QCSnk_state_change(const qc_snk_state_t current_state)
{
    switch(current_state)
    {
        case QC_STATE_INIT:
            break;

        case QC_STATE_DONE:
            QCSnk_notifyEvent(QC_Snk.mode);
            break;

        default:
            break;
    }
}

/*---------------------------------------------------------------
| Name: QCSnk_Done
| Description: QC snk detect done
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
static void QCSnk_Done(void)
{
    QCSnk_state_change(QC_Snk.state);
}

static void QCSnk_clrTimerFunc(void)
{
    QC_Snk.call_back = 0;
};

/************************************************************************************************/
void QCSnk_setState(qc_snk_state_t new_state)
{
    QC_Snk.state = new_state;
    QC_Snk.state_change = true;

    return;
}

void FP6606_QCSnk_Init(void)
{
    m_QCJob_Content.qc_config_fun = QCSnk_Config;
    m_QCJob_Content.qc_detect_fun = 0;
    m_QCJob_Content.qc_result_fun = QCSnk_Result;
    m_QCJob_Content.qc_done_fun = QCSnk_Done;
    m_QCJob_Content.state_change_handler = QCSnk_state_change;
    QCSnk_clrTimerFunc();
    QCSnk_setState(QC_STATE_INIT);
    QC_set_charge_state(CHARGER_INIT);
    QC_Snk.mode = NONE_MODE;               // 180805 add

}

void usb_qc_pe_connection_state_change_handler(tcpc_state_t state)
{
	DEBUG_LOG("state = %d\r\n", state);
    switch (state)
    {
        case TCPC_STATE_UNATTACHED_SNK:
            timer_cancel(ALL_MODE);
            QCSnk_setState(QC_STATE_INIT);

            break;

        case TCPC_STATE_ATTACHED_SNK:
			QCSnk_setState(QC_STATE_STARTUP);

            break;
        default:
            break;
    }

    return;
}

static void QCSnk_reset (bool en)
{
    tcpc_write8(FP6606_REG_QC_SNK_CTRL_3, en ) ;//0x8A.Bit0=QCSnk_reset
}

// time out
static void QC_timeout_config(void)
{
//	DEBUG_LOG("***QC_timeout_config 01\r\n");
    QCSnk_setState(QC_STATE_CONFIG);
}

static void QC_timeout_enable(void)
{
//	DEBUG_LOG("***QC_timeout_enable 02\r\n");
    QCSnk_setState(QC_STATE_ENABLE);
}

static void QC_timeout_detect(void)
{
//	DEBUG_LOG("***QC_timeout_detect 03\r\n");
    QCSnk_setState(QC_STATE_DETECT);
}

static void QC_timeout_result(void)
{
//	DEBUG_LOG("***QC_timeout_result 04\r\n");
    QCSnk_setState(QC_STATE_RESULT);
}

static void QC_timeout_done(void)
{
//	DEBUG_LOG("***QC_timeout_config 01\r\n");
    QCSnk_setState(QC_STATE_DONE);
}

static void QCSnk_setTimerFunc(void (*function)(void))
{
    QC_Snk.call_back = function;
};

static int QCSnk_startTimer(unsigned short timeout_ms)
{
	timeout_ms /= 10;
	QC_Snk.qc_timer = timeout_ms;
//	FP6606_pd_qc.pd_qc_timer = timeout_ms;
    return 0;
};

static int QCSnk_setTimerCallBak(unsigned short timeout_ms,void (*function)(void))
{
    QCSnk_setTimerFunc(*function);
    QCSnk_startTimer(timeout_ms);
//    fp6606_start_timer_flag = 1;
    return 0;
};

//---------------------------------------------------------------
// Name: QCSnk_getVbusVoltage
// Description: get vbus voltage
// Input: N/A
// Output: N/A
// Return: N/A
// Usage Note: N/A
//---------------------------------------------------------------
static unsigned short QCSnk_getVbusVoltage(void)
{
    unsigned short volt;

    tcpc_read16(TCPC_REG_VBUS_VOLTAGE, &volt);

    qc_voltage = volt;

    return(volt & 0x3FF);
}

//---------------------------------------------------------------
// Name: QCSnk_getVbusReady
// Description: check vbus is ready
// Input: N/A
// Output: N/A
// Return: N/A
// Usage Note: N/A
//---------------------------------------------------------------
static bool QCSnk_getVbusReady(unsigned short vbus)
{
    bool res = false;

    switch(QC_Snk.select_volt)
    {
        case TYPE_5_V:
            if(vbus >= SET_VOLT(VOLT_5V))
                res = true;
            break;

        case TYPE_9_V:
            if(vbus >= SET_VOLT(VOLT_9V))
                res = true;
            break;

        case TYPE_12_V:
            if(vbus >= SET_VOLT(VOLT_12V))
                res = true;
            break;

        case TYPE_20_V:
            if(vbus >= SET_VOLT(VOLT_20V))
                res = true;
            break;
    }

//	DEBUG_LOG("*********.\r\n");
    return res;
}

//---------------------------------------------------------------
// Name: QC_mesaVbustimeout
// Description:
// Input: N/A
// Output: N/A
// Return: N/A
// Usage Note: N/A
//---------------------------------------------------------------
static void QC_mesaVbustimeout(void)
{
	if(QC_Snk.charge_state == CHARGER_SET_VOLT_9V)
	{
//		QC_set_charge_state(CHARGER_SET_VOLT_12V);
		if(QCSnk_getVbusReady(QCSnk_getVbusVoltage()))
		{
			// power_ic = 0;
			// light_init();

            switch_qc = 1;
		}
	}
	else if(QC_Snk.charge_state == CHARGER_SET_VOLT_12V)
	{
		QC_set_charge_state(CHARGER_GOTO_QC3);
	}
	else if(QC_Snk.charge_state == CHARGER_GOTO_QC3)
	{
//		QC_set_charge_state(CHARGER_GOTO_QC3);
	}
}

//---------------------------------------------------------------
// Name: QCSnk_chargeGoNextState
// Description: change QC charge state
// Input: N/A
// Output: N/A
// Return: N/A
// Usage Note: N/A
//---------------------------------------------------------------
static void QCSnk_chargeGoNextState(void)
{
    if(QC_Snk.charge_state == CHARGER_STARTUP)
    {
    	QC_set_charge_state(CHARGER_SET_VOLT_9V);
    }
    else if(QC_Snk.charge_state == CHARGER_SET_VOLT_9V)
    {
    	if(QCSnk_getVbusReady(QCSnk_getVbusVoltage()))
    	{
        	// power_ic = 0;
        	// light_init();
            switch_qc = 1;
    	}
    }
    else if(QC_Snk.charge_state == CHARGER_SET_VOLT_12V)
    {
    	QC_set_charge_state(CHARGER_GOTO_QC3);
    }
    else if(QC_Snk.charge_state == CHARGER_GOTO_QC3)
    {
//    	if(qc_voltage_check())
//    	{
//        	QC_set_charge_state(CHARGER_QC3_VOLT_RISING);
//    	}
//    	else
//    	{
//    		power_ic = 0;
//    	}
    }
    else if(QC_Snk.charge_state == CHARGER_QC3_VOLT_RISING)
    {
//    	QC_set_charge_state(CHARGER_QC3_VOLT_FALLING);
    }
}

// set vol function
static void QCSnk_2_0_select5V(void)
{
    tcpc_write8(FP6606_REG_QC_SNK_CTRL_2, 0x00 ) ;//
}

static void QCSnk_2_0_select9V(void)
{
    tcpc_write8(FP6606_REG_QC_SNK_CTRL_2, 0x03 ) ;//D+ = 3.3V,D- = 0.6V
	DEBUG_LOG("QCSnk_2_0_select9V\r\n");
}

static void QCSnk_2_0_select12V(void)
{
    tcpc_write8(FP6606_REG_QC_SNK_CTRL_2, 0x02 ) ;//D+ = 0.6V,D- = 0.6V
}

static void QCSnk_2_0_select20V(void)
{
    tcpc_write8(FP6606_REG_QC_SNK_CTRL_2, 0x05 ) ;//D+ = 3.3V,D- = 3.3V
}

static void QCSnk_3_0_setContinuousMode(void)
{
    tcpc_write8(FP6606_REG_QC_SNK_CTRL_2, 0x04 ) ;//D+ = 0.6V,D- = 3.3V
}

static void QCSnk_detectVbusADC(void)
{
    tcpc_write8(TCPC_REG_COMMAND, TCPC_CMD_ENABLE_VBUS_DETECT);
	DEBUG_LOG("QCSnk_detectVbusADC\r\n");
}

//---------------------------------------------------------------
// Name: QCSnk_setVolt
// Description: setting QC volt
// Input: N/A
// Output: N/A
// Return: N/A
// Usage Note: N/A
//---------------------------------------------------------------
static void QCSnk_setVolt(volt_type_t volt)
{
    QC_Snk.select_volt = volt;

    switch(volt)
    {
        case TYPE_5_V:
            QCSnk_2_0_select5V();
            break;

        case TYPE_9_V:
            QCSnk_2_0_select9V();
            break;

        case TYPE_12_V:
            QCSnk_2_0_select12V();
            break;

        case TYPE_20_V:
            QCSnk_2_0_select20V();
            break;
    }
}

/*---------------------------------------------------------------
| Name: QCSnk_stateMachine
| Description: QC task - Process QC State Machine
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
static void QCSnk_stateMachine(void)
{
    if(!QC_Snk.state_change)   // set in QCSnk_setState()
    {
    	return;
    }

	DEBUG_LOG("QC_Snk.state = %d, QC_Snk.mode = %d\r\n", QC_Snk.state, QC_Snk.mode);
    switch(QC_Snk.state)
    {
        case QC_STATE_INIT:
            QC_Detec_Job = DETECT_SDP;
            //QC_Detec_Job = DETECT_QC_SNK;    // test
            QC_Snk.mode = NONE_MODE;
            QCSnk_reset(1);
            break;

        case QC_STATE_STARTUP:
            QCSnk_reset(0);
            QCSnk_setTimerCallBak(T_CONFIG_MS , QC_timeout_config);

			tcpc_modify8(FP6606_REG_NMOS_SW_CTRL, 0, NMOS_SNK_ON | 0x80);
            break;

        case QC_STATE_CONFIG:
            if(m_QCJob_Content.qc_config_fun != NULL)
            {
                m_QCJob_Content.qc_config_fun();
                QCSnk_setTimerCallBak(T_EN_MS , QC_timeout_enable);
            }
            break;

        case QC_STATE_ENABLE:
            if(m_QCJob_Content.qc_start_enable != NULL)
            {
                m_QCJob_Content.qc_start_enable();

                WaitMs(500);

                if(QC_Detec_Job == DETECT_QC_SNK)
                {
                	QCSnk_setTimerCallBak(T_SNK_DETECT_MS , QC_timeout_detect);
                }
                else
                {
                	QCSnk_setTimerCallBak(T_DETECT_MS , QC_timeout_detect);
                }
            }
            break;

        case QC_STATE_DETECT:
            if(m_QCJob_Content.qc_detect_fun != NULL)
            {
                m_QCJob_Content.qc_detect_fun();
                QCSnk_setTimerCallBak(T_RESULT_MS , QC_timeout_result);
            }
            break;

        case QC_STATE_RESULT:
            if(m_QCJob_Content.qc_result_fun != NULL)
            {
                m_QCJob_Content.qc_result_fun();

                if(QC_Snk.mode == NONE_MODE)
                {
                    QCSnk_setTimerCallBak(T_CONFIG_MS , QC_timeout_config);
                }
                else if(QC_Snk.mode == QCSNK_DCP_MODE)
                {
                    QC_Detec_Job = DETECT_QC_SNK;       // why rewrite again
                    QCSnk_setTimerCallBak(T_CONFIG_MS , QC_timeout_config);
                }
                else
                {
                    QCSnk_setTimerCallBak(T_DONE_MS , QC_timeout_done);
                }
            }
            break;

        case QC_STATE_DONE:
            if(m_QCJob_Content.qc_done_fun != NULL)
            {
                m_QCJob_Content.qc_done_fun();
            }
            break;

    }
    QC_Snk.state_change = false;
}

/*---------------------------------------------------------------
| Name: QC_charger_machine
| Description:
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
static void QC_charger_machine(void)
{
    if(!QC_Snk.charge_state_change)
        return;

	DEBUG_LOG("QC_Snk.charge_state = %d\r\n", QC_Snk.charge_state);

	//---------------------------------
    // charger state machine / mask now
    //---------------------------------
    switch(QC_Snk.charge_state)
    {
        case CHARGER_INIT:
            break;

        case CHARGER_STARTUP:

            QCSnk_setTimerCallBak(200, QCSnk_chargeGoNextState);
//            QCSnk_setTimerCallBak(T_MEAS_VBUS_MS, QCSnk_chargeGoNextState);
            break;

        case CHARGER_SET_VOLT_9V:
            QCSnk_setVolt(TYPE_9_V);
            QCSnk_detectVbusADC();
            QCSnk_setTimerCallBak(300, QCSnk_chargeGoNextState);
//            QCSnk_setTimerCallBak(T_MEAS_VBUS_MS, QCSnk_chargeGoNextState);
            break;

       case CHARGER_SET_VOLT_12V:
            QCSnk_setVolt(TYPE_12_V);
            QCSnk_detectVbusADC();
            QCSnk_setTimerCallBak(T_MEAS_VBUS_MS, QCSnk_chargeGoNextState);

        case CHARGER_DETECT:
            QCSnk_setTimerCallBak(T_MEAS_VBUS_MS, QC_mesaVbustimeout);
            break;

        case CHARGER_GOTO_QC3:
        	DEBUG_LOG("CHARGER_GOTO_QC3 \r\n");
            QCSnk_3_0_setContinuousMode();
            QCSnk_setTimerCallBak(T_WAIT_NEXT_STATE_MS, QCSnk_chargeGoNextState);
            break;

        case CHARGER_QC3_VOLT_RISING:
            break;
    }

    QC_Snk.charge_state_change = 0;
}

/*---------------------------------------------------------------
| Name: QC_checkEventTimOut
| Description: QC Task Timer out check event
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
//static void QC_checkEventTimOut(void)
void QC_checkEventTimOut(void)
{
    if((QC_Snk.call_back) && (QC_Snk.qc_timer == 0))
//        if((QC_Snk.call_back) && (FP6606_pd_qc.pd_qc_timer == 0))
    {
        QC_Snk.call_back();
        QC_Snk.call_back = NULL;
    }
}

/*---------------------------------------------------------------
| Name: QC_task
| Description: QC Task
| Input: N/A
| Output: N/A
| Return: N/A
| Usage Note: N/A
---------------------------------------------------------------*/
void FP6606_QCSnk_task(void)
{
    QCSnk_stateMachine();     // detect mode
    QC_charger_machine();     // QC2.0 & 3.0
    return;
}
