// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/connect-iot-hub?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode

/*********************** INCLUDES ***************************/
#include "Arduino.h"
#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "NEO_HC_SR04.h"
#include "NEO_TELEMETRY.h"
#include "azure_iot.h"

#include "DevKitMQTTClient.h"
#include "config.h"
#include "parson.h"

/********************** CONSTANTS ***************************/
#define HC_SR04_TRIG_PIN D1 /* PIN1 */
#define HC_SR04_ECHO_PIN PB_0 /* PIN2 */

extern Mutex i2c_mutex;
static bool hasWifi = false;
bool hasIoTHub = false;
bool doReset=false;
static int sentMessageCount = 0;
int loop_interval_ms = 1000;
Timer main_tim;

/************** PRIVATE GLOBAL VARIABLES ********************/
/* Telemetry */
static float distance = 0.0;
static float temperature = 0.0;
static float humidity = 0.0;
static float pressure = 0.0;

/************ PRIVATE FUNCTIONS DECLARATION *****************/
static void InitWifi();
void user_led_toggle(void);

/************************ SETUP *****************************/
void setup()
{

  /*HC-SR04 CONFIGURATION*/
  hc_sr04_config_t hc_sr04_config;
  hc_sr04_config.hc_sr04_trig_pin = HC_SR04_TRIG_PIN;
  hc_sr04_config.hc_sr04_echo_pin = HC_SR04_ECHO_PIN;

  /*INITIALIZE PERIPHERALS AND THREADS*/
  Screen.init();
  Screen.print(0, "Team NEO");
  Screen.print(2, "Initializing...");
  
  Screen.print(3, " > Serial");
  Serial.begin(115200);
  
  /* Open Red and Yellow light */
  digitalWrite(RGB_R, LOW);
  digitalWrite(RGB_G, HIGH);
  digitalWrite(RGB_B, LOW);
  
  Screen.print(3, " > WiFi");
  InitWifi();

  if(hasWifi) {
    /* Open Yellow light */
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, LOW);
    Screen.print(3, " > Azure");
    azure_iot_init();
  }

  
  if (hasIoTHub && hasWifi) {
    /* Open Green Light */
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, HIGH);
  }
  else {
    /* Open Red light */
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, HIGH);
  }
  
  Screen.print(3, " > HC_SR04");
  HC_SR04_init(hc_sr04_config);

  Screen.print(3, " > Telemetry");
  telemetry_sensor_init(); 
  
  /*Protect I2C BUS FROM TELEMETRY THREAD*/
  i2c_mutex.lock();
  Screen.print(2, "Running");
  Screen.print(3, "");
  i2c_mutex.unlock();
  
  main_tim.start();
}

/************************* LOOP *****************************/
void loop()
{
  main_tim.reset();
  char serial_buf[64];
  char line1[20];
  char line2[20];
  char line3[20];
  hc_sr04_errors_t err = HC_SR04_get_distance(&distance);

  /*Send distance info*/
  sprintf(line2, "distance : %i", (uint16_t)distance);
  Serial.println(line2);
  
  /*HC-SR04 sensor error management*/
  if (err < 0 ) {
    switch (err) {
      case HC_SR04_TIMEOUT:
        sprintf(line3, "Err : No pulse");
        break;
      case HC_SR04_INVALID :
        sprintf(line3, "Err : Invalid pulse");
        break;
      case HC_SR04_OVER_RANGE :
        sprintf(line3, "No object");
        break;
      default :
        sprintf(line3, "Error");
        break;
    }
  } 
  else {
    sprintf(line3, " ");
  }

  /*Verify if telemetry data is available*/
  if (telemetry_get(&temperature, &humidity, &pressure)) {
    /*Send Telemetry data*/
    sprintf(serial_buf, "temperature = %i\r\nhumidity = %i%%\r\npressure = %i%%", (uint16_t)temperature, (uint16_t)humidity, (uint16_t)pressure);
    Serial.println(serial_buf);
  }

  /*I2C conflict protection*/
  i2c_mutex.lock();
  Screen.print(3, line3);
  Screen.print(2, line2);
  i2c_mutex.unlock();


  /* SEND DATA TO AZURE IOT HUB */
  if (hasIoTHub && hasWifi)
  {
    /* LOCK I2C FOR OLED */
    i2c_mutex.lock();
    if (azure_iot_send_data(&distance, &temperature, &humidity, &pressure))
    {
      snprintf(line1, 20, "sent %i messages", ++sentMessageCount);
      Screen.print(1, line1);
    }
    else
    {
      Screen.print(1, "send fail...");
    }
    /* UNLOCK I2C */
    i2c_mutex.unlock();
  }

  /*Loop timing*/
  user_led_toggle();
  while (main_tim.read_ms() < loop_interval_ms);
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
  char wifiBuff[20];
  Screen.print(2, "Connecting...");
  
  if (WiFi.begin() == WL_CONNECTED)
  {
    hasWifi = true;    
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi");
  }
}




