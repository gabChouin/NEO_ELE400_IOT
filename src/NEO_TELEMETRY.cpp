/* Date     : 2019-07-26
 * Author   : Gabriel Chouinard - Team NEO - ELE400
 *
 * Description :
 *
 *      Source file with all functions used to interface
 *      AZ3166 telemetry sensors.
 *
 */

#include "HTS221Sensor.h"
#include "AzureIotHub.h"
#include "Arduino.h"
#include "parson.h"
#include "NEO_TELEMETRY.h"

/************************ DEFINES ***************************/

/************** PRIVATE GLOBAL VARIABLES ********************/
DevI2C *telemetry_i2c;
HTS221Sensor *telemetry_sensor;
static float humidity;
static float temperature;
Thread telemetry_thread;
Mutex i2c_mutex;
uint8_t data_available_flag = 0;
/************ PRIVATE FUNCTIONS DECLARATION *****************/
void get_telemetry_thread(void);

/************* PUBLIC FUNCTIONS DEFINITION ******************/
void telemetry_sensor_init()
{
    telemetry_i2c = new DevI2C(D14, D15);
    telemetry_sensor = new HTS221Sensor(*telemetry_i2c);
    telemetry_sensor->init(NULL);

    humidity = -1;
    temperature = -1000;
    
    // Create a thread to execute the function get_telemetry_thread
    telemetry_thread.start(get_telemetry_thread);
}

uint8_t telemetry_get(float* telemetry_temp, float* telemetry_humid) {
    if (data_available_flag) {
        *telemetry_temp = temperature;
        *telemetry_humid = humidity;
        data_available_flag = 0;
        return 1;
    } 
    else {
        return 0;
    } 
}

/************* PRIVATE FUNCTIONS DEFINITION *****************/
void get_telemetry_thread(void) {
  while(1) {
    /*PROTECT I2C BUS FROM OLED SCREEN*/
    i2c_mutex.lock();
    telemetry_sensor->reset();
    telemetry_sensor->getHumidity(&humidity);
    telemetry_sensor->getTemperature(&temperature);
    i2c_mutex.unlock();
    /*SET DATA AVAILABLE FLAG*/
    data_available_flag = 1;
    /*WAIT ONE MINUTE*/
    wait(60);
  }
}