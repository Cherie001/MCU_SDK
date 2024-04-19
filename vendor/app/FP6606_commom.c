#include "light.h"
#include "FP6606_ARM.h"
#include "FP6606_common.h"
#include "global_fun.h"
#include <tl_common.h>


void tcpc_write_block(u8 reg, u8 *data, u8 len)
{
	iic_send_buff(reg, 1, data, len);
}

void tcpc_read8(u8 reg, u8 *data)
{
	*data = iic_read_byte(reg, 1);
}


void tcpc_write8(u8 reg, u8 data)
{
	iic_send_byte(reg, 1, data);
};


void tcpc_write16(u8 reg, unsigned short data)
{
	u8 low  = (u8)(data & 0x00FF);
	u8 high = (u8)(data >> 8);

	iic_send_byte(reg, 1, low);
	iic_send_byte(reg+1, 1, high);
}

// Modifies an 8-bit register.
void tcpc_modify8(u8 reg, u8 clr_mask, u8 set_mask)
{
	u8 clr_bit = (clr_mask | set_mask);

	u8 data_b = iic_read_byte(reg, 1);
	u8 data_l = set_mask ? (data_b | set_mask) : (data_b & (~clr_bit & 0xFF));
	iic_send_byte(reg, 1, data_l);
}

void tcpc_read_block(u8 reg, u8 *data, unsigned short len)
{
	i2c_dma_read_buff(reg, 1, data, len);
}

void timer_start1(DEV_MODE dev_mode,unsigned short timeout_ms,void (*function)(void))
{
    FP6606_ARM_setTimerFunc(dev_mode, *function);
    FP6606_ARM_startTimer(dev_mode, timeout_ms);
}


void timer_cancel(DEV_MODE dev_mode)
{
    FP6606_ARM_clrTimerFunc(dev_mode);
    FP6606_ARM_stopTimer(dev_mode);
}






