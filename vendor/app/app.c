/********************************************************************************************************
 * @file     app.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *
 *******************************************************************************************************/
#include "app.h"
#include "adv.h"
#include "led.h"
#include "ntc.h"
#include "fan.h"
#include "gap.h"
#include "log.h"
#include "cool.h"
#include "light.h"
#include "timer.h"
#include "FP6606.h"
#include "app_ntc.h"
#include "app_fan.h"
#include "drivers.h"
#include "app_mac.h"
#include "app_cool.h"
#include "app_debug.h"
#include "tl_common.h"
#include "flash_env.h"
#include "fixed_time.h"
#include "FP6606_ARM.h"
#include "bs_protocol.h"
#include "app_version.h"
#include "digital_tube.h"
#include "stack/ble/ble.h"
#include "app_fixed_time.h"
//#include "FP6606.h"
#include "FP_QC_Sink.h"
#include "FP6606_tcpm.h"

u8  switch_qc = 0;

#define VARY_5V			0xC8	// 200
#define VARY_9V			0x168	// 360
#define VARY_11V		0x1B8	// 440
#define VARY_12V		0x1E0	// 480
#define VARY_15V		0x258	// 600

/*----------------------------------------------------------------------------*/
static u32 gStartTick = 0;
static u32 gStartTick_qc = 0;
u8 power_ic = 1;

bs_vcom_t ble_vcom = {
	.devtype = BS_DEV_BACK_CLAMP_VER1,
	.vcomfd = 0,
	.devname = "backclamp",
};

// FAN��TEC��Light disable
void function_disable()
{
	DEBUG_LOG("%s\n", __FUNCTION__);
	led_switch = 0;
	FAN_DISABLE();
	TEC_DISABLE();
	aw_disable_config();
	//bls_ll_teminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);
}

void tec_disable()
{
	TEC_DISABLE();
}

// void open_led(void)
// {
// 	gpio_set_func(GPIO_LIGHT_EN ,AS_GPIO);
// 	gpio_set_output_en(GPIO_LIGHT_EN, 1);
// 	gpio_set_input_en(GPIO_LIGHT_EN ,0);
// 	gpio_write(GPIO_LIGHT_EN, 1);

// 	i2c_set_pin(I2C_GPIO_GROUP_M_A3A4);
// 	i2c_master_init(0xB6, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*400000)) ); // 400KHz
// 	aw_ic_init();
// 	aw_enable_config();
// }

void FP6606_GPIO_Init(void)
{
	/*Configure GPIO pin : B1_Pin */
	gpio_set_func(GPIO_FP_INT ,AS_GPIO);
	gpio_set_input_en(GPIO_FP_INT ,1);		// Disable input
	gpio_set_output_en(GPIO_FP_INT, 0); 		// Enable output
	gpio_setup_up_down_resistor(GPIO_FP_INT, PM_PIN_PULLUP_10K);
	gpio_set_interrupt_risc0(GPIO_FP_INT,GPIO_Pol_falling);
}

void Telink_hd_init(void)
{
	u8 addr_b8, addr_bb, addr_585;

	blc_pm_select_internal_32k_crystal();

	cpu_wakeup_init();

	clock_init(SYS_CLK_48M_Crystal);

	gpio_init();

	addr_b8  = *(volatile u8*)0x8080b8;	// PA7: afe_0xb8
	addr_bb  = *(volatile u8*)0x8080bb;	// PB2: afe_0xbb
	addr_585 = *(volatile u8*)0x808585;	// PA3/PA4: 0x585

	*(volatile u8*)0x8080b8 = addr_b8 & 0x7F;
	*(volatile u8*)0x8080bb = addr_bb & 0xFB;
	*(volatile u8*)0x808585 = addr_585 & 0x77;

}

void Telink_Ble_init(void)
{
	/* Initialize all configured peripherals */
	peripher_init();

	flash_unlock(FLASH_TYPE_GD);
	/* load customized freq_offset CAP value and TP value. */
	blc_app_loadCustomizedParameters();

	rf_drv_init(RF_MODE_BLE_1M);

	profile_init();

	gap_init();

	bls_pm_setSuspendMask (SUSPEND_DISABLE);
}

void FP6606_IC_init(void)
{
	FP6606_GPIO_Init();

	i2c_set_pin(I2C_GPIO_GROUP_M_A3A4);
	i2c_master_init(0x60, (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*I2C_CLK_SPEED)) ); // 400KHz

	FP6606_FunInit();
}

void power_key_init(void)
{
	// Key_Power_GPIO	PC1
	gpio_set_func(GPIO_KEY_POWER ,AS_GPIO);
	gpio_set_output_en(GPIO_KEY_POWER, 0); 		// Disable output
	gpio_set_input_en(GPIO_KEY_POWER ,1);		// Enable input
}

void check_power_key_on(void)
{
	while(POWER_KEY_STATUS() == POWER_KEY_OFF);
}

void pd_voltage_save(u8 voltage)
{
	flash_set(CUST_USER_PD_VOLTAGE_ADDR,&voltage,sizeof(u8));
}

void check_power_key_off(void)
{
	if(clock_time_exceed(gStartTick,10*1000))//10ms
	{
		if(POWER_KEY_OFF == POWER_KEY_STATUS())
		{
			DEBUG_LOG("Power key off,System reboot.\r\n");

			pd_voltage_save(0xBB);

			start_reboot();
		}
	   gStartTick += 10*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}
}

void profile_init(void)
{
	bs_vcom_register(&ble_vcom);
	mac_profile_init();
	ntc_profile_init();
	fan_profile_init();
	cool_profile_init();
	time_profile_init();
	light_profile_init();
	version_profile_init();
	machine_profile_init();
	bs_prot_init();
}

void peripher_init(void)
{
	log_init();
	power_key_init();
	check_power_key_on();

	DEBUG_LOG("\r\n........................................\r\n");
	DEBUG_LOG("System power on...\r\n");

	ntc_init();
	timer_init();
	fan_init();
	cool_init();

//	flash_env_init();
}

void temperature_level_save()
{
//	flash_set(CUST_USER_COUNT_ADDR,&level,sizeof(level));
//	DEBUG_LOG("fan level save = %d.\r\n",level);
}

void record_temperature(void)
{
	if(clock_time_exceed(gStartTick_qc,50*1000))		// 50ms
	{
		gStartTick_qc += 50*CLOCK_16M_SYS_TIMER_CLK_1MS;
	}
}


void read_source_vol()
{
	u16 rv_0x7071;
	tcpc_read16(0x70, &rv_0x7071);
}

void fp6606_qc3_rising(void)
{
	static u8 first_time = 1;

    if(QC_Snk.charge_state == 6)
    {
		if(first_time)
		{
			gStartTick_qc = clock_time();
			first_time = 0;
		}
    	if(clock_time_exceed(gStartTick_qc,50*1000))		// 50ms
    	{
            //QCSnk_3_0_setVoltageUP();
    		static u8 i = 0;
    		if(i < 40)
    		{
    			QC_Snk.charge_state = CHARGER_QC3_VOLT_RISING;
            	DEBUG_LOG("i = %d \r\n", i);
    			tcpc_write8(FP6606_REG_QC_SNK_CTRL_2, 0x07 );
    			WaitMs(1);
    			tcpc_write8(FP6606_REG_QC_SNK_CTRL_2, 0x06 );
    			i ++;
    		}
    		else
    		{
    			QC_Snk.charge_state = 10;
//    			switch_qc = 1;
    		}
    		gStartTick_qc += 50*CLOCK_16M_SYS_TIMER_CLK_1MS;
    	}
    }
}

u8 qc_voltage_check(void)
{
	u16 rv_0x7071;

	tcpc_read16(0x70, &rv_0x7071);

	DEBUG_LOG("rv_0x7071 = %d \r\n", rv_0x7071 * 25);

	if(rv_0x7071 < 240)
	{
		return 1;
	}
	return 0;

}

void main_loop(void)
{
    check_power_key_off();

    if(power_ic)
	{
		FP6606_ARM_work();

//		DEBUG_LOG("*****************************\r\n");
//		fp6606_qc3_rising();
		if(switch_qc)
		{
			power_ic  = 0;
			switch_qc = 0;

//			fan_init();
//			cool_init();
			light_init();
		}
	}
    else
	{
		blt_sdk_main_loop();

		if(led_switch)
		{
			if(gAdvEnable)
			{
				adv_process();
				led_process();
			}
			else
			{
				timer_process();
			}
		}
		if(AI_level_process() == false)
		{
			ctl_level = 1;
			fan_process();
			cool_process();
		}

		ntc_process();
		high_tem_process();

		shut_down_process();
	}
}


