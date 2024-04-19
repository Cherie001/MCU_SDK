#ifndef _FP6606_ARM_H_
#define _FP6606_ARM_H_


#include "FP6606_common.h"

typedef struct
{
    void (*pd_call_back) (void);      // for pd
    void (*tcpc_call_back) (void);    // for tcpc
    void (*pd_response_back)  (void); // for no response
    void (*pps_call_back) (void);     // for pps
    tcpc_config_t        configuration;
    usb_pd_port_config_t port_config;
    unsigned char          alert_status;
    unsigned short         pd_timer;
    unsigned short         pd_no_response;
    unsigned short         pps_timer;
    unsigned short         tcpc_timer;
}FP6606_pwr_delivery;

typedef struct
{
    u16		pd_qc_timer;
}FP6606_PD_QC_timer;

FP6606_PD_QC_timer FP6606_pd_qc;

extern u8 fp6606_start_timer_flag;
extern u8 fp6606_timer_task;
extern u8 g_pd_tcpc_timer;
extern u8 g_pd_pd_timer;
extern u8 g_pd_no_response;
extern u8 g_pd_pps_timer;
extern FP6606_pwr_delivery fp6606_pd;

void FP6606_FunInit(void);
void FP6606_ARM_work(void);
void pd_qc_count(void);

#endif
