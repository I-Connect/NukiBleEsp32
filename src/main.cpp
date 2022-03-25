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
BatteryReport _batteryReport;
std::list<LogEntry> requestedLogEntries;
std::list<KeypadEntry> requestedKeypadEntries;

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

void batteryReport() {
  uint8_t result = nukiBle.requestBatteryReport(&_batteryReport);
  if (result == 1) {
    log_d("Bat report voltage: %d Crit state: %d, start temp: %d", _batteryReport.batteryVoltage, _batteryReport.criticalBatteryState, _batteryReport.startTemperature);
  } else {
    log_d("Bat report failed: %d", result);
  }
}

void keyTurnerState() {
  uint8_t result = nukiBle.requestKeyTurnerState(&retreivedKeyTurnerState);
  if ( result == 1) {
    log_d("Bat state: %d, lock state: %d %d:%d:%d",
          retreivedKeyTurnerState.criticalBatteryState, retreivedKeyTurnerState.lockState, retreivedKeyTurnerState.currentTimeHour,
          retreivedKeyTurnerState.currentTimeMinute, retreivedKeyTurnerState.currentTimeSecond);
  } else {
    log_d("cmd failed: %d", result);
  }

}

void requestLogEntries() {
  uint8_t result = nukiBle.retreiveLogEntries(0, 10, 0, true);
  if ( result == 1) {
    delay(5000);
    nukiBle.getLogEntries(&requestedLogEntries);
    std::list<LogEntry>::iterator it = requestedLogEntries.begin();
    while (it != requestedLogEntries.end()) {
      log_d("Log[%d] %d-%d-%d %d:%d:%d", it->index, it->timeStampYear, it->timeStampMonth, it->timeStampDay, it->timeStampHour, it->timeStampMinute, it->timeStampSecond);
      it++;
    }
  } else {
    log_d("get log failed: %d", result);
  }
}

void requestKeyPadEntries() {
  uint8_t result = nukiBle.retreiveKeypadEntries(0, 10);
  if ( result == 1) {
    delay(5000);
    nukiBle.getKeypadEntries(&requestedKeypadEntries);
    std::list<KeypadEntry>::iterator it = requestedKeypadEntries.begin();
    while (it != requestedKeypadEntries.end()) {
      log_d("Keypad entry[%d] %d", it->codeId, it->code);
      it++;
    }
  } else {
    log_d("get keypadentries failed: %d", result);
  }
}

void setPincode(uint16_t pincode) {
  uint8_t result = nukiBle.setSecurityPin(pincode);
  if ( result == 1) {
    log_d("Set pincode done");

  } else {
    log_d("Set pincode failed: %d", result);
  }
}

void setup() {
  Serial.begin(115200);
  log_d("Starting NUKI BLE...");
  nukiBle.initialize();

  // nukiBle.savePincode(9999);
  // nukiBle.unPairNuki();
}

void loop() {
  if (!paired) {
    if (nukiBle.pairNuki()) {
      log_d("paired");
      paired = true;

      // setPincode(9999);
      // nukiBle.requestConfig(false);
      // nukiBle.requestConfig(true);


      // nukiBle.requestKeyPadCodes(0, 2);


      //execute action
      // nukiBle.lockAction(LockAction::lock, 0, 0);
      // addKeypadEntry();
    }
  }

  keyTurnerState();
  // batteryReport();
  // requestLogEntries();
  // requestKeyPadEntries();


  delay(20000);
}