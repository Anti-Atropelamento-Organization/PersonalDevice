#include "cmslora.h"
// #include "lorapacket.h"
// LoRaPacket pack;
// ReceivedData rd;
volatile bool CMSLoRa::sendDone = false;
volatile bool CMSLoRa::receiveDone = false;
//Callback chama automaticamente quando o envio é concluido
void IRAM_ATTR CMSLoRa::onSendDone(void) {
  sendDone = true;
}
//Callback chama automaticamente quando o recebimento é concluido
void IRAM_ATTR CMSLoRa::onReceiveDone(void) {
  receiveDone = true;
}
// coloca o rádio em modo StandBy, interrompendo o recebimento
void CMSLoRa::StopReceive()
{
  radio.standby();
}

void CMSLoRa::SpreadingFactor(int SF)
{
  radio.setSpreadingFactor(SF);
}

float CMSLoRa::getSignal(){
  return radio.getRSSI();
}

float CMSLoRa::getDataRate()
{
  return radio.getDataRate();
}
// retorna o tempo que o rádio ficará ocupado enviando o pacote 
unsigned long CMSLoRa::timeOnAir(int size)
{
return radio.getTimeOnAir(size);
}
void CMSLoRa::setBandwidth(float bandwidth)
{
  radio.setBandwidth(bandwidth);
  
}

CMSLoRa::CMSLoRa()
  : module(new Module(LORA_CS, LORA_DIO1, LORA_RESET, LORA_BUSY, spiLoRa)),
    radio(module){}


void CMSLoRa::begin() {
  spiLoRa.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  
  int state = radio.begin(915.0, 125, 7);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("[SX1262] Inicializado com sucesso!");
  } else {
    Serial.print(F("[SX1262] Erro ao inicializar: "));
    Serial.println(state);
  }

  radio.setCRC(true);
  radio.setSyncWord(0x12); 
  radio.setPacketReceivedAction(onReceiveDone);
  
  radio.setWhitening(false);
}

// Em cmslora.cpp

void CMSLoRa::sendData(unsigned char* data, size_t size) {
  radio.standby(); 
  sendDone = false;

  int transmissionState = radio.transmit(data, size);

  if(transmissionState != RADIOLIB_ERR_NONE) {
     Serial.print(F("[cmsLoRa] Erro envio: "));
     Serial.println(transmissionState);
  }

  receiveDone = false; 
  radio.startReceive(); 
}

bool CMSLoRa::receiveData(unsigned char* packetBuffer, size_t bufferSize, unsigned long timeOut){
    
    if (receiveDone) {
        receiveDone = false;

        size_t len = radio.getPacketLength();
        
      
        if (len == 0) {
             radio.startReceive(); 
             return false;
        }

        if (len > bufferSize) len = bufferSize; 

        int state = radio.readData(packetBuffer, len);

        
        radio.startReceive(); 

        if(state == RADIOLIB_ERR_NONE){
            return true;
        } else {
            return false;
        }
    }
    
    return false;
}

bool CMSLoRa::isChannelBusy()
{
  int16_t returnValue = radio.scanChannel();
  if(returnValue == RADIOLIB_LORA_DETECTED)
  {
    return true;
  }
  if(returnValue == RADIOLIB_PREAMBLE_DETECTED)
  {
    return true;
  }
  else if(returnValue == RADIOLIB_CHANNEL_FREE)
  {
    return false;
  }
  return true;
}