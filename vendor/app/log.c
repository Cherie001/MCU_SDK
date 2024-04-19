/********************************************************************************************************
 * @file     log.c
 *
 * @brief    for TLSR chips
 *
 * @author	 Lance.he
 * @date     Jan, 2019
 *
 * @par
 *           
 *******************************************************************************************************/
#include "log.h"
#include "app.h"
#include "led.h"
#include <stack/ble/ble.h>

__attribute__((aligned(4))) unsigned char log_rx_buff[16] = {0};

void uart_init1(void)
{
	uart_set_recbuff((unsigned short *)&log_rx_buff, sizeof(log_rx_buff));
	uart_set_pin(UART_TX_PB4, UART_RX_PB5);  //UART TX/RX pin set
	uart_reset();                            //will reset UART digital registers from 0x90 ~ 0x9f, so UART setting must set after this reset
	uart_init_baudrate(162, 13, PARITY_ODD, STOP_BIT_ONE); //(54,13:21000;;)Baud rate's setting, please use LUA script tool to calculate.
	uart_dma_en(0, 0); 	                                               //UART data in hardware buffer moved by DMA, so we enable them first
}

int putchar(int c)
{
#if SIMULATE_UART_EN
	uart_ndma_send_byte(c);
#endif
	return c;
}

void log_init(void)
{
#if SIMULATE_UART_EN
//#if 0
	uart_set_recbuff((unsigned short *)&log_rx_buff, sizeof(log_rx_buff));
	uart_set_pin(DEBUG_TX_PIN, DEBUG_RX_PIN);
	uart_reset();
#if (CLOCK_SYS_CLOCK_HZ == 48000000)
	uart_init_baudrate(29, 13,PARITY_NONE, STOP_BIT_ONE);
#else
	uart_init_baudrate(9, 13,PARITY_NONE, STOP_BIT_ONE);
#endif
	uart_dma_en(0,0);
#else
    gpio_set_func(DEBUG_TX_PIN ,AS_GPIO);
    gpio_set_output_en(DEBUG_TX_PIN, 0);                                 // Disable output
    gpio_set_input_en(DEBUG_TX_PIN ,0);                                // Disable input
    gpio_setup_up_down_resistor(DEBUG_TX_PIN,GPIO_PULL_UP_DOWN_FLOAT);
    gpio_write(DEBUG_TX_PIN, 0);

    gpio_set_func(DEBUG_RX_PIN ,AS_GPIO);
    gpio_set_output_en(DEBUG_RX_PIN, 0);                                 // Disable output
    gpio_set_input_en(DEBUG_RX_PIN ,0);                                // Disable input;
    gpio_setup_up_down_resistor(DEBUG_RX_PIN,GPIO_PULL_UP_DOWN_FLOAT);
    gpio_write(DEBUG_RX_PIN, 0);
//    gpio_set_func(DEBUG_RX_PIN ,AS_GPIO);
//    gpio_set_output_en(DEBUG_RX_PIN, 1);                                 // Disable output
//    gpio_set_input_en(DEBUG_RX_PIN ,0);                                // Disable input;
//    gpio_write(DEBUG_RX_PIN, 1);
#endif
}

//void debug_log(u8 log_flag)
//{
//    if(log_flag)
//    {
//    	uart_set_recbuff((unsigned short *)&log_rx_buff, sizeof(log_rx_buff));
//    	uart_set_pin(DEBUG_TX_PIN, DEBUG_RX_PIN);
//    	uart_reset();
//    	uart_init_baudrate(29, 13,PARITY_NONE, STOP_BIT_ONE);
//    	uart_dma_en(0,0);
//    }
//    else
//    {
//        gpio_set_func(DEBUG_TX_PIN ,AS_GPIO);
//        gpio_set_output_en(DEBUG_TX_PIN, 0);                                 // Disable output
//        gpio_set_input_en(DEBUG_TX_PIN ,0);                                // Disable input
//        gpio_setup_up_down_resistor(DEBUG_TX_PIN,GPIO_PULL_UP_DOWN_FLOAT);
//        gpio_write(DEBUG_TX_PIN, 0);
//
//        gpio_set_func(DEBUG_RX_PIN ,AS_GPIO);
//        gpio_set_output_en(DEBUG_RX_PIN, 0);                                 // Disable output
//        gpio_set_input_en(DEBUG_RX_PIN ,0);                                // Disable input;
//        gpio_setup_up_down_resistor(DEBUG_RX_PIN,GPIO_PULL_UP_DOWN_FLOAT);
//        gpio_write(DEBUG_RX_PIN, 0);
//    }
//}

