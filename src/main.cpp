// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/connect-iot-hub?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode

#include "Arduino.h"
#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "NEO_HC_SR04.h"
#include "NEO_TELEMETRY.h"

#include "DevKitMQTTClient.h"
#include "config.h"
#include "parson.h"

#define HC_SR04_TRIG_PIN D1 /* PIN1 */
#define HC_SR04_ECHO_PIN D0 /* PIN2 */
#define HC_SR04_INTERVAL_MS 250 /*Stable up to 3m - 3.5 m*/

extern Mutex i2c_mutex;
static bool hasWifi = false;
static bool hasIoTHub = false;

int messageCount = 1;
int sentMessageCount = 0;
static bool messageSending = true;
static uint64_t send_interval_ms;

static void InitWifi();
void user_led_toggle(void);

/************************* TODO *****************************/
/* Integrate protobuf
 * Integrate wifi/azure (Send and receive)
 * Integrate light control
 * Integrate state control (change loop timing + light)
 */

/************************ SETUP *****************************/
void setup()
{
  /*HC-SR04 CONFIGURATION*/
  hc_sr04_config_t hc_sr04_config;
  hc_sr04_config.hc_sr04_trig_pin = HC_SR04_TRIG_PIN;
  hc_sr04_config.hc_sr04_echo_pin = HC_SR04_ECHO_PIN;
  hc_sr04_config.hc_sr04_interval_ms = HC_SR04_INTERVAL_MS;

  /*INITIALIZE PERIPHERALS AND THREADS*/
  Screen.init();
  Screen.print(0, "Team NEO");
  Screen.print(2, "Initializing...");

  Screen.print(3, " > WiFi");
  InitWifi();
  
  Screen.print(3, " > Serial");
  Serial.begin(250000);
  
  Screen.print(3, " > HC_SR04");
  HC_SR04_init(hc_sr04_config);

  Screen.print(3, " > Telemetry");
  telemetry_sensor_init(); 
  
  /*Protect I2C BUS FROM TELEMETRY THREAD*/
  i2c_mutex.lock();
  Screen.print(2, "Running");
  Screen.print(3, "");
  i2c_mutex.unlock();
}

/************************* LOOP *****************************/
void loop()
{
  char line1[20];
  char line2[20];
  char line3[20];
  float distance = 0.0;
  float humidity;
  float temperature;
  hc_sr04_errors_t err = HC_SR04_get_distance(&distance);

  /*Verify if telemetry data is available*/
  if (telemetry_get(&temperature, &humidity)) {
    /*Send Telemetry data*/
    sprintf(line1, "temperature = %i\r\nhumidity = %i%%", (uint16_t)temperature, (uint16_t)humidity);
    Serial.println(line1);
  }
  
  /*HC-SR04 sensor error management*/
  if (err < 0 ) {
    switch (err) {
      case HC_SR04_TIMEOUT:
        sprintf(line3, "Err : No pulse");
        digitalWrite(RGB_R, HIGH);
        digitalWrite(RGB_B, LOW);
        break;
      case HC_SR04_INVALID :
        sprintf(line3, "Err : Invalid pulse");
        digitalWrite(RGB_R, HIGH);
        digitalWrite(RGB_B, LOW);
        break;
      case HC_SR04_OVER_RANGE :
        sprintf(line3, "No object");
        digitalWrite(RGB_R, LOW);
        digitalWrite(RGB_B, HIGH);
        break;
      default :
        sprintf(line3, "Error");
        break;
    }
  } 
  else {
    sprintf(line3, " ");
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_B, LOW);
  }

  /*I2C conflict protection*/
  i2c_mutex.lock();
  Screen.print(3, line3);
  i2c_mutex.unlock();

  /*Send distance info*/
  sprintf(line2, "distance : %i", (uint16_t)distance);
  Serial.println(line2);


  if (hasIoTHub && hasWifi)
  {
    char buff[128];

    // replace the following line with your data sent to Azure IoTHub
    snprintf(buff, 128, "{\"topic\":\"iot\"}");
    
    i2c_mutex.lock();
    if (DevKitMQTTClient_SendEvent(buff))
    {
      Screen.print(1, "Sending...");
    }
    else
    {
      Screen.print(1, "Failure...");
    }
    i2c_mutex.unlock();
  }

  

  /*Loop timing*/
  user_led_toggle();
  delay(200);
}


/********************** Utilities ***************************/

/* user_led_toggle
 * 
 * Description : Toggle user led
 * Return : None.
 */
void user_led_toggle(void) {
  static bool led_status;

  digitalWrite(LED_USER, led_status);
  led_status = !led_status;
}

/* InitWifi
 * 
 * Description : Initialize wifi and hub connection
 * Return : None.
 */
static void InitWifi()
{
  Screen.print(2, "Connecting...");
  
  if (WiFi.begin() == WL_CONNECTED)
  {
    hasWifi = true;    
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());

    if (!DevKitMQTTClient_Init())
    {
      hasIoTHub = false;
      Screen.print(1, "No HUB");
      return;
    }
    hasIoTHub = true;
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi");
  }
}




