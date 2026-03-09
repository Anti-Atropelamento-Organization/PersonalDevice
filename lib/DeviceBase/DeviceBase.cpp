#include "DeviceBase.h"

// Construtor inicializa os arrays de histórico de posições e eventos com zero
DeviceBase::DeviceBase() {
    for (int i = 0; i < 5; i++) {
        last5positions[i][0] = 0;
        last5positions[i][1] = 0;
        last5events[i] = 0;
    }
}

// Configuração inicial do rádio LoRa e parâmetros de transmissão
void DeviceBase::setup() {
    lora.begin();
    lora.SpreadingFactor(9); // Define o speed factor para 9 de início
}

// Usa funções da biblioteca TinyGPS++ para alimentar o sistema com dados seriais vindos do módulo GPS
void DeviceBase::alimentandoGPS() {
    while (SerialGPS.available() > 0) {
        gps.encode(SerialGPS.read());
    }
}

// Getters e Setters para identificação do dispositivo
uint8_t DeviceBase::getID() const { return deviceID; }
void DeviceBase::setID(uint8_t id) { deviceID = id; }

// Getters e Setters para adquirir latitude do dispositivo
double DeviceBase::getLatitude() const { return deviceLatitude; }
void DeviceBase::setLatitude() { deviceLatitude = gps.location.lat(); }

// Método que força um valor da latitude do dispositivo
void DeviceBase::forceLatitude(double lat) { deviceLatitude = lat; }

// Getters e Setters para adquirir longitude do dispositivo
double DeviceBase::getLongitude() const { return deviceLongitude; }
void DeviceBase::setLongitude() { deviceLongitude = gps.location.lng(); }

// Método que força um valor da longitude do dispositivo
void DeviceBase::forceLongitude(double lng) { deviceLongitude = lng; }

// Obtém velocidade (em km/h) do GPS
double DeviceBase::getSpeed() const { return speed; }
void DeviceBase::setSpeed() { speed = gps.speed.kmph(); }

// Obtém velocidade curso/direção (em graus) do GPS
double DeviceBase::getCourse() const { return deviceCourse; }
void DeviceBase::setCourse() { deviceCourse = gps.course.deg(); }

// Retorna a quantidade de satélites que o dispostivio está referênciando
int DeviceBase::getSatValue() {
    return satelites;
}

// Adquire a quantidade de satélites que o dispositivo está referenciando
void DeviceBase::setSatValue() {
    satelites = gps.satellites.value();
}

// Retorna se há satélites para referenciar
bool DeviceBase::getSatValid() {
    return gps.satellites.isValid();
}

// Get e set do Hdop
double DeviceBase::getHdop() const { return deviceHdop; }
void DeviceBase::setHdop() { deviceHdop = gps.hdop.hdop(); }

// Envia pacote de segurança via LoRa 
void DeviceBase::sendSafety() {
    // Se for veículo (tipo 1), envia velocidade e curso; senão, envia 0.0
    double currentSpeed = (deviceType == 1) ? speed : 0.0;
    double currentCourse = (deviceType == 1) ? deviceCourse : 0.0;

    // Constrói o pacote a ser enviado passando as referências
    pckt.safetyPacket(deviceID, deviceType, deviceLatitude, deviceLongitude, safetyPacket, currentSpeed, currentCourse, deviceHdop);

    // Função que envia definitamente o pacote
    lora.sendData(safetyPacket, SAFETY_PACKET_SIZE);
}

// Envia pacote de monitoramento 
void DeviceBase::sendMonitoring() {
    lora.sendData(monitoringPacket, MONITORING_PACKET_SIZE);
}

// Envia log de dados históricos
void DeviceBase::sendLog() {
    lora.sendData(logPacket, LOG_PACKET_SIZE);
}

// Tenta receber um pacote LoRa e decodifica seu conteúdo
bool DeviceBase::receive() {
    if (lora.receiveData(receivedPacket, 255, 100)) { 
        lastPacketID = pckt.decodePacket(receivedPacket, this->deviceType);
        
        if (lastPacketID == 0) {
            return false;
        }
        return true;
    }
    return false;
}

// Verifica se o canal de rádio está livre e prepara o pacote correspondente e o sf antes de enviar
bool DeviceBase::isChannelBusy(int channel) {
    if (channel == SAFETY_CHANNEL) {
        lora.SpreadingFactor(safetySF());
        buildSafetyPacket();
    } else if (channel == MONITORING_CHANNEL) {
        lora.SpreadingFactor(monitoringSF());
        buildMonitoringPacket();
        buildLogPacket();
    }
    return lora.isChannelBusy();
}

// Atualiza configurações do dispositivo via comandos recebidos por Bluetooth
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

// Calcula a distância para outro dispositivo, subtraindo o raio de proteção do dispositivo
float DeviceBase::calculateDistance(double targetLat, double targetLng) {
    double distance = gps.distanceBetween(deviceLatitude, deviceLongitude, targetLat, targetLng);
    if (distance - getRadius(0) < 0) {
        return 0.0;
    }
    return (distance - getRadius(0));
}

// Envia um alerta específico para um ID de destino
void DeviceBase::sendAlert(uint8_t alertType, uint8_t targetID) {
    uint8_t alertPacket[ADVERTISE_PACKET_SIZE];
    pckt.advertisePacket(alertType, targetID, alertPacket);
    lora.sendData(alertPacket, sizeof(alertPacket));
}

// Retorna o raio de segurança solicitado (0, 1 ou 2)
double DeviceBase::getRadius(int index) const {
    if (index >=0 && index < 3) {
        return deviceRadius[index];
    }
    return 0.0;
}

// Define os 3 níveis de raios de segurança baseados no HDOP e tipo de dispositivo
void DeviceBase::setRadius(double hdop) {
    if (deviceType == 1) { // Lógica para Veículos
        for (int i = 0; i < 3; i++) {
            if (i == 0)      deviceRadius[i] = hdop * 1.5;
            else if (i == 1) deviceRadius[i] = (hdop * 1.5) + 9.0;
            else if (i == 2) deviceRadius[i] = (hdop * 1.5) + 25.0;
        }
    } else if (deviceType == 2) { // lógica para pessoas
        for (int i = 0; i < 3; i++) {
            if (i == 0)      deviceRadius[i] = hdop * 1.5;
            else if (i == 1) deviceRadius[i] = (hdop * 1.5) + 10.0;
            else if (i == 2) deviceRadius[i] = (hdop * 1.5) + 50.0;
        }
    }
}

// Gerenciamento de nível de bateria
void DeviceBase::setBatteryLevel(uint8_t level) {
    batteryLevel = PMU->getBatteryPercent(); // Obtém via Power Management Unit
} 
uint8_t DeviceBase::getBatteryLevel() const {
    return batteryLevel;
}

// Verifica se o GPS possui uma localização válida
bool DeviceBase::hasLocation() {
    return gps.location.isValid();
}

// Getters para recuperar dados do último pacote recebido e decodificado
double DeviceBase::getReceivedLat() { return pckt.getLat(); }
double DeviceBase::getReceivedLng() { return pckt.getLng(); }
uint8_t DeviceBase::getReceivedID() { return pckt.getDeviceID(); }
uint8_t DeviceBase::getTypePacket() { return lastPacketID; }
uint16_t DeviceBase::getRandomPacketID() { return pckt.getAckRandomID(); }
uint16_t DeviceBase::getMyRandomLogID() { return this->LogRandomID; }
uint16_t DeviceBase::getMyRandomMonitoringID() { return this->monitoringRandomID; }

// Limpa a lista de eventos armazenados
void DeviceBase::cleanEvents() {
    Serial.println("[DeviceBase] antes: ");
    for(int i = 0; i < 5; i++) Serial.print(String(last5events[i]) + " ");
    
    memset(last5events, 0, sizeof(last5events));
    
    Serial.println("\n[DeviceBase] depois: ");
    for(int i = 0; i < 5; i++) Serial.print(String(last5events[i]) + " ");
    EventIndex = 0;
}

// Adiciona um evento à lista de eventos
void DeviceBase::addEvent(uint8_t event) {
    Serial.println("Evento adicionado: " + String(event));
    if(EventIndex < 5) {
        last5events[EventIndex] = event;
        EventIndex++;
    } else {
        Serial.println("Lista de eventos cheia");
    }
}

// Detecta se a bateria está abaixo do limite e verifica se a ultima bateria era maior que 20 para enviar true apenas nesse degrau 
bool DeviceBase::monitoringBatteryEvent(uint8_t Level) {
    uint8_t lastBatteryLevel = batteryLevel;
    setBatteryLevel(Level);
    return (lastBatteryLevel > 20 && batteryLevel <= 20);
}

// Detecta perda de precisão do sinal GPS e se hdop antes era maior que 5 para não enviar toda hora true
bool DeviceBase::monitoringHdopEvent() {
    double lastHdop = deviceHdop;
    setHdop();
    if(deviceHdop >= 5.0 && lastHdop < 5.0) {
        return true;
    }
    return false;
}

// Detecta perda de sinal de satélites 
bool DeviceBase::monitoringSatEvent() {
    uint8_t lastSat = satelites;
    setSatValue();
    return (lastSat > 6 && satelites <= 6);
}

// Detecta mudança no status de validade da localização GPS
bool DeviceBase::monitoringGPSEvent() {
    if(lastLocationIsValid == true) {
        if(gps.location.isValid() == false) {
            lastLocationIsValid = false;
            return true;
        }
    } else {
        if(gps.location.isValid() == true) {
            lastLocationIsValid = true;
        }
    }
    return false;
}

// Encontra a menor distância registrada entre todos os veículos próximos
double DeviceBase::minDistanceFromVehicle() {
    double minDistanceVehicle = 1000000.0;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (nearbyVehicles[i].id != 0 && nearbyVehicles[i].distance < minDistanceVehicle && nearbyVehicles[i].distance > 0.0) {
            minDistanceVehicle = nearbyVehicles[i].distance;
        }
    }
    return minDistanceVehicle;
};

// Determina o nível de alerta baseado na distância do objeto mais próximo
uint8_t DeviceBase::calculateAlertDistance(){
    double minDistance = minDistanceFromVehicle();
    if(minDistance != 0){
        if (minDistance <= getRadius(1)) return 1;      // Perigo imediato
        else if (minDistance <= getRadius(2)) return 2; // Atenção
        else if (minDistance <= 70.0) return 3;         // Presença detectada
        else return 0;
    }
    return 0;
}

// Monitora se o nível de alerta de distância mudou para disparar um evento
uint8_t DeviceBase::monitoringDistanceEvent() {
    uint8_t actualDistance = calculateAlertDistance();

    if (actualDistance != lastDistanceAlert && actualDistance > 0) {
        lastDistanceAlert = actualDistance; 
        return actualDistance;            
    }

    if (actualDistance == 0) {
        lastDistanceAlert = 0;
    }

    return 0;
}