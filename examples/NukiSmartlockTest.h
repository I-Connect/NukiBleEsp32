/**
 * A BLE client example to connect to Nuki smartlock 2.0
 * author Jeroen
 */

#include "Arduino.h"
#include "NukiLock.h"
#include "NukiConstants.h"
#include "BleScanner.h"

uint32_t deviceId = 2020001;
std::string deviceName = "frontDoor";
NukiLock::NukiLock nukiLock(deviceName, deviceId);
BleScanner::Scanner scanner;

NukiLock::KeyTurnerState retrievedKeyTurnerState;
NukiLock::BatteryReport _batteryReport;
std::list<Nuki::LogEntry> requestedLogEntries;
std::list<Nuki::KeypadEntry> requestedKeypadEntries;
std::list<Nuki::AuthorizationEntry> requestedAuthorizationEntries;
std::list<NukiLock::TimeControlEntry> requestedTimeControlEntries;

void addKeypadEntry() {
  Nuki::NewKeypadEntry newKeypadEntry;
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
  newKeypadEntry.allowedUntilYear = 2023;
  newKeypadEntry.allowedUntilMonth = 1;
  newKeypadEntry.allowedUntilDay = 1;
  newKeypadEntry.allowedUntilHour = 0;
  newKeypadEntry.allowedUntilMin = 0;
  newKeypadEntry.allowedUntilSec = 0;
  newKeypadEntry.allowedFromTimeHour = 0;
  newKeypadEntry.allowedFromTimeMin = 0;
  newKeypadEntry.allowedUntilTimeHour = 23;
  newKeypadEntry.allowedUntilTimeMin = 59;

  nukiLock.addKeypadEntry(newKeypadEntry);
}

void batteryReport() {
  uint8_t result = nukiLock.requestBatteryReport(&_batteryReport);
  if (result == 1) {
    log_d("Bat report voltage: %d Crit state: %d, start temp: %d", _batteryReport.batteryVoltage, _batteryReport.criticalBatteryState, _batteryReport.startTemperature);
  } else {
    log_d("Bat report failed: %d", result);
  }
}

bool keyTurnerState() {
  uint8_t result = nukiLock.requestKeyTurnerState(&retrievedKeyTurnerState);
  if (result == 1) {
    log_d("Bat crit: %d, Bat perc:%d lock state: %d %d:%d:%d",
          nukiLock.isBatteryCritical(), nukiLock.getBatteryPerc(), retrievedKeyTurnerState.lockState, retrievedKeyTurnerState.currentTimeHour,
          retrievedKeyTurnerState.currentTimeMinute, retrievedKeyTurnerState.currentTimeSecond);
  } else {
    log_d("cmd failed: %d", result);
  }
  return result;
}

void requestLogEntries() {
  uint8_t result = nukiLock.retrieveLogEntries(0, 10, 0, true);
  if (result == 1) {
    delay(5000);
    nukiLock.getLogEntries(&requestedLogEntries);
    std::list<Nuki::LogEntry>::iterator it = requestedLogEntries.begin();
    while (it != requestedLogEntries.end()) {
      log_d("Log[%d] %d-%d-%d %d:%d:%d", it->index, it->timeStampYear, it->timeStampMonth, it->timeStampDay, it->timeStampHour, it->timeStampMinute, it->timeStampSecond);
      it++;
    }
  } else {
    log_d("get log failed: %d", result);
  }
}

void requestKeyPadEntries() {
  uint8_t result = nukiLock.retrieveKeypadEntries(0, 10);
  if (result == 1) {
    delay(5000);
    nukiLock.getKeypadEntries(&requestedKeypadEntries);
    std::list<Nuki::KeypadEntry>::iterator it = requestedKeypadEntries.begin();
    while (it != requestedKeypadEntries.end()) {
      log_d("Keypad entry[%d] %d", it->codeId, it->code);
      it++;
    }
  } else {
    log_d("get keypadentries failed: %d", result);
  }
}

void requestAuthorizationEntries() {
  uint8_t result = nukiLock.retrieveAuthorizationEntries(0, 10);
  if (result == 1) {
    delay(5000);
    nukiLock.getAuthorizationEntries(&requestedAuthorizationEntries);
    std::list<Nuki::AuthorizationEntry>::iterator it = requestedAuthorizationEntries.begin();
    while (it != requestedAuthorizationEntries.end()) {
      log_d("Authorization entry[%d] type: %d name: %s", it->authId, it->idType, it->name);
      it++;
    }
  } else {
    log_d("get authorization entries failed: %d", result);
  }
}

void setPincode(uint16_t pincode) {
  uint8_t result = nukiLock.setSecurityPin(pincode);
  if (result == 1) {
    log_d("Set pincode done");

  } else {
    log_d("Set pincode failed: %d", result);
  }
}

void addTimeControl(uint8_t weekdays, uint8_t hour, uint8_t minute, NukiLock::LockAction lockAction) {
  NukiLock::NewTimeControlEntry newEntry;
  newEntry.weekdays = weekdays;
  newEntry.timeHour = hour;
  newEntry.timeMin = minute;
  newEntry.lockAction = lockAction;

  nukiLock.addTimeControlEntry(newEntry);
}

void requestTimeControlEntries() {
  Nuki::CmdResult result = nukiLock.retrieveTimeControlEntries();
  if (result == Nuki::CmdResult::Success) {
    delay(5000);
    nukiLock.getTimeControlEntries(&requestedTimeControlEntries);
    std::list<NukiLock::TimeControlEntry>::iterator it = requestedTimeControlEntries.begin();
    while (it != requestedTimeControlEntries.end()) {
      log_d("TimeEntry[%d] weekdays:%d %d:%d enabled: %d lock action: %d", it->entryId, it->weekdays, it->timeHour, it->timeMin, it->enabled, it->lockAction);
      it++;
    }
  } else {
    log_d("get log failed: %d, error %d", result, nukiLock.getLastError());
  }
}

void getConfig() {
  NukiLock::Config config;
  if (nukiLock.requestConfig(&config) == 1) {
    log_d("Name: %s", config.name);
  } else {
    log_w("getConfig failed");
  }

}

bool notified = false;
class Handler: public Nuki::SmartlockEventHandler {
  public:
    virtual ~Handler() {};
    void notify(Nuki::EventType eventType) {
      notified = true;
    }
};

Handler handler;

void setup() {
  Serial.begin(115200);
  log_d("Starting NUKI BLE...");
  scanner.initialize();
  nukiLock.registerBleScanner(&scanner);
  nukiLock.initialize();

  if (nukiLock.isPairedWithLock()) {
    log_d("paired");
    nukiLock.setEventHandler(&handler);
    getConfig();
    nukiLock.enableLedFlash(false);
  }

  // nukiLock.savePincode(9999);
  // nukiLock.unPairNuki();
}

void loop() {
  scanner.update();
  if (!nukiLock.isPairedWithLock()) {
    if (nukiLock.pairNuki() == Nuki::PairingResult::Success) {
      log_d("paired");
      nukiLock.setEventHandler(&handler);
      getConfig();
    }
  }

  if (notified) {
    if (keyTurnerState()) {
      notified = false;
    }
  }
  delay(500);
}