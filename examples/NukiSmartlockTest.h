/**
 * A BLE client example to connect to Nuki smartlock 2.0
 * author Jeroen
 */

#include "Arduino.h"
#include "NukiBle.h"
#include "NukiConstants.h"

uint32_t deviceId = 2020001;
std::string deviceName = "frontDoor";
Nuki::NukiBle nukiBle(deviceName, deviceId);
BleScanner scanner;

Nuki::KeyTurnerState retrievedKeyTurnerState;
Nuki::BatteryReport _batteryReport;
std::list<Nuki::LogEntry> requestedLogEntries;
std::list<Nuki::KeypadEntry> requestedKeypadEntries;
std::list<Nuki::AuthorizationEntry> requestedAuthorizationEntries;
std::list<Nuki::TimeControlEntry> requestedTimeControlEntries;

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

bool keyTurnerState() {
  uint8_t result = nukiBle.requestKeyTurnerState(&retrievedKeyTurnerState);
  if (result == 1) {
    log_d("Bat crit: %d, Bat perc:%d lock state: %d %d:%d:%d",
          nukiBle.isBatteryCritical(), nukiBle.getBatteryPerc(), retrievedKeyTurnerState.lockState, retrievedKeyTurnerState.currentTimeHour,
          retrievedKeyTurnerState.currentTimeMinute, retrievedKeyTurnerState.currentTimeSecond);
  } else {
    log_d("cmd failed: %d", result);
  }
  return result;
}

void requestLogEntries() {
  uint8_t result = nukiBle.retrieveLogEntries(0, 10, 0, true);
  if (result == 1) {
    delay(5000);
    nukiBle.getLogEntries(&requestedLogEntries);
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
  uint8_t result = nukiBle.retrieveKeypadEntries(0, 10);
  if (result == 1) {
    delay(5000);
    nukiBle.getKeypadEntries(&requestedKeypadEntries);
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
  uint8_t result = nukiBle.retrieveAuthorizationEntries(0, 10);
  if (result == 1) {
    delay(5000);
    nukiBle.getAuthorizationEntries(&requestedAuthorizationEntries);
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
  uint8_t result = nukiBle.setSecurityPin(pincode);
  if (result == 1) {
    log_d("Set pincode done");

  } else {
    log_d("Set pincode failed: %d", result);
  }
}

void addTimeControl(uint8_t weekdays, uint8_t hour, uint8_t minute, Nuki::LockAction lockAction) {
  Nuki::NewTimeControlEntry newEntry;
  newEntry.weekdays = weekdays;
  newEntry.timeHour = hour;
  newEntry.timeMin = minute;
  newEntry.lockAction = lockAction;

  nukiBle.addTimeControlEntry(newEntry);
}

void requestTimeControlEntries() {
  Nuki::CmdResult result = nukiBle.retrieveTimeControlEntries();
  if (result == Nuki::CmdResult::Success) {
    delay(5000);
    nukiBle.getTimeControlEntries(&requestedTimeControlEntries);
    std::list<Nuki::TimeControlEntry>::iterator it = requestedTimeControlEntries.begin();
    while (it != requestedTimeControlEntries.end()) {
      log_d("TimeEntry[%d] weekdays:%d %d:%d enabled: %d lock action: %d", it->entryId, it->weekdays, it->timeHour, it->timeMin, it->enabled, it->lockAction);
      it++;
    }
  } else {
    log_d("get log failed: %d, error %d", result, nukiBle.getLastError());
  }
}

void getConfig() {
  Nuki::Config config;
  if (nukiBle.requestConfig(&config) == 1) {
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
  nukiBle.registerBleScanner(&scanner);
  nukiBle.initialize();

  if (nukiBle.isPairedWithLock()) {
    log_d("paired");
    nukiBle.setEventHandler(&handler);
    getConfig();
    nukiBle.enableLedFlash(false);
  }

  // nukiBle.savePincode(9999);
  // nukiBle.unPairNuki();
}

void loop() {
  scanner.update();
  if (!nukiBle.isPairedWithLock()) {
    if (nukiBle.pairNuki()) {
      log_d("paired");
      nukiBle.setEventHandler(&handler);
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