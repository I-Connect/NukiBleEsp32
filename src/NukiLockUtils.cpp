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

void logLockErrorCode(uint8_t errorCode, bool debug, Print* Log) {
  switch (errorCode) {
    case (uint8_t)ErrorCode::ERROR_BAD_CRC :
      logMessage("ERROR_BAD_CRC", Log, 1);
      break;
    case (uint8_t)ErrorCode::ERROR_BAD_LENGTH :
      logMessage("ERROR_BAD_LENGTH", Log, 1);
      break;
    case (uint8_t)ErrorCode::ERROR_UNKNOWN :
      logMessage("ERROR_UNKNOWN", Log, 1);
      break;
    case (uint8_t)ErrorCode::P_ERROR_NOT_PAIRING :
      logMessage("P_ERROR_NOT_PAIRING", Log, 1);
      break;
    case (uint8_t)ErrorCode::P_ERROR_BAD_AUTHENTICATOR :
      logMessage("P_ERROR_BAD_AUTHENTICATOR", Log, 1);
      break;
    case (uint8_t)ErrorCode::P_ERROR_BAD_PARAMETER :
      logMessage("P_ERROR_BAD_PARAMETER", Log, 1);
      break;
    case (uint8_t)ErrorCode::P_ERROR_MAX_USER :
      logMessage("P_ERROR_MAX_USER", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_AUTO_UNLOCK_TOO_RECENT :
      logMessage("K_ERROR_AUTO_UNLOCK_TOO_RECENT", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_BAD_NONCE :
      logMessage("K_ERROR_BAD_NONCE", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_BAD_PARAMETER :
      logMessage("K_ERROR_BAD_PARAMETER", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_BAD_PIN :
      logMessage("K_ERROR_BAD_PIN", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_BUSY :
      logMessage("K_ERROR_BUSY", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_CANCELED :
      logMessage("K_ERROR_CANCELED", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_CLUTCH_FAILURE :
      logMessage("K_ERROR_CLUTCH_FAILURE", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_CLUTCH_POWER_FAILURE :
      logMessage("K_ERROR_CLUTCH_POWER_FAILURE", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_CODE_ALREADY_EXISTS :
      logMessage("K_ERROR_CODE_ALREADY_EXISTS", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_CODE_INVALID :
      logMessage("K_ERROR_CODE_INVALID", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_1 :
      logMessage("K_ERROR_CODE_INVALID_TIMEOUT_1", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_2 :
      logMessage("K_ERROR_CODE_INVALID_TIMEOUT_2", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_3 :
      logMessage("K_ERROR_CODE_INVALID_TIMEOUT_3", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_DISABLED :
      logMessage("K_ERROR_DISABLED", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_FIRMWARE_UPDATE_NEEDED :
      logMessage("K_ERROR_FIRMWARE_UPDATE_NEEDED", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_INVALID_AUTH_ID :
      logMessage("K_ERROR_INVALID_AUTH_ID", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_MOTOR_BLOCKED :
      logMessage("K_ERROR_MOTOR_BLOCKED", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_MOTOR_LOW_VOLTAGE :
      logMessage("K_ERROR_MOTOR_LOW_VOLTAGE", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_MOTOR_POSITION_LIMIT :
      logMessage("K_ERROR_MOTOR_POSITION_LIMIT", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_MOTOR_POWER_FAILURE :
      logMessage("K_ERROR_MOTOR_POWER_FAILURE", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_MOTOR_TIMEOUT :
      logMessage("K_ERROR_MOTOR_TIMEOUT", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_NOT_AUTHORIZED :
      logMessage("K_ERROR_NOT_AUTHORIZED", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_NOT_CALIBRATED :
      logMessage("K_ERROR_NOT_CALIBRATED", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_POSITION_UNKNOWN :
      logMessage("K_ERROR_POSITION_UNKNOWN", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_REMOTE_NOT_ALLOWED :
      logMessage("K_ERROR_REMOTE_NOT_ALLOWED", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_TIME_NOT_ALLOWED :
      logMessage("K_ERROR_TIME_NOT_ALLOWED", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_TOO_MANY_ENTRIES :
      logMessage("K_ERROR_TOO_MANY_ENTRIES", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_TOO_MANY_PIN_ATTEMPTS :
      logMessage("K_ERROR_TOO_MANY_PIN_ATTEMPTS", Log, 1);
      break;
    case (uint8_t)ErrorCode::K_ERROR_VOLTAGE_TOO_LOW :
      logMessage("K_ERROR_VOLTAGE_TOO_LOW", Log, 1);
      break;
    default:
      logMessage("UNDEFINED ERROR", Log, 1);
  }
}

void logConfig(Config config, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("nukiId: %d", (unsigned int)config.nukiId, Log, 4);
    logMessageVar("name: %s", (const char*)config.name, Log, 4);
    logMessage("latitude : Not reported for privacy", Log, 4);
    logMessage("longitude : Not reported for privacy", Log, 4);
    //logMessageVar("latitude: %f", (const float)config.latitude, Log, 4);
    //logMessageVar("longitude: %f", (const float)config.longitude, Log, 4);
    logMessageVar("autoUnlatch: %d", (unsigned int)config.autoUnlatch, Log, 4);
    logMessageVar("pairingEnabled: %d", (unsigned int)config.pairingEnabled, Log, 4);
    logMessageVar("buttonEnabled: %d", (unsigned int)config.buttonEnabled, Log, 4);
    logMessageVar("ledEnabled: %d", (unsigned int)config.ledEnabled, Log, 4);
    logMessageVar("ledBrightness: %d", (unsigned int)config.ledBrightness, Log, 4);
    logMessageVar("currentTime Year: %d", (unsigned int)config.currentTimeYear, Log, 4);
    logMessageVar("currentTime Month: %d", (unsigned int)config.currentTimeMonth, Log, 4);
    logMessageVar("currentTime Day: %d", (unsigned int)config.currentTimeDay, Log, 4);
    logMessageVar("currentTime Hour: %d", (unsigned int)config.currentTimeHour, Log, 4);
    logMessageVar("currentTime Minute: %d", (unsigned int)config.currentTimeMinute, Log, 4);
    logMessageVar("currentTime Second: %d", (unsigned int)config.currentTimeSecond, Log, 4);
    logMessageVar("timeZoneOffset: %d", (unsigned int)config.timeZoneOffset, Log, 4);
    logMessageVar("dstMode: %d", (unsigned int)config.dstMode, Log, 4);
    logMessageVar("hasFob: %d", (unsigned int)config.hasFob, Log, 4);
    logMessageVar("fobAction1: %d", (unsigned int)config.fobAction1, Log, 4);
    logMessageVar("fobAction2: %d", (unsigned int)config.fobAction2, Log, 4);
    logMessageVar("fobAction3: %d", (unsigned int)config.fobAction3, Log, 4);
    logMessageVar("singleLock: %d", (unsigned int)config.singleLock, Log, 4);
    logMessageVar("advertisingMode: %d", (unsigned int)config.advertisingMode, Log, 4);
    logMessageVar("hasKeypad: %d", (unsigned int)config.hasKeypad, Log, 4);
    if (Log == nullptr) {
      log_d("firmwareVersion: %d.%d.%d", config.firmwareVersion[0], config.firmwareVersion[1], config.firmwareVersion[2]);
      log_d("hardwareRevision: %d.%d", config.hardwareRevision[0], config.hardwareRevision[1]);
    }
    else
    {
      Log->printf("firmwareVersion: %d.%d.%d", config.firmwareVersion[0], config.firmwareVersion[1], config.firmwareVersion[2]);
      Log->println();
      Log->printf("hardwareRevision: %d.%d", config.hardwareRevision[0], config.hardwareRevision[1]);
      Log->println();
    }
    logMessageVar("homeKitStatus: %d", (unsigned int)config.homeKitStatus, Log, 4);
    logMessageVar("timeZoneId: %d", (unsigned int)config.timeZoneId, Log, 4);
    logMessageVar("deviceType: %d", (unsigned int)config.deviceType, Log, 4);
    logMessageVar("wifiCapable: %d", (unsigned int)config.capabilities & 1, Log, 4);
    logMessageVar("threadCapable: %d", (unsigned int)(((unsigned int)config.capabilities & 2) != 0 ? 1 : 0), Log, 4);
    logMessageVar("hasKeypadV2: %d", (unsigned int)config.hasKeypadV2, Log, 4);
    logMessageVar("matterStatus: %d", (unsigned int)config.matterStatus, Log, 4);
    logMessageVar("productVariant: %d", (unsigned int)config.productVariant, Log, 4);
  }
}

void logNewConfig(NewConfig newConfig, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("name: %s", (const char*)newConfig.name, Log, 4);
    logMessageVar("latitude: %f", (const float)newConfig.latitude, Log, 4);
    logMessageVar("longitude: %f", (const float)newConfig.longitude, Log, 4);
    logMessageVar("autoUnlatch: %d", (unsigned int)newConfig.autoUnlatch, Log, 4);
    logMessageVar("pairingEnabled: %d", (unsigned int)newConfig.pairingEnabled, Log, 4);
    logMessageVar("buttonEnabled: %d", (unsigned int)newConfig.buttonEnabled, Log, 4);
    logMessageVar("ledEnabled: %d", (unsigned int)newConfig.ledEnabled, Log, 4);
    logMessageVar("ledBrightness: %d", (unsigned int)newConfig.ledBrightness, Log, 4);
    logMessageVar("timeZoneOffset: %d", (unsigned int)newConfig.timeZoneOffset, Log, 4);
    logMessageVar("dstMode: %d", (unsigned int)newConfig.dstMode, Log, 4);
    logMessageVar("fobAction1: %d", (unsigned int)newConfig.fobAction1, Log, 4);
    logMessageVar("fobAction2: %d", (unsigned int)newConfig.fobAction2, Log, 4);
    logMessageVar("fobAction3: %d", (unsigned int)newConfig.fobAction3, Log, 4);
    logMessageVar("singleLock: %d", (unsigned int)newConfig.singleLock, Log, 4);
    logMessageVar("advertisingMode: %d", (unsigned int)newConfig.advertisingMode, Log, 4);
    logMessageVar("timeZoneId: %d", (unsigned int)newConfig.timeZoneId, Log, 4);
  }
}

void logNewKeypadEntry(NewKeypadEntry newKeypadEntry, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("code:%d", (unsigned int)newKeypadEntry.code, Log, 4);
    logMessageVar("name:%s", (const char*)newKeypadEntry.name, Log, 4);
    logMessageVar("timeLimited:%d", (unsigned int)newKeypadEntry.timeLimited, Log, 4);
    logMessageVar("allowedFromYear:%d", (unsigned int)newKeypadEntry.allowedFromYear, Log, 4);
    logMessageVar("allowedFromMonth:%d", (unsigned int)newKeypadEntry.allowedFromMonth, Log, 4);
    logMessageVar("allowedFromDay:%d", (unsigned int)newKeypadEntry.allowedFromDay, Log, 4);
    logMessageVar("allowedFromHour:%d", (unsigned int)newKeypadEntry.allowedFromHour, Log, 4);
    logMessageVar("allowedFromMin:%d", (unsigned int)newKeypadEntry.allowedFromMin, Log, 4);
    logMessageVar("allowedFromSec:%d", (unsigned int)newKeypadEntry.allowedFromSec, Log, 4);
    logMessageVar("allowedUntilYear:%d", (unsigned int)newKeypadEntry.allowedUntilYear, Log, 4);
    logMessageVar("allowedUntilMonth:%d", (unsigned int)newKeypadEntry.allowedUntilMonth, Log, 4);
    logMessageVar("allowedUntilDay:%d", (unsigned int)newKeypadEntry.allowedUntilDay, Log, 4);
    logMessageVar("allowedUntilHour:%d", (unsigned int)newKeypadEntry.allowedUntilHour, Log, 4);
    logMessageVar("allowedUntilMin:%d", (unsigned int)newKeypadEntry.allowedUntilMin, Log, 4);
    logMessageVar("allowedUntilSec:%d", (unsigned int)newKeypadEntry.allowedUntilSec, Log, 4);
    logMessageVar("allowedWeekdays:%d", (unsigned int)newKeypadEntry.allowedWeekdays, Log, 4);
    logMessageVar("allowedFromTimeHour:%d", (unsigned int)newKeypadEntry.allowedFromTimeHour, Log, 4);
    logMessageVar("allowedFromTimeMin:%d", (unsigned int)newKeypadEntry.allowedFromTimeMin, Log, 4);
    logMessageVar("allowedUntilTimeHour:%d", (unsigned int)newKeypadEntry.allowedUntilTimeHour, Log, 4);
    logMessageVar("allowedUntilTimeMin:%d", (unsigned int)newKeypadEntry.allowedUntilTimeMin, Log, 4);
  }
}

void logKeypadEntry(KeypadEntry keypadEntry, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("codeId:%d", (unsigned int)keypadEntry.codeId, Log, 4);
    logMessageVar("code:%d", (unsigned int)keypadEntry.code, Log, 4);
    logMessageVar("name:%s", (const char*)keypadEntry.name, Log, 4);
    logMessageVar("enabled:%d", (unsigned int)keypadEntry.enabled, Log, 4);
    logMessageVar("dateCreatedYear:%d", (unsigned int)keypadEntry.dateCreatedYear, Log, 4);
    logMessageVar("dateCreatedMonth:%d", (unsigned int)keypadEntry.dateCreatedMonth, Log, 4);
    logMessageVar("dateCreatedDay:%d", (unsigned int)keypadEntry.dateCreatedDay, Log, 4);
    logMessageVar("dateCreatedHour:%d", (unsigned int)keypadEntry.dateCreatedHour, Log, 4);
    logMessageVar("dateCreatedMin:%d", (unsigned int)keypadEntry.dateCreatedMin, Log, 4);
    logMessageVar("dateCreatedSec:%d", (unsigned int)keypadEntry.dateCreatedSec, Log, 4);
    logMessageVar("dateLastActiveYear:%d", (unsigned int)keypadEntry.dateLastActiveYear, Log, 4);
    logMessageVar("dateLastActiveMonth:%d", (unsigned int)keypadEntry.dateLastActiveMonth, Log, 4);
    logMessageVar("dateLastActiveDay:%d", (unsigned int)keypadEntry.dateLastActiveDay, Log, 4);
    logMessageVar("dateLastActiveHour:%d", (unsigned int)keypadEntry.dateLastActiveHour, Log, 4);
    logMessageVar("dateLastActiveMin:%d", (unsigned int)keypadEntry.dateLastActiveMin, Log, 4);
    logMessageVar("dateLastActiveSec:%d", (unsigned int)keypadEntry.dateLastActiveSec, Log, 4);
    logMessageVar("lockCount:%d", (unsigned int)keypadEntry.lockCount, Log, 4);
    logMessageVar("timeLimited:%d", (unsigned int)keypadEntry.timeLimited, Log, 4);
    logMessageVar("allowedFromYear:%d", (unsigned int)keypadEntry.allowedFromYear, Log, 4);
    logMessageVar("allowedFromMonth:%d", (unsigned int)keypadEntry.allowedFromMonth, Log, 4);
    logMessageVar("allowedFromDay:%d", (unsigned int)keypadEntry.allowedFromDay, Log, 4);
    logMessageVar("allowedFromHour:%d", (unsigned int)keypadEntry.allowedFromHour, Log, 4);
    logMessageVar("allowedFromMin:%d", (unsigned int)keypadEntry.allowedFromMin, Log, 4);
    logMessageVar("allowedFromSec:%d", (unsigned int)keypadEntry.allowedFromSec, Log, 4);
    logMessageVar("allowedUntilYear:%d", (unsigned int)keypadEntry.allowedUntilYear, Log, 4);
    logMessageVar("allowedUntilMonth:%d", (unsigned int)keypadEntry.allowedUntilMonth, Log, 4);
    logMessageVar("allowedUntilDay:%d", (unsigned int)keypadEntry.allowedUntilDay, Log, 4);
    logMessageVar("allowedUntilHour:%d", (unsigned int)keypadEntry.allowedUntilHour, Log, 4);
    logMessageVar("allowedUntilMin:%d", (unsigned int)keypadEntry.allowedUntilMin, Log, 4);
    logMessageVar("allowedUntilSec:%d", (unsigned int)keypadEntry.allowedUntilSec, Log, 4);
    logMessageVar("allowedWeekdays:%d", (unsigned int)keypadEntry.allowedWeekdays, Log, 4);
    logMessageVar("allowedFromTimeHour:%d", (unsigned int)keypadEntry.allowedFromTimeHour, Log, 4);
    logMessageVar("allowedFromTimeMin:%d", (unsigned int)keypadEntry.allowedFromTimeMin, Log, 4);
    logMessageVar("allowedUntilTimeHour:%d", (unsigned int)keypadEntry.allowedUntilTimeHour, Log, 4);
    logMessageVar("allowedUntilTimeMin:%d", (unsigned int)keypadEntry.allowedUntilTimeMin, Log, 4);
  }
}

void logUpdatedKeypadEntry(UpdatedKeypadEntry updatedKeypadEntry, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("codeId:%d", (unsigned int)updatedKeypadEntry.codeId, Log, 4);
    logMessageVar("code:%d", (unsigned int)updatedKeypadEntry.code, Log, 4);
    logMessageVar("name:%s", (const char*)updatedKeypadEntry.name, Log, 4);
    logMessageVar("enabled:%d", (unsigned int)updatedKeypadEntry.enabled, Log, 4);
    logMessageVar("timeLimited:%d", (unsigned int)updatedKeypadEntry.timeLimited, Log, 4);
    logMessageVar("allowedFromYear:%d", (unsigned int)updatedKeypadEntry.allowedFromYear, Log, 4);
    logMessageVar("allowedFromMonth:%d", (unsigned int)updatedKeypadEntry.allowedFromMonth, Log, 4);
    logMessageVar("allowedFromDay:%d", (unsigned int)updatedKeypadEntry.allowedFromDay, Log, 4);
    logMessageVar("allowedFromHour:%d", (unsigned int)updatedKeypadEntry.allowedFromHour, Log, 4);
    logMessageVar("allowedFromMin:%d", (unsigned int)updatedKeypadEntry.allowedFromMin, Log, 4);
    logMessageVar("allowedFromSec:%d", (unsigned int)updatedKeypadEntry.allowedFromSec, Log, 4);
    logMessageVar("allowedUntilYear:%d", (unsigned int)updatedKeypadEntry.allowedUntilYear, Log, 4);
    logMessageVar("allowedUntilMonth:%d", (unsigned int)updatedKeypadEntry.allowedUntilMonth, Log, 4);
    logMessageVar("allowedUntilDay:%d", (unsigned int)updatedKeypadEntry.allowedUntilDay, Log, 4);
    logMessageVar("allowedUntilHour:%d", (unsigned int)updatedKeypadEntry.allowedUntilHour, Log, 4);
    logMessageVar("allowedUntilMin:%d", (unsigned int)updatedKeypadEntry.allowedUntilMin, Log, 4);
    logMessageVar("allowedUntilSec:%d", (unsigned int)updatedKeypadEntry.allowedUntilSec, Log, 4);
    logMessageVar("allowedWeekdays:%d", (unsigned int)updatedKeypadEntry.allowedWeekdays, Log, 4);
    logMessageVar("allowedFromTimeHour:%d", (unsigned int)updatedKeypadEntry.allowedFromTimeHour, Log, 4);
    logMessageVar("allowedFromTimeMin:%d", (unsigned int)updatedKeypadEntry.allowedFromTimeMin, Log, 4);
    logMessageVar("allowedUntilTimeHour:%d", (unsigned int)updatedKeypadEntry.allowedUntilTimeHour, Log, 4);
    logMessageVar("allowedUntilTimeMin:%d", (unsigned int)updatedKeypadEntry.allowedUntilTimeMin, Log, 4);
  }
}

void logAuthorizationEntry(AuthorizationEntry authorizationEntry, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("id:%d", (unsigned int)authorizationEntry.authId, Log, 4);
    logMessageVar("idType:%d", (unsigned int)authorizationEntry.idType, Log, 4);
    logMessageVar("name:%s", (const char*)authorizationEntry.name, Log, 4);
    logMessageVar("enabled:%d", (unsigned int)authorizationEntry.enabled, Log, 4);
    logMessageVar("remoteAllowed:%d", (unsigned int)authorizationEntry.remoteAllowed, Log, 4);
    logMessageVar("createdYear:%d", (unsigned int)authorizationEntry.createdYear, Log, 4);
    logMessageVar("createdMonth:%d", (unsigned int)authorizationEntry.createdMonth, Log, 4);
    logMessageVar("createdDay:%d", (unsigned int)authorizationEntry.createdDay, Log, 4);
    logMessageVar("createdHour:%d", (unsigned int)authorizationEntry.createdHour, Log, 4);
    logMessageVar("createdMin:%d", (unsigned int)authorizationEntry.createdMinute, Log, 4);
    logMessageVar("createdSec:%d", (unsigned int)authorizationEntry.createdSecond, Log, 4);
    logMessageVar("lastactYear:%d", (unsigned int)authorizationEntry.lastActYear, Log, 4);
    logMessageVar("lastactMonth:%d", (unsigned int)authorizationEntry.lastActMonth, Log, 4);
    logMessageVar("lastactDay:%d", (unsigned int)authorizationEntry.lastActDay, Log, 4);
    logMessageVar("lastactHour:%d", (unsigned int)authorizationEntry.lastActHour, Log, 4);
    logMessageVar("lastactMin:%d", (unsigned int)authorizationEntry.lastActMinute, Log, 4);
    logMessageVar("lastactSec:%d", (unsigned int)authorizationEntry.lastActSecond, Log, 4);
    logMessageVar("lockCount:%d", (unsigned int)authorizationEntry.lockCount, Log, 4);
    logMessageVar("timeLimited:%d", (unsigned int)authorizationEntry.timeLimited, Log, 4);
    logMessageVar("allowedFromYear:%d", (unsigned int)authorizationEntry.allowedFromYear, Log, 4);
    logMessageVar("allowedFromMonth:%d", (unsigned int)authorizationEntry.allowedFromMonth, Log, 4);
    logMessageVar("allowedFromDay:%d", (unsigned int)authorizationEntry.allowedFromDay, Log, 4);
    logMessageVar("allowedFromHour:%d", (unsigned int)authorizationEntry.allowedFromHour, Log, 4);
    logMessageVar("allowedFromMin:%d", (unsigned int)authorizationEntry.allowedFromMinute, Log, 4);
    logMessageVar("allowedFromSec:%d", (unsigned int)authorizationEntry.allowedFromSecond, Log, 4);
    logMessageVar("allowedUntilYear:%d", (unsigned int)authorizationEntry.allowedUntilYear, Log, 4);
    logMessageVar("allowedUntilMonth:%d", (unsigned int)authorizationEntry.allowedUntilMonth, Log, 4);
    logMessageVar("allowedUntilDay:%d", (unsigned int)authorizationEntry.allowedUntilDay, Log, 4);
    logMessageVar("allowedUntilHour:%d", (unsigned int)authorizationEntry.allowedUntilHour, Log, 4);
    logMessageVar("allowedUntilMin:%d", (unsigned int)authorizationEntry.allowedUntilMinute, Log, 4);
    logMessageVar("allowedUntilSec:%d", (unsigned int)authorizationEntry.allowedUntilSecond, Log, 4);
    logMessageVar("allowedWeekdays:%d", (unsigned int)authorizationEntry.allowedWeekdays, Log, 4);
    logMessageVar("allowedFromTimeHour:%d", (unsigned int)authorizationEntry.allowedFromTimeHour, Log, 4);
    logMessageVar("allowedFromTimeMin:%d", (unsigned int)authorizationEntry.allowedFromTimeMin, Log, 4);
    logMessageVar("allowedUntilTimeHour:%d", (unsigned int)authorizationEntry.allowedUntilTimeHour, Log, 4);
    logMessageVar("allowedUntilTimeMin:%d", (unsigned int)authorizationEntry.allowedUntilTimeMin, Log, 4);
  }
}

void logNewAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("name:%s", (const char*)newAuthorizationEntry.name, Log, 4);
    logMessageVar("idType:%d", (unsigned int)newAuthorizationEntry.idType, Log, 4);
    logMessageVar("remoteAllowed:%d", (unsigned int)newAuthorizationEntry.remoteAllowed, Log, 4);
    logMessageVar("timeLimited:%d", (unsigned int)newAuthorizationEntry.timeLimited, Log, 4);
    logMessageVar("allowedFromYear:%d", (unsigned int)newAuthorizationEntry.allowedFromYear, Log, 4);
    logMessageVar("allowedFromMonth:%d", (unsigned int)newAuthorizationEntry.allowedFromMonth, Log, 4);
    logMessageVar("allowedFromDay:%d", (unsigned int)newAuthorizationEntry.allowedFromDay, Log, 4);
    logMessageVar("allowedFromHour:%d", (unsigned int)newAuthorizationEntry.allowedFromHour, Log, 4);
    logMessageVar("allowedFromMin:%d", (unsigned int)newAuthorizationEntry.allowedFromMinute, Log, 4);
    logMessageVar("allowedFromSec:%d", (unsigned int)newAuthorizationEntry.allowedFromSecond, Log, 4);
    logMessageVar("allowedUntilYear:%d", (unsigned int)newAuthorizationEntry.allowedUntilYear, Log, 4);
    logMessageVar("allowedUntilMonth:%d", (unsigned int)newAuthorizationEntry.allowedUntilMonth, Log, 4);
    logMessageVar("allowedUntilDay:%d", (unsigned int)newAuthorizationEntry.allowedUntilDay, Log, 4);
    logMessageVar("allowedUntilHour:%d", (unsigned int)newAuthorizationEntry.allowedUntilHour, Log, 4);
    logMessageVar("allowedUntilMin:%d", (unsigned int)newAuthorizationEntry.allowedUntilMinute, Log, 4);
    logMessageVar("allowedUntilSec:%d", (unsigned int)newAuthorizationEntry.allowedUntilSecond, Log, 4);
    logMessageVar("allowedWeekdays:%d", (unsigned int)newAuthorizationEntry.allowedWeekdays, Log, 4);
    logMessageVar("allowedFromTimeHour:%d", (unsigned int)newAuthorizationEntry.allowedFromTimeHour, Log, 4);
    logMessageVar("allowedFromTimeMin:%d", (unsigned int)newAuthorizationEntry.allowedFromTimeMin, Log, 4);
    logMessageVar("allowedUntilTimeHour:%d", (unsigned int)newAuthorizationEntry.allowedUntilTimeHour, Log, 4);
    logMessageVar("allowedUntilTimeMin:%d", (unsigned int)newAuthorizationEntry.allowedUntilTimeMin, Log, 4);
  }
}

void logUpdatedAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("id:%d", (unsigned int)updatedAuthorizationEntry.authId, Log, 4);
    logMessageVar("name:%s", (const char*)updatedAuthorizationEntry.name, Log, 4);
    logMessageVar("enabled:%d", (unsigned int)updatedAuthorizationEntry.enabled, Log, 4);
    logMessageVar("remoteAllowed:%d", (unsigned int)updatedAuthorizationEntry.remoteAllowed, Log, 4);
    logMessageVar("timeLimited:%d", (unsigned int)updatedAuthorizationEntry.timeLimited, Log, 4);
    logMessageVar("allowedFromYear:%d", (unsigned int)updatedAuthorizationEntry.allowedFromYear, Log, 4);
    logMessageVar("allowedFromMonth:%d", (unsigned int)updatedAuthorizationEntry.allowedFromMonth, Log, 4);
    logMessageVar("allowedFromDay:%d", (unsigned int)updatedAuthorizationEntry.allowedFromDay, Log, 4);
    logMessageVar("allowedFromHour:%d", (unsigned int)updatedAuthorizationEntry.allowedFromHour, Log, 4);
    logMessageVar("allowedFromMin:%d", (unsigned int)updatedAuthorizationEntry.allowedFromMinute, Log, 4);
    logMessageVar("allowedFromSec:%d", (unsigned int)updatedAuthorizationEntry.allowedFromSecond, Log, 4);
    logMessageVar("allowedUntilYear:%d", (unsigned int)updatedAuthorizationEntry.allowedUntilYear, Log, 4);
    logMessageVar("allowedUntilMonth:%d", (unsigned int)updatedAuthorizationEntry.allowedUntilMonth, Log, 4);
    logMessageVar("allowedUntilDay:%d", (unsigned int)updatedAuthorizationEntry.allowedUntilDay, Log, 4);
    logMessageVar("allowedUntilHour:%d", (unsigned int)updatedAuthorizationEntry.allowedUntilHour, Log, 4);
    logMessageVar("allowedUntilMin:%d", (unsigned int)updatedAuthorizationEntry.allowedUntilMinute, Log, 4);
    logMessageVar("allowedUntilSec:%d", (unsigned int)updatedAuthorizationEntry.allowedUntilSecond, Log, 4);
    logMessageVar("allowedWeekdays:%d", (unsigned int)updatedAuthorizationEntry.allowedWeekdays, Log, 4);
    logMessageVar("allowedFromTimeHour:%d", (unsigned int)updatedAuthorizationEntry.allowedFromTimeHour, Log, 4);
    logMessageVar("allowedFromTimeMin:%d", (unsigned int)updatedAuthorizationEntry.allowedFromTimeMin, Log, 4);
    logMessageVar("allowedUntilTimeHour:%d", (unsigned int)updatedAuthorizationEntry.allowedUntilTimeHour, Log, 4);
    logMessageVar("allowedUntilTimeMin:%d", (unsigned int)updatedAuthorizationEntry.allowedUntilTimeMin, Log, 4);
  }
}

void logNewTimeControlEntry(NewTimeControlEntry newTimeControlEntry, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("weekdays:%d", (unsigned int)newTimeControlEntry.weekdays, Log, 4);
    if (Log == nullptr) {
      log_d("time:%d:%d", (unsigned int)newTimeControlEntry.timeHour, newTimeControlEntry.timeMin);
    }
    else
    {
      Log->printf("time:%d:%d", (unsigned int)newTimeControlEntry.timeHour, newTimeControlEntry.timeMin);
      Log->println();
    }
    logMessageVar("lockAction:%d", (unsigned int)newTimeControlEntry.lockAction, Log, 4);
  }
}

void logTimeControlEntry(TimeControlEntry timeControlEntry, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("entryId:%d", (unsigned int)timeControlEntry.entryId, Log, 4);
    logMessageVar("enabled:%d", (unsigned int)timeControlEntry.enabled, Log, 4);
    logMessageVar("weekdays:%d", (unsigned int)timeControlEntry.weekdays, Log, 4);
    if (Log == nullptr) {
      log_d("time:%d:%d", (unsigned int)timeControlEntry.timeHour, timeControlEntry.timeMin);
    }
    else
    {
      Log->printf("time:%d:%d", (unsigned int)timeControlEntry.timeHour, timeControlEntry.timeMin);
      Log->println();
    }
    logMessageVar("lockAction:%d", (unsigned int)timeControlEntry.lockAction, Log, 4);
  }
}

void logCompletionStatus(CompletionStatus completionStatus, bool debug, Print* Log) {
  switch (completionStatus) {
    case CompletionStatus::Busy :
      logMessage("Completion status: busy", Log, 4);
      break;
    case CompletionStatus::Canceled :
      logMessage("Completion status: canceled", Log, 4);
      break;
    case CompletionStatus::ClutchFailure :
      logMessage("Completion status: clutchFailure", Log, 4);
      break;
    case CompletionStatus::IncompleteFailure :
      logMessage("Completion status: incompleteFailure", Log, 4);
      break;
    case CompletionStatus::LowMotorVoltage :
      logMessage("Completion status: lowMotorVoltage", Log, 4);
      break;
    case CompletionStatus::MotorBlocked :
      logMessage("Completion status: motorBlocked", Log, 4);
      break;
    case CompletionStatus::MotorPowerFailure :
      logMessage("Completion status: motorPowerFailure", Log, 4);
      break;
    case CompletionStatus::OtherError :
      logMessage("Completion status: otherError", Log, 4);
      break;
    case CompletionStatus::Success :
      logMessage("Completion status: success", Log, 4);
      break;
    case CompletionStatus::TooRecent :
      logMessage("Completion status: tooRecent", Log, 4);
      break;
    case CompletionStatus::InvalidCode :
      logMessage("Completion status: invalid code", Log, 4);
      break;
    default:
      logMessage("Completion status: unknown", Log, 2);
      break;
  }
}

void logNukiTrigger(Trigger nukiTrigger, bool debug, Print* Log) {
  switch (nukiTrigger) {
    case Trigger::AutoLock :
      logMessage("Trigger: autoLock", Log, 4);
      break;
    case Trigger::Automatic :
      logMessage("Trigger: automatic", Log, 4);
      break;
    case Trigger::Button :
      logMessage("Trigger: button", Log, 4);
      break;
    case Trigger::Manual :
      logMessage("Trigger: manual", Log, 4);
      break;
    case Trigger::System :
      logMessage("Trigger: system", Log, 4);
      break;
    default:
      logMessage("Trigger: unknown", Log, 2);
      break;
  }
}

void logLockAction(LockAction lockAction, bool debug, Print* Log) {
  switch (lockAction) {
    case LockAction::FobAction1 :
      logMessage("action: autoLock", Log, 4);
      break;
    case LockAction::FobAction2 :
      logMessage("action: automatic", Log, 4);
      break;
    case LockAction::FobAction3 :
      logMessage("action: button", Log, 4);
      break;
    case LockAction::FullLock :
      logMessage("action: manual", Log, 4);
      break;
    case LockAction::Lock :
      logMessage("action: system", Log, 4);
      break;
    case LockAction::LockNgo :
      logMessage("action: system", Log, 4);
      break;
    case LockAction::LockNgoUnlatch :
      logMessage("action: system", Log, 4);
      break;
    case LockAction::Unlatch :
      logMessage("action: system", Log, 4);
      break;
    case LockAction::Unlock :
      logMessage("action: system", Log, 4);
      break;
    default:
      logMessage("action: unknown", Log, 2);
      break;
  }
}

void logKeyturnerState(KeyTurnerState keyTurnerState, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("nukiState: %02x", (unsigned int)keyTurnerState.nukiState, Log, 4);
    logMessageVar("lockState: %d", (unsigned int)keyTurnerState.lockState, Log, 4);
    logNukiTrigger(keyTurnerState.trigger, debug, Log);
    logMessageVar("currentTimeYear: %d", (unsigned int)keyTurnerState.currentTimeYear, Log, 4);
    logMessageVar("currentTimeMonth: %d", (unsigned int)keyTurnerState.currentTimeMonth, Log, 4);
    logMessageVar("currentTimeDay: %d", (unsigned int)keyTurnerState.currentTimeDay, Log, 4);
    logMessageVar("currentTimeHour: %d", (unsigned int)keyTurnerState.currentTimeHour, Log, 4);
    logMessageVar("currentTimeMinute: %d", (unsigned int)keyTurnerState.currentTimeMinute, Log, 4);
    logMessageVar("currentTimeSecond: %d", (unsigned int)keyTurnerState.currentTimeSecond, Log, 4);
    logMessageVar("timeZoneOffset: %d", (unsigned int)keyTurnerState.timeZoneOffset, Log, 4);
    logMessageVar("criticalBatteryState composed value: %d", (unsigned int)keyTurnerState.criticalBatteryState, Log, 4);
    logMessageVar("criticalBatteryState: %d", (unsigned int)(((unsigned int)keyTurnerState.criticalBatteryState) == 1 ? 1 : 0), Log, 4);
    logMessageVar("batteryCharging: %d", (unsigned int)(((unsigned int)keyTurnerState.criticalBatteryState & 2) == 2 ? 1 : 0), Log, 4);
    logMessageVar("batteryPercent: %d", (unsigned int)((keyTurnerState.criticalBatteryState & 0b11111100) >> 1), Log, 4);
    logMessageVar("configUpdateCount: %d", (unsigned int)keyTurnerState.configUpdateCount, Log, 4);
    logMessageVar("lockNgoTimer: %d", (unsigned int)keyTurnerState.lockNgoTimer, Log, 4);
    logLockAction((LockAction)keyTurnerState.lastLockAction, debug, Log);
    logMessageVar("lastLockActionTrigger: %d", (unsigned int)keyTurnerState.lastLockActionTrigger, Log, 4);
    logCompletionStatus(keyTurnerState.lastLockActionCompletionStatus, debug, Log);
    logMessageVar("doorSensorState: %d", (unsigned int)keyTurnerState.doorSensorState, Log, 4);
    logMessageVar("nightModeActive: %d", (unsigned int)keyTurnerState.nightModeActive, Log, 4);
    logMessageVar("accessoryBatteryState composed value: %d", (unsigned int)keyTurnerState.accessoryBatteryState, Log, 4);
    logMessageVar("Keypad bat critical feature supported: %d", (unsigned int)(((unsigned int)keyTurnerState.accessoryBatteryState & 1) == 1 ? 1 : 0), Log, 4);
    logMessageVar("Keypad Battery Critical: %d", (unsigned int)(((unsigned int)keyTurnerState.accessoryBatteryState & 3) == 3 ? 1 : 0), Log, 4);
    logMessageVar("Doorsensor bat critical feature supported: %d", (unsigned int)(((unsigned int)keyTurnerState.accessoryBatteryState & 4) == 4 ? 1 : 0), Log, 4);
    logMessageVar("Doorsensor Battery Critical: %d", (unsigned int)(((unsigned int)keyTurnerState.accessoryBatteryState & 12) == 12 ? 1 : 0), Log, 4);
    logMessageVar("remoteAccessStatus composed value: %d", (unsigned int)keyTurnerState.remoteAccessStatus, Log, 4);
    logMessageVar("remoteAccessEnabled: %d", (unsigned int)(((keyTurnerState.remoteAccessStatus & 1) == 1) ? 1 : 0), Log, 4);
    logMessageVar("bridgePaired: %d", (unsigned int)((((keyTurnerState.remoteAccessStatus >> 1) & 1) == 1) ? 1 : 0), Log, 4);
    logMessageVar("sseConnectedViaWifi: %d", (unsigned int)((((keyTurnerState.remoteAccessStatus >> 2) & 1) == 1) ? 1 : 0), Log, 4);
    logMessageVar("sseConnectionEstablished: %d", (unsigned int)((((keyTurnerState.remoteAccessStatus >> 3) & 1) == 1) ? 1 : 0), Log, 4);
    logMessageVar("isSseConnectedViaThread: %d", (unsigned int)((((keyTurnerState.remoteAccessStatus >> 4) & 1) == 1) ? 1 : 0), Log, 4);
    logMessageVar("threadSseUplinkEnabledByUser: %d", (unsigned int)((((keyTurnerState.remoteAccessStatus >> 5) & 1) == 1) ? 1 : 0), Log, 4);
    logMessageVar("nat64AvailableViaThread: %d", (unsigned int)((((keyTurnerState.remoteAccessStatus >> 6) & 1) == 1) ? 1 : 0), Log, 4);
    logMessageVar("bleConnectionStrength: %d", (unsigned int)keyTurnerState.bleConnectionStrength, Log, 4);
    logMessageVar("wifiConnectionStrength: %d", (unsigned int)keyTurnerState.wifiConnectionStrength, Log, 4);
    logMessageVar("wifiConnectionStatus composed value: %d", (unsigned int)keyTurnerState.wifiConnectionStatus, Log, 4);
    logMessageVar("wifiStatus: %d", (unsigned int)(keyTurnerState.wifiConnectionStatus & 3), Log, 4);
    logMessageVar("sseStatus: %d", (unsigned int)((keyTurnerState.wifiConnectionStatus >> 2) & 3), Log, 4);
    logMessageVar("wifiQuality: %d", (unsigned int)((keyTurnerState.wifiConnectionStatus >> 4) & 15), Log, 4);
    logMessageVar("mqttConnectionStatus composed value: %d", (unsigned int)keyTurnerState.mqttConnectionStatus, Log, 4);
    logMessageVar("mqttStatus: %d", (unsigned int)(keyTurnerState.mqttConnectionStatus & 3), Log, 4);
    logMessageVar("mqttConnectionChannel: %d", (unsigned int)((keyTurnerState.mqttConnectionStatus >> 2) & 1), Log, 4);
    logMessageVar("threadConnectionStatus composed value: %d", (unsigned int)keyTurnerState.threadConnectionStatus, Log, 4);
    logMessageVar("threadConnectionStatus: %d", (unsigned int)(keyTurnerState.threadConnectionStatus & 3), Log, 4);
    logMessageVar("threadSseStatus: %d", (unsigned int)((keyTurnerState.threadConnectionStatus >> 2) & 3), Log, 4);
    logMessageVar("isCommissioningModeActive: %d", (unsigned int)(((unsigned int)keyTurnerState.threadConnectionStatus & 16) != 0 ? 1 : 0), Log, 4);
    logMessageVar("isWifiDisabledBecauseOfThread: %d", (unsigned int)(((unsigned int)keyTurnerState.threadConnectionStatus & 32) != 0 ? 1 : 0), Log, 4);
  }
}

void logBatteryReport(BatteryReport batteryReport, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("batteryDrain:%d", (unsigned int)batteryReport.batteryDrain, Log, 4);
    logMessageVar("batteryVoltage:%d", (unsigned int)batteryReport.batteryVoltage, Log, 4);
    logMessageVar("criticalBatteryState:%d", (unsigned int)batteryReport.criticalBatteryState, Log, 4);
    logMessageVar("lockAction:%d", (unsigned int)batteryReport.lockAction, Log, 4);
    logMessageVar("startVoltage:%d", (unsigned int)batteryReport.startVoltage, Log, 4);
    logMessageVar("lowestVoltage:%d", (unsigned int)batteryReport.lowestVoltage, Log, 4);
    logMessageVar("lockDistance:%d", (unsigned int)batteryReport.lockDistance, Log, 4);
    logMessageVar("startTemperature:%d", (unsigned int)batteryReport.startTemperature, Log, 4);
    logMessageVar("maxTurnCurrent:%d", (unsigned int)batteryReport.maxTurnCurrent, Log, 4);
    logMessageVar("batteryResistance:%d", (unsigned int)batteryReport.batteryResistance, Log, 4);
  }
}

void logLogEntry(LogEntry logEntry, bool debug, Print* Log) {
  if (Log == nullptr) {
    log_d("[%d] type:%d authId:%d name: %s %d-%d-%d %d:%d:%d ", logEntry.index, logEntry.loggingType, logEntry.authId, logEntry.name, logEntry.timeStampYear, logEntry.timeStampMonth, logEntry.timeStampDay, logEntry.timeStampHour, logEntry.timeStampMinute, logEntry.timeStampSecond);
  }
  else
  {
    Log->printf("[%d] type:%d authId:%d name: %s %d-%d-%d %d:%d:%d ", logEntry.index, logEntry.loggingType, logEntry.authId, logEntry.name, logEntry.timeStampYear, logEntry.timeStampMonth, logEntry.timeStampDay, logEntry.timeStampHour, logEntry.timeStampMinute, logEntry.timeStampSecond);;
    Log->println();
  }

  switch (logEntry.loggingType) {
    case LoggingType::LoggingEnabled: {
      logMessageVar("Logging enabled: %d", (unsigned int)logEntry.data[0], Log, 4);
      break;
    }
    case LoggingType::LockAction:
    case LoggingType::Calibration:
    case LoggingType::InitializationRun: {
      logLockAction((LockAction)logEntry.data[0], debug, Log);
      logNukiTrigger((Trigger)logEntry.data[1], debug, Log);
      logMessageVar("Flags: %d", (unsigned int)logEntry.data[2], Log, 4);
      logCompletionStatus((CompletionStatus)logEntry.data[3], debug, Log);
      break;
    }
    case LoggingType::KeypadAction: {
      logLockAction((LockAction)logEntry.data[0], debug, Log);
      logMessageVar("Source: %d", (unsigned int)logEntry.data[1], Log, 4);
      logCompletionStatus((CompletionStatus)logEntry.data[2], debug, Log);
      uint16_t codeId = 0;
      memcpy(&codeId, &logEntry.data[3], 2);
      logMessageVar("Code id: %d", (unsigned int)codeId, Log, 4);
      break;
    }
    case LoggingType::DoorSensor: {
      if (logEntry.data[0] == 0x00) {
        logMessage("Door opened", Log, 4) ;
      }
      if (logEntry.data[0] == 0x01) {
        logMessage("Door closed", Log, 4) ;
      }
      if (logEntry.data[0] == 0x02) {
        logMessage("Door sensor jammed", Log, 4) ;
      }
      break;
    }
    case LoggingType::DoorSensorLoggingEnabled: {
      logMessageVar("Logging enabled: %d", (unsigned int)logEntry.data[0], Log, 4);
      break;
    }
    default:
      logMessage("Unknown logging type", Log, 2);
      break;
  }
}

void logAdvancedConfig(AdvancedConfig advancedConfig, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("totalDegrees: %d", (unsigned int)advancedConfig.totalDegrees, Log, 4);
    logMessageVar("unlockedPositionOffsetDegrees: %d", (unsigned int)advancedConfig.unlockedPositionOffsetDegrees, Log, 4);
    logMessageVar("lockedPositionOffsetDegrees: %f", (const float)advancedConfig.lockedPositionOffsetDegrees, Log, 4);
    logMessageVar("singleLockedPositionOffsetDegrees: %f", (const float)advancedConfig.singleLockedPositionOffsetDegrees, Log, 4);
    logMessageVar("unlockedToLockedTransitionOffsetDegrees: %d", (unsigned int)advancedConfig.unlockedToLockedTransitionOffsetDegrees, Log, 4);
    logMessageVar("lockNgoTimeout: %d", (unsigned int)advancedConfig.lockNgoTimeout, Log, 4);
    logMessageVar("singleButtonPressAction: %d", (unsigned int)advancedConfig.singleButtonPressAction, Log, 4);
    logMessageVar("doubleButtonPressAction: %d", (unsigned int)advancedConfig.doubleButtonPressAction, Log, 4);
    logMessageVar("detachedCylinder: %d", (unsigned int)advancedConfig.detachedCylinder, Log, 4);
    logMessageVar("batteryType: %d", (unsigned int)advancedConfig.batteryType, Log, 4);
    logMessageVar("automaticBatteryTypeDetection: %d", (unsigned int)advancedConfig.automaticBatteryTypeDetection, Log, 4);
    logMessageVar("unlatchDuration: %d", (unsigned int)advancedConfig.unlatchDuration, Log, 4);
    logMessageVar("autoLockTimeOut: %d", (unsigned int)advancedConfig.autoLockTimeOut, Log, 4);
    logMessageVar("autoUnLockDisabled: %d", (unsigned int)advancedConfig.autoUnLockDisabled, Log, 4);
    logMessageVar("nightModeEnabled: %d", (unsigned int)advancedConfig.nightModeEnabled, Log, 4);
    logMessageVar("nightModeStartTime Hour: %d", (unsigned int)advancedConfig.nightModeStartTime[0], Log, 4);
    logMessageVar("nightModeStartTime Minute: %d", (unsigned int)advancedConfig.nightModeStartTime[1], Log, 4);
    logMessageVar("nightModeEndTime Hour: %d", (unsigned int)advancedConfig.nightModeEndTime[0], Log, 4);
    logMessageVar("nightModeEndTime Minute: %d", (unsigned int)advancedConfig.nightModeEndTime[1], Log, 4);
    logMessageVar("nightModeAutoLockEnabled: %d", (unsigned int)advancedConfig.nightModeAutoLockEnabled, Log, 4);
    logMessageVar("nightModeAutoUnlockDisabled: %d", (unsigned int)advancedConfig.nightModeAutoUnlockDisabled, Log, 4);
    logMessageVar("nightModeImmediateLockOnStart: %d", (unsigned int)advancedConfig.nightModeImmediateLockOnStart, Log, 4);
    logMessageVar("autoLockEnabled: %d", (unsigned int)advancedConfig.autoLockEnabled, Log, 4);
    logMessageVar("immediateAutoLockEnabled: %d", (unsigned int)advancedConfig.immediateAutoLockEnabled, Log, 4);
    logMessageVar("autoUpdateEnabled: %d", (unsigned int)advancedConfig.autoUpdateEnabled, Log, 4);
    logMessageVar("motorSpeed: %d", (unsigned int)advancedConfig.motorSpeed, Log, 4);
    logMessageVar("enableSlowSpeedDuringNightMode: %d", (unsigned int)advancedConfig.enableSlowSpeedDuringNightMode, Log, 4);
  }
}

void logNewAdvancedConfig(NewAdvancedConfig newAdvancedConfig, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("unlockedPositionOffsetDegrees: %d", (unsigned int)newAdvancedConfig.unlockedPositionOffsetDegrees, Log, 4);
    logMessageVar("lockedPositionOffsetDegrees: %f", (const float)newAdvancedConfig.lockedPositionOffsetDegrees, Log, 4);
    logMessageVar("singleLockedPositionOffsetDegrees: %f", (const float)newAdvancedConfig.singleLockedPositionOffsetDegrees, Log, 4);
    logMessageVar("unlockedToLockedTransitionOffsetDegrees: %d", (unsigned int)newAdvancedConfig.unlockedToLockedTransitionOffsetDegrees, Log, 4);
    logMessageVar("lockNgoTimeout: %d", (unsigned int)newAdvancedConfig.lockNgoTimeout, Log, 4);
    logMessageVar("singleButtonPressAction: %d", (unsigned int)newAdvancedConfig.singleButtonPressAction, Log, 4);
    logMessageVar("doubleButtonPressAction: %d", (unsigned int)newAdvancedConfig.doubleButtonPressAction, Log, 4);
    logMessageVar("detachedCylinder: %d", (unsigned int)newAdvancedConfig.detachedCylinder, Log, 4);
    logMessageVar("batteryType: %d", (unsigned int)newAdvancedConfig.batteryType, Log, 4);
    logMessageVar("automaticBatteryTypeDetection: %d", (unsigned int)newAdvancedConfig.automaticBatteryTypeDetection, Log, 4);
    logMessageVar("unlatchDuration: %d", (unsigned int)newAdvancedConfig.unlatchDuration, Log, 4);
    logMessageVar("autoUnLockTimeOut: %d", (unsigned int)newAdvancedConfig.autoLockTimeOut, Log, 4);
    logMessageVar("autoUnLockDisabled: %d", (unsigned int)newAdvancedConfig.autoUnLockDisabled, Log, 4);
    logMessageVar("nightModeEnabled: %d", (unsigned int)newAdvancedConfig.nightModeEnabled, Log, 4);
    logMessageVar("nightModeStartTime Hour: %d", (unsigned int)newAdvancedConfig.nightModeStartTime[0], Log, 4);
    logMessageVar("nightModeStartTime Minute: %d", (unsigned int)newAdvancedConfig.nightModeStartTime[1], Log, 4);
    logMessageVar("nightModeEndTime Hour: %d", (unsigned int)newAdvancedConfig.nightModeEndTime[0], Log, 4);
    logMessageVar("nightModeEndTime Minute: %d", (unsigned int)newAdvancedConfig.nightModeEndTime[1], Log, 4);
    logMessageVar("nightModeAutoLockEnabled: %d", (unsigned int)newAdvancedConfig.nightModeAutoLockEnabled, Log, 4);
    logMessageVar("nightModeAutoUnlockDisabled: %d", (unsigned int)newAdvancedConfig.nightModeAutoUnlockDisabled, Log, 4);
    logMessageVar("nightModeImmediateLockOnStart: %d", (unsigned int)newAdvancedConfig.nightModeImmediateLockOnStart, Log, 4);
    logMessageVar("autoLockEnabled: %d", (unsigned int)newAdvancedConfig.autoLockEnabled, Log, 4);
    logMessageVar("immediateAutoLockEnabled: %d", (unsigned int)newAdvancedConfig.immediateAutoLockEnabled, Log, 4);
    logMessageVar("autoUpdateEnabled: %d", (unsigned int)newAdvancedConfig.autoUpdateEnabled, Log, 4);
  }
}

void logWifiScanEntry(WifiScanEntry wifiScanEntry, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("ssid: %s", (const char*)wifiScanEntry.ssid, Log, 4);
    logMessageVar("type: %d", (unsigned int)wifiScanEntry.type, Log, 4);
    logMessageVar("signal raw: %d", (unsigned int)wifiScanEntry.signal, Log, 4);
    logMessageVar("signal: %d", (unsigned int)(wifiScanEntry.signal & 255), Log, 4);
  }
}

void logMqttConfig(MqttConfig mqttConfig, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("enabled: %d", (unsigned int)mqttConfig.enabled, Log, 4);
    logMessageVar("hostName: %s", (const char*)mqttConfig.hostName, Log, 4);
    logMessageVar("userName: %s", (const char*)mqttConfig.userName, Log, 4);
    logMessageVar("secureConnection: %d", (unsigned int)mqttConfig.secureConnection, Log, 4);
    logMessageVar("autoDiscovery: %d", (unsigned int)mqttConfig.autoDiscovery, Log, 4);
    logMessageVar("lockingEnabled: %d", (unsigned int)mqttConfig.lockingEnabled, Log, 4);
  }
}

void logMqttConfigForMigration(MqttConfigForMigration mqttConfigForMigration, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("enabled: %d", (unsigned int)mqttConfigForMigration.enabled, Log, 4);
    logMessageVar("hostName: %s", (const char*)mqttConfigForMigration.hostName, Log, 4);
    logMessageVar("userName: %s", (const char*)mqttConfigForMigration.userName, Log, 4);
    logMessageVar("secureConnection: %d", (unsigned int)mqttConfigForMigration.secureConnection, Log, 4);
    logMessageVar("autoDiscovery: %d", (unsigned int)mqttConfigForMigration.autoDiscovery, Log, 4);
    logMessageVar("lockingEnabled: %d", (unsigned int)mqttConfigForMigration.lockingEnabled, Log, 4);
    logMessageVar("passphrase: %s", (const char*)mqttConfigForMigration.passphrase, Log, 4);
  }
}

void logWifiConfig(WifiConfig wifiConfig, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("serverBridgeId: %d", (unsigned int)wifiConfig.serverBridgeId, Log, 4);
    logMessageVar("wifiEnabled: %d", (unsigned int)wifiConfig.wifiEnabled, Log, 4);
    logMessageVar("wifiExpertSettings composed value: %d", (unsigned int)wifiConfig.wifiExpertSettings, Log, 4);
    logMessageVar("expertSettingsMode: %d", (unsigned int)(wifiConfig.wifiExpertSettings & 3), Log, 4);
    logMessageVar("broadcastFilterSettings: %d", (unsigned int)((wifiConfig.wifiExpertSettings >> 2) & 3), Log, 4);
    logMessageVar("dtimSkipSettings: %d", (unsigned int)((wifiConfig.wifiExpertSettings >> 4) & 7), Log, 4);
    logMessageVar("sseSkipSettings: %d", (unsigned int)((wifiConfig.wifiExpertSettings >> 7) & 7), Log, 4);
    logMessageVar("powersafeMode: %d", (unsigned int)((wifiConfig.wifiExpertSettings >> 10) & 3), Log, 4);
    logMessageVar("activePingEnabled: %d", (unsigned int)((wifiConfig.wifiExpertSettings >> 12) & 1), Log, 4);
  }
}

void logWifiConfigForMigration(WifiConfigForMigration wifiConfigForMigration, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("ssid: %s", (const char*)wifiConfigForMigration.ssid, Log, 4);
    logMessageVar("type: %d", (unsigned int)wifiConfigForMigration.type, Log, 4);
    logMessageVar("passphrase: %s", (const char*)wifiConfigForMigration.passphrase, Log, 4);
  }
}

void logKeypad2Config(Keypad2Config keypad2Config, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("updatePending: %d", (unsigned int)keypad2Config.updatePending, Log, 4);
    logMessageVar("ledBrightness: %d", (unsigned int)keypad2Config.ledBrightness, Log, 4);
    logMessageVar("batteryType: %d", (unsigned int)keypad2Config.batteryType, Log, 4);
    logMessageVar("buttonMode: %d", (unsigned int)keypad2Config.buttonMode, Log, 4);
    logMessageVar("lockAction: %d", (unsigned int)keypad2Config.lockAction, Log, 4);
  }
}

void logDoorSensorConfig(DoorSensorConfig doorSensorConfig, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("enabled: %d", (unsigned int)doorSensorConfig.enabled, Log, 4);
    logMessageVar("doorAjarTimeout: %d", (unsigned int)doorSensorConfig.doorAjarTimeout, Log, 4);
    logMessageVar("doorAjarLoggingEnabled: %d", (unsigned int)doorSensorConfig.doorAjarLoggingEnabled, Log, 4);
    logMessageVar("doorStatusMismatchLoggingEnabled: %d", (unsigned int)doorSensorConfig.doorStatusMismatchLoggingEnabled, Log, 4);
  }
}

void logFingerprintEntry(FingerprintEntry fingerprintEntry, bool debug, Print* Log) {
  if (debug) {
    String hexByte;
    char tmp[16];

    for (size_t i = 0; i < 32; i += 1) {
      sprintf(tmp, "%02x", fingerprintEntry.fingerprintId[i]);
      hexByte += strtol((const char*)tmp, nullptr, 16);
    }
    logMessageVar("fingerprintId:%s", (const char*)hexByte.c_str(), Log, 4);
    logMessageVar("keypadCodeId: %d", (unsigned int)fingerprintEntry.keypadCodeId, Log, 4);
    logMessageVar("name:%s", (const char*)fingerprintEntry.name, Log, 4);
  }
}

void logAccessoryInfo(AccessoryInfo accessoryInfo, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("dateYear: %d", (unsigned int)accessoryInfo.dateYear, Log, 4);
    logMessageVar("dateMonth: %d", (unsigned int)accessoryInfo.dateMonth, Log, 4);
    logMessageVar("dateDay: %d", (unsigned int)accessoryInfo.dateDay, Log, 4);
    logMessageVar("dateHour: %d", (unsigned int)accessoryInfo.dateHour, Log, 4);
    logMessageVar("dateMinute: %d", (unsigned int)accessoryInfo.dateMinute, Log, 4);
    logMessageVar("dateSecond: %d", (unsigned int)accessoryInfo.dateSecond, Log, 4);
    logMessageVar("accessoryNukiId: %d", (unsigned int)accessoryInfo.accessoryNukiId, Log, 4);
    logMessageVar("accessoryType: %d", (unsigned int)accessoryInfo.accessoryType, Log, 4);
    if (Log == nullptr) {
      log_d("firmwareVersion: %d.%d.%d", accessoryInfo.firmwareVersion[0], accessoryInfo.firmwareVersion[1], accessoryInfo.firmwareVersion[2]);
      log_d("hardwareRevision: %d.%d", accessoryInfo.hardwareRevision[0], accessoryInfo.hardwareRevision[1]);
    }
    else
    {
      Log->printf("firmwareVersion: %d.%d.%d", accessoryInfo.firmwareVersion[0], accessoryInfo.firmwareVersion[1], accessoryInfo.firmwareVersion[2]);
      Log->println();
      Log->printf("hardwareRevision: %d.%d", accessoryInfo.hardwareRevision[0], accessoryInfo.hardwareRevision[1]);
      Log->println();
    }
    logMessageVar("productVariantDifferentiator: %d", (unsigned int)accessoryInfo.productVariantDifferentiator, Log, 4);
    logMessageVar("mostRecentBatteryVoltage: %d", (unsigned int)accessoryInfo.mostRecentBatteryVoltage, Log, 4);
    logMessageVar("mostRecentTemperature: %d", (unsigned int)accessoryInfo.mostRecentTemperature, Log, 4);
  }
}

void logDailyStatistics(DailyStatistics dailyStatistics, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("dateYear: %d", (unsigned int)dailyStatistics.dateYear, Log, 4);
    logMessageVar("dateMonth: %d", (unsigned int)dailyStatistics.dateMonth, Log, 4);
    logMessageVar("dateDay: %d", (unsigned int)dailyStatistics.dateDay, Log, 4);
    //logMessageVar("dateHour: %d", (unsigned int)dailyStatistics.dateHour, Log, 4);
    //logMessageVar("dateMinute: %d", (unsigned int)dailyStatistics.dateMinute, Log, 4);
    //logMessageVar("dateSecond: %d", (unsigned int)dailyStatistics.dateSecond, Log, 4);
    logMessageVar("version: %d", (unsigned int)dailyStatistics.version, Log, 4);
    logMessageVar("countSuccessfulLockActions: %d", (unsigned int)dailyStatistics.countSuccessfulLockActions, Log, 4);
    logMessageVar("countErroneousLockActions: %d", (unsigned int)dailyStatistics.countErroneousLockActions, Log, 4);
    logMessageVar("avgCurrentConsumptionLock: %d", (unsigned int)dailyStatistics.avgCurrentConsumptionLock, Log, 4);
    logMessageVar("maxCurrentConsumptionLock: %d", (unsigned int)dailyStatistics.maxCurrentConsumptionLock, Log, 4);
    logMessageVar("batteryMinStartVoltageLock: %d", (unsigned int)dailyStatistics.batteryMinStartVoltageLock, Log, 4);
    logMessageVar("countSuccessfulUnlatchActions: %d", (unsigned int)dailyStatistics.countSuccessfulUnlatchActions, Log, 4);
    logMessageVar("countErroneousUnlatchActions: %d", (unsigned int)dailyStatistics.countErroneousUnlatchActions, Log, 4);
    logMessageVar("avgCurrentConsumptionUnlatch: %d", (unsigned int)dailyStatistics.avgCurrentConsumptionUnlatch, Log, 4);
    logMessageVar("maxCurrentConsumptionUnlatch: %d", (unsigned int)dailyStatistics.maxCurrentConsumptionUnlatch, Log, 4);
    logMessageVar("batteryMinStartVoltageUnlatch: %d", (unsigned int)dailyStatistics.batteryMinStartVoltageUnlatch, Log, 4);
    logMessageVar("incomingCommands: %d", (unsigned int)dailyStatistics.incomingCommands, Log, 4);
    logMessageVar("outgoingCommands: %d", (unsigned int)dailyStatistics.outgoingCommands, Log, 4);
    logMessageVar("maxTemperature: %d", (unsigned int)dailyStatistics.maxTemperature, Log, 4);
    logMessageVar("minTemperature: %d", (unsigned int)dailyStatistics.minTemperature, Log, 4);
    logMessageVar("avgTemperature: %d", (unsigned int)dailyStatistics.avgTemperature, Log, 4);
    logMessageVar("numDoorSensorStatusChanges: %d", (unsigned int)dailyStatistics.numDoorSensorStatusChanges, Log, 4);
    logMessageVar("maxBatteryPercentage: %d", (unsigned int)dailyStatistics.maxBatteryPercentage, Log, 4);
    logMessageVar("minBatteryPercentage: %d", (unsigned int)dailyStatistics.minBatteryPercentage, Log, 4);
    logMessageVar("idleTime: %d", (unsigned int)dailyStatistics.idleTime, Log, 4);
    logMessageVar("connectionTime: %d", (unsigned int)dailyStatistics.connectionTime, Log, 4);
    logMessageVar("actionTime: %d", (unsigned int)dailyStatistics.actionTime, Log, 4);
  }
}

void logGeneralStatistics(GeneralStatistics generalStatistics, bool debug, Print* Log) {
  if (debug) {
    logMessageVar("version: %d", (unsigned int)generalStatistics.version, Log, 4);
    logMessageVar("firstCalibrationYear: %d", (unsigned int)generalStatistics.firstCalibrationYear, Log, 4);
    logMessageVar("firstCalibrationMonth: %d", (unsigned int)generalStatistics.firstCalibrationMonth, Log, 4);
    logMessageVar("firstCalibrationDay: %d", (unsigned int)generalStatistics.firstCalibrationDay, Log, 4);
    logMessageVar("calibrationCount: %d", (unsigned int)generalStatistics.calibrationCount, Log, 4);
    logMessageVar("lockActionCount: %d", (unsigned int)generalStatistics.lockActionCount, Log, 4);
    logMessageVar("unlatchCount: %d", (unsigned int)generalStatistics.unlatchCount, Log, 4);
    logMessageVar("lastRebootDateYear: %d", (unsigned int)generalStatistics.lastRebootDateYear, Log, 4);
    logMessageVar("lastRebootDateMonth: %d", (unsigned int)generalStatistics.lastRebootDateMonth, Log, 4);
    logMessageVar("lastRebootDateDay: %d", (unsigned int)generalStatistics.lastRebootDateDay, Log, 4);
    logMessageVar("lastRebootDateHour: %d", (unsigned int)generalStatistics.lastRebootDateHour, Log, 4);
    logMessageVar("lastRebootDateMinute: %d", (unsigned int)generalStatistics.lastRebootDateMinute, Log, 4);
    logMessageVar("lastRebootDateSecond: %d", (unsigned int)generalStatistics.lastRebootDateSecond, Log, 4);
    logMessageVar("lastChargeDateYear: %d", (unsigned int)generalStatistics.lastChargeDateYear, Log, 4);
    logMessageVar("lastChargeDateMonth: %d", (unsigned int)generalStatistics.lastChargeDateMonth, Log, 4);
    logMessageVar("lastChargeDateDay: %d", (unsigned int)generalStatistics.lastChargeDateDay, Log, 4);
    logMessageVar("lastChargeDateHour: %d", (unsigned int)generalStatistics.lastChargeDateHour, Log, 4);
    logMessageVar("lastChargeDateMinute: %d", (unsigned int)generalStatistics.lastChargeDateMinute, Log, 4);
    logMessageVar("lastChargeDateSecond: %d", (unsigned int)generalStatistics.lastChargeDateSecond, Log, 4);
    logMessageVar("initialBatteryVoltage: %d", (unsigned int)generalStatistics.initialBatteryVoltage, Log, 4);
    logMessageVar("numActionsDuringBatteryCycle: %d", (unsigned int)generalStatistics.numActionsDuringBatteryCycle, Log, 4);
    logMessageVar("numUnexpectedReboots: %d", (unsigned int)generalStatistics.numUnexpectedReboots, Log, 4);
  }
}

void logInternalLogEntry(InternalLogEntry internalLogEntry, bool debug, Print* Log) {
  if (debug) {
    if (Log == nullptr) {
      log_d("[%d] type:%d authId:%d %d-%d-%d %d:%d:%d ", internalLogEntry.index, internalLogEntry.loggingType, internalLogEntry.authId, internalLogEntry.timeStampYear, internalLogEntry.timeStampMonth, internalLogEntry.timeStampDay, internalLogEntry.timeStampHour, internalLogEntry.timeStampMinute, internalLogEntry.timeStampSecond);
    }
    else
    {
      Log->printf("[%d] type:%d authId:%d %d-%d-%d %d:%d:%d ", internalLogEntry.index, internalLogEntry.loggingType, internalLogEntry.authId, internalLogEntry.timeStampYear, internalLogEntry.timeStampMonth, internalLogEntry.timeStampDay, internalLogEntry.timeStampHour, internalLogEntry.timeStampMinute, internalLogEntry.timeStampSecond);;
      Log->println();
    }

    logMessageVar("data: %d", (unsigned int)internalLogEntry.data, Log, 4);
  }
}

void logMessage(const char* message, Print* Log, int level) {
  if (Log == nullptr) {
    switch (level) {
      case 1:
        esp_log_write(ESP_LOG_ERROR, "NukiBle", message);
        break;
      case 2:
        esp_log_write(ESP_LOG_WARN, "NukiBle", message);
        break;
      case 3:
        esp_log_write(ESP_LOG_INFO, "NukiBle", message);
        break;
      case 4:
      default:
        esp_log_write(ESP_LOG_DEBUG, "NukiBle", message);
        break;;
    }
  }
  else
  {
    Log->println(message);
  }
}

void logMessageVar(const char* message, unsigned int var, Print* Log, int level) {
  if (Log == nullptr) {
    switch (level) {
      case 1:
        esp_log_write(ESP_LOG_ERROR, "NukiBle", message, var);
        break;
      case 2:
        esp_log_write(ESP_LOG_WARN, "NukiBle", message, var);
        break;
      case 3:
        esp_log_write(ESP_LOG_INFO, "NukiBle", message, var);
        break;
      case 4:
      default:
        esp_log_write(ESP_LOG_DEBUG, "NukiBle", message, var);
        break;
    }
  }
  else
  {
    Log->printf(message, var);
    Log->println();
  }
}

void logMessageVar(const char* message, const char* var, Print* Log, int level) {
  if (Log == nullptr) {
    switch (level) {
      case 1:
        esp_log_write(ESP_LOG_ERROR, "NukiBle", message, var);
        break;
      case 2:
        esp_log_write(ESP_LOG_WARN, "NukiBle", message, var);
        break;
      case 3:
        esp_log_write(ESP_LOG_INFO, "NukiBle", message, var);
        break;
      case 4:
      default:
        esp_log_write(ESP_LOG_DEBUG, "NukiBle", message, var);
        break;
    }
  }
  else
  {
    Log->printf(message, var);
    Log->println();
  }
}

void logMessageVar(const char* message, const float var, Print* Log, int level) {
  if (Log == nullptr) {
    switch (level) {
      case 1:
        esp_log_write(ESP_LOG_ERROR, "NukiBle", message, var);
        break;
      case 2:
        esp_log_write(ESP_LOG_WARN, "NukiBle", message, var);
        break;
      case 3:
        esp_log_write(ESP_LOG_INFO, "NukiBle", message, var);
        break;
      case 4:
      default:
        esp_log_write(ESP_LOG_DEBUG, "NukiBle", message, var);
        break;
    }
  }
  else
  {
    Log->printf(message, var);
    Log->println();
  }
}

} // namespace Nuki
