// Autor: Lucas Fonseca e Gabriel Fonseca
// Titulo: Sit arduino
//.........................................................................................................................

#include "constants.h"
#include "conf.h"
#include "sd-repository.h"
#include "integration.h"
#include "sensores.h"
#include <stdio.h>
long startTime;

// Pluviometro
extern unsigned long lastPVLImpulseTime;
extern unsigned int rainCounter;

// Anemometro (Velocidade do vento)
extern unsigned long lastVVTImpulseTime;
extern float anemometerCounter;
extern unsigned long smallestDeltatime;
extern Sensors sensors;

const int limit_retry = 10;
int retry_array[limit_retry];
int indexcoco=0;

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

  // 1.3 Inicio das integrações dos serviços externos;
  delay(2000);
  Serial.println("1.3 Integrações externas;");

  // 1.3.1 Estabelecendo conexão com wifi;
  Serial.println("1.3.1 Estabelecendo conexão inicial com wifi\n");
  setupWifi(config.wifi_ssid, config.wifi_password);

  setupMqtt(config.mqtt_server, config.mqtt_port);

  setupSensors();

  connectNtp();

  int now = millis();
  lastVVTImpulseTime = now;
  lastPVLImpulseTime = now;
  startTime = now; 
}

void loop()
{
  // Update Timestamp
  timeClient.update();
  int timestamp = timeClient.getEpochTime();
  convertTimeToLocaleDate(timestamp);

  Serial.print("\n1. Iniciando nova interação (");
  Serial.println(String(formatedDateString) + "T" + String(timeClient.getFormattedTime())+ String(")"));
  
  // Establish internet connection
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("Wifi: Desconectado");
  } else  {
    Serial.println("Wifi: Conectado ["+String(WiFi.localIP()) + "]");
    Serial.print("MQTT: ");
    int isMqttConnected = connectMqtt(config.mqtt_username, config.mqtt_password, config.mqtt_topic);
    if(isMqttConnected == 1){
      Serial.println("Iniciando re-envio de metricas");
      indexcoco=0;
      loopThroughFiles("/retries", limit_retry, retryMeasurementCallback);
      removeRetryMeasurement();
    }
  }

  // Timeout
  while (millis() < startTime + config.interval)
  { 
    mqttClient.loop(); 
  };
  startTime = millis();

  // Controllers
  Data.wind_dir = getWindDir();
  Data.wind_speed = 3.052 * (ANEMOMETER_CIRC * anemometerCounter) / (INTERVAL / 1000.0); // m/s
  Data.wind_gust = (3052.0f * ANEMOMETER_CIRC) / smallestDeltatime;
  Data.rain_acc = rainCounter * VOLUME_PLUVIOMETRO;
  DHTRead(Data.humidity, Data.temperature);
  BMPRead(Data.pressure);

  presentation(timestamp);
  // 
  anemometerCounter = 0;
  rainCounter = 0;
  smallestDeltatime = 4294967295;
}

void presentation(long timestamp)
{
  Serial.println("\n2. Resultado das medições:\n");

  Serial.print("Timestamp..........:  ");
  Serial.println(timestamp);

  Serial.print("Velocidade do vento:  ");
  Serial.println(Data.wind_speed);

  Serial.print("Chuva acumulada....:  ");
  Serial.println(Data.rain_acc);

  Serial.print("Umidade............:  ");
  Serial.println(Data.humidity);

  Serial.print("Temperatura........:  ");
  Serial.print(Data.temperature);
  Serial.println("°C");

  Serial.print("Direção do vento...:  ");
  Serial.print(strVals[Data.wind_dir]);
  Serial.println(Data.wind_dir);

  Serial.print("Pressao............:  ");
  Serial.print(Data.pressure);
  Serial.println(" hPa");

  Serial.print("menor: ");
  Serial.println(smallestDeltatime);
  Serial.print("menor: ");
  Serial.println((3052.0f * ANEMOMETER_CIRC) / smallestDeltatime);

  // local storage
  Serial.println("\n3. Gravando em disco:");
  storeMeasurement("/metricas", formatedDateString, csv_output);

  // mqqt
  parseData(timestamp);
  Serial.println("\n4. Enviando Resultado:  \n");
  bool measurementSent = sendMeasurementToMqtt(config.mqtt_topic, json_output);
  if(measurementSent == false){
    Serial.print('4.2 Salvando Dados para serem enviados posteriorment');
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
    Data.pressure == -1 ? "null": String(Data.pressure), 
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
    Data.pressure == -1 ? "null": String(Data.pressure), 
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
  Serial.println("Trying to resend metrics: [" + String(fileName) + "]");
  Serial.println(payload);
  bool measurementSent = sendMeasurementToMqtt(config.mqtt_topic, payload);
  if(measurementSent == true) {
    int x = 0;
	  sscanf(fileName, " %d",&x);
    retry_array[indexcoco]=x;
    indexcoco++;
  }
}

void removeRetryMeasurement() {
  for(int n= 0 ; n < limit_retry ; n ++ ){
    int timestamp = retry_array[n];
    if(!timestamp) continue;
    String filePath =  "/retries/" + String(timestamp) + ".txt";
    Serial.println("Trying to remove retry file: [" + String(filePath) + "]");
    removeFile(filePath.c_str()); 
    retry_array[n]=0;
  }
}