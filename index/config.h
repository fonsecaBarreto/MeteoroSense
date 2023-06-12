#pragma once
#include "BluetoothSerial.h"
#include <string>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;
String device_name = "ESTACAO-ESP32-BT";
String prev_message="";
String message = "";

struct {
  char ssid[255];
  char ssid_password[255];
  char api_url[255];
} environment_variables;

void config(){
    Serial.println('Iniciando configurações iniciais');    
    SerialBT.begin(device_name); 
    Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
}

void loadEnvVariables(char* str) {
    char* line = strtok(str, ";");
    while (line != NULL) {
        char key[255];
        char value[255];
        if (sscanf(line, "%[^=]=%[^}]", key, value) == 2) {
          if (strcmp(key, "ssid") == 0){
            strcpy(environment_variables.ssid, value);
          }
          else if (strcmp(key, "ssid_password") == 0){
            strcpy(environment_variables.ssid_password, value);
          }
          else if (strcmp(key, "api_url") == 0){
            strcpy(environment_variables.api_url, value);
          }
        }
        line = strtok(NULL, ";");
    }
    return;
}

void blueToothConnection() {

  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }

  if (SerialBT.available()) {
    char incomming_char = SerialBT.read();

    if(incomming_char == '\n') {
      Serial.print("action message:");
      Serial.println(message);
      Serial.println();
      if(prev_message.equals("config\r")){
        char* str_copy = (char*) message.c_str();
        loadEnvVariables(str_copy);
        printf("Variabes de ambinete aqui: \n ");
        printf("- ssid: %s\n - ssid_password: %s \n - api_url: %s \n", environment_variables.ssid, environment_variables.ssid_password, environment_variables.api_url);
      } 
      prev_message = message;
      message = "";
    } else {
      message+=String(incomming_char);
    }
  }

  delay(100);
}
 