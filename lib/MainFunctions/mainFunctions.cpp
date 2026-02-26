#include "mainFunctions.h"

/* ##########################
DEVICE MAIN FUNCTIONS
########################## */

void mainFunctions::ReceivePacketDevice(DeviceBase& device, SimpleTimer& st, unsigned long& jitterTargetTime, bool& waitingToSend, bool& hasTarget, bool& ackMonitoring, bool& ackLog) {
  if (device.receive()){
    uint8_t lastPacketID = device.getTypePacket();

    if(lastPacketID == SAFETY_PACKET){
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
    }else if(lastPacketID == ACK_PACKET){
      Serial.println(">> PACOTE ACK RECEBIDO");
      Serial.println("My Random monitoring: " + String(device.getMyRandomMonitoringID()));
      Serial.println("My Random log: " + String(device.getMyRandomLogID()));
      uint16_t receivedAckID = device.getRandomPacketID(); 
      Serial.println("Packet Random: " + String(receivedAckID));
      if (receivedAckID == device.getMyRandomMonitoringID()) {
          ackMonitoring = true;
          Serial.println("ACK MONITORING");
      }
      
      if (receivedAckID == device.getMyRandomLogID()) {
          ackLog = true;
          Serial.println("ACK LOG");
          device.cleanEvents();
      }
    }
  }
    
}

static inline bool hasFixSimplePersonal(PersonalDevice& personal) {
  return personal.hasLocation() && personal.getSatValid() && personal.getHdop() <= 10.0;
}

void mainFunctions::SendPacketSafety(DeviceBase& device, SimpleTimer& st_safety, unsigned long& jitterTargetTime) {
    
    unsigned long now = millis();

      if (hasFixSimplePersonal((PersonalDevice&) device)|| true) {
      
      // Executa o envio baseado no timer que disparou
      if(device.isChannelBusy(SAFETY_CHANNEL)) {
          Serial.println("Canal de segurança ocupado no momento do envio. Reagendando...");
          jitterTargetTime = now + random(100, 1000); 
          return;
      }
      device.sendSafety();
      st_safety.reset();
      Serial.println(">>> Enviado: SAFETY");
        
      // Reseta o jitterTargetTime para o presente, já que o envio foi concluído
      jitterTargetTime = 0;
    }
}

void mainFunctions::SendPacketLog(DeviceBase& device, SimpleTimer& st_monitoring, unsigned long& jitterTargetTime, bool ackMonitoring, bool ackLog){

  unsigned long now = millis();
  if(device.isChannelBusy(MONITORING_CHANNEL)) {
      Serial.println("Canal ocupado. Reagendando...");
      jitterTargetTime = now + random(100, 1000); 
      return;
  }

  if (!ackMonitoring && ackLog){
    device.sendMonitoring();
    Serial.println(">>> Transmitindo: MONITORING");
  }

  else if(ackMonitoring && !ackLog){
    device.sendLog();
    Serial.println(">>> Transmitindo: LOG");
  }
}

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
      //Serial.print("Minimum distance from vehicle: ");
      //Serial.print(minDistance);
      //Serial.println(" meters");
      
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
    //Serial.println("Level: " + String(lastLevel));
  }
}

void mainFunctions::ProcessData(PersonalDevice& personal, uint8_t id, double srcLat, double srcLng) {
    double dist = personal.calculateDistance(srcLat, srcLng);
    /* Serial.println();
    Serial.println("Distance calculada: " + String(dist) + " meters");
    Serial.println(); */
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