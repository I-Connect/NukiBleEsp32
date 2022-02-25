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

void setup() {
  Serial.begin(115200);
  log_d("Starting NUKI BLE...");
  nukiBle.initialize();

  // nukiBle.unPairNuki();
}

void addKeyPadEntry() {
  KeyPadEntry keyPadEntry;
  unsigned char nameBuff[20] = "test";

  keyPadEntry.code = 111111;
  memcpy(keyPadEntry.name, nameBuff, 20);
  keyPadEntry.timeLimited = 1;
  keyPadEntry.allowedFromYear = 2022;
  keyPadEntry.allowedFromMonth = 2;
  keyPadEntry.allowedFromDay = 1;
  keyPadEntry.allowedFromHour = 0;
  keyPadEntry.allowedFromMin = 0;
  keyPadEntry.allowedFromSec = 0;
  keyPadEntry.allowedUntillYear = 2023;
  keyPadEntry.allowedUntillMonth = 1;
  keyPadEntry.allowedUntillDay = 1;
  keyPadEntry.allowedUntillHour = 0;
  keyPadEntry.allowedUntillMin = 0;
  keyPadEntry.allowedUntillSec = 0;
  keyPadEntry.allowedFromTimeHour = 0;
  keyPadEntry.allowedFromTimeMin = 0;
  keyPadEntry.allowedUntillTimeHour = 23;
  keyPadEntry.allowedUntillTimeMin = 59;

  nukiBle.addKeypadEntry(keyPadEntry);
}

void loop() {
  if (!paired) {
    if (nukiBle.pairNuki()) {
      log_d("paired");
      paired = true;

      // nukiBle.requestOpeningsClosingsSummary(); FAILS
      // nukiBle.requestAuthorizationEntryCount(); FAILS
      nukiBle.updateKeyTurnerState();
      // nukiBle.requestConfig(false);
      // nukiBle.requestConfig(true);
      // nukiBle.requestBatteryReport();

      //execute action
      // nukiBle.lockAction(LockAction::lock, 0, 0);
      addKeyPadEntry();
    }
  }

  delay(10000);
}