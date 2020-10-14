/**
 * A BLE client example to connect to Nuki smartlock 2.0
 * author Jeroen
 */

#include "BLEDevice.h"
#include "esp_log.h"
#include "Arduino.h"

//test BLE address, replace with your own
std::string myNukiAddr="54:d2:72:4F:98:48";

void setup(){
    Serial.begin(115200);
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("");


}

void loop(){

}