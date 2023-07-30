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

union {
    char ch;
    struct {
        bool div  : 1;
        bool vvt  : 1;
        bool dht  : 1;
        bool bmp  : 1;
        bool aux1 : 1;
        bool aux2 : 1;
        bool aux3 : 1;
        bool aux4 : 1;
    } bits;
}sensors;

// dto
struct
{
  float wind_speed = 0;
  float rain_acc = 0;
  float humidity = 0;
  float temperature = 0;
  float pressure = 0;
  int wind_dir = -1;
} Data;


char json_output[240]{0};

void DataToJson(long timestamp){
  const char* csv_header = "{\"timestamp\": %i, \"temperatura\": %.2f, \"umidade_ar\": %.2f, \"velocidade_vento\": %.2f, \"dir_vento\": %d, \"volume_chuva\": %.2f, \"pressao\": %.2f, \"uid\": \"%s\", \"identidade\": \"%s\"}";
  sprintf(json_output, csv_header,timestamp,Data.temperature,Data.humidity, Data.wind_speed,Data.wind_dir, Data.rain_acc,Data.pressure,STATION_ID,STATION_P );
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
  Serial.println("\n\n Sistema Integrado de meteorologia \n");

  connectWifi();
  connectNtp();
  connectMqtt();
  
  beginDHT();
  beginBMP();

  int now = millis();
  lastVVTImpulseTime = now;
  lastPVLImpulseTime = now;
  startTime = now;
}

void loop()
 {

  timeClient.update();
  int timestamp = timeClient.getEpochTime();

  while(millis() < startTime + INTERVAL);
  startTime = millis();

  // controllers
  Data.wind_dir = getWindDir();
  Data.wind_speed = 2.625 * (ANEMOMETER_CIRC * anemometerCounter) / (INTERVAL / 1000.0); // m/s
  Data.rain_acc = rainCounter * VOLUME_PLUVIOMETRO;
  if (sensors.bits.dht)DHTRead(Data.humidity, Data.temperature);
  if (sensors.bits.bmp)BMPRead(Data.pressure);

  // presentation
  presentation(timestamp);
 
  anemometerCounter = 0;
  rainCounter = 0;
}



void presentation(long timestamp)
{
  Serial.print("....................\n");

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

  Serial.print("Pressao............: ");
  Serial.print(Data.pressure);
  Serial.println(" hPa");

  // mqqt 
  DataToJson(timestamp);
  Serial.println(json_output);
  Serial.print("Enviando...........:  ");

  sendMeasurementToMqtt(json_output);
  Serial.println("ok\n");

}

