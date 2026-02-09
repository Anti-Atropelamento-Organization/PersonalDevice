#include "BLEHandle.h"

BLEHandle::BLEHandle(PersonalDevice& device) : _device(device) {}

void BLEHandle::begin(const char* deviceName) {
    BLEDevice::init(deviceName);
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(CONFIG_SERVICE_UUID);
    
    BLECharacteristic *pChar = pService->createCharacteristic(
                                    CONFIG_CHARACTERISTIC_UUID,
                                    BLECharacteristic::PROPERTY_WRITE
                                );

    // Passamos a referência do device para o callback
    pChar->setCallbacks(new MyBLECallbacks(_device));
    
    pService->start();
    BLEDevice::startAdvertising();
}

void BLEHandle::stop() {
    BLEDevice::deinit(false);
}

// Implementação do Callback
void MyBLECallbacks::onWrite(BLECharacteristic* pChar) {
    std::string value = pChar->getValue();
    if (value.length() > 0) {
        // Atualiza o device diretamente
        _device.updateFromBluetooth(String(value.c_str()));
    }
}