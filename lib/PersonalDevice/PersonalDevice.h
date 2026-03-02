#ifndef PERSONALDEVICE_H
#define PERSONALDEVICE_H

#include "DeviceBase.h"

class PersonalDevice : public DeviceBase {
public:
    PersonalDevice();

    double getLatitudeLive() { return gps.location.isValid() ? gps.location.lat() : NAN; }
    double getLongitudeLive() { return gps.location.isValid() ? gps.location.lng() : NAN; }
    double getSpeedLive() { return gps.speed.isValid() ? gps.speed.kmph() : NAN; }
    double getCourseLive() { return gps.course.isValid() ? gps.course.deg() : NAN; }

    int isValidSend(double minDistance);



    void updateVehicleList(uint8_t id, double dist);
    void cleanOldVehicles();
    void saveLastPosition();
    

protected:
    void buildSafetyPacket() override;
    void buildMonitoringPacket() override;
    void buildLogPacket() override;
    void onReceiveDecoded() override;

private:
    
    uint32_t lastMinResetMs = 0;
};

#endif