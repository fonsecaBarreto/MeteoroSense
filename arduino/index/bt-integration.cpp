#include "bt-integration.h"
#include <iostream>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Station BLE service & characteristics
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CONFIGURATION_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define HEALTH_CHECK_UUID "7c4c8722-8b05-4cca-b5d2-05ec864f90ee"

BLEServer *pServer = nullptr;
BLECharacteristic *pConfigCharacteristic = nullptr;
BLECharacteristic *pHealthCharacteristic = nullptr;
bool deviceConnected = false;
int (*jscb)(const std::string &json);

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer){
        std::cout << "Novo aparelho conectado.\n";
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer){
        deviceConnected = false;
        BLEDevice::startAdvertising();
    }
};

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        if (!rxValue.empty()){
            std::cout << "Received message: " << std::endl;
            std::cout << rxValue << std::endl;
            if (jscb) {
                jscb(rxValue);
            }
        }
    }
};

void BLE::SetConfigCallback(int (*jsCallback)(const std::string &json)) { jscb = jsCallback; }

void BLE::Init(const char *boardName, const std::string &currentConfig) {
    std::cout << currentConfig << std::endl;
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
        BLECharacteristic::PROPERTY_READ   | 
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY);

    pHealthCharacteristic = pService->createCharacteristic(
        HEALTH_CHECK_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE);

    pConfigCharacteristic->setCallbacks(new MyCallbacks());
    if (currentConfig.length() > 0) {
        std::cout << "DoingSomething\n";
        pConfigCharacteristic->setValue(currentConfig);
    }

    pHealthCharacteristic->setCallbacks(new MyCallbacks());
    pHealthCharacteristic->setValue("");



    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    pHealthCharacteristic->addDescriptor(new BLE2902());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter

    BLEDevice::startAdvertising();
    std::cout << "Waiting a client set json\n";
}

void BLE::updateValue(const char *characteristicId, const std::string &newValue){
    if (newValue.length() > 0) {
        pHealthCharacteristic->setValue(newValue);
        pHealthCharacteristic->notify();
    }
}

bool BLE::isDeviceConnected(){
    return deviceConnected;
}