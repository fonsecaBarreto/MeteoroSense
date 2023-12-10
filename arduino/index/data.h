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

struct HealthCheck {
  const char *softwareVersion;
  int timestamp;
  bool isWifiConnected;
  bool isMqttConnected;
  int wifiDbmLevel;
  int timeRemaining;
};

char hcJsonOutput[240]{0};
char hcCsvOutput[240]{0};
const char *parseHealthCheckData(HealthCheck hc, int type = 1) {
  if (type == 1) {
    const char *hc_dto = "%s,%d,%d,%i,%i,%i";
    sprintf(hcCsvOutput, hc_dto,
            hc.softwareVersion,
            hc.isWifiConnected ? 1 : 0,
            hc.isMqttConnected ? 1 : 0,
            hc.wifiDbmLevel,
            hc.timestamp,
            hc.timeRemaining);
    return hcCsvOutput;
  } else {
    const char *json_template = "{\"isWifiConnected\": %d, \"isMqttConnected\": %d, \"wifiDbmLevel\": %i, \"timestamp\": %i}";
    sprintf(hcJsonOutput, json_template,
            hc.isWifiConnected ? 1 : 0,
            hc.isMqttConnected ? 1 : 0,
            hc.wifiDbmLevel,
            hc.timestamp);
    return hcJsonOutput;
  }
}
// --- Metrics data  ---

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

char metricsjsonOutput[240]{0};
char metricsCsvOutput[240]{0};
char csvHeader[200]{0};

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