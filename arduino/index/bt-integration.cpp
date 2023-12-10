
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

bool deviceConnected = false;
int (*characteristicCB)(const char *uid, const std::string &content);
class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer){
        std::cout << "Novo dispositivo conectado.\n";
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer){
        deviceConnected = false;
        BLEDevice::startAdvertising();
    }
};

class CharacteristicsCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        const char *characteristicUid = pCharacteristic->getUUID().toString().c_str();
        if (rxValue.empty() || !characteristicCB) return;
        characteristicCB(characteristicUid, rxValue);
    }
};

void BLE::Init(const char *boardName, int (*callback)(const char *uid, const std::string &content)) {

    characteristicCB = callback;

    char boardOutputName[120]{0};
    const char *boardNameTemplate = "SIT-BOARD-%s";
    sprintf(boardOutputName, boardNameTemplate, boardName);

    // Create the BLE Device
    BLEDevice::init(boardOutputName);

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create the Configuration Characteristic
    pConfigCharacteristic = pService->createCharacteristic(
        CONFIGURATION_UUID,
        BLECharacteristic::PROPERTY_READ   | 
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY);

    pConfigCharacteristic->setValue("");
    pConfigCharacteristic->setCallbacks(new CharacteristicsCallback());

    // Create the Health Check Characteristic
    pHealthCharacteristic = pService->createCharacteristic(
        HEALTH_CHECK_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_NOTIFY );

    pHealthCharacteristic->setValue("");

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
}

void BLE::updateValue(const char *characteristicId, const std::string &newValue){
    if (newValue.length() == 0) return;
    std::cout << "\n  - Emitindo valores via Bluetooth (" << newValue << ") \n";
    if(characteristicId == HEALTH_CHECK_UUID){
        pHealthCharacteristic->setValue(newValue);
        pHealthCharacteristic->notify();
    }else if(characteristicId == CONFIGURATION_UUID) {
        pConfigCharacteristic->setValue(newValue);
        pConfigCharacteristic->notify();
    } else {
        std::cout << "Characteristica não encontrada.\n";
        std::cout << newValue;
    }
}

bool BLE::stop(){
    BLEDevice::getAdvertising()->stop();
    deviceConnected = false;
    BLEDevice::deinit(true);
    return true;
}