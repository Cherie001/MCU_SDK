#ifndef GLOBAL_FUN_H_
#define GLOBAL_FUN_H_



typedef enum
{
    TCPC_MODE = 0,
    PD_MODE,
    PD_NO_RESPONSE,
    PPS_MODE,
    ALL_MODE
}DEV_MODE;

typedef enum
{
    TX_STATUS_SUCCESS = 0,
    TX_STATUS_DISCARDED,
    TX_STATUS_FAILED
} tx_status_t;





void FP6606_ARM_clrTimerFunc(DEV_MODE dev_mode);
void FP6606_ARM_startTimer(DEV_MODE dev_mode, unsigned int timeout_ms);
void FP6606_ARM_stopTimer(DEV_MODE dev_mode);




















#endif
