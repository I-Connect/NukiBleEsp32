/**
 * A BLE client example to connect to Nuki smartlock 2.0
 * author Jeroen
 */

#include "Arduino.h"
#include "NukiBle.h"
#include "NukiConstants.h"

uint32_t deviceId = 2020001;
std::string deviceName = "frontDoor";
NukiBle nukiBle(deviceName, deviceId);

bool paired = false;

KeyTurnerState retreivedKeyTurnerState;

void setup() {
  Serial.begin(115200);
  log_d("Starting NUKI BLE...");
  nukiBle.initialize();

  // nukiBle.unPairNuki();
}

void addKeypadEntry() {
  NewKeypadEntry newKeypadEntry;
  unsigned char nameBuff[20] = "test";

  newKeypadEntry.code = 111111;
  memcpy(newKeypadEntry.name, nameBuff, 20);
  newKeypadEntry.timeLimited = 1;
  newKeypadEntry.allowedFromYear = 2022;
  newKeypadEntry.allowedFromMonth = 2;
  newKeypadEntry.allowedFromDay = 1;
  newKeypadEntry.allowedFromHour = 0;
  newKeypadEntry.allowedFromMin = 0;
  newKeypadEntry.allowedFromSec = 0;
  newKeypadEntry.allowedUntillYear = 2023;
  newKeypadEntry.allowedUntillMonth = 1;
  newKeypadEntry.allowedUntillDay = 1;
  newKeypadEntry.allowedUntillHour = 0;
  newKeypadEntry.allowedUntillMin = 0;
  newKeypadEntry.allowedUntillSec = 0;
  newKeypadEntry.allowedFromTimeHour = 0;
  newKeypadEntry.allowedFromTimeMin = 0;
  newKeypadEntry.allowedUntillTimeHour = 23;
  newKeypadEntry.allowedUntillTimeMin = 59;

  nukiBle.addKeypadEntry(newKeypadEntry);
}

void loop() {
  if (!paired) {
    if (nukiBle.pairNuki()) {
      log_d("paired");
      paired = true;

      // nukiBle.requestKeyTurnerState(&keyTurnerState);
      // nukiBle.requestConfig(false);
      // nukiBle.requestConfig(true);
      // nukiBle.requestBatteryReport();
      nukiBle.requestKeyPadCodes(0, 2);
      // nukiBle.requestLogEntries(0, 10, 0, true);

      //execute action
      // nukiBle.lockAction(LockAction::lock, 0, 0);
      // addKeypadEntry();
    }
  }

  uint8_t result = nukiBle.requestKeyTurnerState(&retreivedKeyTurnerState);
  if ( result == 1) {
    log_d("Bat state: %d, lock state: %d %d:%d:%d",
          retreivedKeyTurnerState.criticalBatteryState, retreivedKeyTurnerState.lockState, retreivedKeyTurnerState.currentTimeHour,
          retreivedKeyTurnerState.currentTimeMinute, retreivedKeyTurnerState.currentTimeSecond);
  } else {
    log_d("cmd failed: %d", result);
  }


  delay(20000);
}