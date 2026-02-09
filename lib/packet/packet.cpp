#include "packet.h"

// Variáveis globais para armazenar o último dado decodificado
SafetyData safetyPacketData;
MonitoringData monitoringPacketData;
AdvertiseData advertisePacketData;

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

void packet::monitoringPacket(uint8_t ID,  uint8_t deviceType, double latitude, double longitude, uint8_t batteryLevel, int32_t last5positions[5][2], uint8_t last5events[5], uint8_t status, uint8_t *returnPacket) {
    MonitoringPayload pkt;
    memset(&pkt, 0, sizeof(MonitoringPayload));

    pkt.packetType = MONITORING_PACKET;
    pkt.id = ID;
    pkt.deviceType = deviceType;
    pkt.lat = latitude; // Cuidado: double no ESP32 é 8 bytes. Se o receptor for 8-bit, pode dar erro.
    pkt.lng = longitude;
    pkt.batteryLevel = batteryLevel;

    memcpy(pkt.last5positions, last5positions, sizeof(pkt.last5positions));
    memcpy(pkt.last5events, last5events, sizeof(pkt.last5events));
    pkt.status = status;

    memcpy(returnPacket, &pkt, sizeof(MonitoringPayload));
}

void packet::advertisePacket(uint8_t ID, uint8_t deviceID, uint8_t *returnPacket) {
    AdvertisePayload pkt;
    memset(&pkt, 0, sizeof(AdvertisePayload));
    
    pkt.packetType = ADVERTISE_PACKET;
    pkt.id = ID;
    pkt.deviceID = deviceID;

    memcpy(returnPacket, &pkt, sizeof(AdvertisePayload));
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
        monitoringPacketData.ID = pkt->id; // Agora lê corretamente
        monitoringPacketData.deviceType = pkt->deviceType;
        monitoringPacketData.lat = pkt->lat;
        monitoringPacketData.lng = pkt->lng;
        monitoringPacketData.batteryLevel = pkt->batteryLevel;
        monitoringPacketData.status = pkt->status;

        memcpy(monitoringPacketData.last5positions, pkt->last5positions, sizeof(pkt->last5positions));
        memcpy(monitoringPacketData.last5events, pkt->last5events, sizeof(pkt->last5events));

        Serial.print("ID: "); Serial.println(monitoringPacketData.ID);

    } else if (packetID == ADVERTISE_PACKET) {
        AdvertisePayload *pkt = (AdvertisePayload*)receivedPacket;
        

        advertisePacketData.deviceID = pkt->deviceID;
        advertisePacketData.ID = pkt->id; // Lê ID do remetente
        
    
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
    return 0; // Se não for nenhum conhecido
}

uint8_t packet::getDeviceType() {
    // Mesma lógica, expanda se precisar para outros pacotes
    if (_lastDecodedPacketType == SAFETY_PACKET) return safetyPacketData.deviceType;
    if (_lastDecodedPacketType == MONITORING_PACKET) return monitoringPacketData.deviceType;
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
    return (float)safetyPacketData.lat / 1000000.0;
}

float packet::getLng() {
    return (float)safetyPacketData.lng / 1000000.0;
}

float packet::getHdop() {
    return safetyPacketData.hdop;
}