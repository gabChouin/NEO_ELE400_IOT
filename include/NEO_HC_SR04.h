/* Date     : 2019-07-03
 * Author   : Gabriel Chouinard - Team NEO - ELE400
 *
 * Description :
 *
 *      Source file with all functions used to interface
 *      HC-SR04 distance sensors.
 *
 */


#ifndef NEO_HCSR04_H
#define NEO_HCSR04_H

/*********************** INCLUDES ***************************/
#include "Arduino.h"
/************************ DEFINES ***************************/

typedef struct {
    uint8_t hc_sr04_trig_pin;
    uint8_t hc_sr04_echo_pin;
} hc_sr04_config_t;

typedef enum {
    HC_SR04_NO_ERR = 0,
    HC_SR04_TIMEOUT = -1,
    HC_SR04_INVALID = -2,
    HC_SR04_OVER_RANGE = -3
} hc_sr04_errors_t;
/************* PUBLIC FUNCTIONS DECLARATION *****************/

/* HC_SR04_init
 * 
 * Description : Initialize HC_SR04 Sensor 
 * Return : None.
 */
void HC_SR04_init(hc_sr04_config_t hc_sr04_init);

/* HC_SR04_thread
 * 
 * Description : HC_SR04 sensor thread
 * Return : None.
 */
void HC_SR04_thread(void);

/* HC_SR04_get_distance
 * 
 * Description : Send 10us Trig signal and measure echo pulse 
 * Return : (hc_sr04_errors_t) Error code.
 */
hc_sr04_errors_t HC_SR04_get_distance(float *hc_sr04_distance);

#endif