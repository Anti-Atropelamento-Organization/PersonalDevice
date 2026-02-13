#include "DeviceBase.h"

DeviceBase::DeviceBase() {
    for (int i = 0; i < 5; i++) {
        last5positions[i][0] = 0;
        last5positions[i][1] = 0;
        last5events[i] = 0;
    }
}

void DeviceBase::setup() {
    lora.begin();
    lora.SpreadingFactor(7);
    lora.receiveData(receivedPacket, 0, 0);
}

void DeviceBase::alimentandoGPS() {
    while (SerialGPS.available() > 0) {
        gps.encode(SerialGPS.read());
    }
}

uint8_t DeviceBase::getID() const { return deviceID; }
void DeviceBase::setID(uint8_t id) { deviceID = id; }

double DeviceBase::getLatitude() const { return deviceLatitude; }
void DeviceBase::setLatitude() { deviceLatitude = gps.location.lat(); }
void DeviceBase::forceLatitude(double lat) { deviceLatitude = lat; }

double DeviceBase::getLongitude() const { return deviceLongitude; }
void DeviceBase::setLongitude() { deviceLongitude = gps.location.lng(); }
void DeviceBase::forceLongitude(double lng) { deviceLongitude = lng; }

double DeviceBase::getSpeed() const { return speed; }
void DeviceBase::setSpeed() { speed = gps.speed.kmph(); }

double DeviceBase::getCourse() const { return deviceCourse; }
void DeviceBase::setCourse() { deviceCourse = gps.course.deg(); }

int DeviceBase::getSatValue() {

    return satelites;
}

void DeviceBase::setSatValue() {
    satelites = gps.satellites.value();
}


bool DeviceBase::getSatValid() {
    return gps.satellites.isValid();
}

double DeviceBase::getHdop() const { return deviceHdop; }
void DeviceBase::setHdop() { deviceHdop = gps.hdop.hdop(); }


void DeviceBase::sendSafety() {
    double currentSpeed = (deviceType == 1) ? speed : 0.0;
    double currentCourse = (deviceType == 1) ? deviceCourse : 0.0;

    pckt.safetyPacket(deviceID, deviceType, deviceLatitude, deviceLongitude, safetyPacket, currentSpeed, currentCourse, deviceHdop);

    lora.sendData(safetyPacket, SAFETY_PACKET_SIZE);
}

void DeviceBase::sendMonitoring() {
    lora.sendData(monitoringPacket, MONITORING_PACKET_SIZE);
}

bool DeviceBase::receive() {

    if (lora.receiveData(receivedPacket, MONITORING_PACKET_SIZE, 100)) { 
        
        uint8_t result = pckt.decodePacket(receivedPacket, this->deviceType);
        
        if (result == 0) {
            return false;
        }

        onReceiveDecoded();
        return true;
    }
    return false;
}

bool DeviceBase::isChannelBusy(int channel) {
    if (channel == SAFETY_CHANNEL) {
        lora.SpreadingFactor(safetySF());
        buildSafetyPacket();
    } else if (channel == MONITORING_CHANNEL) {
        lora.SpreadingFactor(monitoringSF());
        buildMonitoringPacket();
    }
    return lora.isChannelBusy();
}

void DeviceBase::updateFromBluetooth(String rawData) {
    int firstSemi = rawData.indexOf(';');
    int secondSemi = rawData.indexOf(';', firstSemi + 1);

    if (firstSemi != -1 && secondSemi != -1) {
        uint8_t novoID = (uint8_t)rawData.substring(0, firstSemi).toInt();
        float novaLat = rawData.substring(firstSemi + 1, secondSemi).toFloat();
        float novaLng = rawData.substring(secondSemi + 1).toFloat();

        setID(novoID);
        setLatitude();
        setLongitude();
        Serial.println("\n>>> Dados atualizados via BLE.");
    }
}

float DeviceBase::calculateDistance(double targetLat, double targetLng) {
    double distance = gps.distanceBetween(deviceLatitude, deviceLongitude, targetLat, targetLng);
    if (distance - getRadius(0) < 0) {
        return 0.0;
    }
    return (distance - getRadius(0));
}

void DeviceBase::sendAlert(uint8_t alertType, uint8_t targetID) {
    uint8_t alertPacket[ADVERTISE_PACKET_SIZE];
    pckt.advertisePacket(alertType, targetID, alertPacket);
    lora.sendData(alertPacket, sizeof(alertPacket));
}

double DeviceBase::getRadius(int index) const {
    if (index >=0 && index < 3) {
        return deviceRadius[index];
    }
    return 0.0;
}

void DeviceBase::setRadius(double hdop) {
    if (deviceType == 1) {
        for (int i = 0; i < 3; i++) {
            if (i == 0) {
                deviceRadius[i] = hdop * 1.5;
            } else if (i == 1) {
                deviceRadius[i] = (hdop * 1.5) + 9.0;
            } else if (i == 2) {
                deviceRadius[i] = (hdop * 1.5) + 25.0;
            }
        }
    } else if (deviceType == 2) {
        for (int i = 0; i < 3; i++) {
            if (i == 0) {
                deviceRadius[i] = hdop * 1.5;
            } else if (i == 1) {
                deviceRadius[i] = (hdop * 1.5) + 1.0;
            } else if (i == 2) {
                deviceRadius[i] = (hdop * 1.5) + 2.0;
            }
        }
    }
}

bool DeviceBase::hasLocation() {
    return gps.location.isValid();
}

double DeviceBase::getReceivedLat() {
    return pckt.getLat(); 
}

double DeviceBase::getReceivedLng() {
    return pckt.getLng(); 
}

uint8_t DeviceBase::getReceivedID() {
    return pckt.getDeviceID();
}