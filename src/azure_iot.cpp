// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/connect-iot-hub?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode

/*********************** INCLUDES ***************************/
#include "Arduino.h"
#include "DevKitMQTTClient.h"
#include "SystemTime.h"
#include "azure_iot.h"

/********************** CONSTANTS ***************************/
/* Device info */
const char *roomSchema = "neo-sensors"; 
const char *deviceType = "AZ3166-NEO";
const char *deviceFirmware = "1.0.1";
/* Units */
const char *distanceUnit = "cm";
const char *temperatureUnit = "C";
const char *humidityUnit = "%";
const char *pressureUnit = "psig";
/* Device location */
const char *deviceLocation = "ETS, Montreal";
const double deviceLatitude = 45.494914;
const double deviceLongitude = -73.562654;
/* TWIN PROPERTIES */
const char *messageTemplate="{\\\"distance\\\":${distance},\\\"distance_unit\\\":\\\"${distance_unit}\\\",\\\"temperature\\\":${temperature},\\\"temperature_unit\\\":\\\"${temperature_unit}\\\", \\\"humidity\\\":${humidity},\\\"humidity_unit\\\":\\\"${humidity_unit}\\\",\\\"pressure\\\":${pressure},\\\"pressure_unit\\\":\\\"${pressure_unit}\\\"}";
const char *jsonFields="{\"distance\": \"Double\", \"distance_unit\": \"Text\",\"temperature\": \"Double\", \"temperature_unit\": \"Text\",\"humidity\": \"Double\",\"humidity_unit\": \"Text\",\"pressure\": \"Double\",\"pressure_unit\": \"Text\" }";
const char *twinProperties="{\"Protocol\": \"MQTT\", \"SupportedMethods\": \"GreenLightOn,YellowLightOn,RedLightOn,Reboot\", \"Telemetry\": { \"%s\": {\"MessageTemplate\": \"%s\",\"MessageSchema\": {\"Name\": \"%s\",\"Format\": \"JSON\",\"Fields\": %s } } },\"Type\": \"%s\",\"Firmware\": \"%s\",\"Model\":\"AZ3166\",\"Location\": \"%s\",\"Latitude\": %f,\"Longitude\": %f}";

/************** PRIVATE GLOBAL VARIABLES ********************/
extern bool doReset;
extern bool hasIoTHub;
extern int loop_interval_ms;

/************ PRIVATE FUNCTIONS DECLARATION *****************/
void twinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int length);
int device_method_callback(const char *methodName, const unsigned char *payload, int length, unsigned char **response, int *responseLength);

/************* PUBLIC FUNCTIONS DEFINITION ******************/

void azure_iot_init() {
    //setup the MQTT Client

  char reportedProperties[2048];
  snprintf(reportedProperties,2048, twinProperties,roomSchema,messageTemplate,roomSchema,jsonFields,deviceType,deviceFirmware,deviceLocation,deviceLatitude,deviceLongitude);
  Serial.println("Reported Properties : ");
  Serial.println(reportedProperties);

    DevKitMQTTClient_SetDeviceMethodCallback(&device_method_callback);
    DevKitMQTTClient_SetDeviceTwinCallback(&twinCallback);
    DevKitMQTTClient_SetOption(OPTION_MINI_SOLUTION_NAME, "NEO_IOT");
    hasIoTHub = DevKitMQTTClient_Init(true);//set to true to use twin
    if (!hasIoTHub) {
      Screen.print(2, "No HUB");
    }
    else {
      Serial.println("HUB Connected");
      // Send the Twin data for the Remote Monitoring
      bool infoSent=azure_iot_send_device_info();
      LogInfo("*** Twin update: %s",infoSent?"yes":"no");
    }
}

bool azure_iot_send_data(float *azure_iot_distance, float *azure_iot_temperature, float *azure_iot_humidity, float *azure_iot_pressure) {
  time_t t = time(NULL);
  char buf[sizeof "2011-10-08T07:07:09Z"];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&t));

  char sensorData[256];
  sprintf_s(sensorData, sizeof(sensorData), "{\"distance\":%s,\"distance_unit\":\"%s\",\"temperature\":%s,\"temperature_unit\":\"%s\",\"humidity\":%s,\"humidity_unit\":\"%s\",\"pressure\":%s,\"pressure_unit\":\"%s\"}", f2s(*azure_iot_distance, 1), distanceUnit, f2s(*azure_iot_temperature, 1), temperatureUnit, f2s(*azure_iot_humidity, 1), humidityUnit,f2s(*azure_iot_pressure, 1), pressureUnit);
  EVENT_INSTANCE* message = DevKitMQTTClient_Event_Generate(sensorData, MESSAGE);

  DevKitMQTTClient_Event_AddProp(message, "$$CreationTimeUtc", buf);
  DevKitMQTTClient_Event_AddProp(message, "$$MessageSchema", roomSchema);
  DevKitMQTTClient_Event_AddProp(message, "$$ContentType", "JSON");
  
  return DevKitMQTTClient_SendEventInstance(message);
}

bool azure_iot_send_device_info()
{
  char reportedProperties[2048];
  snprintf(reportedProperties,2048, twinProperties,roomSchema,messageTemplate,roomSchema,jsonFields,deviceType,deviceFirmware,deviceLocation,deviceLatitude,deviceLongitude);
  return DevKitMQTTClient_ReportState(reportedProperties);
}

/************* PRIVATE FUNCTIONS DEFINITION *****************/

/* twinCallback
 * 
 * Description : Callback function for twin state
 * Return : None.
 */
void twinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int length){
  LogInfo("*** Twin State: %s",updateState?"Complete":"Partial");
}

/* device_method_callback
 * 
 * Description : Callback function for methods
 * Return : success code.
 */
int device_method_callback(const char *methodName, const unsigned char *payload, int length, unsigned char **response, int *responseLength){
  String payload_str = (char *)payload;
  LogInfo("*** Remote method: %s",methodName);  

  if(strcmp(methodName,"GreenLightOn")==0){
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, HIGH);
    if(payload_str.indexOf("interval")){
      String interval_str = payload_str.substring(payload_str.indexOf(":") + 1, payload_str.indexOf("}"));
      loop_interval_ms = interval_str.toInt();
    }

    const char *ok="{\"result\":\"OK\"}";
    *responseLength=strlen(ok);
    *response = (unsigned char*)malloc(*responseLength);
    strncpy((char *)(*response), ok, *responseLength);
    return 200;
  }

  if(strcmp(methodName,"YellowLightOn")==0){
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, LOW);
    if(payload_str.indexOf("interval")){
      String interval_str = payload_str.substring(payload_str.indexOf(":") + 1, payload_str.indexOf("}"));
      loop_interval_ms = interval_str.toInt();
    }
    
    const char *reset="{\"result\":\"OK\"}";    
    *responseLength=strlen(reset);
    *response = (unsigned char*)malloc(*responseLength);
    strncpy((char *)(*response), reset, *responseLength);
    return 201;
  }

  if(strcmp(methodName,"RedLightOn")==0){
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, HIGH);
    if(payload_str.indexOf("interval")){
      String interval_str = payload_str.substring(payload_str.indexOf(":") + 1, payload_str.indexOf("}"));
      loop_interval_ms = interval_str.toInt();
    }
    
    const char *reset="{\"result\":\"OK\"}";    
    *responseLength=strlen(reset);
    *response = (unsigned char*)malloc(*responseLength);
    strncpy((char *)(*response), reset, *responseLength);
    return 202;
  }

  if(strcmp(methodName,"Reboot")==0){
    doReset=true;
    
    const char *reset="{\"result\":\"RESET\"}";    
    *responseLength=strlen(reset);
    *response = (unsigned char*)malloc(*responseLength);
    strncpy((char *)(*response), reset, *responseLength);
    return 203;
  }

  LogError("*** Remote method: %s not found",methodName);
  return 500;
}
