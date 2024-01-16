#pragma once

// constants
#define INTERVAL 60000       // Intervalo de Tempo entre medições [ms]
#define DEBOUNCE_DELAY 25   // [ms]

// pinos
#define DHTPIN 17
#define ANEMOMETER_PIN 18
#define PLV_PIN 5
#define VANE_PIN 36
#define DHTTYPE DHT22       // Define o tipo de sensor (DHT11 ou DHT22)
#define LED3 33 // Green
#define LED1 26 // Yellow
#define LED2 14 // Red


// Anemometro (Velocidade do vento)
#define ANEMOMETER_CIRC (2.0 * 3.14159265 * 0.085)  // Circunferência anemometro (m)

// Pluviometro
#define VOLUME_PLUVIOMETRO 0.34  // Volume do pluviometro em mm

//Biruta
#define NUMDIRS 8
inline int adc[NUMDIRS] = {800, 600, 515, 420, 350, 295, 1900, 1200};
inline char* strVals[NUMDIRS] = {"E", "NE", "N", "NW", "W", "SW", "S", "SE"};
inline char dirOffset = 0;

inline bool falha = false;
