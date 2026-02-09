#ifndef MAINFUNCTIONS_H
#define MAINFUNCTIONS_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "PersonalDevice.h"
#include "VehicleDevice.h"
#include "packet.h"
#include "SimpleTimer.h"
#include "DeviceBase.h"

class mainFunctions {
public:

    void SendAlertDest();
    void SetVehicleConst(VehicleDevice& vehicle);
    void SetPersonalConst(PersonalDevice& personal);
    void ReceivePacketDevice(DeviceBase& device, SimpleTimer& st, unsigned long& jitterTargetTime, bool& waitingToSend, bool& hasTarget);
    void SendPacketDevice(DeviceBase& device, SimpleTimer& st, unsigned long& jitterTargetTime, bool& waitingToSend);
    void SendTime(PersonalDevice& personal, SimpleTimer& st, bool& hasTarget, int& level, int& lastLevel);
    void ProcessData(PersonalDevice& personal, uint8_t id, double srcLat, double srcLng);
    void ActiveAlert(PersonalDevice& personal);
};

#endif