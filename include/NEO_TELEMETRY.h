/* Date     : 2019-07-26
 * Author   : Gabriel Chouinard - Team NEO - ELE400
 *
 * Description :
 *
 *      Source file with all functions used to interface
 *      AZ3166 telemetry sensors.
 *
 */


#ifndef NEO_TELEMETRY_H
#define NEO_TELEMETRY_H

/*********************** INCLUDES ***************************/
#include "Arduino.h"
/************************ DEFINES ***************************/

/************* PUBLIC FUNCTIONS DECLARATION *****************/

/* telemetry_sensor_init
 * 
 * Description : Initialize telemetry Sensor 
 * Return : None.
 */
void telemetry_sensor_init(void);

/* telemetry_get
 * 
 * Description : Check if there are telemetry data available 
 * Return : 1 if data available, else 0.
 */
uint8_t telemetry_get(float* telemetry_temp, float* telemetry_humid, float* telemetry_pressure);

#endif