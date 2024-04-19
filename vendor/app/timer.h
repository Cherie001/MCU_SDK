/********************************************************************************************************
 * @file     timer.h
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#ifndef _TIMER_H
#define _TIMER_H

#include "app_config.h"

#define HW_TIMER_BASE_TOMS		10

void timer_init(void);
void timer_process(void);
#endif /* APP_H_ */
