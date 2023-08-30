#pragma once

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <ArduinoJson.h>

const int chipSelectPin = 32;
const int mosiPin = 23;
const int misoPin = 27;
const int clockPin = 25;


void loadConfiguration(fs::FS &fs, const char *filename, Config &config)
{

  Serial.printf("Carregando Arquivo de configuração... %s\n", filename);

  StaticJsonDocument<512> doc;
  SPI.begin(clockPin, misoPin, mosiPin);

  if (SD.begin(chipSelectPin, SPI)) {
   
    File file = fs.open(filename);
    if (!file) Serial.println("Não foi possivel encontrar arquivo de configuração");

    DeserializationError error = deserializeJson(doc, file);
    if (error){
      Serial.println(F("** Formato do arquivo de configuração inválido **"));
      Serial.println(error.c_str());
    }
    file.close();
  } else {
    Serial.println(" ** Não foi possivel estabelecer conexão com leitor de Cartão SD. **\n");
  }
 
  strlcpy(config.station_uid, doc["STATION_UID"] | "01", sizeof(config.station_uid));
  strlcpy(config.station_name, doc["STATION_NAME"] | "est001", sizeof(config.station_name));
  strlcpy(config.wifi_ssid, doc["WIFI_SSID"] | "wifi", sizeof(config.wifi_ssid));
  strlcpy(config.wifi_password, doc["WIFI_PASSWORD"] | "wifi_password", sizeof(config.wifi_password));
  strlcpy(config.mqtt_server, doc["MQTT_SERVER"] | "localhost:3000", sizeof(config.mqtt_server));
  strlcpy(config.mqtt_username, doc["MQTT_USERNAME"] | "telemetria", sizeof(config.mqtt_username));
  strlcpy(config.mqtt_password, doc["MQTT_PASSWORD"] | "telemetria_password", sizeof(config.mqtt_password));
  strlcpy(config.mqtt_topic, doc["MQTT_TOPIC"] | "/fake_topic", sizeof(config.mqtt_topic));
  config.mqtt_port = doc["MQTT_PORT"] | 1883;
  config.interval = doc["INTERVAL"] | 60000;

  Serial.print("STATION_UID: ");
  Serial.println(config.station_uid);

  Serial.print("STATION_NAME: ");
  Serial.println(config.station_name);

  Serial.print("WIFI_SSID: ");
  Serial.println(config.wifi_ssid);

  Serial.print("WIFI_PASSWORD: ");
  Serial.println(config.wifi_password);

  Serial.print("MQTT_SERVER: ");
  Serial.println(config.mqtt_server);

  Serial.print("MQTT_USERNAME: ");
  Serial.println(config.mqtt_username);

  Serial.print("MQTT_PASSWORD: ");
  Serial.println(config.mqtt_password);

  Serial.print("MQRR_TOPIC: ");
  Serial.println(config.mqtt_topic);

  Serial.print("MQRR_PORT: ");
  Serial.println(config.mqtt_port);

  Serial.println();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Salvando nova metrica no cartçao SD: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Falha ao encontrar cartão SD");
        return;
    }
    if(file.print(message)){
        Serial.println("Nova linha salva com sucesso.");
    } else {
        Serial.println("Falha ao salvar nova linha");
    }
    file.close();
}

void storeMeasurement( String fileName, const char *payload){
  char charArray[100];
  fileName.concat(".txt");
  fileName.toCharArray(charArray, fileName.length() + 1);
  appendFile(SD, charArray, payload);
}


/*if(first)
  {
    readLine(SD,"/dados.txt");
    first = false;
  }
*/

/* void readLine(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
         sendMeasurementToMqtt(config.mqtt_topic, file.readStringUntil('\n').c_str());
    }
    file.close();
}
 */