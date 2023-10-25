// Autor: Lucas Fonseca e Gabriel Fonseca
// Titulo: Sit arduino
// Versão: 1.6 (BlueTooth);
//.........................................................................................................................

#include "constants.h"
#include "data.h"
#include "sd-repository.h"
#include "integration.h"
#include "sensores.h"
#include <stdio.h>
#include "esp_system.h"
#include "bt-integration.h"
long startTime;

#include <string>
#include <vector>

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
bool BT_ENABLED = 0;
bool forceRestart = false;

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
int timeRemaining=0;

void setup() {
  // 1. Arduino - Sistema Integrado de meteorologia
  delay(3000);
  Serial.begin(115200);
  Serial.printf("\n///////////////////////////////////\nSistema Integrado de meteorologia\n///////////////////////////////////\n\n");
  
  Serial.printf("1. Configuração inicial;\n");
  pinMode(PLV_PIN, INPUT_PULLDOWN);
  pinMode(ANEMOMETER_PIN, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PLV_PIN), pluviometerChange, RISING);
  attachInterrupt(digitalPinToInterrupt(ANEMOMETER_PIN), anemometerChange, FALLING);

  // 1.1 Iniciando Cartão SD
  Serial.printf("\n1.1 Iniciando cartão SD");
  initSdCard();

  // 1.2 Criando diretorios padrões
  Serial.printf("\n\n1.2 Criando diretorios padrões");
  createDirectory("/metricas");
  createDirectory("/logs");
  storeLog("\n\nEstação iniciada;");

  // 1.3 Configuração Inicial;
  delay(1000);
  std::string jsonConfig;
  Serial.println("\n1.3 Carregando variaveis;");
  storeLog("\n- Carregando variaveis: ... ");
  loadConfiguration("  - Carregando variaveis", SD, configFileName, config, jsonConfig);
  storeLog("ok;");

  // 1.4 Inciando Bluetooth
  Serial.println("\n1.4 Iniciando bluetooth;");
  BLE::SetConfigCallback(SaveConfigFile);
  BLE::Init(config.station_name, jsonConfig);

  // 1.5 Estabelecendo conexão com wifi;
  delay(1000);
  Serial.println("1.5 Wifi;");
  storeLog("\n- Conectando ao wifi: ... ");
  setupWifi("  - Wifi", config.wifi_ssid, config.wifi_password);
  int nivelDbm = (WiFi.RSSI()) * -1;
  storeLog((String(nivelDbm) + ";").c_str());

  // 1.6 Estabelecendo conexão com NTP;
  delay(1000);
  Serial.println("\n1.6 NTP;");
  storeLog("\n- Conectando ao NTP: ... ");
  connectNtp("  - NTP");
  storeLog("ok;");

  // 1.7 Configuração incial MQTT broker;
  delay(1000);
  Serial.println("\n1.7 MQTT;");
  storeLog("\n- Conectando ao MQTT: ... ");
  setupMqtt("  - MQTT", config.mqtt_server, config.mqtt_port, config.mqtt_username, config.mqtt_password, config.mqtt_topic);
  storeLog("ok;");

  // 1.8 Iniciando controllers;
  delay(1000);
  Serial.println("\n\n1.8 Iniciando controllers;");
  setupSensors();

  // 1.9 Definindo variáives auxiliares globais;
  int now = millis();
  lastVVTImpulseTime = now;
  lastPVLImpulseTime = now;
  startTime = now;

  // 2; Inicio
  Serial.printf("\n------------------- PRIMEIRA ITERAÇÃO -------------------\n\n");
  int timestamp = timeClient.getEpochTime();
  convertTimeToLocaleDate(timestamp);
  String dataHora = String(formatedDateString) + "T" + timeClient.getFormattedTime();
  storeLog(("\n" + dataHora + "\n").c_str());
  healthCheck.timestamp = timestamp;
}

void loop() {

  // 1. Watchers
  if (forceRestart == true){
    Serial.println("Reiniciando Arduino a força;");
    storeLog("Reiniciando Arduino a força;");
    delay(1000);
    ESP.restart();
  }

  // 2. Health check
  healthCheck.isWifiConnected = WiFi.status() == WL_CONNECTED;
  healthCheck.wifiDbmLevel = !healthCheck.isWifiConnected ? 0 : (WiFi.RSSI()) * -1;
  healthCheck.isMqttConnected = mqttClient.loop();
  healthCheck.timeRemaining = timeRemaining;

  const char * hcJson = healthCheck.toJson();

  Serial.printf("\n\nColetando dados, proximo resultado em %d segundos...", (timeRemaining / 1000));
  Serial.printf("\n%s",hcJson);

  // 3 Garantindo conexão com mqqt broker;
  if (healthCheck.isWifiConnected && !healthCheck.isMqttConnected) {
    healthCheck.isMqttConnected = connectMqtt("\n  - MQTT", config.mqtt_username, config.mqtt_password, config.mqtt_topic);
  }

  // 4 Atualizando BLE advertsting value
  if (BLE::isDeviceConnected()){
    BLE::updateValue("7c4c8722-8b05-4cca-b5d2-05ec864f90ee", hcJson);
  }

  // 5 Garantindo Tempo ocioso para captação de metricas 60s
  timeRemaining = startTime + config.interval - millis();
  if (timeRemaining > 0) {
    unsigned long startMillis = millis();
    while (millis() - startMillis < 5000);
    return;
  }

  // 6 Ping NTP
  timeClient.update();
  int timestamp = timeClient.getEpochTime();
  convertTimeToLocaleDate(timestamp);
  healthCheck.timestamp = timestamp;

  // 7 Computando dados
  Serial.printf("\n\n3. Computando dados ...\n");

  Data.wind_dir = getWindDir();
  Data.wind_speed = 3.052 * (ANEMOMETER_CIRC * anemometerCounter) / (INTERVAL / 1000.0); // m/s
  Data.wind_gust = (3052.0f * ANEMOMETER_CIRC) / smallestDeltatime;
  Data.rain_acc = rainCounter * VOLUME_PLUVIOMETRO;
  DHTRead(Data.humidity, Data.temperature);
  BMPRead(Data.pressure);

  // 7.1 Redefinido variaveis de medição
  startTime = millis();
  anemometerCounter = 0;
  rainCounter = 0;
  smallestDeltatime = 4294967295;

  // 8. Apresentação dos Dados
  Serial.printf("\n\n4. Nova metrica (%sT%s)", formatedDateString, timeClient.getFormattedTime());
  Serial.println("\nResultado:");
  Serial.printf("Timestamp..........: %d\n", timestamp);
  Serial.printf("Velocidade do vento: %f\n", Data.wind_speed);
  Serial.printf("Chuva acumulada....: %f\n", Data.rain_acc);
  Serial.printf("Umidade............: %f\n", Data.humidity);
  Serial.printf("Temperatura........: %f°C\n", Data.temperature);
  Serial.printf("Raja de vento......: %f\n", Data.wind_gust);
  Serial.printf("Direção do vento...: %s (%d)\n", strVals[Data.wind_dir], Data.wind_dir);
  Serial.printf("Pressao............: %f hpa\n", Data.pressure);

  // Transformando dados
  parseData(timestamp);
  Serial.printf("\nResultado CSV:\n%s", csv_output);
  Serial.printf("\nResultado JSON:\n%s\n", json_output);

  // Armazenamento local
  Serial.println("\n4.1. Gravando em disco:");
  storeMeasurement("/metricas", formatedDateString, csv_output);

  // Enviando Dados Remotamente
  Serial.println("\n4.2 Enviando Resultados remotamente:  ");
  bool measurementSent = sendMeasurementToMqtt(config.mqtt_topic, json_output);

  // Update metric advertsting 
  if (BLE::isDeviceConnected()){
    // healthCheck.currentMetrics = json_output;
    healthCheck.timestamp = timestamp;
  }

  Serial.printf("\n------------------- PROXIMA ITERAÇÃO -------------------\n");
}

// callbacks
int SaveConfigFile(const std::string &json) {
  createFile(SD, "/config.txt", json.c_str());
  forceRestart = true;
  return 1;
}

// utils
void parseData(long timestamp) {
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