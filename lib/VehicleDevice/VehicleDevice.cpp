#include "VehicleDevice.h"

VehicleDevice::PeerInfo VehicleDevice::peers[256];

static inline int circularDistDeg(int a, int b) {
    int d = abs(a - b);
    return (d > 180) ? (360 - d) : d;
}

VehicleDevice::VehicleDevice() : DeviceBase() {
    deviceType = 1; 
    for (int i = 0; i < 360; i++) bubble360_[i] = 0.0f;
}


void VehicleDevice::buildSafetyPacket() {
    // com speed e course
    pckt.safetyPacket(deviceID, deviceType, deviceLatitude, deviceLongitude,
                     safetyPacket, speed, deviceCourse, deviceHdop);
}

void VehicleDevice::buildMonitoringPacket() {
    pckt.monitoringPacket(deviceID, deviceType, deviceLatitude, deviceLongitude,
                          batteryLevel, last5positions, last5events, status, satelites, deviceHdop, nearbyVehicles, monitoringPacket);
}

void VehicleDevice::onReceiveDecoded() {
    Serial.println("Received Packet:");
    Serial.print("ID: "); Serial.println(pckt.getDeviceID());
    Serial.print("Latitude: "); Serial.println(pckt.getLat());
    Serial.print("Longitude: "); Serial.println(pckt.getLng());
    Serial.print("Hdop: "); Serial.println(pckt.getHdop());
    Serial.print("\n");
}

void VehicleDevice::buildDynamicBubble360(
    float headingDeg,
    float baseRadius,
    float maxExtra,
    int   qntdSetores,
    float exponent
) {
  int center = (int)lroundf(headingDeg);
  center %= 360;
  if (center < 0) center += 360;

  if (qntdSetores < 0) qntdSetores = 0;
  if (qntdSetores > 180) qntdSetores = 180;

  const float invW = (qntdSetores > 0) ? (1.0f / (float)qntdSetores) : 0.0f;

  for (int i = 0; i < 360; i++) {
    int d = circularDistDeg(i, center);

    float extra = 0.0f;

    if (qntdSetores == 0) {
      extra = (d == 0) ? maxExtra : 0.0f;
    } else if (d <= qntdSetores) {
      float t = d * invW;                  
      float gain = cosf(t * (PI * 0.5f));  
      if (exponent > 0.0f) gain = powf(gain, exponent);
      extra = maxExtra * gain;
    }

    bubble360_[i] = baseRadius + extra;
  }

    //vehicle.buildDynamicBubble360(heading, 8.0f, 12.0f, 45, 2.0f);
}


/*
###########################
CÁLCULO DE DIREÇÃO ENTRE CAMINHÃO E PESSOA
###########################
*/
static inline float wrap360(float a) {
  while (a < 0) a += 360.0f;
  while (a >= 360.0f) a -= 360.0f;
  return a;
}

float bearingFromTruckDeg(double truckLat, double truckLng,
                          double personLat, double personLng) {

  const double R = 6371000.0;

  double lat1 = truckLat * DEG_TO_RAD;
  double lat2 = personLat * DEG_TO_RAD;

  double dLat = (personLat - truckLat) * DEG_TO_RAD;
  double dLon = (personLng - truckLng) * DEG_TO_RAD;

  double dy = dLat * R;
  double dx = dLon * R * cos((lat1 + lat2) * 0.5);

  float ang = (float)(atan2(dx, dy) * RAD_TO_DEG);
  return wrap360(ang);
}

float VehicleDevice::sectionRadiusAtPersona(float angPersona) {
    int deg = (int)lroundf(angPersona);
    deg = (deg % 360 + 360) % 360;
    float r = getBubbleAt(deg);
    return r;
}
