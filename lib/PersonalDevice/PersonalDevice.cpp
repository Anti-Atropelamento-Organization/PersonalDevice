#include "PersonalDevice.h"

PersonalDevice::PersonalDevice() : DeviceBase() {
    deviceType = 2;
}

void PersonalDevice::buildSafetyPacket() {
  pckt.safetyPacket(deviceID, deviceType, deviceLatitude, deviceLongitude, safetyPacket, 0.0, 0.0, deviceHdop);
}

void PersonalDevice::buildMonitoringPacket() {
    pckt.monitoringPacket(deviceID, deviceType, deviceLatitude, deviceLongitude,
                          batteryLevel, last5positions, last5events, status, satelites, deviceHdop, monitoringPacket);
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

double PersonalDevice::minDistanceFromVehicle() {
  double minDistanceVehicle = 1000000.0;
  for (int i = 0; i < MAX_VEHICLES; i++) {
    if (nearbyVehicles[i].id != 0 && nearbyVehicles[i].distance < minDistanceVehicle) {
      minDistanceVehicle = nearbyVehicles[i].distance;
    }
  }
  return minDistanceVehicle;
};