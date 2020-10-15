/**
 * A BLE client example to connect to Nuki smartlock 2.0
 * author Jeroen
 */

#include "BLEDevice.h"
#include "esp_log.h"
#include "Arduino.h"
#include "nukiBle.h"

//test BLE address, replace with your own
std::string myNukiAddr="54:d2:72:4F:98:48";
NukiBle nukiBle(myNukiAddr);

void setup(){
    Serial.begin(115200);
    log_d("Starting Arduino BLE Client application...");
    BLEDevice::init("");
}

void loop(){
    if(!nukiBle.isPaired){
        nukiBle.pairBle();
    }
    else{
        log_d("Attempting to pair failed");
    }
    delay(1000);

}