/**
 * A BLE client example to connect to Nuki smartlock 2.0
 * author Jeroen
 */

#include "BLEDevice.h"
#include "esp_log.h"
#include "Arduino.h"
#include "nukiBle.h"

//test BLE address, replace with your own
std::string myNukiAddr = "54:d2:72:4F:98:84";
uint32_t deviceId = 2020001;
uint8_t deviceName[] = "C-Sense";
NukiBle nukiBle(myNukiAddr, deviceId, deviceName);

void setup() {
  Serial.begin(115200);
  log_d("Starting Arduino BLE Client application...");
  nukiBle.initialize();
  nukiBle.connect();

  uint16_t payload = (uint16_t)nukiCommand::keyturnerStates;

  nukiBle.sendEncryptedMessage(nukiCommand::requestData, (char*)&payload, 2);

}

void loop() {


  delay(5000);

}