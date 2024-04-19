#include "pd_qc.h"
#include "FP6606_ARM.h"
#include <tl_common.h>

/*----------------------------------------------------------------------------
 PHY Event Alert
*----------------------------------------------------------------------------*/
void FP6606_ARM_eventHandler(void)
{
    fp6606_pd.alert_status = 1;
}


void GPIO_FP_INT_EXTI_Callback()
{
    FP6606_ARM_eventHandler();
}






















