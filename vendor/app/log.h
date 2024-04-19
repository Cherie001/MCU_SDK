/********************************************************************************************************
 * @file     ota.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#ifndef _LOG_H
#define _LOG_H

#include "app_config.h"
#include "tl_common.h"

#define	DEBUG_LOG				printf
#define DEBUG_LOG_BUF           array_printf
void log_init(void);
void uart_init1(void);
void debug_log(u8 log_flag);

#endif /* APP_H_ */
