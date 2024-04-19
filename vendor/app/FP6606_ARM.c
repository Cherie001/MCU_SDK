#include "log.h"
#include "app.h"
#include "pd_qc.h"
#include "FP6606_ARM.h"
#include "global_fun.h"
#include "global_value.h"
#include "FP6606_common.h"
#include <tl_common.h>
#include "FP_QC_Sink.h"
#include "FP6606_tcpm.h"
#include "stack/ble/ble.h"
#include "FP6606_usb_pd_policy_engine.h"



#define VOL_5V  5000
#define VOL_9V  9000
#define VOL_12V 12000	//wellsian
#define VOL_15V 15000	//wellsian

u8  fp6606_start_timer_flag = 0;
u8  fp6606_timer_task = 0;
u16 count_qc = 0;


extern void QC_checkEventTimOut(void);

//FP6606_PD_QC_timer FP6606_pd_qc;

FP6606_pwr_delivery fp6606_pd =
{
    .configuration.role = ROLE_SNK,		// ROLE_SNK = 1
    .configuration.rp_val = RP_HIGH_CURRENT,	// RP_HIGH_CURRENT = 2        /* 3.0A */
/*
#ifdef SNK_PPS
    .port_config.num_snk_pdos = 3,
#else
    .port_config.num_snk_pdos = 2,
#endif
*/
#ifdef SNK_PPS 				//wellsian
    .port_config.num_snk_pdos = 5,
#else
    .port_config.num_snk_pdos = 4,
#endif

    .port_config.snk_caps[0] =
    {
        .SupplyType = SUPPLY_TYPE_FIXED,
        .PeakI = PEAK_CURRENT_2,
        .MinV = PDO_VOLT(VOL_5V),    // NA
        .MaxV = PDO_VOLT(VOL_5V),    // NA
        .MaxOperatingCurrent = PDO_CURR(3000),
        .MinOperatingCurrent = PDO_CURR(500),
        .OperationalCurrent = PDO_CURR(2900),
        .MaxOperatingPower = 0,      // N/A
        .MinOperatingPower = 0,      // N/A
        .OperationalPower = 500,     // N/A    // no use in fixed
    },

    .port_config.snk_caps[1] =
    {
        .SupplyType = SUPPLY_TYPE_FIXED,
        .PeakI = PEAK_CURRENT_2,
        .MinV = PDO_VOLT(VOL_9V),
        .MaxV = PDO_VOLT(VOL_9V),
        .MaxOperatingCurrent = PDO_CURR(2700),
        .MinOperatingCurrent = PDO_CURR(500),
        .OperationalCurrent = PDO_CURR(2500),
        .MaxOperatingPower = 0,
        .MinOperatingPower = 0,
        .OperationalPower = 500,
    },

    .port_config.snk_caps[2] =
    {
        .SupplyType = SUPPLY_TYPE_FIXED,
        .PeakI = PEAK_CURRENT_2,
        .MinV = PDO_VOLT(VOL_12V),
        .MaxV = PDO_VOLT(VOL_12V),
        .MaxOperatingCurrent = PDO_CURR(3000),
        .MinOperatingCurrent = PDO_CURR(500),
        .OperationalCurrent = PDO_CURR(2700),
        .MaxOperatingPower = 0,
        .MinOperatingPower = 0,
        .OperationalPower = 500,
    },

    .port_config.snk_caps[3] =
    {
        .SupplyType = SUPPLY_TYPE_FIXED,
        .PeakI = PEAK_CURRENT_2,
        .MinV = PDO_VOLT(VOL_15V),
        .MaxV = PDO_VOLT(VOL_15V),
        .MaxOperatingCurrent = PDO_CURR(2700),
        .MinOperatingCurrent = PDO_CURR(1500),
        .OperationalCurrent = PDO_CURR(2000),
        .MaxOperatingPower = 0,
        .MinOperatingPower = 0,
        .OperationalPower = 500,
    },
#ifdef SNK_PPS
    .port_config.snk_caps[4] =
    {
        .SupplyType = SUPPLY_TYPE_APDO,
        .Psy = PROGRAM_PSY,
        .MaxV = 110,
        .MinV = 50,
        .MaxOperatingCurrent = 60,            // maximum current / 50mA unit
        .OperationalVoltage = 250,            // 250 = 5V / 20mV per bit
    },
#endif

};

//char FP6606_ARM_i2cRead(unsigned char dev_addr,unsigned char reg, unsigned char *value, unsigned short num_of_regs)
//{
//    I2C_StatusTypeDef ret = I2C_ERROR;
//    if(g_i2c_type.I2cRead)
//    {
//        while((ret = g_i2c_type.I2cRead(dev_addr , reg , value , num_of_regs)) != I2C_OK)
//        {
//            FP6606_ARM_i2cErrorHanlder(dev_addr,ret);
//        }
//    }
//        return ret;}
//return 0;
//}


/*----------------------------------------------------------------------------
 initialize PD Config
*----------------------------------------------------------------------------*/
static void FP6606_ARM_init(FP6606_pwr_delivery *FP6606_pd)
{
    usb_pd_init(&FP6606_pd->port_config);
}

/*----------------------------------------------------------------------------
 Get Config From Flash
*----------------------------------------------------------------------------*/
static SET_RESULT FP6606_ARM_setConfig(FP6606_pwr_delivery *FP6606_pd)
{
    SET_RESULT ret ;

    g_PD_Ver = 2;       // PD3.0
    g_sop_flag = 1;
    g_sop_p_flag = 0;
    g_sop_p_p_flag = 0;
    g_hrd_rst_flag = 1;
    g_cable_rst_flag = 0;

    ret = (SET_RESULT)tcpm_init(&FP6606_pd->configuration);

    return ret;
}




/*----------------------------------------------------------------------------
 Check Timer Event Task
*----------------------------------------------------------------------------*/
static void FP6606_ARM_checkEventTimOut(void)
{
//	DEBUG_LOG("        \r\n");
//	DEBUG_LOG("Enter FP6606_ARM_checkEventTimOut(void).\r\n");
    if((fp6606_pd.pd_call_back) && (fp6606_pd.pd_timer == 0))
    {
        fp6606_pd.pd_call_back();
        fp6606_pd.pd_call_back = NULL;
    }

    if((fp6606_pd.tcpc_call_back) && (fp6606_pd.tcpc_timer == 0))
    {
        fp6606_pd.tcpc_call_back();
        fp6606_pd.tcpc_call_back = NULL;
    }

    if((fp6606_pd.pd_response_back) && (fp6606_pd.pd_response_back == 0))
    {
        fp6606_pd.pd_response_back();
        fp6606_pd.pd_response_back = NULL;
    }

    if((fp6606_pd.pps_call_back) && (fp6606_pd.pps_timer == 0))
    {
        fp6606_pd.pps_call_back();
        fp6606_pd.pps_call_back = NULL;
    }
}

/*----------------------------------------------------------------------------
  Set Timer CallBack Function
*----------------------------------------------------------------------------*/
void FP6606_ARM_setTimerFunc(DEV_MODE dev_mode, void (*function)(void))
{
//	DEBUG_LOG("        \r\n");
//	DEBUG_LOG("Enter FP6606_ARM_setTimerFunc(DEV_MODE dev_mode, void (*function)(void))\r\n");
    switch(dev_mode)
    {
        case TCPC_MODE:
            fp6606_pd.tcpc_call_back = function;
            break;

        case PD_MODE:
            fp6606_pd.pd_call_back = function;
            break;

        case PD_NO_RESPONSE:
            fp6606_pd.pd_response_back = function;
            break;

        case PPS_MODE:
            fp6606_pd.pps_call_back = function;
            break;
    }
    return;
};


/*----------------------------------------------------------------------------
  Clear Timer CallBack Function
*----------------------------------------------------------------------------*/
void FP6606_ARM_clrTimerFunc(DEV_MODE dev_mode)
{
    switch(dev_mode)
    {
        case TCPC_MODE:
            fp6606_pd.tcpc_call_back = NULL;
            break;

        case PD_MODE:
            fp6606_pd.pd_call_back = NULL;
            break;

        case PD_NO_RESPONSE:
            fp6606_pd.pd_response_back = NULL;
            break;

        case PPS_MODE:
            fp6606_pd.pps_call_back = NULL;
            break;

        case ALL_MODE:
            fp6606_pd.tcpc_call_back = NULL;
            fp6606_pd.pd_call_back = NULL;
            fp6606_pd.pd_response_back = NULL;
            fp6606_pd.pps_call_back = NULL;
            break;
    }
    return ;
};

/*----------------------------------------------------------------------------
  Start Timer Task
*----------------------------------------------------------------------------*/
void FP6606_ARM_startTimer(DEV_MODE dev_mode, unsigned int timeout_ms)
{
	timeout_ms /= 10;

    switch(dev_mode)
    {
        case TCPC_MODE:
        	fp6606_pd.tcpc_timer = timeout_ms;
        	FP6606_pd_qc.pd_qc_timer = fp6606_pd.tcpc_timer;
            break;

        case PD_MODE:
        	fp6606_pd.pd_timer = timeout_ms;
        	FP6606_pd_qc.pd_qc_timer = fp6606_pd.pd_timer;
            break;

        case PD_NO_RESPONSE:
        	fp6606_pd.pd_no_response = timeout_ms;
        	FP6606_pd_qc.pd_qc_timer = fp6606_pd.pd_no_response;
            break;

        case PPS_MODE:
        	fp6606_pd.pps_timer = timeout_ms;
        	FP6606_pd_qc.pd_qc_timer = fp6606_pd.pps_timer;
            break;
    }
    return;
};

/*----------------------------------------------------------------------------
  Stop Timer Task
*----------------------------------------------------------------------------*/
void FP6606_ARM_stopTimer(DEV_MODE dev_mode)
{
    FP6606_ARM_clrTimerFunc(dev_mode);
    switch(dev_mode)
    {
        case TCPC_MODE:
            fp6606_pd.tcpc_timer = 0;
        	FP6606_pd_qc.pd_qc_timer = fp6606_pd.tcpc_timer;
            break;

        case PD_MODE:
            fp6606_pd.pd_timer = 0;
        	FP6606_pd_qc.pd_qc_timer = fp6606_pd.pd_timer;
            break;

        case PD_NO_RESPONSE:
            fp6606_pd.pd_no_response = 0;
        	FP6606_pd_qc.pd_qc_timer = fp6606_pd.pd_no_response;
            break;

        case PPS_MODE:
            fp6606_pd.pps_timer = 0;
        	FP6606_pd_qc.pd_qc_timer = fp6606_pd.pps_timer;
            break;
        default:
        	break;
    }
    return;
};


/*----------------------------------------------------------------------------
 Timer Task
*----------------------------------------------------------------------------*/
static void Timer_task(void)
{
//	if(fp6606_timer_task)
	{
		fp6606_timer_task = 0;

	    FP6606_ARM_checkEventTimOut();
    	QC_checkEventTimOut();    		// Timer Event
	}
}

/*----------------------------------------------------------------------------
 Init FP6606 Main Task
*----------------------------------------------------------------------------*/
void FP6606_FunInit(void)
{
	SET_RESULT ret;

	ret = FP6606_ARM_setConfig(&fp6606_pd);

    if(ret == SET_FALSE) //always return 0
    {
        while(1)
        {
        	DEBUG_LOG("error.\r\n");
        }
    }
    FP6606_ARM_init(&fp6606_pd);

#if USE_QC_SNK_TASK
    FP6606_QCSnk_Init();
#endif

    FP6606_ARM_clrTimerFunc(ALL_MODE);
}

void pd_qc_count(void)
{
	if(fp6606_pd.pd_timer)
	{
		fp6606_pd.pd_timer --;
//			DEBUG_LOG(" fp6606_pd.pd_timer = %d.\r\n", fp6606_pd.pd_timer);
	}

	if(fp6606_pd.tcpc_timer)
	{
		fp6606_pd.tcpc_timer --;
//			DEBUG_LOG(" fp6606_pd.tcpc_timer = %d.\r\n", fp6606_pd.tcpc_timer);
	}

	if(fp6606_pd.pd_no_response)
	{
		fp6606_pd.pd_no_response --;
//			DEBUG_LOG(" fp6606_pd.pd_no_response = %d.\r\n", fp6606_pd.pd_no_response);
	}

	if(fp6606_pd.pps_timer)
	{
		fp6606_pd.pps_timer --;
//			DEBUG_LOG(" fp6606_pd.pps_timer = %d.\r\n", fp6606_pd.pps_timer);
	}
	if(QC_Snk.qc_timer)
	{
		QC_Snk.qc_timer--;
//			DEBUG_LOG(" QC_Snk.qc_timer = %d.\r\n", QC_Snk.qc_timer);
	}
	/*********************************************/

	if(count_qc >= 800)
	{
		switch_qc = 1;
//					DEBUG_LOG("timer count_qc = %d.\r\n", count_qc);
	}
	else
	{
		count_qc ++;
//								DEBUG_LOG("timer count_qc = %d.\r\n", count_qc);
//		switch_qc = 0;
	}
}

/*----------------------------------------------------------------------------
 FP6606 Main Task
*----------------------------------------------------------------------------*/
void FP6606_ARM_work(void)
{
	if(fp6606_pd.alert_status)
	{
		fp6606_pd.alert_status = 0;
		tcpm_alert_event();
	}

	Timer_task();

	tcpm_connection_task();            // handle tcpm state machine

	usb_pd_pe_state_machine();

#if USE_QC_SNK_TASK
	FP6606_QCSnk_task();
#endif
}



























