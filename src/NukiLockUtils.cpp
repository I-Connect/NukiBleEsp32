/**
 * @file NukiUtills.cpp
 * Implementation of generic/helper functions
 *
 * Created on: 2022
 * License: GNU GENERAL PUBLIC LICENSE (see LICENSE)
 *
 * This library implements the communication from an ESP32 via BLE to a Nuki smart lock.
 * Based on the Nuki Smart Lock API V2.2.1
 * https://developer.nuki.io/page/nuki-smart-lock-api-2/2/
 *
 */

#include "NukiLockUtils.h"

namespace NukiLock {


void cmdResultToString(const CmdResult state, char* str) {
  switch (state) {
    case CmdResult::Success:
      strcpy(str, "success");
      break;
    case CmdResult::Failed:
      strcpy(str, "failed");
      break;
    case CmdResult::TimeOut:
      strcpy(str, "timeOut");
      break;
    case CmdResult::Working:
      strcpy(str, "working");
      break;
    case CmdResult::NotPaired:
      strcpy(str, "notPaired");
      break;
    case CmdResult::Error:
      strcpy(str, "error");
      break;
    default:
      strcpy(str, "undefined");
      break;
  }
}

void logLockErrorCode(uint8_t errorCode) {
  switch (errorCode) {
    case (uint8_t)ErrorCode::ERROR_BAD_CRC :
      log_e("ERROR_BAD_CRC");
      break;
    case (uint8_t)ErrorCode::ERROR_BAD_LENGTH :
      log_e("ERROR_BAD_LENGTH");
      break;
    case (uint8_t)ErrorCode::ERROR_UNKNOWN :
      log_e("ERROR_UNKNOWN");
      break;
    case (uint8_t)ErrorCode::P_ERROR_NOT_PAIRING :
      log_e("P_ERROR_NOT_PAIRING");
      break;
    case (uint8_t)ErrorCode::P_ERROR_BAD_AUTHENTICATOR :
      log_e("P_ERROR_BAD_AUTHENTICATOR");
      break;
    case (uint8_t)ErrorCode::P_ERROR_BAD_PARAMETER :
      log_e("P_ERROR_BAD_PARAMETER");
      break;
    case (uint8_t)ErrorCode::P_ERROR_MAX_USER :
      log_e("P_ERROR_MAX_USER");
      break;
    case (uint8_t)ErrorCode::K_ERROR_AUTO_UNLOCK_TOO_RECENT :
      log_e("K_ERROR_AUTO_UNLOCK_TOO_RECENT");
      break;
    case (uint8_t)ErrorCode::K_ERROR_BAD_NONCE :
      log_e("K_ERROR_BAD_NONCE");
      break;
    case (uint8_t)ErrorCode::K_ERROR_BAD_PARAMETER :
      log_e("K_ERROR_BAD_PARAMETER");
      break;
    case (uint8_t)ErrorCode::K_ERROR_BAD_PIN :
      log_e("K_ERROR_BAD_PIN");
      break;
    case (uint8_t)ErrorCode::K_ERROR_BUSY :
      log_e("K_ERROR_BUSY");
      break;
    case (uint8_t)ErrorCode::K_ERROR_CANCELED :
      log_e("K_ERROR_CANCELED");
      break;
    case (uint8_t)ErrorCode::K_ERROR_CLUTCH_FAILURE :
      log_e("K_ERROR_CLUTCH_FAILURE");
      break;
    case (uint8_t)ErrorCode::K_ERROR_CLUTCH_POWER_FAILURE :
      log_e("K_ERROR_CLUTCH_POWER_FAILURE");
      break;
    case (uint8_t)ErrorCode::K_ERROR_CODE_ALREADY_EXISTS :
      log_e("K_ERROR_CODE_ALREADY_EXISTS");
      break;
    case (uint8_t)ErrorCode::K_ERROR_CODE_INVALID :
      log_e("K_ERROR_CODE_INVALID");
      break;
    case (uint8_t)ErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_1 :
      log_e("K_ERROR_CODE_INVALID_TIMEOUT_1");
      break;
    case (uint8_t)ErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_2 :
      log_e("K_ERROR_CODE_INVALID_TIMEOUT_2");
      break;
    case (uint8_t)ErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_3 :
      log_e("K_ERROR_CODE_INVALID_TIMEOUT_3");
      break;
    case (uint8_t)ErrorCode::K_ERROR_DISABLED :
      log_e("K_ERROR_DISABLED");
      break;
    case (uint8_t)ErrorCode::K_ERROR_FIRMWARE_UPDATE_NEEDED :
      log_e("K_ERROR_FIRMWARE_UPDATE_NEEDED");
      break;
    case (uint8_t)ErrorCode::K_ERROR_INVALID_AUTH_ID :
      log_e("K_ERROR_INVALID_AUTH_ID");
      break;
    case (uint8_t)ErrorCode::K_ERROR_MOTOR_BLOCKED :
      log_e("K_ERROR_MOTOR_BLOCKED");
      break;
    case (uint8_t)ErrorCode::K_ERROR_MOTOR_LOW_VOLTAGE :
      log_e("K_ERROR_MOTOR_LOW_VOLTAGE");
      break;
    case (uint8_t)ErrorCode::K_ERROR_MOTOR_POSITION_LIMIT :
      log_e("K_ERROR_MOTOR_POSITION_LIMIT");
      break;
    case (uint8_t)ErrorCode::K_ERROR_MOTOR_POWER_FAILURE :
      log_e("K_ERROR_MOTOR_POWER_FAILURE");
      break;
    case (uint8_t)ErrorCode::K_ERROR_MOTOR_TIMEOUT :
      log_e("K_ERROR_MOTOR_TIMEOUT");
      break;
    case (uint8_t)ErrorCode::K_ERROR_NOT_AUTHORIZED :
      log_e("K_ERROR_NOT_AUTHORIZED");
      break;
    case (uint8_t)ErrorCode::K_ERROR_NOT_CALIBRATED :
      log_e("K_ERROR_NOT_CALIBRATED");
      break;
    case (uint8_t)ErrorCode::K_ERROR_POSITION_UNKNOWN :
      log_e("K_ERROR_POSITION_UNKNOWN");
      break;
    case (uint8_t)ErrorCode::K_ERROR_REMOTE_NOT_ALLOWED :
      log_e("K_ERROR_REMOTE_NOT_ALLOWED");
      break;
    case (uint8_t)ErrorCode::K_ERROR_TIME_NOT_ALLOWED :
      log_e("K_ERROR_TIME_NOT_ALLOWED");
      break;
    case (uint8_t)ErrorCode::K_ERROR_TOO_MANY_ENTRIES :
      log_e("K_ERROR_TOO_MANY_ENTRIES");
      break;
    case (uint8_t)ErrorCode::K_ERROR_TOO_MANY_PIN_ATTEMPTS :
      log_e("K_ERROR_TOO_MANY_PIN_ATTEMPTS");
      break;
    case (uint8_t)ErrorCode::K_ERROR_VOLTAGE_TOO_LOW :
      log_e("K_ERROR_VOLTAGE_TOO_LOW");
      break;
    default:
      log_e("UNDEFINED ERROR");
  }
}

void logConfig(Config config) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("nukiId :%d", config.nukiId);
  log_d("name :%s", config.name);
  log_d("latitide :%f", config.latitide);
  log_d("longitude :%f", config.longitude);
  log_d("autoUnlatch :%d", config.autoUnlatch);
  log_d("pairingEnabled :%d", config.pairingEnabled);
  log_d("buttonEnabled :%d", config.buttonEnabled);
  log_d("ledEnabled :%d", config.ledEnabled);
  log_d("ledBrightness :%d", config.ledBrightness);
  log_d("currentTime Year :%d", config.currentTimeYear);
  log_d("currentTime Month :%d", config.currentTimeMonth);
  log_d("currentTime Day :%d", config.currentTimeDay);
  log_d("currentTime Hour :%d", config.currentTimeHour);
  log_d("currentTime Minute :%d", config.currentTimeMinute);
  log_d("currentTime Second :%d", config.currentTimeSecond);
  log_d("timeZoneOffset :%d", config.timeZoneOffset);
  log_d("dstMode :%d", config.dstMode);
  log_d("hasFob :%d", config.hasFob);
  log_d("fobAction1 :%d", config.fobAction1);
  log_d("fobAction2 :%d", config.fobAction2);
  log_d("fobAction3 :%d", config.fobAction3);
  log_d("singleLock :%d", config.singleLock);
  log_d("advertisingMode :%d", config.advertisingMode);
  log_d("hasKeypad :%d", config.hasKeypad);
  log_d("firmwareVersion :%d.%d.%d", config.firmwareVersion[0], config.firmwareVersion[1], config.firmwareVersion[2]);
  log_d("hardwareRevision :%d.%d", config.hardwareRevision[0], config.hardwareRevision[1]);
  log_d("homeKitStatus :%d", config.homeKitStatus);
  log_d("timeZoneId :%d", config.timeZoneId);
  log_d("hasKeypadV2 :%d", config.hasKeypadV2);
  #endif
}

void logNewConfig(NewConfig newConfig) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("name :%s", newConfig.name);
  log_d("latitide :%f", newConfig.latitide);
  log_d("longitude :%f", newConfig.longitude);
  log_d("autoUnlatch :%d", newConfig.autoUnlatch);
  log_d("pairingEnabled :%d", newConfig.pairingEnabled);
  log_d("buttonEnabled :%d", newConfig.buttonEnabled);
  log_d("ledEnabled :%d", newConfig.ledEnabled);
  log_d("ledBrightness :%d", newConfig.ledBrightness);
  log_d("timeZoneOffset :%d", newConfig.timeZoneOffset);
  log_d("dstMode :%d", newConfig.dstMode);
  log_d("fobAction1 :%d", newConfig.fobAction1);
  log_d("fobAction2 :%d", newConfig.fobAction2);
  log_d("fobAction3 :%d", newConfig.fobAction3);
  log_d("singleLock :%d", newConfig.singleLock);
  log_d("advertisingMode :%d", newConfig.advertisingMode);
  log_d("timeZoneId :%d", newConfig.timeZoneId);
  #endif
}

void logNewKeypadEntry(NewKeypadEntry newKeypadEntry) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("code:%d", newKeypadEntry.code);
  log_d("name:%s", newKeypadEntry.name);
  log_d("timeLimited:%d", newKeypadEntry.timeLimited);
  log_d("allowedFromYear:%d", newKeypadEntry.allowedFromYear);
  log_d("allowedFromMonth:%d", newKeypadEntry.allowedFromMonth);
  log_d("allowedFromDay:%d", newKeypadEntry.allowedFromDay);
  log_d("allowedFromHour:%d", newKeypadEntry.allowedFromHour);
  log_d("allowedFromMin:%d", newKeypadEntry.allowedFromMin);
  log_d("allowedFromSec:%d", newKeypadEntry.allowedFromSec);
  log_d("allowedUntilYear:%d", newKeypadEntry.allowedUntilYear);
  log_d("allowedUntilMonth:%d", newKeypadEntry.allowedUntilMonth);
  log_d("allowedUntilDay:%d", newKeypadEntry.allowedUntilDay);
  log_d("allowedUntilHour:%d", newKeypadEntry.allowedUntilHour);
  log_d("allowedUntilMin:%d", newKeypadEntry.allowedUntilMin);
  log_d("allowedUntilSec:%d", newKeypadEntry.allowedUntilSec);
  log_d("allowedWeekdays:%d", newKeypadEntry.allowedWeekdays);
  log_d("allowedFromTimeHour:%d", newKeypadEntry.allowedFromTimeHour);
  log_d("allowedFromTimeMin:%d", newKeypadEntry.allowedFromTimeMin);
  log_d("allowedUntilTimeHour:%d", newKeypadEntry.allowedUntilTimeHour);
  log_d("allowedUntilTimeMin:%d", newKeypadEntry.allowedUntilTimeMin);
  #endif
}

void logKeypadEntry(KeypadEntry keypadEntry) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("codeId:%d", keypadEntry.codeId);
  log_d("code:%d", keypadEntry.code);
  log_d("name:%s", keypadEntry.name);
  log_d("enabled:%d", keypadEntry.enabled);
  log_d("dateCreatedYear:%d", keypadEntry.dateCreatedYear);
  log_d("dateCreatedMonth:%d", keypadEntry.dateCreatedMonth);
  log_d("dateCreatedDay:%d", keypadEntry.dateCreatedDay);
  log_d("dateCreatedHour:%d", keypadEntry.dateCreatedHour);
  log_d("dateCreatedMin:%d", keypadEntry.dateCreatedMin);
  log_d("dateCreatedSec:%d", keypadEntry.dateCreatedSec);
  log_d("dateLastActiveYear:%d", keypadEntry.dateLastActiveYear);
  log_d("dateLastActiveMonth:%d", keypadEntry.dateLastActiveMonth);
  log_d("dateLastActiveDay:%d", keypadEntry.dateLastActiveDay);
  log_d("dateLastActiveHour:%d", keypadEntry.dateLastActiveHour);
  log_d("dateLastActiveMin:%d", keypadEntry.dateLastActiveMin);
  log_d("dateLastActiveSec:%d", keypadEntry.dateLastActiveSec);
  log_d("lockCount:%d", keypadEntry.lockCount);
  log_d("timeLimited:%d", keypadEntry.timeLimited);
  log_d("allowedFromYear:%d", keypadEntry.allowedFromYear);
  log_d("allowedFromMonth:%d", keypadEntry.allowedFromMonth);
  log_d("allowedFromDay:%d", keypadEntry.allowedFromDay);
  log_d("allowedFromHour:%d", keypadEntry.allowedFromHour);
  log_d("allowedFromMin:%d", keypadEntry.allowedFromMin);
  log_d("allowedFromSec:%d", keypadEntry.allowedFromSec);
  log_d("allowedUntilYear:%d", keypadEntry.allowedUntilYear);
  log_d("allowedUntilMonth:%d", keypadEntry.allowedUntilMonth);
  log_d("allowedUntilDay:%d", keypadEntry.allowedUntilDay);
  log_d("allowedUntilHour:%d", keypadEntry.allowedUntilHour);
  log_d("allowedUntilMin:%d", keypadEntry.allowedUntilMin);
  log_d("allowedUntilSec:%d", keypadEntry.allowedUntilSec);
  log_d("allowedWeekdays:%d", keypadEntry.allowedWeekdays);
  log_d("allowedFromTimeHour:%d", keypadEntry.allowedFromTimeHour);
  log_d("allowedFromTimeMin:%d", keypadEntry.allowedFromTimeMin);
  log_d("allowedUntilTimeHour:%d", keypadEntry.allowedUntilTimeHour);
  log_d("allowedUntilTimeMin:%d", keypadEntry.allowedUntilTimeMin);
  #endif
}

void logUpdatedKeypadEntry(UpdatedKeypadEntry updatedKeypadEntry) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("codeId:%d", updatedKeypadEntry.codeId);
  log_d("code:%d", updatedKeypadEntry.code);
  log_d("name:%s", updatedKeypadEntry.name);
  log_d("enabled:%d", updatedKeypadEntry.enabled);
  log_d("timeLimited:%d", updatedKeypadEntry.timeLimited);
  log_d("allowedFromYear:%d", updatedKeypadEntry.allowedFromYear);
  log_d("allowedFromMonth:%d", updatedKeypadEntry.allowedFromMonth);
  log_d("allowedFromDay:%d", updatedKeypadEntry.allowedFromDay);
  log_d("allowedFromHour:%d", updatedKeypadEntry.allowedFromHour);
  log_d("allowedFromMin:%d", updatedKeypadEntry.allowedFromMin);
  log_d("allowedFromSec:%d", updatedKeypadEntry.allowedFromSec);
  log_d("allowedUntilYear:%d", updatedKeypadEntry.allowedUntilYear);
  log_d("allowedUntilMonth:%d", updatedKeypadEntry.allowedUntilMonth);
  log_d("allowedUntilDay:%d", updatedKeypadEntry.allowedUntilDay);
  log_d("allowedUntilHour:%d", updatedKeypadEntry.allowedUntilHour);
  log_d("allowedUntilMin:%d", updatedKeypadEntry.allowedUntilMin);
  log_d("allowedUntilSec:%d", updatedKeypadEntry.allowedUntilSec);
  log_d("allowedWeekdays:%d", updatedKeypadEntry.allowedWeekdays);
  log_d("allowedFromTimeHour:%d", updatedKeypadEntry.allowedFromTimeHour);
  log_d("allowedFromTimeMin:%d", updatedKeypadEntry.allowedFromTimeMin);
  log_d("allowedUntilTimeHour:%d", updatedKeypadEntry.allowedUntilTimeHour);
  log_d("allowedUntilTimeMin:%d", updatedKeypadEntry.allowedUntilTimeMin);
  #endif
}

void logAuthorizationEntry(AuthorizationEntry authorizationEntry) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("id:%d", authorizationEntry.authId);
  log_d("idType:%d", authorizationEntry.idType);
  log_d("name:%s", authorizationEntry.name);
  log_d("enabled:%d", authorizationEntry.enabled);
  log_d("remoteAllowed:%d", authorizationEntry.remoteAllowed);
  log_d("createdYear:%d", authorizationEntry.createdYear);
  log_d("createdMonth:%d", authorizationEntry.createdMonth);
  log_d("createdDay:%d", authorizationEntry.createdDay);
  log_d("createdHour:%d", authorizationEntry.createdHour);
  log_d("createdMin:%d", authorizationEntry.createdMinute);
  log_d("createdSec:%d", authorizationEntry.createdSecond);
  log_d("lastactYear:%d", authorizationEntry.lastActYear);
  log_d("lastactMonth:%d", authorizationEntry.lastActMonth);
  log_d("lastactDay:%d", authorizationEntry.lastActDay);
  log_d("lastactHour:%d", authorizationEntry.lastActHour);
  log_d("lastactMin:%d", authorizationEntry.lastActMinute);
  log_d("lastactSec:%d", authorizationEntry.lastActSecond);
  log_d("lockCount:%d", authorizationEntry.lockCount);
  log_d("timeLimited:%d", authorizationEntry.timeLimited);
  log_d("allowedFromYear:%d", authorizationEntry.allowedFromYear);
  log_d("allowedFromMonth:%d", authorizationEntry.allowedFromMonth);
  log_d("allowedFromDay:%d", authorizationEntry.allowedFromDay);
  log_d("allowedFromHour:%d", authorizationEntry.allowedFromHour);
  log_d("allowedFromMin:%d", authorizationEntry.allowedFromMinute);
  log_d("allowedFromSec:%d", authorizationEntry.allowedFromSecond);
  log_d("allowedUntilYear:%d", authorizationEntry.allowedUntilYear);
  log_d("allowedUntilMonth:%d", authorizationEntry.allowedUntilMonth);
  log_d("allowedUntilDay:%d", authorizationEntry.allowedUntilDay);
  log_d("allowedUntilHour:%d", authorizationEntry.allowedUntilHour);
  log_d("allowedUntilMin:%d", authorizationEntry.allowedUntilMinute);
  log_d("allowedUntilSec:%d", authorizationEntry.allowedUntilSecond);
  log_d("allowedWeekdays:%d", authorizationEntry.allowedWeekdays);
  log_d("allowedFromTimeHour:%d", authorizationEntry.allowedFromTimeHour);
  log_d("allowedFromTimeMin:%d", authorizationEntry.allowedFromTimeMin);
  log_d("allowedUntilTimeHour:%d", authorizationEntry.allowedUntilTimeHour);
  log_d("allowedUntilTimeMin:%d", authorizationEntry.allowedUntilTimeMin);
  #endif
}

void logNewAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("name:%s", newAuthorizationEntry.name);
  log_d("idType:%d", newAuthorizationEntry.idType);
  log_d("remoteAllowed:%d", newAuthorizationEntry.remoteAllowed);
  log_d("timeLimited:%d", newAuthorizationEntry.timeLimited);
  log_d("allowedFromYear:%d", newAuthorizationEntry.allowedFromYear);
  log_d("allowedFromMonth:%d", newAuthorizationEntry.allowedFromMonth);
  log_d("allowedFromDay:%d", newAuthorizationEntry.allowedFromDay);
  log_d("allowedFromHour:%d", newAuthorizationEntry.allowedFromHour);
  log_d("allowedFromMin:%d", newAuthorizationEntry.allowedFromMinute);
  log_d("allowedFromSec:%d", newAuthorizationEntry.allowedFromSecond);
  log_d("allowedUntilYear:%d", newAuthorizationEntry.allowedUntilYear);
  log_d("allowedUntilMonth:%d", newAuthorizationEntry.allowedUntilMonth);
  log_d("allowedUntilDay:%d", newAuthorizationEntry.allowedUntilDay);
  log_d("allowedUntilHour:%d", newAuthorizationEntry.allowedUntilHour);
  log_d("allowedUntilMin:%d", newAuthorizationEntry.allowedUntilMinute);
  log_d("allowedUntilSec:%d", newAuthorizationEntry.allowedUntilSecond);
  log_d("allowedWeekdays:%d", newAuthorizationEntry.allowedWeekdays);
  log_d("allowedFromTimeHour:%d", newAuthorizationEntry.allowedFromTimeHour);
  log_d("allowedFromTimeMin:%d", newAuthorizationEntry.allowedFromTimeMin);
  log_d("allowedUntilTimeHour:%d", newAuthorizationEntry.allowedUntilTimeHour);
  log_d("allowedUntilTimeMin:%d", newAuthorizationEntry.allowedUntilTimeMin);
  #endif
}

void logUpdatedAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("id:%d", updatedAuthorizationEntry.authId);
  log_d("name:%s", updatedAuthorizationEntry.name);
  log_d("enabled:%d", updatedAuthorizationEntry.enabled);
  log_d("remoteAllowed:%d", updatedAuthorizationEntry.remoteAllowed);
  log_d("timeLimited:%d", updatedAuthorizationEntry.timeLimited);
  log_d("allowedFromYear:%d", updatedAuthorizationEntry.allowedFromYear);
  log_d("allowedFromMonth:%d", updatedAuthorizationEntry.allowedFromMonth);
  log_d("allowedFromDay:%d", updatedAuthorizationEntry.allowedFromDay);
  log_d("allowedFromHour:%d", updatedAuthorizationEntry.allowedFromHour);
  log_d("allowedFromMin:%d", updatedAuthorizationEntry.allowedFromMinute);
  log_d("allowedFromSec:%d", updatedAuthorizationEntry.allowedFromSecond);
  log_d("allowedUntilYear:%d", updatedAuthorizationEntry.allowedUntilYear);
  log_d("allowedUntilMonth:%d", updatedAuthorizationEntry.allowedUntilMonth);
  log_d("allowedUntilDay:%d", updatedAuthorizationEntry.allowedUntilDay);
  log_d("allowedUntilHour:%d", updatedAuthorizationEntry.allowedUntilHour);
  log_d("allowedUntilMin:%d", updatedAuthorizationEntry.allowedUntilMinute);
  log_d("allowedUntilSec:%d", updatedAuthorizationEntry.allowedUntilSecond);
  log_d("allowedWeekdays:%d", updatedAuthorizationEntry.allowedWeekdays);
  log_d("allowedFromTimeHour:%d", updatedAuthorizationEntry.allowedFromTimeHour);
  log_d("allowedFromTimeMin:%d", updatedAuthorizationEntry.allowedFromTimeMin);
  log_d("allowedUntilTimeHour:%d", updatedAuthorizationEntry.allowedUntilTimeHour);
  log_d("allowedUntilTimeMin:%d", updatedAuthorizationEntry.allowedUntilTimeMin);
  #endif
}

void logNewTimeControlEntry(NewTimeControlEntry newTimeControlEntry) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("weekdays:%d", newTimeControlEntry.weekdays);
  log_d("time:%d:%d", newTimeControlEntry.timeHour, newTimeControlEntry.timeMin);
  log_d("lockAction:%d", newTimeControlEntry.lockAction);
  #endif
}

void logTimeControlEntry(TimeControlEntry timeControlEntry) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("entryId:%d", timeControlEntry.entryId);
  log_d("enabled:%d", timeControlEntry.enabled);
  log_d("weekdays:%d", timeControlEntry.weekdays);
  log_d("time:%d:%d", timeControlEntry.timeHour, timeControlEntry.timeMin);
  log_d("lockAction:%d", timeControlEntry.lockAction);
  #endif
}

void logCompletionStatus(CompletionStatus completionStatus) {
  switch (completionStatus) {
    case CompletionStatus::Busy :
      log_d("Completion status: busy");
      break;
    case CompletionStatus::Canceled :
      log_d("Completion status: canceled");
      break;
    case CompletionStatus::ClutchFailure :
      log_d("Completion status: clutchFailure");
      break;
    case CompletionStatus::IncompleteFailure :
      log_d("Completion status: incompleteFailure");
      break;
    case CompletionStatus::LowMotorVoltage :
      log_d("Completion status: lowMotorVoltage");
      break;
    case CompletionStatus::MotorBlocked :
      log_d("Completion status: motorBlocked");
      break;
    case CompletionStatus::MotorPowerFailure :
      log_d("Completion status: motorPowerFailure");
      break;
    case CompletionStatus::OtherError :
      log_d("Completion status: otherError");
      break;
    case CompletionStatus::Success :
      log_d("Completion status: success");
      break;
    case CompletionStatus::TooRecent :
      log_d("Completion status: tooRecent");
      break;
    case CompletionStatus::InvalidCode :
      log_d("Completion status: invalid code");
      break;
    default:
      log_w("Completion status: unknown");
      break;
  }
}

void logNukiTrigger(Trigger nukiTrigger) {
  switch (nukiTrigger) {
    case Trigger::AutoLock :
      log_d("Trigger: autoLock");
      break;
    case Trigger::Automatic :
      log_d("Trigger: automatic");
      break;
    case Trigger::Button :
      log_d("Trigger: button");
      break;
    case Trigger::Manual :
      log_d("Trigger: manual");
      break;
    case Trigger::System :
      log_d("Trigger: system");
      break;
    default:
      log_w("Trigger: unknown");
      break;
  }
}

void logLockAction(LockAction lockAction) {
  switch (lockAction) {
    case LockAction::FobAction1 :
      log_d("action: autoLock");
      break;
    case LockAction::FobAction2 :
      log_d("action: automatic");
      break;
    case LockAction::FobAction3 :
      log_d("action: button");
      break;
    case LockAction::FullLock :
      log_d("action: manual");
      break;
    case LockAction::Lock :
      log_d("action: system");
      break;
    case LockAction::LockNgo :
      log_d("action: system");
      break;
    case LockAction::LockNgoUnlatch :
      log_d("action: system");
      break;
    case LockAction::Unlatch :
      log_d("action: system");
      break;
    case LockAction::Unlock :
      log_d("action: system");
      break;
    default:
      log_w("action: unknown");
      break;
  }
}

void logKeyturnerState(KeyTurnerState keyTurnerState) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("nukiState: %02x", keyTurnerState.nukiState);
  log_d("lockState: %d", keyTurnerState.lockState);
  logNukiTrigger(keyTurnerState.trigger);
  log_d("currentTimeYear: %d", keyTurnerState.currentTimeYear);
  log_d("currentTimeMonth: %d", keyTurnerState.currentTimeMonth);
  log_d("currentTimeDay: %d", keyTurnerState.currentTimeDay);
  log_d("currentTimeHour: %d", keyTurnerState.currentTimeHour);
  log_d("currentTimeMinute: %d", keyTurnerState.currentTimeMinute);
  log_d("currentTimeSecond: %d", keyTurnerState.currentTimeSecond);
  log_d("timeZoneOffset: %d", keyTurnerState.timeZoneOffset);
  log_d("criticalBatteryState composed value: %d", keyTurnerState.criticalBatteryState);
  log_d("configUpdateCount: %d", keyTurnerState.configUpdateCount);
  log_d("lockNgoTimer: %d", keyTurnerState.lockNgoTimer);
  logLockAction((LockAction)keyTurnerState.lastLockAction);
  log_d("lastLockActionTrigger: %d", keyTurnerState.lastLockActionTrigger);
  logCompletionStatus(keyTurnerState.lastLockActionCompletionStatus);
  log_d("doorSensorState: %d", keyTurnerState.doorSensorState);
  log_d("nightModeActive: %d", keyTurnerState.nightModeActive);
  log_d("Keypad bat critical feature supported: %d", keyTurnerState.accessoryBatteryState & 1);
  log_d("Keypad Battery Critical: %d", keyTurnerState.accessoryBatteryState & 2);
  #endif
}

void logBatteryReport(BatteryReport batteryReport) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("batteryDrain:%d", batteryReport.batteryDrain);
  log_d("batteryVoltage:%d", batteryReport.batteryVoltage);
  log_d("criticalBatteryState:%d", batteryReport.criticalBatteryState);
  log_d("lockAction:%d", batteryReport.lockAction);
  log_d("startVoltage:%d", batteryReport.startVoltage);
  log_d("lowestVoltage:%d", batteryReport.lowestVoltage);
  log_d("lockDistance:%d", batteryReport.lockDistance);
  log_d("startTemperature:%d", batteryReport.startTemperature);
  log_d("maxTurnCurrent:%d", batteryReport.maxTurnCurrent);
  log_d("batteryResistance:%d", batteryReport.batteryResistance);
  #endif
}

void logLogEntry(LogEntry logEntry) {
  log_d("[%d] type:%d authId:%d name: %s %d-%d-%d %d:%d:%d ", logEntry.index, logEntry.loggingType, logEntry.authId, logEntry.name, logEntry.timeStampYear, logEntry.timeStampMonth, logEntry.timeStampDay, logEntry.timeStampHour, logEntry.timeStampMinute, logEntry.timeStampSecond);

  switch (logEntry.loggingType) {
    case LoggingType::LoggingEnabled: {
      log_d("Logging enabled: %d", logEntry.data[0]);
      break;
    }
    case LoggingType::LockAction:
    case LoggingType::Calibration:
    case LoggingType::InitializationRun: {
      logLockAction((LockAction)logEntry.data[0]);
      logNukiTrigger((Trigger)logEntry.data[1]);
      log_d("Flags: %d", logEntry.data[2]);
      logCompletionStatus((CompletionStatus)logEntry.data[3]);
      break;
    }
    case LoggingType::KeypadAction: {
      logLockAction((LockAction)logEntry.data[0]);
      log_d("Source: %d", logEntry.data[1]);
      logCompletionStatus((CompletionStatus)logEntry.data[2]);
      uint16_t codeId = 0;
      memcpy(&codeId, &logEntry.data[3], 2);
      log_d("Code id: %d", codeId);
      break;
    }
    case LoggingType::DoorSensor: {
      if (logEntry.data[0] == 0x00) {
        log_d("Door opened") ;
      }
      if (logEntry.data[0] == 0x01) {
        log_d("Door closed") ;
      }
      if (logEntry.data[0] == 0x02) {
        log_d("Door sensor jammed") ;
      }
      break;
    }
    case LoggingType::DoorSensorLoggingEnabled: {
      log_d("Logging enabled: %d", logEntry.data[0]);
      break;
    }
    default:
      log_w("Unknown logging type");
      break;
  }
}

void logAdvancedConfig(AdvancedConfig advancedConfig) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("totalDegrees :%d", advancedConfig.totalDegrees);
  log_d("unlockedPositionOffsetDegrees :%d", advancedConfig.unlockedPositionOffsetDegrees);
  log_d("lockedPositionOffsetDegrees :%f", advancedConfig.lockedPositionOffsetDegrees);
  log_d("singleLockedPositionOffsetDegrees :%f", advancedConfig.singleLockedPositionOffsetDegrees);
  log_d("unlockedToLockedTransitionOffsetDegrees :%d", advancedConfig.unlockedToLockedTransitionOffsetDegrees);
  log_d("lockNgoTimeout :%d", advancedConfig.lockNgoTimeout);
  log_d("singleButtonPressAction :%d", advancedConfig.singleButtonPressAction);
  log_d("doubleButtonPressAction :%d", advancedConfig.doubleButtonPressAction);
  log_d("detachedCylinder :%d", advancedConfig.detachedCylinder);
  log_d("batteryType :%d", advancedConfig.batteryType);
  log_d("automaticBatteryTypeDetection :%d", advancedConfig.automaticBatteryTypeDetection);
  log_d("unlatchDuration :%d", advancedConfig.unlatchDuration);
  log_d("autoLockTimeOut :%d", advancedConfig.autoLockTimeOut);
  log_d("autoUnLockDisabled :%d", advancedConfig.autoUnLockDisabled);
  log_d("nightModeEnabled :%d", advancedConfig.nightModeEnabled);
  log_d("nightModeStartTime Hour :%d", advancedConfig.nightModeStartTime[0]);
  log_d("nightModeStartTime Minute :%d", advancedConfig.nightModeStartTime[1]);
  log_d("nightModeEndTime Hour :%d", advancedConfig.nightModeEndTime[0]);
  log_d("nightModeEndTime Minute :%d", advancedConfig.nightModeEndTime[1]);
  log_d("nightModeAutoLockEnabled :%d", advancedConfig.nightModeAutoLockEnabled);
  log_d("nightModeAutoUnlockDisabled :%d", advancedConfig.nightModeAutoUnlockDisabled);
  log_d("nightModeImmediateLockOnStart :%d", advancedConfig.nightModeImmediateLockOnStart);
  log_d("autoLockEnabled :%d", advancedConfig.autoLockEnabled);
  log_d("immediateAutoLockEnabled :%d", advancedConfig.immediateAutoLockEnabled);
  log_d("autoUpdateEnabled :%d", advancedConfig.autoUpdateEnabled);
  #endif
}

void logNewAdvancedConfig(NewAdvancedConfig newAdvancedConfig) {
  #ifdef DEBUG_NUKI_READABLE_DATA
  log_d("unlockedPositionOffsetDegrees :%d", newAdvancedConfig.unlockedPositionOffsetDegrees);
  log_d("lockedPositionOffsetDegrees :%f", newAdvancedConfig.lockedPositionOffsetDegrees);
  log_d("singleLockedPositionOffsetDegrees :%f", newAdvancedConfig.singleLockedPositionOffsetDegrees);
  log_d("unlockedToLockedTransitionOffsetDegrees :%d", newAdvancedConfig.unlockedToLockedTransitionOffsetDegrees);
  log_d("lockNgoTimeout :%d", newAdvancedConfig.lockNgoTimeout);
  log_d("singleButtonPressAction :%d", newAdvancedConfig.singleButtonPressAction);
  log_d("doubleButtonPressAction :%d", newAdvancedConfig.doubleButtonPressAction);
  log_d("detachedCylinder :%d", newAdvancedConfig.detachedCylinder);
  log_d("batteryType :%d", newAdvancedConfig.batteryType);
  log_d("automaticBatteryTypeDetection :%d", newAdvancedConfig.automaticBatteryTypeDetection);
  log_d("unlatchDuration :%d", newAdvancedConfig.unlatchDuration);
  log_d("autoUnLockTimeOut :%d", newAdvancedConfig.autoLockTimeOut);
  log_d("autoUnLockDisabled :%d", newAdvancedConfig.autoUnLockDisabled);
  log_d("nightModeEnabled :%d", newAdvancedConfig.nightModeEnabled);
  log_d("nightModeStartTime Hour :%d", newAdvancedConfig.nightModeStartTime[0]);
  log_d("nightModeStartTime Minute :%d", newAdvancedConfig.nightModeStartTime[1]);
  log_d("nightModeEndTime Hour :%d", newAdvancedConfig.nightModeEndTime[0]);
  log_d("nightModeEndTime Minute :%d", newAdvancedConfig.nightModeEndTime[1]);
  log_d("nightModeAutoLockEnabled :%d", newAdvancedConfig.nightModeAutoLockEnabled);
  log_d("nightModeAutoUnlockDisabled :%d", newAdvancedConfig.nightModeAutoUnlockDisabled);
  log_d("nightModeImmediateLockOnStart :%d", newAdvancedConfig.nightModeImmediateLockOnStart);
  log_d("autoLockEnabled :%d", newAdvancedConfig.autoLockEnabled);
  log_d("immediateAutoLockEnabled :%d", newAdvancedConfig.immediateAutoLockEnabled);
  log_d("autoUpdateEnabled :%d", newAdvancedConfig.autoUpdateEnabled);
  #endif
}

} // namespace Nuki
