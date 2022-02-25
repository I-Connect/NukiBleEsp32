#pragma once

#include "sodium/crypto_secretbox.h"
#include "Crc16.h"

void printBuffer(const byte* buff, const uint8_t size, const boolean asChars, const char* header) {
  #ifdef DEBUG_NUKI_HEX_DATA
  delay(10); //delay otherwise first part of print will not be shown
  char tmp[16];

  if (strlen(header) > 0) {
    Serial.print(header);
    Serial.print(": ");
  }
  for (int i = 0; i < size; i++) {
    if (asChars) {
      Serial.print((char)buff[i]);
    } else {
      sprintf(tmp, "%02x", buff[i]);
      Serial.print(tmp);
      Serial.print(" ");
    }
  }
  Serial.println();
  #endif
}

bool checkCharArrayEmpty(unsigned char* array, uint16_t len) {
  uint16_t zeroCount = 0;
  for (size_t i = 0; i < len; i++) {
    if (array[i] == 0) {
      zeroCount++;
    }
  }
  if (zeroCount == len) {
    return false;
  }
  return true;
}

int NukiBle::encode(unsigned char* output, unsigned char* input, unsigned long long len, unsigned char* nonce, unsigned char* keyS) {
  int result = crypto_secretbox_easy(output, input, len, nonce, keyS);

  if (result) {
    log_d("Encryption failed (length %i, given result %i)\n", len, result);
    return -1;
  }
  return len;
}

int NukiBle::decode(unsigned char* output, unsigned char* input, unsigned long long len, unsigned char* nonce, unsigned char* keyS) {

  int result = crypto_secretbox_open_easy(output, input, len, nonce, keyS);

  if (result) {
    log_w("Decryption failed (length %i, given result %i)\n", len, result);
    return -1;
  }
  return len;
}

void NukiBle::generateNonce(unsigned char* hexArray, uint8_t nrOfBytes) {

  for (int i = 0 ; i < nrOfBytes ; i++) {
    randomSeed(millis());
    hexArray[i] = random(0, 65500);
  }
  printBuffer((byte*)hexArray, nrOfBytes, false, "Nonce");
}

void logKeyPadEntry(KeyPadEntry keyPadEntry) {
  log_d("code:%d", keyPadEntry.code);
  log_d("name:%s", keyPadEntry.name);
  log_d("timeLimited:%d", keyPadEntry.timeLimited);
  log_d("allowedFromYear:%d", keyPadEntry.allowedFromYear);
  log_d("allowedFromMonth:%d", keyPadEntry.allowedFromMonth);
  log_d("allowedFromDay:%d", keyPadEntry.allowedFromDay);
  log_d("allowedFromHour:%d", keyPadEntry.allowedFromHour);
  log_d("allowedFromMin:%d", keyPadEntry.allowedFromMin);
  log_d("allowedFromSec:%d", keyPadEntry.allowedFromSec);
  log_d("allowedUntillYear:%d", keyPadEntry.allowedUntillYear);
  log_d("allowedUntillMonth:%d", keyPadEntry.allowedUntillMonth);
  log_d("allowedUntillDay:%d", keyPadEntry.allowedUntillDay);
  log_d("allowedUntillHour:%d", keyPadEntry.allowedUntillHour);
  log_d("allowedUntillMin:%d", keyPadEntry.allowedUntillMin);
  log_d("allowedUntillSec:%d", keyPadEntry.allowedUntillSec);
  log_d("allowedWeekdays:%d", keyPadEntry.allowedWeekdays);
  log_d("allowedFromTimeHour:%d", keyPadEntry.allowedFromTimeHour);
  log_d("allowedFromTimeMin:%d", keyPadEntry.allowedFromTimeMin);
  log_d("allowedUntillTimeHour:%d", keyPadEntry.allowedUntillTimeHour);
  log_d("allowedUntillTimeMin:%d", keyPadEntry.allowedUntillTimeMin);
}

void logKeyturnerState(KeyTurnerState keyTurnerState) {
  log_d("nukiState: %02x", keyTurnerState.nukiState);
  log_d("lockState: %d", keyTurnerState.lockState);
  log_d("trigger: %d", keyTurnerState.trigger);
  log_d("currentTimeYear: %d", keyTurnerState.currentTimeYear);
  log_d("currentTimeMonth: %d", keyTurnerState.currentTimeMonth);
  log_d("currentTimeDay: %d", keyTurnerState.currentTimeDay);
  log_d("currentTimeHour: %d", keyTurnerState.currentTimeHour);
  log_d("currentTimeMinute: %d", keyTurnerState.currentTimeMinute);
  log_d("currentTimeSecond: %d", keyTurnerState.currentTimeSecond);
  log_d("timeZoneOffset: %d", keyTurnerState.timeZoneOffset);
  log_d("criticalBatteryState: %d", keyTurnerState.criticalBatteryState);
  log_d("configUpdateCount: %d", keyTurnerState.configUpdateCount);
  log_d("lockNgoTimer: %d", keyTurnerState.lockNgoTimer);
  log_d("lastLockAction: %d", keyTurnerState.lastLockAction);
  log_d("lastLockActionTrigger: %d", keyTurnerState.lastLockActionTrigger);
  log_d("lastLockActionCompletionStatus: %d", keyTurnerState.lastLockActionCompletionStatus);
  log_d("doorSensorState: %d", keyTurnerState.doorSensorState);
}

void logAdvancedConfig(AdvancedConfig advancedConfig) {
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
  log_d("autoLockDisabled :%d", advancedConfig.autoLockDisabled);
  log_d("nightModeEnabled :%d", advancedConfig.nightModeEnabled);
  log_d("nightModeStartTime Hour :%d", advancedConfig.nightModeStartTime[0]);
  log_d("nightModeStartTime Minute :%d", advancedConfig.nightModeStartTime[1]);
  log_d("nightModeEndTime Hour :%d", advancedConfig.nightModeEndTime[0]);
  log_d("nightModeEndTime Minute :%d", advancedConfig.nightModeEndTime[1]);
  log_d("nightModeAutoLockEnabled :%d", advancedConfig.nightModeAutoLockEnabled);
  log_d("nightModeAutoUnlockDisabled :%d", advancedConfig.nightModeAutoUnlockDisabled);
  log_d("nightModeImmediateLockOnStart :%d", advancedConfig.nightModeImmediateLockOnStart);
}