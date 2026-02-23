#include "packet.h"

// Variáveis globais para armazenar o último dado decodificado
SafetyData safetyPacketData;
MonitoringData monitoringPacketData;
AdvertiseData advertisePacketData;
LogData logPacketData;
AckData ackData;

// Construtor
packet::packet() {
    _lastDecodedPacketType = 0;
}

// --- FUNÇÕES DE MAPEAMENTO ---
uint8_t packet::mapDoubleToUint8(double value) {
    return (uint8_t)((value * 255) / 360); 
}

int32_t packet::mapDoubleToInt32(double value) {
    return (int32_t)(value * 1000000);
}

float packet::mapUint8ToFloat(uint8_t value) {
    return ((float)value * 360.0) / 255.0;
}

// --- CONSTRUTORES DE PACOTE (TX) ---

void packet::safetyPacket(uint8_t ID, uint8_t deviceType, double latitude, double longitude, uint8_t *returnPacket, double speed, double course, double hdop) {
    SafetyPayload pkt;
    // memset garante que não vai sujeira de memória
    memset(&pkt, 0, sizeof(SafetyPayload));

    pkt.packetType = SAFETY_PACKET;
    pkt.id = ID;
    pkt.deviceType = deviceType;
    pkt.lat = mapDoubleToInt32(latitude);
    pkt.lng = mapDoubleToInt32(longitude);
    pkt.hdop = mapDoubleToUint8(hdop);

    if (deviceType == VEHICLE_DEVICE) {
        pkt.speed = mapDoubleToUint8(speed);
        pkt.course = mapDoubleToUint8(course); 
    } else {
        pkt.speed = 0;
        pkt.course = 0;
    }

    memcpy(returnPacket, &pkt, sizeof(SafetyPayload));
}

void packet::monitoringPacket(uint8_t ID,  uint8_t deviceType, uint16_t randomID, double latitude, double longitude, uint8_t batteryLevel, uint8_t status, uint8_t satellites, double hdop, uint8_t *returnPacket) {
    MonitoringPayload pkt;
    memset(&pkt, 0, sizeof(MonitoringPayload));

    pkt.packetType = MONITORING_PACKET;
    pkt.id = ID;
    pkt.deviceType = deviceType;
    pkt.randomID = randomID;
    pkt.lat = latitude; // Cuidado: double no ESP32 é 8 bytes. Se o receptor for 8-bit, pode dar erro.
    pkt.lng = longitude;
    pkt.batteryLevel = batteryLevel;
    pkt.satellites = satellites;
    pkt.hdop = hdop;
    pkt.status = status;

    memcpy(returnPacket, &pkt, sizeof(MonitoringPayload));
}

void packet::ackPacket(uint8_t ID, uint16_t RandomID, uint8_t *returnPacket) {
    AckPayload pkt;
    memset(&pkt, 0, sizeof(AckPayload));

    pkt.ID = ID;
    pkt.RandomID = RandomID;

    memcpy(returnPacket, &pkt, ACK_PACKET_SIZE);
}

void packet::advertisePacket(uint8_t ID, uint8_t deviceID, uint8_t *returnPacket) {
    AdvertisePayload pkt;
    memset(&pkt, 0, sizeof(AdvertisePayload));
    
    pkt.packetType = ADVERTISE_PACKET;
    pkt.id = ID;
    pkt.deviceID = deviceID;

    memcpy(returnPacket, &pkt, ADVERTISE_PACKET_SIZE);
}

void packet::logPacket(uint8_t ID, uint8_t deviceID, uint16_t randomID, int32_t last5positions[5][2], uint8_t last5events[5], ActiveVehicles nearbyVehicles[MAX_VEHICLES], uint8_t *returnPacket) {
    LogPayLoad pkt;
    memset(&pkt, 0, sizeof(LogPayLoad));

    pkt.packetType = LOG_PACKET;
    pkt.id = ID;
    pkt.deviceType = VEHICLE_DEVICE; // Supondo que só veículos enviam log
    pkt.randomID = randomID;
    memcpy(pkt.last5positions, last5positions, sizeof(pkt.last5positions));
    memcpy(pkt.last5events, last5events, sizeof(pkt.last5events));
    memcpy(pkt.nearbyVehicles, nearbyVehicles, sizeof(pkt.nearbyVehicles));

    memcpy(returnPacket, &pkt, LOG_PACKET_SIZE);
}

// --- DECODIFICADOR (RX) ---

uint8_t packet::decodePacket(uint8_t *receivedPacket, uint8_t myDeviceType) {
    uint8_t packetID = receivedPacket[0];
    uint8_t packetType = receivedPacket[2];
    int32_t longitude = receivedPacket[5];


    _lastDecodedPacketType = packetID; 

    if (packetType == myDeviceType){
        Serial.println("Ignorando pacote do mesmo tipo." + String(packetType) + " " + String(myDeviceType));
        return 0;
    }
    if (packetID == SAFETY_PACKET) {
        SafetyPayload *pkt = (SafetyPayload*)receivedPacket;

        safetyPacketData.packetID = pkt->packetType;
        safetyPacketData.ID = pkt->id;
        safetyPacketData.deviceType = pkt->deviceType;
        safetyPacketData.lat = pkt->lat;
        safetyPacketData.lng = pkt->lng;
        safetyPacketData.hdop = mapUint8ToFloat(pkt->hdop);
        
        if (pkt->deviceType == VEHICLE_DEVICE) {
            safetyPacketData.speed = mapUint8ToFloat(pkt->speed);
            safetyPacketData.course = mapUint8ToFloat(pkt->course);
        } else {
            safetyPacketData.speed = 0;
            safetyPacketData.course = 0;
        }

    } else if (packetID == MONITORING_PACKET) {
        MonitoringPayload *pkt = (MonitoringPayload*)receivedPacket;

        monitoringPacketData.packetID = pkt->packetType;
        monitoringPacketData.ID = pkt->id; 
        monitoringPacketData.deviceType = pkt->deviceType;
        monitoringPacketData.randomID = pkt->randomID;
        monitoringPacketData.lat = pkt->lat;
        monitoringPacketData.lng = pkt->lng;
        monitoringPacketData.batteryLevel = pkt->batteryLevel;
        monitoringPacketData.status = pkt->status;
        monitoringPacketData.satellites = pkt->satellites;
        monitoringPacketData.hdop = pkt->hdop;

        Serial.println("[decodePacket] Pacote de monitoramento decodificado:");
        Serial.println("ID: " + String(monitoringPacketData.ID));
        Serial.println("Device Type: " + String(monitoringPacketData.deviceType));
        Serial.println("Latitude: " + String(monitoringPacketData.lat, 6));
        Serial.println("Longitude: " + String(monitoringPacketData.lng, 6));
        Serial.println("Battery Level: " + String(monitoringPacketData.batteryLevel));
        Serial.println("Status: " + String(monitoringPacketData.status));
        Serial.println("Satellites: " + String(monitoringPacketData.satellites));
        Serial.println("HDOP: " + String(monitoringPacketData.hdop, 2));

    } else if (packetID == ADVERTISE_PACKET) {
        AdvertisePayload *pkt = (AdvertisePayload*)receivedPacket;
        
        advertisePacketData.deviceID = pkt->deviceID;
        advertisePacketData.ID = pkt->id; // Lê ID do remetente
        
    
    } else if(packetID == LOG_PACKET) {
        LogPayLoad *pkt = (LogPayLoad*)receivedPacket;

        logPacketData.packetID = pkt->packetType;
        logPacketData.ID = pkt->id;
        logPacketData.deviceType = pkt->deviceType;
        logPacketData.randomID = pkt->randomID;
        memcpy(logPacketData.last5positions, pkt->last5positions, sizeof(pkt->last5positions));
        memcpy(logPacketData.last5events, pkt->last5events, sizeof(pkt->last5events));
        memcpy(logPacketData.nearbyVehicles, pkt->nearbyVehicles, sizeof(pkt->nearbyVehicles));

        Serial.println("[decodePacket] Pacote de log decodificado:");
        Serial.println("ID: " + String(logPacketData.ID));
        Serial.println("Device Type: " + String(logPacketData.deviceType));
            for (int i = 0; i < 5; i++) {
                Serial.println("Last Position " + String(i) + ": (" + String(logPacketData.last5positions[i][0]) + ", " + String(logPacketData.last5positions[i][1]) + ")");
                Serial.println("Last Event " + String(i) + ": " + String(logPacketData.last5events[i]));
            }
            for (int i = 0; i < MAX_VEHICLES; i++) {
                Serial.println("Nearby Vehicle " + String(i) + ": ID=" + String(logPacketData.nearbyVehicles[i].id) + ", Distance=" + String(logPacketData.nearbyVehicles[i].distance, 2) + "m, lastSeen=" + String(logPacketData.nearbyVehicles[i].lastSeenMs / 1000) + "s ago");
            }

    }
    else if(packetID == ACK_PACKET) {
        AckPayload *pkt = (AckPayload*)receivedPacket;

        ackData.ID = pkt->ID;
        ackData.RandomID = pkt->RandomID;
    }
    return packetID;
}

// --- GETTERS CORRIGIDOS ---

uint8_t packet::getPacketID() {
    return _lastDecodedPacketType;
}

// A CORREÇÃO PRINCIPAL ESTÁ AQUI:
uint8_t packet::getDeviceID() {
    if (_lastDecodedPacketType == SAFETY_PACKET) {
        return safetyPacketData.ID;
    } 
    else if (_lastDecodedPacketType == MONITORING_PACKET) {
        return monitoringPacketData.ID;
    }
    else if (_lastDecodedPacketType == ADVERTISE_PACKET) {
        return advertisePacketData.ID;
    }
    else if(_lastDecodedPacketType == LOG_PACKET) {
        return logPacketData.ID;
    }
    else if(_lastDecodedPacketType == ACK_PACKET) {
        return ackData.ID;
    }
    return 0; // Se não for nenhum conhecido
}
uint16_t packet::getAckRandomID() {
    if(_lastDecodedPacketType == ACK_PACKET) {
        return ackData.RandomID;
    }
    return 0;
}

uint8_t packet::getDeviceType() {
    // Mesma lógica, expanda se precisar para outros pacotes
    if (_lastDecodedPacketType == SAFETY_PACKET) return safetyPacketData.deviceType;
    if (_lastDecodedPacketType == MONITORING_PACKET) return monitoringPacketData.deviceType;
    if(_lastDecodedPacketType == LOG_PACKET) return logPacketData.deviceType;
    return 0;
}

float packet::getSpeed() {
    return safetyPacketData.speed;
}

float packet::getCourse() {
    return safetyPacketData.course;
}

uint8_t packet::getAdvertiseID() {
    return advertisePacketData.ID;
}

float packet::getLat() {

    if(_lastDecodedPacketType == SAFETY_PACKET) {
        return (float)safetyPacketData.lat / 1000000.0;
    } else if(_lastDecodedPacketType == MONITORING_PACKET) {
        return monitoringPacketData.lat;
    }
    return 0.0;
}

float packet::getLng() {
    if(_lastDecodedPacketType == SAFETY_PACKET) {
        return (float)safetyPacketData.lng / 1000000.0;
    } else if(_lastDecodedPacketType == MONITORING_PACKET) {
        return monitoringPacketData.lng;
    }
    return 0.0;
}

float packet::getHdop() {
    if(_lastDecodedPacketType == SAFETY_PACKET) {
        return safetyPacketData.hdop;
    } else if(_lastDecodedPacketType == MONITORING_PACKET) {
        return monitoringPacketData.hdop;
    }
    return 0.0;
}
uint8_t packet::getBatteryLevel() {
    if(_lastDecodedPacketType == MONITORING_PACKET) {
        return monitoringPacketData.batteryLevel;
    }
    return 0;
}
uint8_t packet::getStatus() {
    if(_lastDecodedPacketType == MONITORING_PACKET) {
        return monitoringPacketData.status;
    }
    return 0;
}
uint8_t packet::getSatellites() {
    if(_lastDecodedPacketType == MONITORING_PACKET) {
        return monitoringPacketData.satellites;
    }
    return 0;
}
void packet::getLast5Positions(int32_t (&positions)[5][2]) {
    if(_lastDecodedPacketType == LOG_PACKET) {
        memcpy(positions, logPacketData.last5positions, sizeof(logPacketData.last5positions));
    } else {
        memset(positions, 0, sizeof(positions));
    }
}
void packet::getLast5Events(uint8_t (&events)[5]) {
    if(_lastDecodedPacketType == LOG_PACKET) {
        memcpy(events, logPacketData.last5events, sizeof(logPacketData.last5events));
    } else {
        memset(events, 0, sizeof(events));
    }
}
void packet::getNearbyVehicles(ActiveVehicles (&vehicles)[MAX_VEHICLES]) {
    if(_lastDecodedPacketType == LOG_PACKET) {
        memcpy(vehicles, logPacketData.nearbyVehicles, sizeof(logPacketData.nearbyVehicles));
    } else {
        memset(vehicles, 0, sizeof(vehicles));
    }
}
