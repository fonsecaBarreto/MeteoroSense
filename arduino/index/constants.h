#pragma once

// constants
#define STATION_ID "02"
#define STATION_P "est002"
#define INTERVAL 60000       // Intervalo de Tempo entre medições [ms]
#define DEBOUNCE_DELAY 25   // [ms]

// pinos
#define DHTPIN 17
#define ANEMOMETER_PIN 18
#define PLV_PIN 5
#define VANE_PIN 36
#define DHTTYPE DHT22       // Define o tipo de sensor (DHT11 ou DHT22)

// Anemometro (Velocidade do vento)
#define ANEMOMETER_CIRC (2.0 * 3.14159265 * 0.085)  // Circunferência anemometro (m)

// Pluviometro
#define VOLUME_PLUVIOMETRO 0.25  // Volume do pluviometro em mm

//Biruta
#define NUMDIRS 8
//inline int adc[NUMDIRS] = {480,600,800,1150,1820,250,305,380};
//inline char* strVals[NUMDIRS] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW" };
inline int adc[NUMDIRS] = {800, 600, 480, 380, 305, 250, 1820, 1150};
inline char* strVals[NUMDIRS] = {"E", "NE", "N", "NW", "W", "SW", "S", "SE"};
inline char dirOffset = 0;

inline bool falha = false;
