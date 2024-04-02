#include "NukiOpener.h"
#include "NukiUtils.h"
#include "NukiOpenerUtils.h"

namespace NukiOpener {
NukiOpener::NukiOpener(const std::string& deviceName, const uint32_t deviceId)
  : NukiBle(deviceName,
            deviceId,
            openerPairingServiceUUID,
            openerServiceUUID,
            openerGdioUUID,
            openerUserDataUUID,
            deviceName + "opener") {
}

Nuki::CmdResult NukiOpener::lockAction(const LockAction lockAction, const uint32_t nukiAppId, const uint8_t flags, const char* nameSuffix, const uint8_t nameSuffixLen) {
  Action action;
  unsigned char payload[5 + nameSuffixLen] = {0};
  memcpy(payload, &lockAction, sizeof(LockAction));
  memcpy(&payload[sizeof(LockAction)], &nukiAppId, 4);
  memcpy(&payload[sizeof(LockAction) + 4], &flags, 1);
  uint8_t payloadLen = 0;
  if (nameSuffix) {
    memcpy(&payload[sizeof(LockAction) + 4 + 1], nameSuffix, nameSuffixLen);
    payloadLen = sizeof(LockAction) + 4 + 1 + nameSuffixLen;
  } else {
    payloadLen = sizeof(LockAction) + 4 + 1;
  }

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndAccept;
  action.command = Command::LockAction;
  memcpy(action.payload, &payload, payloadLen);
  action.payloadLen = payloadLen;

  return executeAction(action);
}


Nuki::CmdResult NukiOpener::requestOpenerState(OpenerState* state) {
  Action action;
  uint16_t payload = (uint16_t)Command::KeyturnerStates;

  memset(&action, 0, sizeof(action));
  action.cmdType = Nuki::CommandType::Command;
  action.command = Command::RequestData;
  memcpy(&action.payload[0], &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    // printBuffer((byte*)&retrievedKeyTurnerState, sizeof(retrievedKeyTurnerState), false, "retreived Keyturner state");
    memcpy(state, &openerState, sizeof(OpenerState));
  }
  return result;
}

void NukiOpener::retrieveOpenerState(OpenerState* state) {
  memcpy(state, &openerState, sizeof(OpenerState));
}


Nuki::CmdResult NukiOpener::requestBatteryReport(BatteryReport* retrievedBatteryReport) {
  Action action;
  uint16_t payload = (uint16_t)Command::BatteryReport;

  memset(&action, 0, sizeof(action));
  action.cmdType = Nuki::CommandType::Command;
  action.command = Command::RequestData;
  memcpy(&action.payload[0], &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    memcpy(retrievedBatteryReport, &batteryReport, sizeof(batteryReport));
  }
  return result;
}


Nuki::CmdResult NukiOpener::requestConfig(Config* retrievedConfig) {
  Action action;

  memset(&action, 0, sizeof(action));
  action.cmdType = Nuki::CommandType::CommandWithChallenge;
  action.command = Command::RequestConfig;

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    memcpy(retrievedConfig, &config, sizeof(Config));
  }
  return result;
}

Nuki::CmdResult NukiOpener::requestAdvancedConfig(AdvancedConfig* retrievedAdvancedConfig) {
  Action action;

  memset(&action, 0, sizeof(action));
  action.cmdType = Nuki::CommandType::CommandWithChallenge;
  action.command = Command::RequestAdvancedConfig;

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    memcpy(retrievedAdvancedConfig, &advancedConfig, sizeof(AdvancedConfig));
  }
  return result;
}


//basic config change methods
Nuki::CmdResult NukiOpener::setName(const std::string& name) {

  if (name.length() <= 32) {
    Config oldConfig;
    Nuki::CmdResult result = requestConfig(&oldConfig);
    if (result == Nuki::CmdResult::Success) {
      memset(oldConfig.name, 0, sizeof(oldConfig.name));
      memcpy(oldConfig.name, name.c_str(), name.length());
      result = setFromConfig(oldConfig);
    }
    return result;
  } else {
    log_w("setName, too long (max32)");
    return Nuki::CmdResult::Failed;
  }
}

Nuki::CmdResult NukiOpener::setLatitude(const float degrees) {
  Config oldConfig;
  Nuki::CmdResult result = requestConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.latitude = degrees;
    result = setFromConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setLongitude(const float degrees) {
  Config oldConfig;
  Nuki::CmdResult result = requestConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.longitude = degrees;
    result = setFromConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setFobAction(const uint8_t fobActionNr, const uint8_t fobAction) {
  Config oldConfig;
  Nuki::CmdResult result = requestConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    switch (fobActionNr) {
      case 1:
        oldConfig.fobAction1 = fobAction;
        break;
      case 2:
        oldConfig.fobAction2 = fobAction;
        break;
      case 3:
        oldConfig.fobAction3 = fobAction;
        break;
      default:
        return Nuki::CmdResult::Error;
    }
    result = setFromConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setOperatingMode(const uint8_t opmode) {
  Config oldConfig;
  Nuki::CmdResult result = requestConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.operatingMode = opmode;
    result = setFromConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::enableDst(const bool enable) {
  Config oldConfig;
  Nuki::CmdResult result = requestConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.dstMode = enable;
    result = setFromConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setTimeZoneOffset(const int16_t minutes) {
  Config oldConfig;
  Nuki::CmdResult result = requestConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.timeZoneOffset = minutes;
    result = setFromConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setTimeZoneId(const TimeZoneId timeZoneId) {
  Config oldConfig;
  Nuki::CmdResult result = requestConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.timeZoneId = timeZoneId;
    result = setFromConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::enableButton(const bool enable) {
  Config oldConfig;
  Nuki::CmdResult result = requestConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.buttonEnabled = enable;
    result = setFromConfig(oldConfig);
  }
  return result;
}


//advanced config change methods
Nuki::CmdResult NukiOpener::setIntercomID(const uint16_t intercomID) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.intercomID = intercomID;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setBusModeSwitch(const bool busModeSwitch) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.busModeSwitch = busModeSwitch;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setShortCircuitDuration(const uint16_t duration) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.shortCircuitDuration = duration;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setElectricStrikeDelay(const uint16_t delay) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.electricStrikeDelay = delay;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::enableRandomElectricStrikeDelay(const bool enable) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.randomElectricStrikeDelay = enable;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setElectricStrikeDuration(const uint16_t duration) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.electricStrikeDuration = duration;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::disableRtoAfterRing(const bool disable) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.disableRtoAfterRing = disable;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setRtoTimeout(const uint8_t timeout) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.rtoTimeout = timeout;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setDoorbellSuppression(const uint8_t suppression) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.doorbellSuppression = suppression;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setDoorbellSuppressionDuration(const uint16_t duration) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.doorbellSuppressionDuration = duration;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setSoundRing(const uint8_t sound) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.soundRing = sound;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setSoundOpen(const uint8_t sound) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.soundOpen = sound;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setSoundRto(const uint8_t sound) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.soundRto = sound;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setSoundCm(const uint8_t sound) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.soundCm = sound;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::enableSoundConfirmation(const bool enable) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.soundConfirmation = enable;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setSingleButtonPressAction(const ButtonPressAction action) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.singleButtonPressAction = action;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setDoubleButtonPressAction(const ButtonPressAction action) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.doubleButtonPressAction = action;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}



Nuki::CmdResult NukiOpener::setBatteryType(const BatteryType type) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.batteryType = type;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::enableAutoBatteryTypeDetection(const bool enable) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.automaticBatteryTypeDetection = enable;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::enablePairing(const bool enable) {
  Config oldConfig;
  Nuki::CmdResult result = requestConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.pairingEnabled = enable;
    result = setFromConfig(oldConfig);
  }
  return result;
}

CmdResult NukiOpener::enableLedFlash(const bool enable) {
  Config oldConfig;
  CmdResult result = requestConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.ledFlashEnabled = enable;
    result = setFromConfig(oldConfig);
  }
  return result;
}

CmdResult NukiOpener::setSoundLevel(const uint8_t value) {
  AdvancedConfig oldConfig;
  Nuki::CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.soundLevel = value;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

Nuki::CmdResult NukiOpener::setAdvertisingMode(const AdvertisingMode mode) {
  Config oldConfig;
  Nuki::CmdResult result = requestConfig(&oldConfig);
  if (result == Nuki::CmdResult::Success) {
    oldConfig.advertisingMode = mode;
    result = setFromConfig(oldConfig);
  }
  return result;
}


Nuki::CmdResult NukiOpener::addTimeControlEntry(NewTimeControlEntry newTimeControlEntry) {
//TODO verify data validity
  Action action;
  unsigned char payload[sizeof(NewTimeControlEntry)] = {0};
  memcpy(payload, &newTimeControlEntry, sizeof(NewTimeControlEntry));

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::AddTimeControlEntry;
  memcpy(action.payload, &payload, sizeof(NewTimeControlEntry));
  action.payloadLen = sizeof(NewTimeControlEntry);

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addTimeControlEntry, payloadlen: %d", sizeof(NewTimeControlEntry));
    printBuffer(action.payload, sizeof(NewTimeControlEntry), false, "new time control content: ");
    logNewTimeControlEntry(newTimeControlEntry);
    #endif
  }
  return result;
}

Nuki::CmdResult NukiOpener::updateTimeControlEntry(TimeControlEntry TimeControlEntry) {
  //TODO verify data validity
  Action action;
  unsigned char payload[sizeof(TimeControlEntry)] = {0};
  memcpy(payload, &TimeControlEntry, sizeof(TimeControlEntry));

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::UpdateTimeControlEntry;
  memcpy(action.payload, &payload, sizeof(TimeControlEntry));
  action.payloadLen = sizeof(TimeControlEntry);

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addTimeControlEntry, payloadlen: %d", sizeof(TimeControlEntry));
    printBuffer(action.payload, sizeof(TimeControlEntry), false, "updated time control content: ");
    logTimeControlEntry(TimeControlEntry);
    #endif
  }
  return result;
}

Nuki::CmdResult NukiOpener::removeTimeControlEntry(uint8_t entryId) {
//TODO verify data validity
  Action action;
  unsigned char payload[1] = {0};
  memcpy(payload, &entryId, 1);

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::RemoveTimeControlEntry;
  memcpy(action.payload, &payload, 1);
  action.payloadLen = 1;

  return executeAction(action);
}

Nuki::CmdResult NukiOpener::retrieveTimeControlEntries() {
  Action action;

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::RequestTimeControlEntries;
  action.payloadLen = 0;

  listOfTimeControlEntries.clear();

  return executeAction(action);
}

void NukiOpener::getTimeControlEntries(std::list<TimeControlEntry>* requestedTimeControlEntries) {
  requestedTimeControlEntries->clear();
  std::list<TimeControlEntry>::iterator it = listOfTimeControlEntries.begin();
  while (it != listOfTimeControlEntries.end()) {
    requestedTimeControlEntries->push_back(*it);
    it++;
  }
}

void NukiOpener::getLogEntries(std::list<LogEntry>* requestedLogEntries) {
  requestedLogEntries->clear();

  for (const auto& it : listOfLogEntries) {
    requestedLogEntries->push_back(it);
  }
}

Nuki::CmdResult NukiOpener::retrieveLogEntries(const uint32_t startIndex, const uint16_t count, const uint8_t sortOrder, bool const totalCount) {
  Action action;
  unsigned char payload[8] = {0};
  memcpy(payload, &startIndex, 4);
  memcpy(&payload[4], &count, 2);
  memcpy(&payload[6], &sortOrder, 1);
  memcpy(&payload[7], &totalCount, 1);

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::RequestLogEntries;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  listOfLogEntries.clear();

  return executeAction(action);
}

bool NukiOpener::isBatteryCritical() {
  return openerState.criticalBatteryState & 1;
}

const ErrorCode NukiOpener::getLastError() const {
  return (ErrorCode)errorCode;
}

Nuki::CmdResult NukiOpener::setConfig(NewConfig newConfig) {
  Action action;
  unsigned char payload[sizeof(NewConfig)] = {0};
  memcpy(payload, &newConfig, sizeof(NewConfig));

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::SetConfig;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  return executeAction(action);
}

Nuki::CmdResult NukiOpener::setFromConfig(const Config config) {
  NewConfig newConfig;
  createNewConfig(&config, &newConfig);
  return setConfig(newConfig);
}

Nuki::CmdResult NukiOpener::setFromAdvancedConfig(const AdvancedConfig config) {
  NewAdvancedConfig newConfig;
  createNewAdvancedConfig(&config, &newConfig);
  return setAdvancedConfig(newConfig);
}

Nuki::CmdResult NukiOpener::setAdvancedConfig(NewAdvancedConfig newAdvancedConfig) {
  Action action;
  unsigned char payload[sizeof(NewAdvancedConfig)] = {0};
  memcpy(payload, &newAdvancedConfig, sizeof(NewAdvancedConfig));

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::SetAdvancedConfig;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  return executeAction(action);
}

void NukiOpener::createNewConfig(const Config* oldConfig, NewConfig* newConfig) {
  memcpy(newConfig->name, oldConfig->name, sizeof(newConfig->name));
  newConfig->latitude = oldConfig->latitude;
  newConfig->longitude = oldConfig->longitude;
  newConfig->capabilities = oldConfig->capabilities;
  newConfig->pairingEnabled = oldConfig->pairingEnabled;
  newConfig->buttonEnabled = oldConfig->buttonEnabled;
  newConfig->ledFlashEnabled = oldConfig->ledFlashEnabled;
  newConfig->timeZoneOffset = oldConfig->timeZoneOffset;
  newConfig->dstMode = oldConfig->dstMode;
  newConfig->fobAction1 = oldConfig->fobAction1;
  newConfig->fobAction2 = oldConfig->fobAction2;
  newConfig->fobAction3 = oldConfig->fobAction3;
  newConfig->operatingMode = oldConfig->operatingMode;
  newConfig->advertisingMode = oldConfig->advertisingMode;
  newConfig->timeZoneId = oldConfig->timeZoneId;
}

void NukiOpener::createNewAdvancedConfig(const AdvancedConfig* oldConfig, NewAdvancedConfig* newConfig) {
  newConfig->intercomID = oldConfig->intercomID;
  newConfig->busModeSwitch = oldConfig->busModeSwitch;
  newConfig->shortCircuitDuration = oldConfig->shortCircuitDuration;
  newConfig->electricStrikeDelay = oldConfig->electricStrikeDelay;
  newConfig->randomElectricStrikeDelay = oldConfig->randomElectricStrikeDelay;
  newConfig->electricStrikeDuration = oldConfig->electricStrikeDuration;
  newConfig->disableRtoAfterRing = oldConfig->disableRtoAfterRing;
  newConfig->rtoTimeout = oldConfig->rtoTimeout;
  newConfig->doorbellSuppression = oldConfig->doorbellSuppression;
  newConfig->doorbellSuppressionDuration = oldConfig->doorbellSuppressionDuration;
  newConfig->soundRing = oldConfig->soundRing;
  newConfig->soundOpen = oldConfig->soundOpen;
  newConfig->soundRto = oldConfig->soundRto;
  newConfig->soundCm = oldConfig->soundCm;
  newConfig->soundConfirmation = oldConfig->soundConfirmation;
  newConfig->soundLevel = oldConfig->soundLevel;
  newConfig->singleButtonPressAction = oldConfig->singleButtonPressAction;
  newConfig->doubleButtonPressAction = oldConfig->doubleButtonPressAction;
  newConfig->batteryType = oldConfig->batteryType;
  newConfig->automaticBatteryTypeDetection = oldConfig->automaticBatteryTypeDetection;
}


void NukiOpener::handleReturnMessage(Command returnCode, unsigned char* data, uint16_t dataLen) {
  extendDisonnectTimeout();

  switch (returnCode) {
    case Command::KeyturnerStates : {
      printBuffer((byte*)data, dataLen, false, "keyturnerStates");
      memcpy(&openerState, data, sizeof(openerState));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logKeyturnerState(openerState);
      #endif
      break;
    }
    case Command::BatteryReport : {
      printBuffer((byte*)data, dataLen, false, "batteryReport");
      memcpy(&batteryReport, data, sizeof(batteryReport));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logBatteryReport(batteryReport);
      #endif
      break;
    }
    case Command::Config : {
      memcpy(&config, data, sizeof(config));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logConfig(config);
      #endif
      printBuffer((byte*)data, dataLen, false, "config");
      break;
    }
    case Command::AdvancedConfig : {
      memcpy(&advancedConfig, data, sizeof(advancedConfig));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logAdvancedConfig(advancedConfig);
      #endif
      printBuffer((byte*)data, dataLen, false, "advancedConfig");
      break;
    }
    case Command::TimeControlEntry : {
      printBuffer((byte*)data, dataLen, false, "timeControlEntry");
      TimeControlEntry timeControlEntry;
      memcpy(&timeControlEntry, data, sizeof(timeControlEntry));
      listOfTimeControlEntries.push_back(timeControlEntry);
      break;
    }
    case Command::LogEntry : {
      printBuffer((byte*)data, dataLen, false, "logEntry");
      LogEntry logEntry;
      memcpy(&logEntry, data, sizeof(logEntry));
      listOfLogEntries.push_back(logEntry);
      #ifdef DEBUG_NUKI_READABLE_DATA
      logLogEntry(logEntry);
      #endif
      break;
    }
    default:
      NukiBle::handleReturnMessage(returnCode, data, dataLen);
  }
  lastMsgCodeReceived = returnCode;
}

void NukiOpener::logErrorCode(uint8_t errorCode) {
  logOpenerErrorCode(errorCode);
}


}