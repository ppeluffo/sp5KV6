/*------------------------------------------------------------------------------------
 * l_rtc.h
 * Autor: Pablo Peluffo @ 2015
 * Basado en Proycon AVRLIB de Pascal Stang.
 *
 * Son funciones que impelementan la API de acceso al RTC del sistema con FRTOS.
 *
 *
*/

// --------------------------------------------------------------------------------
// LIBRERIA PARA EL RTC M79410 USADO EN LOS DATALOGGER SERIE SPX.
// --------------------------------------------------------------------------------

#ifndef AVRLIBFRTOS_RTC_SP5KFRTOS_H_
#define AVRLIBFRTOS_RTC_SP5KFRTOS_H_

#include "frtos_io.h"
#include "l_i2c.h"
#include "stdint.h"

//--------------------------------------------------------------------------------
// API START

typedef struct
{
	// Tamanio: 7 byte.
	// time of day
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	// date
	uint8_t day;
	uint8_t month;
	uint16_t year;

} RtcTimeType_t;

#define RTC_read( rdAddress, data, length ) I2C_read( BUSADDR_RTC_DS1340, rdAddress, data, length );
#define RTC_write( wrAddress, data, length ) I2C_write( BUSADDR_RTC_DS1340, wrAddress, data, length );

void RTC_start(RtcTimeType_t *rtc);
bool RTC_read_dtime(RtcTimeType_t *rtc);
bool RTC_write_dtime(RtcTimeType_t *rtc);

void RTC_rtc2str(char *str, RtcTimeType_t *rtc);
bool RTC_str2rtc(char *str, RtcTimeType_t *rtc);

// API END
//--------------------------------------------------------------------------------

// Direcciones de registros

#define RTC_DS1340_RTCSEC		0x00
#define RTC_DS1340_RTCMIN		0x01
#define RTC_DS1340_RTCHOUR		0x02
#define RTC_DS1340_RTCWKDAY		0x03
#define RTC_DS1340_RTCDATE		0x04
#define RTC_DS1340_RTCMTH		0x05
#define RTC_DS1340_RTCYEAR		0x06
#define RTC_DS1340_CONTROL		0x07


#endif /* AVRLIBFRTOS_RTC_SP5KFRTOS_H_ */
