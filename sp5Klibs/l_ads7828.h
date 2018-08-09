/*
 * l_ads7828.h
 *
 *  Created on: 9 ago. 2018
 *      Author: pablo
 */

#ifndef SRC_SP5KLIBS_L_ADS7828_H_
#define SRC_SP5KLIBS_L_ADS7828_H_

#include "frtos_io.h"
#include "l_i2c.h"
#include "stdint.h"

#define 	ADS7828_CMD_SD   		0x80	//ADS7828 Single-ended/Differential Select bit.
#define 	ADS7828_CMD_PDMODE0		0x00	//ADS7828 Mode 0: power down
#define 	ADS7828_CMD_PDMODE1 	0x04	//ADS7828 Mode 1: Ref OFF, converter ON
#define 	ADS7828_CMD_PDMODE2   	0x08	//ADS7828 Mode 2: Ref ON, converter OFF
#define 	ADS7828_CMD_PDMODE3   	0x0C	//ADS7828 Mode 3: Ref ON, converter ON.

int16_t ADC_read( uint8_t adc_channel );

#define AN_read_BAT()	ADC_read(1)
#define AN_read_CH0()	ADC_read(3)		// AIN0->ADC3
#define AN_read_CH1()	ADC_read(5)		// AIN1->ADC5
#define AN_read_CH2()	ADC_read(7)		// AIN2->ADC7

#endif /* SRC_SP5KLIBS_L_ADS7828_H_ */
