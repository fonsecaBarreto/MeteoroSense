// Autor: Lucas Fonseca e Gabriel Fonseca
// Titulo: Integração HTTP
// Data: 20/05/2023
//.........................................................................................................................

#include <DHT.h>
#include "integration.h"
#define DHTPIN 21      // Define o pino de conexão do sensor ao Arduino
#define DHTTYPE DHT11  // Define o tipo de sensor (DHT11 ou DHT22)
// Temperatura, umidade
DHT dht(DHTPIN, DHTTYPE);
#define INTERVAL 5000  // Intervalo de Tempo entre medições (ms)
#define DEBOUNCE_DELAY 25
// Anemometro (Velocidade do vento)
#define ANEMOMETER_PIN 18
#define ANEMOMETER_CIRC (2 * 3.14159265 * 0.145)  // Circunferência anemometro (m)
unsigned long lastVVTImpulseTime = 0;
unsigned int anemometerCounter = 0;
// Pluviometro
#define PLV_PIN 5
#define VOLUME_PLUVIOMETRO 0.33  // Volume do pluviometro em ml
unsigned long lastPVLImpulseTime = 0;
unsigned rainCounter = 0;
// Biruta (Direção do vento)

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n Sistema Integrado de meteorologia \n");
  // integrações
  connectWifi();
  // pinos
  pinMode(ANEMOMETER_PIN, INPUT_PULLUP);
  pinMode(PLV_PIN, INPUT_PULLUP);

  // initial setup
  int now = millis();
  lastVVTImpulseTime = now;
  lastPVLImpulseTime = now;
  attachInterrupt(digitalPinToInterrupt(ANEMOMETER_PIN), anemometerChange, FALLING);
  attachInterrupt(digitalPinToInterrupt(PLV_PIN), pluviometerChange, FALLING);
  dht.begin();
}

struct
{
  float wind_speed = 0;
  float rain_acc = 0;
  float humidity = 0;
  float temperature = 0;
  float pressure = 0;
} Data;

void loop() {
  Serial.println("\nIniciando medições");

  anemometerCounter = 0;
  rainCounter = 0;

  long startTime = millis();
  while (millis() < startTime + INTERVAL) {}

  // calc
  Data.wind_speed = (ANEMOMETER_CIRC * anemometerCounter) / (INTERVAL / 1000.0f);  // em segundos
  Data.rain_acc = rainCounter * VOLUME_PLUVIOMETRO;
  DHTRead();

  // presentation
  Serial.print("Velocidade do vento: ");
  Serial.println(Data.wind_speed);
  Serial.print("Chuva acumulada: ");
  Serial.println(Data.rain_acc);
  Serial.print("Umidade: ");
  Serial.print(Data.humidity);
  Serial.print("%\t");
  Serial.print("Temperatura: ");
  Serial.print(Data.temperature);
  Serial.println("°C");

  const char* csv_header ="timestamp,wind_speed,rain_cc,humidity,temperature\n%d,%.2f,%.2f,%.2f,%.2f";
  char csv_output[255];
  sprintf(csv_output, csv_header, startTime + INTERVAL, Data.wind_speed, Data.rain_acc, Data.humidity,Data.temperature);
  sendMeasurement(csv_output);
}

void anemometerChange() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastVVTImpulseTime >= DEBOUNCE_DELAY) {
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
    return;
  }

  Data.humidity = humidity;
  Data.temperature = temperature;
}