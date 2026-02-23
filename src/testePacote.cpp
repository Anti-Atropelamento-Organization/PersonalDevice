#include "packet.h"
#include "cmslora.h"


CMSLoRa lora;
packet pkt;
uint8_t logPL[LOG_PACKET_SIZE];
uint8_t receivedPL[LOG_PACKET_SIZE];
uint8_t MonitoringPL[MONITORING_PACKET_SIZE];
uint8_t receivedMpl[MONITORING_PACKET_SIZE];

int32_t last5positions[5][2] = {
    {12345678, 87654321},
    {22345678, 77654321},
    {32345678, 67654321},
    {42345678, 57654321},
    {52345678, 47654321}
};
uint8_t last5events[5] = {1, 2, 3, 4, 5};
ActiveVehicles nearbyVehicles[MAX_VEHICLES] = {
    {10, 1.5, 0},
    {20, 2.5, 0},
    {30, 3.5, 0},
    {40, 4.5, 0},
    {50, 5.5, 0}
};

void setup()
{
    Serial.begin(115200);
    lora.begin();
    lora.SpreadingFactor(9);
    pkt.logPacket(12, PERSONAL_DEVICE, last5positions, last5events, nearbyVehicles, logPL);

    //  Serial.println("Log Packet Criado:");
    // uint8_t decodedID = pkt.decodePacket(logPL, VEHICLE_DEVICE);

    // Serial.print("Decoded ID: "); Serial.println(decodedID);
    // Serial.println("Last 5 Positions:");
    // int32_t positions[5][2];
    // pkt.getLast5Positions(positions);
    // for (int i = 0; i < 5; i++) {
    //     Serial.print("Position "); Serial.print(i); Serial.print(": Lat = ");
    //     Serial.print((float)positions[i][0] / 1000000.0, 6); Serial.print(", Lng = ");
    //     Serial.println((float)positions[i][1] / 1000000.0, 6);
    // }
    // Serial.println("Last 5 Events:");
    // uint8_t events[5];
    // pkt.getLast5Events(events);
    // for (int i = 0; i < 5; i++) {
    //     Serial.print("Event "); Serial.print(i); Serial.print(": ");
    //     Serial.println(events[i]);
    // }
    // Serial.println("Nearby Vehicles:");
    // ActiveVehicles vehicles[MAX_VEHICLES];
    // pkt.getNearbyVehicles(vehicles);
    // for (int i = 0; i < MAX_VEHICLES; i++) {
    //     Serial.print("Vehicle ID: "); Serial.print(vehicles[i].id);
    //     Serial.print(", Distance: "); Serial.print(vehicles[i].distance, 2);
    //     Serial.print(", Last Seen: "); Serial.println(vehicles[i].lastSeenMs);
    // }

    delay(2000);

    Serial.println("##################################################################");

    pkt.monitoringPacket(34, PERSONAL_DEVICE, -23.55052, -46.633308, 85, 1, 7, 0.9, MonitoringPL);
    // Serial.println("Monitoring Packet Criado:");
    // pkt.decodePacket(MonitoringPL, VEHICLE_DEVICE);
    // Serial.print("Decoded ID: "); Serial.println(pkt.getDeviceID());
    // Serial.print("Decoded Latitude: "); Serial.println(pkt.getLat(), 6);
    // Serial.print("Decoded Longitude: "); Serial.println(pkt.getLng(), 6);
    // Serial.print("Decoded Hdop: "); Serial.println(pkt.getHdop(), 2);
    // Serial.print("Decoded Battery Level: "); Serial.println(pkt.getBatteryLevel());
    // Serial.print("Decoded Status: "); Serial.println(pkt.getStatus());
    // Serial.print("Decoded Satellites: "); Serial.println(pkt.getSatellites());


}
void loop()
{
    lora.sendData(logPL, LOG_PACKET_SIZE);
    delay(5000);
    lora.sendData(MonitoringPL, MONITORING_PACKET_SIZE);
    delay(5000);
}