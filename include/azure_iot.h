// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/connect-iot-hub?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode

#ifndef AZURE_IOT_H
#define AZURE_IOT_H

/*********************** INCLUDES ***************************/

/************************ DEFINES ***************************/

/************* PUBLIC FUNCTIONS DECLARATION *****************/

/* azure_iot_init
 * 
 * Description : Initialization function for azure
 * Return : None.
 */
void azure_iot_init();

/* azure_iot_send_data
 * 
 * Description : Send device telemetry data
 * Return : Success status.
 */
bool azure_iot_send_data(float *azure_iot_distance, float *azure_iot_temperature, float *azure_iot_humidity, float *azure_iot_pressure);

/* azure_iot_send_device_info
 * 
 * Description : Send device informations
 * Return : Success status.
 */
bool azure_iot_send_device_info();

#endif