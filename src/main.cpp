/**
 * A BLE client example to connect to Nuki smartlock 2.0
 * author Jeroen
 */

#include "Arduino.h"
#include "nukiBle.h"

uint32_t deviceId = 2020001;
std::string deviceName = "frontDoor";
NukiBle nukiBle(deviceName, deviceId);

bool paired = false;

void setup() {
  Serial.begin(115200);
  log_d("Starting NUKI BLE...");
  nukiBle.initialize();

  // nukiBle.unPairNuki();
}

void loop() {
  if (!paired) {
    if (nukiBle.pairNuki()) {
      log_d("paired");
      paired = true;
    }
  }

  if (paired) {
    //Get data
    // nukiBle.requestOpeningsClosingsSummary(); FAILS
    // nukiBle.requestAuthorizationEntryCount(); FAILS
    nukiBle.updateKeyTurnerState();
    nukiBle.requestConfig(false);
    nukiBle.requestConfig(true);
    nukiBle.requestBatteryReport();

    //execute action
    // nukiBle.lockAction(LockAction::lock, 0, 0);
  }

  delay(10000);
}