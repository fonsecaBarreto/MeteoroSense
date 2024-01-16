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
float anemometerCounter = 0.0f;
unsigned long lastVVTImpulseTime = 0;
unsigned long smallestDeltatime=4294967295;
unsigned int gustIndex = 0;  
unsigned int previousCounter= 0;
Sensors sensors;

void setupSensors(){
  // Inciando DHT
  Serial.println("Iniciando DHT");
  dht.begin();

  // Iniciando BMP
  Serial.println('Iniciando BMP ');
  beginBMP();
}

void beginBMP()
{
  sensors.bits.bmp= bmp.begin();
  if (!sensors.bits.bmp) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
  }
}

// Controllers

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
  hum = dht.readHumidity();        // umidade relativa
  temp = dht.readTemperature();  //  temperatura em graus Celsius
  if (isnan(hum) || isnan(temp)){
    Serial.println("Falha ao ler o sensor DHT!");
  }
}

//Pressao
void BMPRead(float& press)
{
  if(sensors.bits.bmp){
    // float temperature = bmp.readTemperature(); // isnan(temperature)
    float pressure = bmp.readPressure() / 100.0; // Convert Pa to hPa
    if (isnan(pressure)) {
      Serial.println("Falha ao ler o sensor BMP180!");
      press = -1;
      return;
    }
    press = pressure;
  }else{
    beginBMP();
  }
}

int rps[20]{0};

void WindGustRead(unsigned int now)
{
  static unsigned int lastAssignement = 0;

  int gustInterval = now-lastAssignement;
    if(gustInterval>=3000)
    {
      lastAssignement= now;
      int revolutions = anemometerCounter- previousCounter;
      previousCounter=anemometerCounter;
      rps[gustIndex++] = revolutions;
      gustIndex = gustIndex%20;
    }
}
void windGustReset(){gustIndex=0;  previousCounter = 0;}

int findMax(int arr[], int size) {
    if (size <= 0) {
        printf("Array is empty.\n");
        return 0; 
    }
    int max = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}