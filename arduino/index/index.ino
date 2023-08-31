// Autor: Lucas Fonseca e Gabriel Fonseca
// Titulo: Sit arduino
//.........................................................................................................................

#include "constants.h"
#include "conf.h"
#include "sd-repository.h"
#include "integration.h"
#include "sensores.h"

long startTime;

// Pluviometro
extern unsigned long lastPVLImpulseTime;
extern unsigned int rainCounter;

// Anemometro (Velocidade do vento)
extern unsigned long lastVVTImpulseTime;
extern float anemometerCounter;
extern unsigned long smallestDeltatime;
extern Sensors sensors;

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
  Serial.begin(115200);
  pinMode(PLV_PIN, INPUT_PULLDOWN);
  pinMode(ANEMOMETER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PLV_PIN), pluviometerChange, RISING);
  attachInterrupt(digitalPinToInterrupt(ANEMOMETER_PIN), anemometerChange, FALLING);

  delay(2600);

  Serial.println("\n///////////////////////////////////\nSistema Integrado de meteorologia\n///////////////////////////////////\n");

  loadConfiguration(SD, configFileName, config);

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
    Serial.print("Wifi: Conectado ");
    Serial.println("["+String(WiFi.localIP()) + "]");
    Serial.print("MQTT: ");
    connectMqtt(config.mqtt_username, config.mqtt_password, config.mqtt_topic);
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
