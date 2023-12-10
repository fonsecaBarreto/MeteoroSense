#pragma once
#include<string>

// Station BLE service & characteristics
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CONFIGURATION_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define HEALTH_CHECK_UUID "7c4c8722-8b05-4cca-b5d2-05ec864f90ee"

class BLE {
  public:
  static void Init(const char* boardName, int (*callback)(const char *uid, const std::string &content));
  static void updateValue(const char *characteristicId, const std::string &newValue);
  static bool stop();
};

