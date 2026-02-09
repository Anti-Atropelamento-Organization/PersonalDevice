#include "packet.h"
packet pckt;
uint8_t safetyPacket[13];
void setup()
{
    Serial.begin(9600);
    pckt.safetyPacket(1, VEHICLE_DEVICE, 37.7749, -122.4194, safetyPacket, 40.0, 30.0);
    pckt.decodePacket(safetyPacket);
    Serial.print("Packet ID: ");
    if(pckt.getPacketID() == SAFETY_PACKET) {
        Serial.println("Safety Packet");
    }
    else if(pckt.getPacketID() == MONITORING_PACKET) {
        Serial.println("Monitoring Packet");
    }
    else if(pckt.getPacketID() == ADVERTISE_PACKET) {
        Serial.println("Advertise Packet");
    }
    Serial.print("Device Type: ");
    pckt.getDeviceType() == PERSONAL_DEVICE ? Serial.println("Personal Device") : Serial.println("Vehicle Device");
    Serial.print("Latitude: ");
    Serial.println(pckt.getLat() / 1000000.0, 6);
    Serial.print("Longitude: ");
    Serial.println(pckt.getLng() / 1000000.0, 6);
    Serial.print("Speed: ");
    Serial.println(pckt.getSpeed());
    Serial.print("Course: ");
    Serial.println(pckt.getCourse());
    

}
void loop()
{
}