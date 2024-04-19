#ifndef _APP_NTC_H_
#define _APP_NTC_H_

#define ADC_Channel_VBAT_NTC_SENSE              2
#define ADC_Schedule_VBAT_NTC_SENSE             0

#define ADC_Channel_VBATT_SENSE                 3
#define ADC_Schedule_VBATT_SENSE                1

#define ADC_Channel_VBUS_SENSESEP               4
#define ADC_Schedule_VBUS_SENSESEP              2

#define ADC_Channel_VBUS_SENSESEN               5
#define ADC_Schedule_VBUS_SENSESEN              3

#define ADC_Channel_NTC1_DET                    6
#define ADC_Schedule_NTC1_DET                   4

#define ADC_Channel_NTC2_DET                    7
#define ADC_Schedule_NTC2_DET                   5

#define ADC_TIM_PERIOD                      1000000///(100)//100KHz,0.05ms     //(10000)10KHz


#define ADC_TEMP_NTC     GPIO_PB3 //GPIO_PB3

typedef enum
{
    ADC_COLLECT_NULL,                   //ADC only collect battery voltage
    ADC_COLLECT_BATTERY,                //ADC only collect battery voltage
    ADC_COLLECT_ALL                     //ADC collect battery and x and y voltage
} T_ADC_COLLECT_TYPE;


typedef enum
{
	ADC_NTC_VAL = 0x00,
	ADC_NTC_TUBE,
	ADC_NTC_DISABLE,
	ADC_NTC_READ,
	ADC_NTC_TYPE_READ,
	ADC_NTC_APP_READ_VAL,
	ADC_NTC_CMD_MAX,
}bs_adc_cmd_t;

int ntc_profile_init(void);
#endif
