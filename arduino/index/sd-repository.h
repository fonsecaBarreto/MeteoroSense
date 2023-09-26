#pragma once

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <ArduinoJson.h>

const int chipSelectPin = 32;
const int mosiPin = 23;
const int misoPin = 27;
const int clockPin = 25;

const int RETRY_INTERVAL = 5000;

void loadConfiguration(const char *contextName, fs::FS &fs, const char *filename, Config &config)
{
  Serial.printf("%s: Carregando variáveis de ambiente \n", contextName);

  StaticJsonDocument<512> doc;
  SPI.begin(clockPin, misoPin, mosiPin);

  int attemptCount = 0;
  bool success = false;
  while (success == false) {

    Serial.printf("%s: Iniciando leitura do arquivo de configuração %s (tentativa: %d) \n", contextName, filename, attemptCount + 1);

    if (SD.begin(chipSelectPin, SPI)){
      File file = fs.open(filename);

      if (file){
        DeserializationError error = deserializeJson(doc, file);
        if (!error){
          strlcpy(config.station_uid, doc["STATION_UID"] | "", sizeof(config.station_uid));
          strlcpy(config.station_name, doc["STATION_NAME"] | "", sizeof(config.station_name));
          strlcpy(config.wifi_ssid, doc["WIFI_SSID"] | "", sizeof(config.wifi_ssid));
          strlcpy(config.wifi_password, doc["WIFI_PASSWORD"] | "", sizeof(config.wifi_password));
          strlcpy(config.mqtt_server, doc["MQTT_SERVER"] | "", sizeof(config.mqtt_server));
          strlcpy(config.mqtt_username, doc["MQTT_USERNAME"] | "", sizeof(config.mqtt_username));
          strlcpy(config.mqtt_password, doc["MQTT_PASSWORD"] | "", sizeof(config.mqtt_password));
          strlcpy(config.mqtt_topic, doc["MQTT_TOPIC"] | "", sizeof(config.mqtt_topic));
          config.mqtt_port = doc["MQTT_PORT"] | 1883;
          config.interval = doc["INTERVAL"] | 60000;
          file.close();
          success = true;
          continue;
        }
        Serial.printf("%s: [ ERROR ] Formato inválido (JSON)\n", contextName);
        Serial.println(error.c_str());
      }
      Serial.printf("%s: [ ERROR ] Arquivo de configuração não encontrado\n", contextName);
    } else {
      Serial.printf("%s: [ ERROR ] Cartão SD não encontrado.\n", contextName);
    }

    Serial.printf("%s: Proxima tentativa de re-leitura em %d segundos ... \n\n\n", contextName, (RETRY_INTERVAL / 1000));
    attemptCount++;
    delay(RETRY_INTERVAL);
  }

  Serial.printf("%s: Variáveis de ambiente carregadas com sucesso!\n\n", contextName);
  Serial.printf("STATION_UID: %s\n", config.station_uid);
  Serial.printf("STATION_NAME: %s\n", config.station_name);
  Serial.printf("WIFI_SSID: %s\n", config.wifi_ssid);
  Serial.printf("WIFI_PASSWORD: %s\n", config.wifi_password);
  Serial.printf("MQTT_SERVER: %s\n", config.mqtt_server);
  Serial.printf("MQTT_USERNAME: %s\n",config.mqtt_username);
  Serial.printf("MQTT_PASSWORD: %s\n", config.mqtt_password);
  Serial.printf("MQRR_TOPIC: %s\n", config.mqtt_topic);
  Serial.printf("MQRR_PORT: %s\n", config.mqtt_port);
  Serial.printf("READ_INTERVAL: %s\n\n", config.interval);
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf(" - Salvando dados no cartao SD: %s\n", path); 

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println(" - Falha ao encontrar cartão SD");
        return;
    }
    if(file.print(message)){
        Serial.println(" - Nova linha salva com sucesso.");
    } else {
        Serial.println(" - Falha ao salvar nova linha");
    }
    file.close();
}

void storeMeasurement(String directory, String fileName,const char *payload){
  String path = directory + "/" + fileName + ".txt";
  if (!SD.exists(directory)) {
    if (SD.mkdir(directory)) {
      Serial.println(" - Diretorio criado com sucesso!");
    } else {
      Serial.println(" - Falha ao criar diretorio de metricas.");
    }
  }
  Serial.println(" - Atualizando arquivo.");
  appendFile(SD, path.c_str(), payload);
}

/*   
if (!SD.exists(path.c_str())){
    Serial.println(" - Criando novo arquivo.");
    appendFile(SD, path.c_str(), header);
  } 
*/

String readFileContent(File file) {
  String content = "";
  while (file.available()) {
    content += (char)file.read();
  }
  return content;
}


int removeFile(const char * filePath) {
  Serial.println(String("Deltando aquivo:") + filePath);
  if (SD.remove(filePath)) {
    Serial.println("' deleted.");
    return 1;
  } else {
    Serial.print("Error deleting '");
    return 0;
  }
}

void loopThroughFiles(const char* dirName, int max, std::function<void(char*, char*)> callback ) {
  File root = SD.open(dirName);

  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }

  while (true) {
    File entry = root.openNextFile();
    if (!entry || max == 0) break;

    if (!entry.isDirectory()) {
      Serial.print("File: ");
      Serial.println(entry.name());
      
      String fileContent = readFileContent(entry);
      callback((char *)entry.name(), (char *) fileContent.c_str());
      max -=1;
    }
    entry.close();
  }
  root.close();
}
