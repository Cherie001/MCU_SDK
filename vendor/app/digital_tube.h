/********************************************************************************************************
 * @file    digital_tube.h
 *
 * @brief    for TLSR chips
 *
 * @author
 * @date     November, 2020
 *
 * @par
 *
 *******************************************************************************************************/
#ifndef _DIGITAL_TUBE_H
#define _DIGITAL_TUBE_H

extern u8 tube_test_flag;
extern u8 spi_display_data[3];

void spi_init(void);
void spi_display_temp(void);
void spi_display_allclose(void);
void spi_display_alldisplay(void);
void temperture_divide(u8 value);

#endif

