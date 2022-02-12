/**
 * A BLE client example to connect to Nuki smartlock 2.0
 * author Jeroen
 */

#include "BLEDevice.h"
#include "esp_log.h"
#include "Arduino.h"
#include "nukiBle.h"

//test BLE address, 0x replace with your own
std::string myNukiAddr = "54:d2:72:4F:98:84";
uint32_t deviceId = 2020001;
uint8_t deviceName[] = "C-Sense";
NukiBle nukiBle(myNukiAddr, deviceId, deviceName);


//values from example pdf for encryption test
// byte keyS[] =   {0x21, 0x7F, 0xCB, 0x0F, 0x18, 0xCA, 0xF2, 0x84, 0xE9, 0xBD, 0xEA, 0x0B, 0x94, 0xB8, 0x3B, 0x8D, 0x10, 0x86, 0x7E, 0xD7, 0x06, 0xBF, 0xDE, 0xDB, 0xD2, 0x38, 0x1F, 0x4C, 0xB3, 0xB8, 0xF7, 0x30};
// byte nonce[] = {0x37, 0x91, 0x7F, 0x1A, 0xF3, 0x1E, 0xC5, 0x94, 0x07, 0x05, 0xF3, 0x4D, 0x1E, 0x55, 0x50, 0x60, 0x7D, 0x5B, 0x2F, 0x9F, 0xE7, 0xD4, 0x96, 0xB6};
// byte input[] = {0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0C, 0x00, 0x41, 0x8D};

byte output[26] = {0};

void setup() {
  Serial.begin(115200);
  log_d("Starting Arduino BLE Client application...");

  //start encryption test
  // NukiBle::encode(input, output, sizeof(input), nonce, keyS);
  // // log_d("67 0D 12 49 26 00 43 66 53 2E 8D 92 7A 33 FE 84 E7 82 A9 59 4D 39 15 7D 06 5E");
  // printBuffer(output, sizeof(output), false, "output:                 ");
  //end encryption test

  nukiBle.initialize();

  // uint16_t payload = (uint16_t)nukiCommand::keyturnerStates;
  // nukiBle.sendEncryptedMessage(nukiCommand::requestData, (char*)&payload, 2);

  nukiBle.updateKeyTurnerState();
}

void loop() {


  delay(5000);

}