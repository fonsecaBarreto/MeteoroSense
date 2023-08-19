// Autor: Lucas Fonseca e Gabriel Fonseca
// Titulo: Sit arduino
//.........................................................................................................................

#include "constants.h"
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

// dto
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


char json_output[240]{0};

void DataToJson(long timestamp){
  const char* csv_header = "{\"timestamp\": %i, \"temperatura\": %.2f, \"umidade_ar\": %.2f, \"velocidade_vento\": %.2f, \"rajada_vento\": %.2f, \"dir_vento\": %d, \"volume_chuva\": %.2f, \"pressao\": %.2f, \"uid\": \"%s\", \"identidade\": \"%s\"}";
  sprintf(json_output, csv_header,timestamp,Data.temperature,Data.humidity, Data.wind_speed,Data.wind_gust, Data.wind_dir, Data.rain_acc,Data.pressure,STATION_ID,STATION_P );
}

void setup()
 {
  sensors.ch = (char)255;
  pinMode(ANEMOMETER_PIN, INPUT_PULLUP);
  pinMode(PLV_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(ANEMOMETER_PIN), anemometerChange, FALLING);
  attachInterrupt(digitalPinToInterrupt(PLV_PIN), pluviometerChange, RISING);

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n///////////////////////////////////////\nSistema Integrado de meteorologia\n///////////////////////////////////////\n");

  connectWifi();
  connectNtp();
  setupMqtt();
  
  beginDHT();
  beginBMP();

  int now = millis();
  lastVVTImpulseTime = now;
  lastPVLImpulseTime = now;
  startTime = now;
}

void loop()
 {
  // Update time stamp
  timeClient.update();
  int timestamp = timeClient.getEpochTime();
  Serial.print("\n1. Iniciando nova interação (");
  Serial.print(timeClient.getFormattedTime());
  Serial.println(")\n");

  // Reconnect mqqtt if disconected
  if (!mqttClient.connected()) connectMqtt();

  // Timeout 
  while(millis() < startTime + INTERVAL){
    mqttClient.loop();
  };
  startTime = millis();

  // Controllers
  Data.wind_dir = getWindDir();
  Data.wind_speed = 3.052 * (ANEMOMETER_CIRC * anemometerCounter) / (INTERVAL / 1000.0); // m/s
  Data.wind_gust = (3052.0f * ANEMOMETER_CIRC)/smallestDeltatime;
  Data.rain_acc = rainCounter * VOLUME_PLUVIOMETRO;
  
  if (sensors.bits.dht)DHTRead(Data.humidity, Data.temperature);
  if (sensors.bits.bmp)BMPRead(Data.pressure);
  else beginBMP();

  presentation(timestamp);
 
  anemometerCounter = 0;
  rainCounter = 0;
  smallestDeltatime=4294967295;
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
  Serial.print("1/menor: ");
  Serial.println( (3052.0f * ANEMOMETER_CIRC)/smallestDeltatime);

  // mqqt 
  DataToJson(timestamp);
  Serial.println("\n3. Enviando Resultado:  \n");
  sendMeasurementToMqtt(json_output);
  Serial.println("..........................................");
}

