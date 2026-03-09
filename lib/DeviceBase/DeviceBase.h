#ifndef DEVICEBASE_H
#define DEVICEBASE_H

#include "packet.h"
#include "cmslora.h"
#include <Arduino.h>
#include "LoRaBoards.h"
#include <TinyGPS++.h>

#define MONITORING_CHANNEL 1 //canal de pacotes de monitoramento e log
#define SAFETY_CHANNEL 2 //canal de pacotes safety

#define T_BEAM_TX_GPS 8 // pino TX para o GPS do T-Beam
#define T_BEAM_RX_GPS 9 // pino RX para o GPS do T-Beam
#define T_BEAM_CTRL_GPS 7 
#define T_BEAM_PPS_GPS 6

#define HELTEC_TX_GPS 33 // pino TX para o GPS do Heltec
#define HELTEC_RX_GPS 34 // pino RX para o GPS do Heltec
#define HELTEC_CTRL_GPS 35
#define HELTEC_PPS_GPS 36 

// ################# EVENTS #################


// Defines para os eventos do pacote LOG, assim, enviamos inteiros ao invés de strings
#define VEHICLE_TOO_CLOSE_EVENT 1 
#define VEHICLE_CLOSE_EVENT 2
#define VEHICLE_AREA_EVENT 3
#define LOW_BATTERY_EVENT 4
#define LOW_HDOP_EVENT 5
#define LOW_SATTELITES_EVENT 6
#define NO_GPS_EVENT 7

class DeviceBase {
public:

    // Explicação detalhada dos métodos no arquivo cpp

    //#####################################
    //CONSTRUTOR 
    //#####################################
    DeviceBase();
    virtual ~DeviceBase() {}

    //#####################################
    //MÉTODOS LORA E GPS
    //#####################################
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

    void setBatteryLevel(uint8_t level);
    uint8_t getBatteryLevel() const;

    void sendSafety();
    void sendMonitoring();
    void sendLog();
    bool receive();

    bool isChannelBusy(int channel);

    void updateFromBluetooth(String rawData);

    float calculateDistance(double targetLat, double targetLng);

    void sendAlert(uint8_t alertType, uint8_t targetID);
    void cleanEvents();

    int getSatValue();

    bool getSatValid();
    void setSatValue();

    double getHdop() const;
    void setHdop();

    double getRadius(int index) const;
    void setRadius(double hdop);

    bool hasLocation();

    double getReceivedLat();
    double getReceivedLng();

    uint8_t getReceivedID();

    uint8_t getTypePacket();

    uint16_t getRandomPacketID();

    uint16_t getMyRandomMonitoringID();

    uint16_t getMyRandomLogID();

    uint8_t calculateAlertDistance();
    double minDistanceFromVehicle();

    
    //#####################################
    //MÉTODOS PARA EVENTOS DO PACOTE LOG
    //#####################################
    void addEvent(uint8_t event);

    uint8_t monitoringDistanceEvent();

    bool monitoringBatteryEvent(uint8_t Level);

    bool monitoringHdopEvent();

    bool monitoringSatEvent();

    bool monitoringGPSEvent();


protected:

    virtual void buildSafetyPacket() = 0; // método que contrói o pacote safety
    virtual void buildMonitoringPacket() = 0; // método que contrói o pacote monitoring
    virtual void buildLogPacket() {}; // método que contrói o pacote LOG

    virtual void onReceiveDecoded() {} // método que printa as variáveis de um pacote recebido
    virtual uint8_t safetySF() const { return 7; } // define o speed factor do safety
    virtual uint8_t monitoringSF() const { return 9; } // define o speed factor do monitoring e log

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
    uint8_t satelites = 0;

    uint8_t safetyPacket[SAFETY_PACKET_SIZE];
    uint8_t monitoringPacket[MONITORING_PACKET_SIZE];
    uint8_t logPacket[LOG_PACKET_SIZE];
    uint8_t receivedPacket[255];

    double speed = 0.0;
    double deviceCourse = 0.0;
    double deviceHdop = 0.0;
    double deviceRadius[3] = {0.0, 0.0, 0.0};

    ActiveVehicles nearbyVehicles[MAX_VEHICLES];

    packet pckt;
    CMSLoRa lora;
    TinyGPSPlus gps;

    uint8_t lastPacketID;

    uint16_t monitoringRandomID;
    uint16_t LogRandomID;
    
    int EventIndex = 0;
    bool lastLocationIsValid = false;
    
    // Variáveis para rastrear eventos anteriores
    uint8_t lastDistanceAlert = 0;

};

#endif