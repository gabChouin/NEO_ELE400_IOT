// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/connect-iot-hub?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode

#ifndef AZURE_IOT_H
#define AZURE_IOT_H

/*********************** INCLUDES ***************************/

/************************ DEFINES ***************************/

/************* PUBLIC FUNCTIONS DECLARATION *****************/
void azure_iot_init();
bool azure_iot_send_data(float *azure_iot_distance, float *azure_iot_temperature, float *azure_iot_humidity, float *azure_iot_pressure);
bool azure_iot_send_device_info();

#endif