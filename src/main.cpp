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

KeyTurnerState retrievedKeyTurnerState;
BatteryReport _batteryReport;
std::list<LogEntry> requestedLogEntries;
std::list<KeypadEntry> requestedKeypadEntries;
std::list<AuthorizationEntry> requestedAuthorizationEntries;
std::list<TimeControlEntry> requestedTimeControlEntries;

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

bool keyTurnerState() {
  uint8_t result = nukiBle.requestKeyTurnerState(&retrievedKeyTurnerState);
  if (result == 1) {
    log_d("Bat crit: %d, Bat perc:%d lock state: %d %d:%d:%d",
          nukiBle.batteryCritical(), nukiBle.getBatteryPerc(), retrievedKeyTurnerState.lockState, retrievedKeyTurnerState.currentTimeHour,
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
  uint8_t result = nukiBle.retrieveKeypadEntries(0, 10);
  if (result == 1) {
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

void requestAuthorizationEntries() {
  uint8_t result = nukiBle.retrieveAuthorizationEntries(0, 10);
  if (result == 1) {
    delay(5000);
    nukiBle.getAuthorizationEntries(&requestedAuthorizationEntries);
    std::list<AuthorizationEntry>::iterator it = requestedAuthorizationEntries.begin();
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

void addTimeControl(uint8_t weekdays, uint8_t hour, uint8_t minute, LockAction lockAction) {
  NewTimeControlEntry newEntry;
  newEntry.weekdays = weekdays;
  newEntry.timeHour = hour;
  newEntry.timeMin = minute;
  newEntry.lockAction = lockAction;

  nukiBle.addTimeControlEntry(newEntry);
}

void requestTimeControlEntries() {
  NukiCmdResult result = nukiBle.retrieveTimeControlEntries();
  if (result == NukiCmdResult::Success) {
    delay(5000);
    nukiBle.getTimeControlEntries(&requestedTimeControlEntries);
    std::list<TimeControlEntry>::iterator it = requestedTimeControlEntries.begin();
    while (it != requestedTimeControlEntries.end()) {
      log_d("TimeEntry[%d] weekdays:%d %d:%d enabled: %d lock action: %d", it->entryId, it->weekdays, it->timeHour, it->timeMin, it->enabled, it->lockAction);
      it++;
    }
  } else {
    log_d("get log failed: %d, error %d", result, nukiBle.getLastError());
  }
}

void getConfig() {
  Config config;
  if (nukiBle.requestConfig(&config) == 1) {
    log_d("Name: %s", config.name);
  } else {
    log_w("getConfig failed");
  }

}

bool notified = false;
class Handler: public NukiSmartlockEventHandler {
  public:
    virtual ~Handler() {};
    void notify(NukiEventType eventType) {
      notified = true;
    }
};

Handler handler;

void setup() {
  Serial.begin(115200);
  log_d("Starting NUKI BLE...");
  nukiBle.initialize();

  // nukiBle.savePincode(9999);
  // nukiBle.unPairNuki();
}

void loop() {
  nukiBle.update();
  if (!paired) {
    if (nukiBle.pairNuki()) {
      log_d("paired");
      paired = true;
      nukiBle.setEventHandler(&handler);
    }
  }

  if (notified) {
    if (keyTurnerState()) {
      notified = false;
    }

  }
  delay(500);
}