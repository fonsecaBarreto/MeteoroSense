#define INTERVAL 5000                                   // Intervalo de Tempo entre medições (ms)
#define DEBOUNCE_DELAY 25
// Anemometro
#define ANEMOMETER_PIN 18
#define ANEMOMETER_CIRC (2 * 3.14159265 * 0.145)        // Circunferência anemometro (m)
unsigned long lastVVTImpulseTime = 0;
unsigned int anemometerCounter = 0; 
// Pluviometro
#define PLV_PIN 5
#define VOLUME_PLUVIOMETRO 0.33                         // Volume do pluviometro em ml 
unsigned long lastPVLImpulseTime = 0;
unsigned rainCounter = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("\n\n Sistema Integrado de meteorologia \n");

  // pinos
  pinMode(ANEMOMETER_PIN, INPUT_PULLUP);
  pinMode(PLV_PIN, INPUT_PULLUP);             

  // initial setup
  int now = millis();
  lastVVTImpulseTime = now;
  lastPVLImpulseTime = now;
  attachInterrupt(digitalPinToInterrupt(ANEMOMETER_PIN), anemometerChange, FALLING);
  attachInterrupt(digitalPinToInterrupt(PLV_PIN), pluviometerChange, FALLING); 
}

void loop() {
  Serial.println("\nIniciando medições");

  anemometerCounter = 0;
  rainCounter= 0;
  float wind_speed = 0;
  float rain_acc = 0;

  long startTime = millis();
  while (millis() < startTime + INTERVAL) {}

  // calc
  wind_speed = (ANEMOMETER_CIRC * anemometerCounter) / (INTERVAL / 1000.0f);                  // em segundos
  rain_acc = rainCounter * VOLUME_PLUVIOMETRO;

  Serial.print("Velocidade do vento: ");
  Serial.println(wind_speed);

  Serial.print("Chuva acumulada: ");
  Serial.println(rain_acc);
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
