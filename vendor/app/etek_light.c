//
//
//
//void etek_ic_init()
//{
//
//}
//
//
//
//
//void i2c_led_init(void)
//{
//	u8 ic_address_aw = i2c_dma_read_byte(0x10, 1);
//	u8 ic_address_et = i2c_dma_read_byte(0x1A, 1);
//
//	if(ic_address_aw == 0x23)
//	{
//		ic_address = 0x23;
//	}
//	if(ic_address_et = 0x4F)
//	{
//		ic_address = 0x22;
//	}
//
//	switch(ic_address)
//	{
//	case 0x23:
//		i2c_master_init(AW2015_ADDR, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*I2C_CLK_SPEED)) ); // 400KHz
//		i2c_dma_write_byte(0x11,1,0x03);
//		i2c_dma_write_byte(0x12,1,0x00);
//		i2c_dma_write_byte(0x13,1,0x00);
//		break;
//	case 0x22:
//
//		break;
//	default:
//		break;
//	}
//	if(ic_address == 0x23)
//	{
//
//	}
//	else if(ic_address == 0x22)
//	{
//
//	}
//	else
//	{
//
//	}
//
//}
