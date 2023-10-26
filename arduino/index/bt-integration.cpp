
#pragma once
#include "esp_system.h"
#include "bt-integration.h"
#include <iostream>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = nullptr;
BLECharacteristic *pConfigCharacteristic = nullptr;
BLECharacteristic *pHealthCharacteristic = nullptr;
BLECharacteristic *pMetricsCharacteristic = nullptr;

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

    pConfigCharacteristic->setCallbacks(new MyCallbacks());
    if (currentConfig.length() > 0) {
        std::cout << "DoingSomething\n";
        pConfigCharacteristic->setValue(currentConfig);
    }

    // Create the Health Check Characteristic
    pHealthCharacteristic = pService->createCharacteristic(
        HEALTH_CHECK_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE);

    pHealthCharacteristic->setCallbacks(new MyCallbacks());
    pHealthCharacteristic->setValue("");

    // Create the Metrics Characteristic
    pMetricsCharacteristic = pService->createCharacteristic(
        METRICS_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE);

    pMetricsCharacteristic->setCallbacks(new MyCallbacks());
    pMetricsCharacteristic->setValue("");

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
        if(characteristicId == HEALTH_CHECK_UUID){
            pHealthCharacteristic->setValue(newValue);
            pHealthCharacteristic->notify();
        }else if(characteristicId == METRICS_UUID){
            pMetricsCharacteristic->setValue(newValue);
            pMetricsCharacteristic->notify();
        }
    }
}

bool BLE::isDeviceConnected(){
    return deviceConnected;
}