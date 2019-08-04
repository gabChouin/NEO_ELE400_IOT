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

#include "EventBase.pb.h"
#include "LightControl.pb.h"
#include "NEO_PROTO.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

#define NEO_VERSION 1

#define HC_SR04_TRIG_PIN D1 /* PIN1 */
#define HC_SR04_ECHO_PIN D0 /* PIN2 */
#define HC_SR04_INTERVAL_MS 250 /*Stable up to 3m - 3.5 m*/
#define MESSAGE_MAX_LEN 256

extern Mutex i2c_mutex;
static bool hasWifi = false;
static bool hasIoTHub = false;

int messageCount = 1;
int sentMessageCount = 0;
static bool messageSending = true;
static uint64_t send_interval_ms;
EventBase event_base = EventBase_init_zero;
NEO_PROTO neo_proto = NEO_PROTO_init_zero;

static float distance = 0.0;
static float humidity = 0.0;
static float temperature = 0.0;

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
  event_base.version = NEO_VERSION;
  neo_proto.version = NEO_VERSION;
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
  bool encode_success;
  char line1[20];
  char line2[20];
  char line3[20];
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

  /*I2C conflict protection*/
  i2c_mutex.lock();
  Screen.print(3, line3);
  i2c_mutex.unlock();

  /*Send distance info*/
  sprintf(line2, "distance : %i", (uint16_t)distance);
  Serial.println(line2);

  /* SEND DATA TO AZURE IOT HUB */
  if (hasIoTHub && hasWifi)
  {
    uint8_t custom_buf[40] = {0};
    uint8_t enc_buf[MESSAGE_MAX_LEN] = {0};
    
    neo_proto.deviceTime = millis();
    neo_proto.distance = distance;
    neo_proto.temperature = temperature;
    neo_proto.humidity = humidity;
    /* CREATE OUTPUT STREAM */
    pb_ostream_t ostream = pb_ostream_from_buffer(custom_buf, 40);
    pb_ostream_t ostream_event_base = pb_ostream_from_buffer(enc_buf, 256);
    
    /* LOCK I2C FOR OLED */
    i2c_mutex.lock();

    if(pb_encode(&ostream, &NEO_PROTO_msg, &neo_proto)) {
      snprintf((char *)event_base.payload, 40, "%s,", custom_buf);
      encode_success = pb_encode(&ostream_event_base, &EventBase_msg, &event_base);
    }

    if(encode_success){
      EVENT_INSTANCE* message = DevKitMQTTClient_Event_Generate((char*)enc_buf, MESSAGE);
      if (DevKitMQTTClient_SendEventInstance(message))
      {
        snprintf(line1, 20, "sent %i messages", ++sentMessageCount);
        Screen.print(1, line1);
      }
      else
      {
        Screen.print(1, "send fail...");
      }
    }
    else {
      Screen.print(1, "enc fail...");
    }
    
    i2c_mutex.unlock();
  }

  

  /*Loop timing*/
  user_led_toggle();
  delay(2000);
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

    if (!DevKitMQTTClient_Init()) {
      hasIoTHub = false;
      Screen.print(1, "No HUB");
      return;
    }
    else {
      hasIoTHub = true;
    }
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi");
  }
}




