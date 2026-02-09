#include "mainFunctions.h"

/* ##########################
DEVICE MAIN FUNCTIONS
########################## */

void mainFunctions::ReceivePacketDevice(DeviceBase& device, SimpleTimer& st, unsigned long& jitterTargetTime, bool& waitingToSend, bool& hasTarget) {
  if (device.receive()) {
    uint8_t srcId = device.getReceivedID();
    double srcLat = device.getReceivedLat();
    double srcLng = device.getReceivedLng();

    this->ProcessData((PersonalDevice&) device, srcId, srcLat, srcLng);
    hasTarget = true;
  }else {
    hasTarget = false;
  }

  if (st.isReady() && !waitingToSend) {
    st.reset(); 

    long jitter = random(0, 201);
    jitterTargetTime = millis() + jitter; 
    waitingToSend = true;
  }
}

static inline bool hasFixSimplePersonal(PersonalDevice& personal) {
  return personal.hasLocation() && personal.getSatValid() && personal.getHdop() <= 10.0;
}

void mainFunctions::SendPacketDevice(DeviceBase& device, SimpleTimer& st, unsigned long& jitterTargetTime, bool& waitingToSend) {
  if (waitingToSend && millis() >= jitterTargetTime) {
    if (!device.isChannelBusy(SAFETY_CHANNEL) && hasFixSimplePersonal((PersonalDevice&) device)) {
    device.sendSafety();
    Serial.println();
    Serial.println("###########################");
    Serial.println("Pacote enviado");
    Serial.println("Device ID: " + String(device.getID()));
    Serial.println("Device Latitude: " + String(device.getLatitude(), 6));
    Serial.println("Device Longitude: " + String(device.getLongitude(), 6));
    Serial.println("Satellites: " + String(device.getSatValue()));
    Serial.println("Hdop: " + String(device.getHdop()));
    Serial.println("###########################");
    Serial.println();

    waitingToSend = false; 
    }
  }
}

/* ##########################
VEHICLE MAIN FUNCTIONS
########################## */

static inline bool hasFixSimpleVehicle(VehicleDevice& vehicle) {
  return vehicle.hasLocation() && vehicle.getSatValid() && vehicle.getHdop() <= 2.0;
}

void mainFunctions::SetVehicleConst(VehicleDevice& vehicle) {

  vehicle.alimentandoGPS();
  bool fixOk = hasFixSimpleVehicle(vehicle);
  bool satValid = vehicle.getSatValid();
  uint32_t sats = satValid ? vehicle.getSatValue() : 0;
  vehicle.setHdop();
  
  if (fixOk) {
    vehicle.setLatitude();
    vehicle.setLongitude();
    vehicle.setSpeed();
    vehicle.setCourse();
    vehicle.setRadius(vehicle.getHdop());
  }
}

/* ##########################
PERSONAL MAIN FUNCTIONS
########################## */



void mainFunctions::SetPersonalConst(PersonalDevice& personal) {
  personal.alimentandoGPS();
  bool fixOk = hasFixSimplePersonal(personal);
  bool satValid = personal.getSatValid();
  uint32_t sats = satValid ? personal.getSatValue() : 0;
  personal.setHdop();
  
  if (fixOk) {
    personal.setLatitude();
    personal.setLongitude();
    personal.setRadius(personal.getHdop());
  }
}

void mainFunctions::SendTime(PersonalDevice& personal, SimpleTimer& st, bool& hasTarget, int& level, int& lastLevel) {
 double minDistance = personal.minDistanceFromVehicle();

 static uint32_t lasttime = 0; 
  uint32_t now = millis();

  if (now - lasttime >= 1000) {
      Serial.print("Minimum distance from vehicle: ");
      Serial.print(minDistance);
      Serial.println(" meters");
      
      lasttime = now;
  }

  if (hasTarget) {
    level = personal.isValidSend(minDistance);
  }

  if (level != lastLevel) {
    if (level == 1) st.setInterval(3000);
    else if (level == 2) st.setInterval(10000);
    else if (level == 3) st.setInterval(20000);
    st.reset();
    lastLevel = level;
    Serial.println("Level: " + String(lastLevel));
  }
}

void mainFunctions::ProcessData(PersonalDevice& personal, uint8_t id, double srcLat, double srcLng) {
    double dist = personal.calculateDistance(srcLat, srcLng);
    Serial.println();
    Serial.println("Distance calculada: " + String(dist) + " meters");
    Serial.println();
    personal.updateVehicleList(id, dist);
};

void mainFunctions::ActiveAlert(PersonalDevice& personal) {
  double minDistance = personal.minDistanceFromVehicle();
  if (minDistance < personal.getRadius(1) - personal.getRadius(0)) {
    Serial.println("ALERTA: Veículo muito próximo!");
  } else if (minDistance < personal.getRadius(2) - personal.getRadius(1)) {
    Serial.println("ALERTA: Veículo próximo!");
  } else if (minDistance < 30.0) {
    Serial.println("ALERTA: Veículo na área!");
  } else {
    Serial.println("Nenhum veículo próximo.");
  }
}