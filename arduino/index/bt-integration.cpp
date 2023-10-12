#include "bt-integration.h"
#include <iostream>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = nullptr;
BLECharacteristic* pConfigCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;
std::string currentConfig;

// callbacks
int(*jscb)(const std::string& json);

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CONFIGURATION_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        std::cout << "Novo aparelho conectado.\n";
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        BLEDevice::startAdvertising();
    }

};

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        if (!rxValue.empty()) {
            // currentConfig = rxValue;
            std::cout << "Received message: " << std::endl;
            std::cout << rxValue << std::endl;
            if(jscb)jscb(rxValue);
            // Process the received message as needed
        }
    }
};

void BLE::SetConfigCallback(int(*jsCallback)(const std::string& json)){jscb=jsCallback;}

void BLE::Init(const char* boardName, const std::string& currentConfig) {
  std::cout<<currentConfig<<std::endl;
  // Create the BLE Device
  BLEDevice::init(boardName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create the Configuration Characteristic
  pConfigCharacteristic = pService->createCharacteristic(
      CONFIGURATION_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );

  BLEDescriptor *pDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
  pDescriptor->setValue("Estacao");

  pConfigCharacteristic->addDescriptor(pDescriptor);
  pConfigCharacteristic->setCallbacks(new MyCallbacks());
  if (currentConfig.length()>0) {
    std::cout<<"DoingSomething\n";
    pConfigCharacteristic->setValue(currentConfig);
  }
  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter

  BLEDevice::startAdvertising();
  std::cout <<"Waiting a client set json\n";

}
