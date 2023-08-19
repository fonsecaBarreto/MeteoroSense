#include "sensores.h"
#include "constants.h"
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <cfloat>
//Temperatura e Humidade
DHT dht(DHTPIN, DHTTYPE);
//Pressao
Adafruit_BMP085 bmp;

// Pluviometro
unsigned long lastPVLImpulseTime = 0;
unsigned int rainCounter = 0;
// Anemometro (Velocidade do vento)
unsigned long lastVVTImpulseTime = 0;
float anemometerCounter = 0.0f;
unsigned long smallestDeltatime=4294967295;

Sensors sensors;

void beginDHT()
{
dht.begin();
}

void beginBMP()
{
  bool bmp_Begin = bmp.begin();
  sensors.bits.bmp=bmp_Begin;
  if (!bmp_Begin) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    
  }

}

int getWindDir() {
  long long val, x, reading;
  val = analogRead(VANE_PIN);
  int closestIndex = 0;
  int closestDifference = std::abs(val - adc[0]);

  for (int i = 1; i < NUMDIRS; i++) {
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
  unsigned long deltaTime = currentMillis - lastVVTImpulseTime;
  if (deltaTime >= DEBOUNCE_DELAY) {
    smallestDeltatime = min(deltaTime, smallestDeltatime);
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

//Humidade, Temperatura
void DHTRead(float& hum, float& temp) {
  float humidity = dht.readHumidity();        // umidade relativa
  float temperature = dht.readTemperature();  //  temperatura em graus Celsius

  // Verifica se alguma leitura falhou
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Falha ao ler o sensor DHT!");
    falha = true;
    hum = -1;
    temp = -273.15;
    return;
  }
  else falha = false;
  

  hum = humidity;
  temp = temperature;
}

//Pressao
void BMPRead(float& press)
{
  // float temperature = bmp.readTemperature(); // isnan(temperature)
  float pressure = bmp.readPressure() / 100.0; // Convert Pa to hPa
  if (isnan(pressure)) 
  {
    Serial.println("Falha ao ler o sensor BMP180!");
    press = -1;
    return;
  }
  press = pressure;
}