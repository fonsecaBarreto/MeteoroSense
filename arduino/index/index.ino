// Autor: Lucas Fonseca e Gabriel Fonseca
// Titulo: Sit arduino
// Versão: 1.5;
//.........................................................................................................................

#include "constants.h"
#include "conf.h"
#include "sd-repository.h"
#include "integration.h"
#include "sensores.h"
#include <stdio.h>
#include "esp_system.h"
long startTime;

// Pluviometro
extern unsigned long lastPVLImpulseTime;
extern unsigned int rainCounter;

// Anemometro (Velocidade do vento)
extern unsigned long lastVVTImpulseTime;
extern float anemometerCounter;
extern unsigned long smallestDeltatime;
extern Sensors sensors;

const int limit_retry = 2;
int retry_array[limit_retry];
int indexResent = 0;

/******* Objeto de Transferência de Dados *******/
struct
{
  float wind_speed = 0;
  float wind_gust = 0;
  float rain_acc = 0;
  float humidity = 0;
  float temperature = 0;
  float pressure = 0;
  int wind_dir = -1;
} Data;

String formatedDateString = "";
char json_output[240]{0};
char csv_header[200]{0};
char csv_output[200]{0};

void setup()
{
  // 1. Arduino - Sistema Integrado de meteorologia
  delay(3000);
  Serial.begin(115200);
  Serial.println("\n///////////////////////////////////\nSistema Integrado de meteorologia\n///////////////////////////////////\n");
  Serial.println("1. Configuração inicial;");

  // 1.1 Setup inicial dos pinos;
  Serial.println("  - Iniciando pinos");
  pinMode(PLV_PIN, INPUT_PULLDOWN);
  pinMode(ANEMOMETER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PLV_PIN), pluviometerChange, RISING);
  attachInterrupt(digitalPinToInterrupt(ANEMOMETER_PIN), anemometerChange, FALLING);

  // 1.2 Configuração Inicial;
  delay(2000);
  Serial.println("\n1.2 Carregando variaveis;");
  loadConfiguration("  - Carregando variaveis", SD, configFileName, config);

  // 1.3 Estabelecendo conexão com wifi;
  delay(2000);
  Serial.println("1.3 Wifi;");
  setupWifi("  - Wifi", config.wifi_ssid, config.wifi_password);

  // 1.4 Estabelecendo conexão com NTP;
  delay(2000);
  Serial.println("\n1.4 NTP;");
  connectNtp("  - NTP");

  // 1.5 Configuração incial MQTT broker;
  delay(2000);
  Serial.println("\n1.5 MQTT;");
  setupMqtt("  - MQTT", config.mqtt_server, config.mqtt_port, config.mqtt_username, config.mqtt_password, config.mqtt_topic);

  // 1.6 Iniciando controllers;
  delay(1000);
  Serial.println("\n\n1.6 Iniciando controllers;");
  setupSensors();

  // 1.7 Definindo variáives auxiliares globais;
  int now = millis();
  lastVVTImpulseTime = now;
  lastPVLImpulseTime = now;
  startTime = now;

  // 2; Inicio
  Serial.printf("\n------------------- PRIMEIRA ITERAÇÃO -------------------\n\n");
}

void loop()
{
  // 2. Inicio Loop;
  int now = millis();
  int timeRemaining = startTime + config.interval - now;
  bool isMqttConnected = false;
  bool isWifiConnected = false;

  // 2.2 Garantindo conexão com mqqt broker;
  if (isWifiConnected = WiFi.status() == WL_CONNECTED){ 
    isMqttConnected = mqttClient.loop();
  }

  // 2.1 Tempo ocioso para captação de metricas 60s
  if (timeRemaining > 0){
    if (timeRemaining % 10000 == 0 ){
      Serial.printf("\n\n * Coletando dados, proximo resultado em %d segundos...", (timeRemaining / 1000));
      Serial.printf("\n  - WIFI: %s", isWifiConnected ? "Contectado" : "Desconectado");
      Serial.printf("\n  - MQTT: %s", isMqttConnected == true ? "Contectado" : "Desconectado");

      int totalRetries = countDirectoryFiles("/retries");
      Serial.printf("\n  - Total metricas atrasadas: %d", totalRetries);

      if (!isMqttConnected) {
        isMqttConnected = connectMqtt("\n  - MQTT", config.mqtt_username, config.mqtt_password, config.mqtt_topic);
      } else if(totalRetries > 0){
          Serial.printf("\n  - MQTT: Realizando re-envio de metricas atrasadas (%d/%d)\n", limit_retry, totalRetries);
          indexResent = 0;
          loopThroughFiles("/retries", limit_retry, retryMeasurementCallback);
          Serial.println();
          removeRetryMeasurement();
      }

      while(now >= millis());
    }
    return;
  }

  // 3 Computando dados 
  Serial.printf("\n\n3. Computando dados ...\n");

  // 3.1 Captando dados dos controller 
  Data.wind_dir = getWindDir();
  Data.wind_speed = 3.052 * (ANEMOMETER_CIRC * anemometerCounter) / (INTERVAL / 1000.0); // m/s
  Data.wind_gust = (3052.0f * ANEMOMETER_CIRC) / smallestDeltatime;
  Data.rain_acc = rainCounter * VOLUME_PLUVIOMETRO;
  DHTRead(Data.humidity, Data.temperature);
  BMPRead(Data.pressure);

  // 3.2 Redefinido variaveis de medição
  startTime = millis();
  anemometerCounter = 0;
  rainCounter = 0;
  smallestDeltatime = 4294967295;

  // 3.3 Ping NTP 
  timeClient.update();
  int timestamp = timeClient.getEpochTime();
  convertTimeToLocaleDate(timestamp);

  // 4. Apresentando Dados
  Serial.printf("\n\n4. Nova metrica (%sT%s)", formatedDateString, timeClient.getFormattedTime());
  presentation(timestamp);

  // 5; Fim
  Serial.printf("\n------------------- PROXIMA ITERAÇÃO -------------------\n");

}


void presentation(long timestamp) {
  // Resultado das medições
  Serial.println("\nResultado:");
  Serial.printf("Timestamp..........: %d\n", timestamp);
  Serial.printf("Velocidade do vento: %f\n", Data.wind_speed);
  Serial.printf("Chuva acumulada....: %f\n", Data.rain_acc);
  Serial.printf("Umidade............: %f\n", Data.humidity);
  Serial.printf("Temperatura........: %f°C\n", Data.temperature);
  Serial.printf("Raja de vento......: %f\n", Data.wind_gust);
  Serial.printf("Direção do vento...: %s (%d)\n", strVals[Data.wind_dir], Data.wind_dir);
  Serial.printf("Pressao............: %f hpa\n",Data.pressure);

  // Parsing data 
  parseData(timestamp);
  Serial.printf("\nResultado CSV:\n%s", csv_output);
  Serial.printf("\nResultado JSON:\n%s\n", json_output);

  // local storage
  Serial.println("\n4.1. Gravando em disco:");
  storeMeasurement("/metricas", formatedDateString, csv_output);

  // Enviando Dados Remotamente
  Serial.println("\n4.2 Enviando Resultados remotamente:  ");
  bool  measurementSent = sendMeasurementToMqtt(config.mqtt_topic, json_output);

  // Arquivando dados com falha
  if (measurementSent == false){
    Serial.printf("\n4.3 Arquivando Dados para serem re-enviados posteriomente");
    storeMeasurement("/retries", String(timestamp), json_output);
  }
}

void parseData(long timestamp)
{
  // parse measurements data to json
  const char *json_template = "{\"timestamp\": %i, \"temperatura\": %s, \"umidade_ar\": %s, \"velocidade_vento\": %.2f, \"rajada_vento\": %.2f, \"dir_vento\": %d, \"volume_chuva\": %.2f, \"pressao\": %s, \"uid\": \"%s\", \"identidade\": \"%s\"}";
  sprintf(json_output, json_template,
          timestamp,
          isnan(Data.temperature) ? "null" : String(Data.temperature),
          isnan(Data.humidity) ? "null" : String(Data.humidity),
          Data.wind_speed,
          Data.wind_gust,
          Data.wind_dir,
          Data.rain_acc,
          Data.pressure == -1 ? "null" : String(Data.pressure),
          config.station_uid,
          config.station_name);

  // parse measurement data to csv
  const char *csv_template = "%i,%s,%s,%.2f,%.2f,%d,%.2f,%s,%s,%s\n";
  sprintf(csv_header, "%s\ntimestamp,temperatura,umidade_ar,velocidade_vento,rajada_vento,dir_vento,volume_chuva,pressao,uid,identidade", formatedDateString);
  sprintf(csv_output, csv_template,
          timestamp,
          isnan(Data.temperature) ? "null" : String(Data.temperature),
          isnan(Data.humidity) ? "null" : String(Data.humidity),
          Data.wind_speed,
          Data.wind_gust,
          Data.wind_dir,
          Data.rain_acc,
          Data.pressure == -1 ? "null" : String(Data.pressure),
          config.station_uid,
          config.station_name);
}

void convertTimeToLocaleDate(long timestamp) {
  struct tm *ptm = gmtime((time_t *)&timestamp);
  int day = ptm->tm_mday;
  int month = ptm->tm_mon + 1;
  int year = ptm->tm_year + 1900;
  formatedDateString = String(day) + "-" + String(month) + "-" + String(year);
}

void retryMeasurementCallback(char *fileName, char *payload) {
  Serial.println("    Reenviando arquivo: [" + String(fileName) + "]");
  bool measurementSent = sendMeasurementToMqtt(config.mqtt_topic, payload);
  if (measurementSent == true) {
    int nomeTimeStamp = 0;
    sscanf(fileName, " %d", &nomeTimeStamp);
    retry_array[indexResent] = nomeTimeStamp;
    indexResent++;
  }
}

void removeRetryMeasurement() {
  for (int n = 0; n < limit_retry; n++){
    int timestamp = retry_array[n];
    if (!timestamp){
      continue;
    
}
    String filePath = "/retries/" + String(timestamp) + ".txt";
    Serial.println("    Removendo arquivos: [" + String(filePath) + "]");
    removeFile(filePath.c_str());
    retry_array[n] = 0;
  }
}