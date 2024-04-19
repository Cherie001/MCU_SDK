#ifndef _FIXED_TIME_H_
#define _FIXED_TIME_H_


#define	 TIME_SCAALE 	900		// 15 minute
//#define	 TIME_SCAALE 	10		// 15 minute

extern u8 shut_down_flag;
extern u8 shut_down_time;

extern u16 gDownTick;

void shut_down_process(void);

#endif

