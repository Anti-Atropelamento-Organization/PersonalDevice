#ifndef VEHICLEDEVICE_H
#define VEHICLEDEVICE_H

#include "DeviceBase.h"

class VehicleDevice : public DeviceBase {
public:
    VehicleDevice();

    void buildDynamicBubble360(
        float headingDeg,
        float baseRadius,
        float maxExtra,
        int   qntdSetores,
        float exponent
    );

    struct PeerInfo {
        bool     valid = false;
        float    distance_m = 0.0f;
        uint32_t lastSeenMs = 0;
    };

    

    const float* getBubble360() const { return bubble360_; }

    float getBubbleAt(int deg) const { return bubble360_[(deg % 360 + 360) % 360]; }

    float bearingFromTruckDeg(float truckLat, float truckLng, float targetLat, float targetLng);

    float sectionRadiusAtPersona(float angPersona);

protected:
    void buildSafetyPacket() override;
    void buildMonitoringPacket() override;
    void onReceiveDecoded() override;

private:
    float bubble360_[360];
    static PeerInfo peers[256];
};

#endif

