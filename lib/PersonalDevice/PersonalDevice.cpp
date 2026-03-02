#include "PersonalDevice.h"

PersonalDevice::PersonalDevice() : DeviceBase() {
    deviceType = PERSONAL_DEVICE;
}

void PersonalDevice::buildSafetyPacket() {
  pckt.safetyPacket(deviceID, deviceType, deviceLatitude, deviceLongitude, safetyPacket, 0.0, 0.0, deviceHdop);
}

void PersonalDevice::buildMonitoringPacket() {
  monitoringRandomID = (uint16_t)random(0, 65536);
  pckt.monitoringPacket(deviceID, deviceType, monitoringRandomID, deviceLatitude, deviceLongitude, batteryLevel, status, satelites, deviceHdop, monitoringPacket);
}

void PersonalDevice::buildLogPacket() {
  LogRandomID = (uint16_t)random(0, 65536);
  Serial.println("Battery Level in buildLogPacket: " + String(batteryLevel));
  Serial.println("Hdop in buildLogPacket: " + String(deviceHdop));
  pckt.logPacket(deviceID, deviceType, LogRandomID ,last5positions, last5events, nearbyVehicles, logPacket);
}

void PersonalDevice::onReceiveDecoded() {
    Serial.println();
    Serial.println("Received Packet:");
    Serial.print("ID: "); Serial.println(pckt.getDeviceID());
    Serial.print("Latitude: "); Serial.println(pckt.getLat(), 6);
    Serial.print("Longitude: "); Serial.println(pckt.getLng(), 6);
    Serial.print("Hdop: "); Serial.println(pckt.getHdop(), 2);
    Serial.println();
}

int PersonalDevice::isValidSend(double minDistance) {

  if (minDistance < 5.0f) {
    return 1;
  } else if (minDistance <= 10.0f) {
    return 2;
  } else {  
    return 3;
  }
}


void PersonalDevice::updateVehicleList(uint8_t id, double dist) {
    int firstEmptyIndex = -1;
    bool found = false;

    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (nearbyVehicles[i].id == id) {
            nearbyVehicles[i].distance = dist;
            nearbyVehicles[i].lastSeenMs = millis();
            found = true;
            break;
        }

        if (firstEmptyIndex == -1 && (nearbyVehicles[i].id == 0)) {
            firstEmptyIndex = i;
        }
    }

    if (!found && firstEmptyIndex != -1) {
        nearbyVehicles[firstEmptyIndex].id = id;
        nearbyVehicles[firstEmptyIndex].distance = dist;
        nearbyVehicles[firstEmptyIndex].lastSeenMs = millis();
    }
}

void PersonalDevice::cleanOldVehicles() {
    uint32_t now = millis();
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (nearbyVehicles[i].id != 0 && (now - nearbyVehicles[i].lastSeenMs > 12000)) {
            nearbyVehicles[i].id = 0; 
            nearbyVehicles[i].distance = 0.0;
            nearbyVehicles[i].lastSeenMs = 0;
        }
    }
}
void PersonalDevice::saveLastPosition() {
  static int index_lat = 0;
  static int index_lng = 0;
  if(index_lat >= 5)
  {
    for(int i = 0; i < 4; i++) {
      last5positions[i][0] = last5positions[i + 1][0];
      last5positions[i][1] = last5positions[i + 1][1];
    }
    index_lat = 4;
    index_lng = 4;
  }
  last5positions[index_lat][0] = (int32_t)(deviceLatitude * 10000000);
  last5positions[index_lng][1] = (int32_t)(deviceLongitude * 10000000);
  index_lat += 1;
  index_lng += 1;
}

