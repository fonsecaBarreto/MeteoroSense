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
String formatedDateString = "";
int timeRemaining=0;
std::string jsonConfig;

void logIt(const std::string &message, bool store = false){
  Serial.print(message.c_str());
  if(store == true){
    storeLog(message.c_str());
  }
}

void setup() {
  delay(3000);
  Serial.begin(115200);
  logIt("\n >> Sistema Integrado de meteorologia << \n");

  pinMode(PLV_PIN, INPUT_PULLDOWN);
  pinMode(ANEMOMETER_PIN, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PLV_PIN), pluviometerChange, RISING);
  attachInterrupt(digitalPinToInterrupt(ANEMOMETER_PIN), anemometerChange, FALLING);

  logIt("\nIniciando cartão SD");
  initSdCard();

  logIt("\nCriando diretorios padrões");
  createDirectory("/metricas");
  createDirectory("/logs");

  logIt("\n1. Estação iniciada;", true);
  loadConfiguration(SD, configFileName, config, jsonConfig);

  logIt("\n1.1 Iniciando bluetooth;", true);
  BLE::SetConfigCallback(SaveConfigFile);
  BLE::Init(config.station_name, jsonConfig);

  logIt("\n1.2 Estabelecendo conexão com wifi ", true);
  setupWifi("  - Wifi", config.wifi_ssid, config.wifi_password);
  int nivelDbm = (WiFi.RSSI()) * -1;
  storeLog((String(nivelDbm) + ";").c_str());

  logIt("\n1.3 Estabelecendo conexão com NTP;", true);
  connectNtp("  - NTP");

  logIt("\n1.4 Estabelecendo conexão com MQTT;", true);
  setupMqtt("  - MQTT", config.mqtt_server, config.mqtt_port, config.mqtt_username, config.mqtt_password, config.mqtt_topic);

  logIt("\n\n1.5 Iniciando controllers;", true);
  setupSensors();

  int now = millis();
  lastVVTImpulseTime = now;
  lastPVLImpulseTime = now;
  startTime = now;

  // 2; Inicio
  Serial.printf("\n >> PRIMEIRA ITERAÇÃO\n");
  int timestamp = timeClient.getEpochTime();
  convertTimeToLocaleDate(timestamp);
  String dataHora = String(formatedDateString) + "T" + timeClient.getFormattedTime();
  storeLog(("\n" + dataHora + "\n").c_str());
  healthCheck.timestamp = timestamp;
}

void loop() {

  // Health check
  healthCheck.isWifiConnected = WiFi.status() == WL_CONNECTED;
  healthCheck.wifiDbmLevel = !healthCheck.isWifiConnected ? 0 : (WiFi.RSSI()) * -1;
  healthCheck.isMqttConnected = mqttClient.loop();
  healthCheck.timeRemaining = timeRemaining;

  const char * hcJson = healthCheck.toJson();

  Serial.printf("\n\nColetando dados, proximo resultado em %d segundos...", (timeRemaining / 1000));
  Serial.printf("\n%s",hcJson);

  // Garantindo conexão com mqqt broker;
  if (healthCheck.isWifiConnected && !healthCheck.isMqttConnected) {
    healthCheck.isMqttConnected = connectMqtt("\n  - MQTT", config.mqtt_username, config.mqtt_password, config.mqtt_topic);
  }

  // Atualizando BLE advertsting value
  if (BLE::isDeviceConnected()){
    BLE::updateValue(HEALTH_CHECK_UUID, hcJson);
  }

  // Garantindo Tempo ocioso para captação de metricas 60s
  timeRemaining = startTime + config.interval - millis();
  if (timeRemaining > 0) {
    unsigned long startMillis = millis();
    while (millis() - startMillis < 5000);
    return;
  }

  // Ping NTP
  timeClient.update();
  int timestamp = timeClient.getEpochTime();
  convertTimeToLocaleDate(timestamp);
  healthCheck.timestamp = timestamp;

  // Computando dados
  Serial.printf("\n\n Computando dados ...\n");

  Data.timestamp = timestamp;
  Data.wind_dir = getWindDir();
  Data.wind_speed = 3.052 * (ANEMOMETER_CIRC * anemometerCounter) / (INTERVAL / 1000.0); // m/s
  Data.wind_gust = (3052.0f * ANEMOMETER_CIRC) / smallestDeltatime;
  Data.rain_acc = rainCounter * VOLUME_PLUVIOMETRO;
  DHTRead(Data.humidity, Data.temperature);
  BMPRead(Data.pressure);

  // Redefinido variaveis de medição
  startTime = millis();
  anemometerCounter = 0;
  rainCounter = 0;
  smallestDeltatime = 4294967295;

  // 8. Apresentação dos Dados
  parseData();
  Serial.printf("\nResultado CSV:\n%s", metricsCsvOutput); 
  Serial.printf("\nResultado JSON:\n%s\n", metricsjsonOutput);

  // Armazenamento local
  Serial.println("\n Gravando em disco:");
  storeMeasurement("/metricas", formatedDateString, metricsCsvOutput);

  // Enviando Dados Remotamente
  Serial.println("\n Enviando Resultados:  ");
  bool measurementSent = sendMeasurementToMqtt(config.mqtt_topic, metricsjsonOutput);

  // Update metrics advertsting value
  if (BLE::isDeviceConnected()){
    BLE::updateValue(METRICS_UUID, metricsjsonOutput);
  }

  Serial.printf("\n >> PROXIMA ITERAÇÃO\n");
}

// callbacks
int SaveConfigFile(const std::string &json) {
  createFile(SD, "/config.txt", json.c_str());

  logIt("Configuração alterar via bluetooth", true);
  logIt("Reiniciando Arduino a força;", true);
  delay(1000);
  ESP.restart();
  return 1;
}

void convertTimeToLocaleDate(long timestamp) {
  struct tm *ptm = gmtime((time_t *)&timestamp);
  int day = ptm->tm_mday;
  int month = ptm->tm_mon + 1;
  int year = ptm->tm_year + 1900;
  formatedDateString = String(day) + "-" + String(month) + "-" + String(year);
}

