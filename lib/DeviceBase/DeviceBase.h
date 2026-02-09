#ifndef DEVICEBASE_H
#define DEVICEBASE_H

#include "packet.h"
#include "cmslora.h"
#include <Arduino.h>
#include "LoRaBoards.h"
#include <TinyGPS++.h>

#define MONITORING_CHANNEL 1
#define SAFETY_CHANNEL 2

class DeviceBase {
public:
    DeviceBase();
    virtual ~DeviceBase() {}

    virtual void setup();

    void alimentandoGPS();

    uint8_t getID() const;
    void setID(uint8_t id);

    double getLatitude() const;
    void setLatitude();
    void forceLatitude(double lat);

    double getLongitude() const;
    void setLongitude();
    void forceLongitude(double lng);

    double getSpeed() const;
    void setSpeed();

    double getCourse() const;
    void setCourse();

    void sendSafety();
    void sendMonitoring();
    bool receive();

    bool isChannelBusy(int channel);

    void updateFromBluetooth(String rawData);

    float calculateDistance(double targetLat, double targetLng);

    void sendAlert(uint8_t alertType, uint8_t targetID);

    int getSatValue();

    bool getSatValid();

    double getHdop() const;
    void setHdop();

    double getRadius(int index) const;
    void setRadius(double hdop);

    bool hasLocation();

    double getReceivedLat();
    double getReceivedLng();

    uint8_t getReceivedID();

protected:

    virtual void buildSafetyPacket() = 0;
    virtual void buildMonitoringPacket() = 0;

    virtual void onReceiveDecoded() {}
    virtual uint8_t safetySF() const { return 7; }
    virtual uint8_t monitoringSF() const { return 9; }

protected:
    uint8_t deviceID = 0;
    uint8_t destId = 0;
    double deviceLatitude = 0.0;
    double deviceLongitude = 0.0;
    uint8_t batteryLevel = 100;
    uint8_t status = 0;
    uint8_t deviceType = 0; 
    int32_t last5positions[5][2];
    uint8_t last5events[5];

    uint8_t safetyPacket[SAFETY_PACKET_SIZE];
    uint8_t monitoringPacket[MONITORING_PACKET_SIZE];
    uint8_t receivedPacket[MONITORING_PACKET_SIZE];

    double speed = 0.0;
    double deviceCourse = 0.0;
    double deviceHdop = 0.0;
    double deviceRadius[3] = {0.0, 0.0, 0.0};

    packet pckt;
    CMSLoRa lora;
    TinyGPSPlus gps;
};

#endif