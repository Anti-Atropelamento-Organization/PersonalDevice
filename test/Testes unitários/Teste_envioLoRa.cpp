#include "packet.h"
#include "cmslora.h"
uint8_t safetyPacket[SAFETY_PACKET_SIZE];
uint8_t monitoringPacket[MONITORING_PACKET_SIZE];

packet pckt;
CMSLoRa lora;

void setup()
{
    Serial.begin(9600);
    lora.begin();
    pckt.safetyPacket(1, PERSONAL_DEVICE, 37.7749, -122.4194, safetyPacket);
}
void loop()
{
    Serial.println("Sending Safety Packet...");
    lora.sendData(safetyPacket, SAFETY_PACKET_SIZE);
    delay(10000);
}