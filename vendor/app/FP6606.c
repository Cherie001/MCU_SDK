#include "log.h"
#include "light.h"
#include "FP6606.h"
#include "FP6606_common.h"


#define CC_SAMPLE_RATE(rate)  (rate & 0x3) << 2
#define CC_DEBOUNCE_CHG(cnt)  (cnt & 0x3) << 5


void FP6606_init(void)
{
//    u8 i;
	tcpc_write8(0x80, 1 << 4 );
    tcpc_write16(0x12, 0x8f8f);
    tcpc_write8(0xC0, 0x01);
    tcpc_write8(0x98, 0x10);
    tcpc_write8(0x25, 0x10 | 0x04 | 0x08);
    tcpc_write8(0x26, 0xD0);
    tcpc_write8(0x81, 0x30);
    tcpc_write8(0x1B, 0x09);
    tcpc_write8(0x94, CC_SAMPLE_RATE(1));
    tcpc_write8(0x87, (0xA0 | CC_DEBOUNCE_CHG(0) | (1<<0)));
    tcpc_write8(0x9D , 1 << 4);
    tcpc_modify8(0x81, 0, 1 << 7);
    tcpc_modify8(0x82, 0, 1 << 7);
    tcpc_write8(0x1F , 0x80);
    tcpc_write16(0x10 , 0x0200);
    tcpc_write8(0x9C, 0x84);
    tcpc_write16(0x92, 0x000c);	//(FP6606_INT_VCONN_OC | FP6606_INT_OTSD1_STAT ));
    tcpc_write8(0x95, 0xA3);
    tcpc_write8(0x96, 0x0B);
	tcpc_write8(0x97, 0x10);
    tcpc_write8(0x94, 0x41);
    tcpc_write8(0xCB, 0x2f);
    tcpc_write8(0xC0, 0x81);
    tcpc_write8(0x87, 0x00);
    tcpc_write8(0xCD, 0x07);

//    tcpc_read8(0x1F, &i);
//    DEBUG_LOG("**i = 0x%x.\r\n",i);

//    u8 rv_0x10 = iic_read_byte(0x10, 1);
//    u8 rv_0x11 = iic_read_byte(0x11, 1);
//    u16 k;
//    tcpc_read16(0x12, &k);

//	DEBUG_LOG("rv_0x10 = 0x%x.\r\n",rv_0x10);
//	DEBUG_LOG("rv_0x11 = 0x%x.\r\n",rv_0x11);
//	while(1);
//  DEBUG_LOG("k = 0x%x.\r\n",k);
}




char tcpc_read16(unsigned char reg, unsigned short *data)
{
	u32 high;
	u8  low;
	low = iic_read_byte(reg, 1);
	high  = iic_read_byte(reg+1, 1);

	*data = (high << 8) | low;

	return *data;
}


void FP6606_vendorISR(void)
{
    unsigned short  irq_status;

    tcpc_read16(FP6606_REG_INT_STATUS, &irq_status);

    // Clear interrupt status.
    tcpc_write16(FP6606_REG_INT_STATUS, irq_status);

    return;
}













