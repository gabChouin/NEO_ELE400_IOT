/* Date     : 2019-07-03
 * Author   : Gabriel Chouinard - Team NEO - ELE400
 *
 * Description :
 *
 *      Source file with all functions used to interface
 *      HC-SR04 distance sensors.
 *
 */


/*********************** INCLUDES ***************************/
#include "NEO_HC_SR04.h"

/************************ DEFINES ***************************/
#define HCSR04_MAX_DIST_CM 400
#define HCSR04_US_TO_CM 58
#define HCSR04_MAX_DIST_US ((HCSR04_MAX_DIST_CM)*(HCSR04_US_TO_CM))
#define ECHO_MIN_WAIT_US 450
#define ECHO_MAX_WAIT_US 650


/************** PRIVATE GLOBAL VARIABLES ********************/
hc_sr04_config_t config;
float distance = 0.0;


/************ PRIVATE FUNCTIONS DECLARATION *****************/
void send_trig(void);
hc_sr04_errors_t get_echo(void);

/************* PUBLIC FUNCTIONS DEFINITION ******************/
void HC_SR04_init(hc_sr04_config_t hc_sr04_config) {
  config = hc_sr04_config;
}

hc_sr04_errors_t HC_SR04_get_distance(float *hc_sr04_distance) {
  hc_sr04_errors_t err = get_echo();
  *hc_sr04_distance = distance;
  return err;
}

/************* PRIVATE FUNCTIONS DEFINITION *****************/

hc_sr04_errors_t get_echo(void) {
  Timer t;

  /* send 10us pulse on trig pin*/
  send_trig();

  /* Read echo pins */
  t.start();
  while(!digitalRead(config.hc_sr04_echo_pin)) {
    if (t.read_us() >= ECHO_MAX_WAIT_US) {
      t.stop();
      t.reset();
      return HC_SR04_TIMEOUT; /* No pulse detected */
    }
  }
  if (t.read_us() <= ECHO_MIN_WAIT_US) {
    t.stop();
    t.reset();
    return HC_SR04_INVALID; /* Invalid pulse */
  }
  t.reset();
  while(digitalRead(config.hc_sr04_echo_pin)) {
  }
  t.stop();

  if (t.read_us() > HCSR04_MAX_DIST_US) {
    distance = HCSR04_MAX_DIST_CM;
    return HC_SR04_OVER_RANGE; /* Invalid pulse */
  }
  distance = (float)(t.read_us()) / HCSR04_US_TO_CM;

  return HC_SR04_NO_ERR;
}

void send_trig(void) {
  digitalWrite(config.hc_sr04_trig_pin, LOW);
  delayMicroseconds(3); /* 10us verified with oscilloscope */
  digitalWrite(config.hc_sr04_trig_pin, HIGH);
}