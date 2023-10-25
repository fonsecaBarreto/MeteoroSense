#pragma once
// --- Config data  ---
struct Config {
  char station_uid[64];
  char station_name[64];
  char wifi_ssid[64];
  char wifi_password[64];
  char mqtt_server[64];
  char mqtt_username[64];
  char mqtt_password[64];
  char mqtt_topic[64];
  int mqtt_port;
  int interval;
};

struct Config config;
const char *configFileName = "/config.txt";

// --- HeachCheck data  ---

char hcSaida[512]{0};
struct HealthCheck {
  const char *softwareVersion;
  int timestamp;
  bool isWifiConnected;
  bool isMqttConnected;
  int wifiDbmLevel;
  int timeRemaining;

  const char *toJson(){
    const char *json_template = "{\"softwareVersion\": \"%s\", \"isWifiConnected\": %d, \"isMqttConnected\": %d, \"wifiDbmLevel\": %i, \"timestamp\": %i,  \"timeRemaining\": %i }";
    sprintf(hcSaida, json_template,
            softwareVersion,
            isWifiConnected ? 1 : 0,
            isMqttConnected ? 1 : 0,
            wifiDbmLevel,
            timestamp,
            timeRemaining);
    return hcSaida;
  }
};

struct HealthCheck healthCheck = {"1.6", 0, false, false, 0, 0};

// --- Metrics data  ---

char metricsjsonOutput[240]{0};
char metricsCsvOutput[240]{0};
char csvHeader[200]{0};

struct Metrics {
  float wind_speed = 0;
  float wind_gust = 0;
  float rain_acc = 0;
  float humidity = 0;
  float temperature = 0;
  float pressure = 0;
  int wind_dir = -1;
  long timestamp;
} Data;

void parseData() {
  // parse measurements data to json
  const char *json_template = "{\"timestamp\": %i, \"temperatura\": %s, \"umidade_ar\": %s, \"velocidade_vento\": %.2f, \"rajada_vento\": %.2f, \"dir_vento\": %d, \"volume_chuva\": %.2f, \"pressao\": %s, \"uid\": \"%s\", \"identidade\": \"%s\"}";
  sprintf(metricsjsonOutput, json_template,
          Data.timestamp,
          isnan(Data.temperature) ? "null" : String(Data.temperature),
          isnan(Data.humidity) ? "null" : String(Data.humidity),
          Data.wind_speed,
          Data.wind_gust,
          Data.wind_dir,
          Data.rain_acc,
          Data.pressure == -1 ? "null" : String(Data.pressure),
          config.station_uid,
          config.station_name);

  // parse measurement data to csv
  const char *csv_template = "%i,%s,%s,%.2f,%.2f,%d,%.2f,%s,%s,%s\n";
  sprintf(metricsCsvOutput, csv_template,
          Data.timestamp,
          isnan(Data.temperature) ? "null" : String(Data.temperature),
          isnan(Data.humidity) ? "null" : String(Data.humidity),
          Data.wind_speed,
          Data.wind_gust,
          Data.wind_dir,
          Data.rain_acc,
          Data.pressure == -1 ? "null" : String(Data.pressure),
          config.station_uid,
          config.station_name);
}