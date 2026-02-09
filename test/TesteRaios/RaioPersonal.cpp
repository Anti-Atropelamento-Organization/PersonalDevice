#include "PersonalDevice.h"
PersonalDevice device;
void setup() {
    Serial.begin(9600);
    device.setup();
    device.setID(1);
    device.setLatitude();
    device.setLongitude();
}
void loop() {
    device.setHdop();
    device.setRadius(device.getHdop());
    Serial.println("Radius[0]: " + String(device.getRadius(0)));
    Serial.println("Radius[1]: " + String(device.getRadius(1)));
    Serial.println("Radius[2]: " + String(device.getRadius(2)));
    delay(5000);

    
}