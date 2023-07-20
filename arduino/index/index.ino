// Autor: Lucas Fonseca e Gabriel Fonseca
// Titulo: Sit arduino
//.........................................................................................................................

#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

#include "integration.h"

// pinos
#define DHTPIN 17
#define ANEMOMETER_PIN 18
#define PLV_PIN 5
#define VANE_PIN 36
#define DHTTYPE DHT22  // Define o tipo de sensor (DHT11 ou DHT22)
// constants
#define STATION_ID "001"
#define STATION_P "est001"
#define INTERVAL 60000  // Intervalo de Tempo entre medições (ms)
#define DEBOUNCE_DELAY 25

// Anemometro (Velocidade do vento)
#define ANEMOMETER_CIRC (2.0 * 3.14159265 * 0.085)  // Circunferência anemometro (m)
unsigned long lastVVTImpulseTime = 0;
unsigned int anemometerCounter = 0;

// Pluviometro
#define VOLUME_PLUVIOMETRO 2.5  // Volume do pluviometro em ml
unsigned long lastPVLImpulseTime = 0;
unsigned rainCounter = 0;

// Biruta (Direção do vento)
#define NUMDIRS 8
//int adc2[NUMDIRS] = {144, 50, 475, 260, 860, 700, 2190, 2070, 3750, 3020, 3350, 2489, 2832, 1315, 1530, 100 };
//char* strVals2[NUMDIRS] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW" };
int adc[NUMDIRS] = {480,600,800,1150,1820,250,305,380};
char* strVals[NUMDIRS] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW" };
unsigned int vane_dir = 0;
byte dirOffset = 0;
// Temperatura, umidade
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

bool falha = false;

// dto
struct
{
  float wind_speed = 0;
  float rain_acc = 0;
  float humidity = 0;
  float temperature = 0;
  float pressure = 0;
  int wind_dir = -1;
  unsigned long SmallestTime=~0;
} Data;

char json_output[240]{0};

void DataToJson(long timestamp){
  const char* csv_header = "{\"timestamp\": %i, \"temperatura\": %.2f, \"umidade_ar\": %.2f, \"velocidade_vento\": %.2f, \"dir_vento\": %d, \"volume_chuva\": %.2f, \"uid\": \"%s\", \"identidade\": \"%s\"}";
  sprintf(json_output, csv_header,timestamp,Data.temperature,Data.humidity, Data.wind_speed,Data.wind_dir, Data.rain_acc,STATION_ID,STATION_P );
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n Sistema Integrado de meteorologia \n");
  connectWifi();
  connectNtp();
  connectMqtt();
  pinMode(ANEMOMETER_PIN, INPUT_PULLUP);
  pinMode(PLV_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(ANEMOMETER_PIN), anemometerChange, FALLING);
  attachInterrupt(digitalPinToInterrupt(PLV_PIN), pluviometerChange, RISING);
  dht.begin();

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    while (1);
  }



  int now = millis();
  lastVVTImpulseTime = now;
  lastPVLImpulseTime = now;
}

void loop() {

  timeClient.update();
  int timestamp = timeClient.getEpochTime();

  Serial.print("Iniciando medição em: ");
  Serial.println(timestamp);

  anemometerCounter = 0;
  rainCounter = 0;
  Data.SmallestTime=~0;
  long startTime = millis();
  while (millis() < startTime + INTERVAL) {}

  // calc
  Data.wind_dir = getWindDir();
  Data.wind_speed = (ANEMOMETER_CIRC * anemometerCounter) / (INTERVAL / 1000.0f);  // em segundos
  Data.rain_acc = rainCounter * VOLUME_PLUVIOMETRO;
  DHTRead();

  // presentation
  Serial.print("....................\n");
  Serial.print("Velocidade do vento:  ");
  Serial.println(Data.wind_speed);
  Serial.print("Velocidade Maxima: ");
  Serial.println(1000.0/Data.SmallestTime);

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

  DataToJson(timestamp);
  Serial.println(json_output);

  Serial.print("Enviando...........:  ");
  sendMeasurementToMqtt(json_output);

  Serial.println("ok\n");

  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0; // Convert Pa to hPa

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

}

int getWindDir() {
  long long val, x, reading;
  val = (analogRead(VANE_PIN)+analogRead(VANE_PIN)+analogRead(VANE_PIN))/3;
  int closestIndex = 0;
  int closestDifference = std::abs(val - adc[0]);

  for (int i = 1; i < 16; i++) {
    int difference = std::abs(val - adc[i]);
    if (difference < closestDifference) {
      closestDifference = difference;
      closestIndex = i;
    }
  }
  return closestIndex;
}

void anemometerChange() {
  unsigned long currentMillis = millis();
  if (unsigned long deltaTime = currentMillis - lastVVTImpulseTime >= DEBOUNCE_DELAY) {

    if (deltaTime< Data.SmallestTime)Data.SmallestTime = deltaTime;
    anemometerCounter++;
    lastVVTImpulseTime = currentMillis;
  }
}

void pluviometerChange() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastPVLImpulseTime >= DEBOUNCE_DELAY) {
    rainCounter++;
    lastPVLImpulseTime = currentMillis;
  }
}

void DHTRead() {
  float humidity = dht.readHumidity();        // umidade relativa
  float temperature = dht.readTemperature();  //  temperatura em graus Celsius

  // Verifica se alguma leitura falhou
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Falha ao ler o sensor DHT!");
    falha = true;
    return;
  }
  else falha = false;
  

  Data.humidity = humidity;
  Data.temperature = temperature;
}