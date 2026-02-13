#ifndef PACKET_H
#define PACKET_H

#include <Arduino.h>

// Definição dos Tipos de Pacote
#define SAFETY_PACKET      0x01
#define MONITORING_PACKET  0x02
#define ADVERTISE_PACKET   0x03

#define VEHICLE_DEVICE     0x01
#define ALERT_ADVERTISE    0xA1 
#define ALERT_INTERLOCK    0xA2 

static const int MAX_VEHICLES = 10;

struct ActiveVehicles {
uint8_t id;
double distance;
uint32_t lastSeenMs;
};

// --- ESTRUTURAS COMPACTADAS (PACKED) ---

struct __attribute__((packed)) SafetyPayload {
    uint8_t packetType;
    uint8_t id;
    uint8_t deviceType;
    int32_t lat;
    int32_t lng;
    uint8_t hdop;
    uint8_t speed;
    uint8_t course;
};

// !!! ADICIONE ISSO AQUI !!!
#define SAFETY_PACKET_SIZE sizeof(SafetyPayload) 


struct __attribute__((packed)) MonitoringPayload {
    uint8_t packetType;
    uint8_t id;
    uint8_t deviceType;
    double lat;
    double lng;
    uint8_t batteryLevel;
    int32_t last5positions[5][2];
    uint8_t last5events[5];
    uint8_t status;
    uint8_t satellites;
    double hdop;
    ActiveVehicles nearbyVehicles[MAX_VEHICLES];
};

// !!! ADICIONE ISSO AQUI !!!
#define MONITORING_PACKET_SIZE sizeof(MonitoringPayload)


struct __attribute__((packed)) AdvertisePayload {
    uint8_t packetType;
    uint8_t id;        
    uint8_t deviceID;  
};

// !!! ADICIONE ISSO AQUI !!!
#define ADVERTISE_PACKET_SIZE sizeof(AdvertisePayload)


// --- Estruturas auxiliares (Dados já decodificados) ---
struct SafetyData {
    uint8_t packetID;
    uint8_t ID;
    uint8_t deviceType;
    int32_t lat;
    int32_t lng;
    float hdop;
    float speed;
    float course;
};

struct MonitoringData {
    uint8_t packetID;
    uint8_t ID;
    uint8_t deviceType;
    double lat;
    double lng;
    uint8_t batteryLevel;
    int32_t last5positions[5][2];
    uint8_t last5events[5];
    uint8_t status;
    uint8_t satellites;
    double hdop;
    ActiveVehicles nearbyVehicles[MAX_VEHICLES];
};

struct AdvertiseData {
    uint8_t packetID;
    uint8_t ID;
    uint8_t deviceID;
};

class packet {
private:
    uint8_t _lastDecodedPacketType; 

public:
    packet(); 

    // Helpers
    uint8_t mapDoubleToUint8(double value);
    int32_t mapDoubleToInt32(double value);
    float mapUint8ToFloat(uint8_t value);

    // Construtores (TX)
    void safetyPacket(uint8_t ID, uint8_t deviceType, double latitude,  double longitude, uint8_t *returnPacket, double speed, double course, double hdop);
    void monitoringPacket(uint8_t ID,  uint8_t deviceType, double latitude, double longitude, uint8_t batteryLevel, int32_t last5positions[5][2], uint8_t last5events[5], uint8_t status, uint8_t satellites, double hdop, ActiveVehicles nearbyVehicles[MAX_VEHICLES], uint8_t *returnPacket);
    void advertisePacket(uint8_t ID, uint8_t deviceID, uint8_t *returnPacket);

    // Decodificação (RX)
    uint8_t decodePacket(uint8_t *receivedPacket, uint8_t myDeviceType);

    // Getters
    uint8_t getPacketID();
    uint8_t getDeviceID(); 
    uint8_t getDeviceType();
    float getSpeed();
    float getCourse();
    uint8_t getAdvertiseID();

    float getLat();
    float getLng();
    float getHdop();
    uint8_t getBatteryLevel();
    uint8_t getStatus();
    uint8_t getSatellites();
    void getLast5Positions(int32_t (&positions)[5][2]);
    void getLast5Events(uint8_t (&events)[5]);

};

#endif