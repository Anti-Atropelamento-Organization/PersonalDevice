#ifndef BLEHANDLE_H
#define BLEHANDLE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "PersonalDevice.h"

// UUIDs fixos para o projeto
#define CONFIG_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CONFIG_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class BLEHandle {
public:
    BLEHandle(PersonalDevice& device); // Construtor recebe o crachá por referência
    void begin(const char* deviceName);
    void stop();

private:
    PersonalDevice& _device; // Referência ao seu objeto crachá
};

// Classe interna de Callbacks (escondida no .cpp depois)
class MyBLECallbacks : public BLECharacteristicCallbacks {
public:
    MyBLECallbacks(PersonalDevice& device) : _device(device) {}
    void onWrite(BLECharacteristic* pChar) override;
private:
    PersonalDevice& _device;
};

#endif