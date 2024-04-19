#ifndef FP_QC_SINK_H
#define FP_QC_SINK_H
#include "global_value.h"
#include <tl_common.h>




typedef enum
{
    QC_STATE_INIT    = 0,
    QC_STATE_STARTUP = 1,
    QC_STATE_CONFIG  = 2,
    QC_STATE_ENABLE  = 3,
    QC_STATE_DETECT  = 4,
    QC_STATE_RESULT  = 5,
    QC_STATE_DONE    = 6
} qc_snk_state_t;

typedef enum
{
    CHARGER_INIT 				= 0,
    CHARGER_STARTUP    			= 1,
    CHARGER_SET_VOLT_9V			= 2,
    CHARGER_SET_VOLT_12V		= 3,
    CHARGER_DETECT				= 4,
    CHARGER_GOTO_QC3			= 5,
    CHARGER_QC3_VOLT_RISING		= 6,
    CHARGER_QC3_VOLT_FALLING	= 7,
    CHARGER_TEST_END			= 8
} qc_snk_charger_state_t;

typedef enum
{
    TYPE_5_V 	= 0,
    TYPE_9_V 	= 1,
    TYPE_12_V 	= 2,
    TYPE_20_V	= 3
} volt_type_t;



// 1=SDP,2=CDP,3=DCP
typedef enum
{
    NONE_MODE = 0,
    QCSNK_SDP_MODE ,	//1
    QCSNK_CDP_MODE ,	//2
    QCSNK_DCP_MODE ,	//3
    QCSNK_SNK_MODE		//4
} QC_SNK_MODE;


typedef enum
{
    // QC Snk Ctrl
    SNK_DETECT_EN = (1 << 2),
    DCP_DETECT_EN = (1 << 1),
    SDP_DETECT_EN = (1 << 0),
} qc_snk_ctrl_1_t;

typedef struct
{
    void (*call_back) (void);
    unsigned int           qc_timer;
    QC_SNK_MODE                mode;
    bool                   state_change;
    bool                   charge_state_change;
    qc_snk_state_t         state;
    qc_snk_charger_state_t charge_state;
    volt_type_t            select_volt;
}QC_Sink_t;

typedef enum
{
    DETECT_SDP = 0,
    DETECT_DCP ,	//1
    DETECT_CDP ,	//2
    DETECT_QC_SNK ,	//3
    DETECT_DONE,	//4
} QC_DETECT_STEP;


typedef void (*QC_CONFIG)(void);
typedef void (*QC_START_ENABLE)(void);
typedef void (*QC_DETECT)(void);
typedef void (*QC_RESULT)(void);
typedef void (*QC_DONE)(void);
typedef void (*STATE_CHANGE)(qc_snk_state_t current_state);

typedef struct
{
    //QC_INIT qc_init_fun;
    QC_CONFIG qc_config_fun;
    QC_START_ENABLE qc_start_enable;
    QC_DETECT qc_detect_fun;
    QC_RESULT qc_result_fun;
    QC_DONE qc_done_fun;
    STATE_CHANGE state_change_handler;
} QC_JOB_TYPE;


typedef enum
{
    DCP_MODE_DETECT = (1 << 2),
    SDP_MODE_DETECT = (1 << 1),
    SNK_MODE_DETECT = (1 << 0),
} qc_snk_status_t;

extern QC_Sink_t      QC_Snk;;

void FP6606_QCSnk_task(void);
void FP6606_QCSnk_Init(void);

#endif



















