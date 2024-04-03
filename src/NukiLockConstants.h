#pragma once
/**
 * @file NukiConstants.h
 * Definitions of constants and data types based on Nuki smart lock api
 *
 * Created: 2022
 * License: GNU GENERAL PUBLIC LICENSE (see LICENSE)
 *
 * This library implements the communication from an ESP32 via BLE to a Nuki smart lock.
 * Based on the Nuki Smart Lock API V2.2.1
 * https://developer.nuki.io/page/nuki-smart-lock-api-2/2/
 *
 */

#include "NimBLEUUID.h"
#include "NukiConstants.h"

namespace NukiLock {

using namespace Nuki;

struct Action {
  CommandType cmdType;
  Command command;
  unsigned char payload[100] {0};
  uint8_t payloadLen = 0;
};

enum class ErrorCode : uint8_t {
  ERROR_BAD_CRC	                    = 0xFD,
  ERROR_BAD_LENGTH	                = 0xFE,
  ERROR_UNKNOWN	                    = 0xFF,
  P_ERROR_NOT_PAIRING	              = 0x10,
  P_ERROR_BAD_AUTHENTICATOR	        = 0x11,
  P_ERROR_BAD_PARAMETER	            = 0x12,
  P_ERROR_MAX_USER	                = 0x13,
  K_ERROR_NOT_AUTHORIZED	          = 0x20,
  K_ERROR_BAD_PIN	                  = 0x21,
  K_ERROR_BAD_NONCE	                = 0x22,
  K_ERROR_BAD_PARAMETER	            = 0x23,
  K_ERROR_INVALID_AUTH_ID	          = 0x24,
  K_ERROR_DISABLED	                = 0x25,
  K_ERROR_REMOTE_NOT_ALLOWED	      = 0x26,
  K_ERROR_TIME_NOT_ALLOWED	        = 0x27,
  K_ERROR_TOO_MANY_PIN_ATTEMPTS	    = 0x28,
  K_ERROR_TOO_MANY_ENTRIES	        = 0x29,
  K_ERROR_CODE_ALREADY_EXISTS	      = 0x2A,
  K_ERROR_CODE_INVALID	            = 0x2B,
  K_ERROR_CODE_INVALID_TIMEOUT_1    = 0x2C,
  K_ERROR_CODE_INVALID_TIMEOUT_2    = 0x2D,
  K_ERROR_CODE_INVALID_TIMEOUT_3	  = 0x2E,
  K_ERROR_AUTO_UNLOCK_TOO_RECENT	  = 0x40,
  K_ERROR_POSITION_UNKNOWN	        = 0x41,
  K_ERROR_MOTOR_BLOCKED	            = 0x42,
  K_ERROR_CLUTCH_FAILURE	          = 0x43,
  K_ERROR_MOTOR_TIMEOUT	            = 0x44,
  K_ERROR_BUSY	                    = 0x45,
  K_ERROR_CANCELED	                = 0x46,
  K_ERROR_NOT_CALIBRATED	          = 0x47,
  K_ERROR_MOTOR_POSITION_LIMIT	    = 0x48,
  K_ERROR_MOTOR_LOW_VOLTAGE	        = 0x49,
  K_ERROR_MOTOR_POWER_FAILURE	      = 0x4A,
  K_ERROR_CLUTCH_POWER_FAILURE	    = 0x4B,
  K_ERROR_VOLTAGE_TOO_LOW	          = 0x4C,
  K_ERROR_FIRMWARE_UPDATE_NEEDED	  = 0x4D
};

enum class State : uint8_t {
  Uninitialized   = 0x00,
  PairingMode     = 0x01,
  DoorMode        = 0x02,
  MaintenanceMode = 0x04
};

enum class LockState : uint8_t {
  Uncalibrated    = 0x00,
  Locked          = 0x01,
  Unlocking       = 0x02,
  Unlocked        = 0x03,
  Locking         = 0x04,
  Unlatched       = 0x05,
  UnlockedLnga    = 0x06,
  Unlatching      = 0x07,
  Calibration     = 0xFC,
  BootRun         = 0xFD,
  MotorBlocked    = 0xFE,
  Undefined       = 0xFF
};

enum class Trigger : uint8_t {
  System          = 0x00,
  Manual          = 0x01,
  Button          = 0x02,
  Automatic       = 0x03,
  AutoLock        = 0x06
};


enum class LockAction : uint8_t {
  Unlock          = 0x01,
  Lock            = 0x02,
  Unlatch         = 0x03,
  LockNgo         = 0x04,
  LockNgoUnlatch  = 0x05,
  FullLock        = 0x06,
  FobAction1      = 0x81,
  FobAction2      = 0x82,
  FobAction3      = 0x83
};

enum class KeypadActionSource : uint8_t {
  ArrowKey = 0x00,
  Code = 0x01
};

enum class KeypadAction : uint8_t {
  Intelligent     = 0x00,
  Unlock          = 0x01,
  Lock            = 0x02,
  Unlatch         = 0x03,
  LockNgo         = 0x04
};

enum class ButtonPressAction : uint8_t {
  NoAction          = 0x00,
  Intelligent       = 0x01,
  Unlock            = 0x02,
  Lock              = 0x03,
  Unlatch           = 0x04,
  LockNgo           = 0x05,
  ShowStatus        = 0x06
};

enum class CompletionStatus : uint8_t {
  Success           = 0x00,
  MotorBlocked      = 0x01,
  Canceled          = 0x02,
  TooRecent         = 0x03,
  Busy              = 0x04,
  LowMotorVoltage   = 0x05,
  ClutchFailure     = 0x06,
  MotorPowerFailure = 0x07,
  IncompleteFailure = 0x08,
  Failure           = 0x0b,
  InvalidCode       = 0xE0,
  OtherError        = 0xFE,
  Unknown           = 0xFF
};

struct __attribute__((packed)) KeyTurnerState {
  State nukiState = State::Uninitialized;
  LockState lockState = LockState::Undefined;
  Trigger trigger;
  uint16_t currentTimeYear;
  uint8_t currentTimeMonth;
  uint8_t currentTimeDay;
  uint8_t currentTimeHour;
  uint8_t currentTimeMinute;
  uint8_t currentTimeSecond;
  int16_t timeZoneOffset;
  uint8_t criticalBatteryState;
  uint8_t configUpdateCount;
  bool lockNgoTimer;
  LockAction lastLockAction;
  Trigger lastLockActionTrigger;
  CompletionStatus lastLockActionCompletionStatus;
  DoorSensorState doorSensorState = DoorSensorState::Unavailable;
  uint16_t nightModeActive;
  uint8_t accessoryBatteryState;
};

struct __attribute__((packed)) Config {
  uint32_t nukiId;
  unsigned char name[32];
  float latitude;
  float longitude;
  uint8_t autoUnlatch;
  uint8_t pairingEnabled;
  uint8_t buttonEnabled;
  uint8_t ledEnabled;
  uint8_t ledBrightness;
  uint16_t currentTimeYear;
  uint8_t currentTimeMonth;
  uint8_t currentTimeDay;
  uint8_t currentTimeHour;
  uint8_t currentTimeMinute;
  uint8_t currentTimeSecond;
  int16_t timeZoneOffset;
  uint8_t dstMode;
  uint8_t  hasFob;
  uint8_t  fobAction1;
  uint8_t  fobAction2;
  uint8_t  fobAction3;
  uint8_t  singleLock;
  AdvertisingMode advertisingMode;
  uint8_t hasKeypad;
  unsigned char firmwareVersion[3];
  unsigned char hardwareRevision[2];
  uint8_t homeKitStatus;
  TimeZoneId timeZoneId;
  uint8_t undocumented1;
  uint8_t undocumented2;
  uint8_t hasKeypadV2;
};

struct __attribute__((packed)) NewConfig {
  unsigned char name[32];
  float latitude;
  float longitude;
  uint8_t autoUnlatch;
  uint8_t pairingEnabled;
  uint8_t buttonEnabled;
  uint8_t ledEnabled;
  uint8_t ledBrightness;
  int16_t timeZoneOffset;
  uint8_t dstMode;
  uint8_t  fobAction1;
  uint8_t  fobAction2;
  uint8_t  fobAction3;
  uint8_t  singleLock;
  AdvertisingMode advertisingMode;
  TimeZoneId timeZoneId;
};

struct __attribute__((packed)) AdvancedConfig {
  uint16_t totalDegrees;
  int16_t unlockedPositionOffsetDegrees;
  int16_t lockedPositionOffsetDegrees;
  int16_t singleLockedPositionOffsetDegrees;
  int16_t unlockedToLockedTransitionOffsetDegrees;
  uint8_t lockNgoTimeout;
  ButtonPressAction singleButtonPressAction;
  ButtonPressAction doubleButtonPressAction;
  uint8_t detachedCylinder;
  BatteryType batteryType;
  uint8_t automaticBatteryTypeDetection;
  uint8_t unlatchDuration;
  uint16_t autoLockTimeOut;
  uint8_t autoUnLockDisabled;
  uint8_t nightModeEnabled;
  unsigned char nightModeStartTime[2];
  unsigned char nightModeEndTime[2];
  uint8_t nightModeAutoLockEnabled;
  uint8_t nightModeAutoUnlockDisabled;
  uint8_t  nightModeImmediateLockOnStart;
  uint8_t autoLockEnabled;
  uint8_t immediateAutoLockEnabled;
  uint8_t autoUpdateEnabled;
};

struct __attribute__((packed)) NewAdvancedConfig {
  int16_t unlockedPositionOffsetDegrees;
  int16_t lockedPositionOffsetDegrees;
  int16_t singleLockedPositionOffsetDegrees;
  int16_t unlockedToLockedTransitionOffsetDegrees;
  uint8_t lockNgoTimeout;
  ButtonPressAction singleButtonPressAction;
  ButtonPressAction doubleButtonPressAction;
  uint8_t detachedCylinder;
  BatteryType batteryType;
  uint8_t automaticBatteryTypeDetection;
  uint8_t unlatchDuration;
  uint16_t autoLockTimeOut;
  uint8_t autoUnLockDisabled;
  uint8_t nightModeEnabled;
  unsigned char nightModeStartTime[2];
  unsigned char nightModeEndTime[2];
  uint8_t nightModeAutoLockEnabled;
  uint8_t nightModeAutoUnlockDisabled;
  uint8_t  nightModeImmediateLockOnStart;
  uint8_t autoLockEnabled;
  uint8_t immediateAutoLockEnabled;
  uint8_t autoUpdateEnabled;
};

struct __attribute__((packed)) BatteryReport {
  uint16_t batteryDrain;
  uint16_t batteryVoltage;
  uint8_t criticalBatteryState;
  LockAction lockAction;
  uint16_t startVoltage;
  uint16_t lowestVoltage;
  uint16_t lockDistance;
  int8_t startTemperature;
  uint16_t maxTurnCurrent;
  uint16_t batteryResistance;
};


struct __attribute__((packed)) TimeControlEntry {
  uint8_t entryId;
  uint8_t enabled;
  uint8_t weekdays;
  uint8_t timeHour;
  uint8_t timeMin;
  LockAction lockAction;
};

struct __attribute__((packed)) NewTimeControlEntry {
  uint8_t weekdays;
  uint8_t timeHour;
  uint8_t timeMin;
  LockAction lockAction;
};

enum class LoggingType : uint8_t {
  LoggingEnabled            = 0x01,
  LockAction                = 0x02,
  Calibration               = 0x03,
  InitializationRun         = 0x04,
  KeypadAction              = 0x05,
  DoorSensor                = 0x06,
  DoorSensorLoggingEnabled  = 0x07
};

struct __attribute__((packed)) LogEntry {
  uint32_t index;
  uint16_t timeStampYear;
  uint8_t timeStampMonth;
  uint8_t timeStampDay;
  uint8_t timeStampHour;
  uint8_t timeStampMinute;
  uint8_t timeStampSecond;
  uint32_t authId;
  uint8_t name[32];
  LoggingType loggingType;
  uint8_t data[5];
};

inline void lockactionToString(const LockAction action, char* str) {
  switch (action) {
    case LockAction::Unlock:
      strcpy(str, "Unlock");
      break;
    case LockAction::Lock:
      strcpy(str, "Lock");
      break;
    case LockAction::Unlatch:
      strcpy(str, "Unlatch");
      break;
    case LockAction::LockNgo:
      strcpy(str, "LockNgo");
      break;
    case LockAction::LockNgoUnlatch:
      strcpy(str, "LockNgoUnlatch");
      break;
    case LockAction::FullLock:
      strcpy(str, "FullLock");
      break;
    case LockAction::FobAction1:
      strcpy(str, "FobAction1");
      break;
    case LockAction::FobAction2:
      strcpy(str, "FobAction2");
      break;
    case LockAction::FobAction3:
      strcpy(str, "FobAction3");
      break;
    default:
      strcpy(str, "Unknown");
      break;
  }
}

inline void lockstateToString(const LockState state, char* str) {
  switch (state) {
    case LockState::Uncalibrated:
      strcpy(str, "uncalibrated");
      break;
    case LockState::Locked:
      strcpy(str, "locked");
      break;
    case LockState::Unlocking:
      strcpy(str, "unlocking");
      break;
    case LockState::Locking:
      strcpy(str, "locking");
      break;
    case LockState::Unlocked:
      strcpy(str, "unlocked");
      break;
    case LockState::Unlatched:
      strcpy(str, "unlatched");
      break;
    case LockState::UnlockedLnga:
      strcpy(str, "unlockedLnga");
      break;
    case LockState::Unlatching:
      strcpy(str, "unlatching");
      break;
    case LockState::Calibration:
      strcpy(str, "calibration");
      break;
    case LockState::BootRun:
      strcpy(str, "bootRun");
      break;
    case LockState::MotorBlocked:
      strcpy(str, "motorBlocked");
      break;
    default:
      strcpy(str, "undefined");
      break;
  }
}

inline void triggerToString(const Trigger trigger, char* str) {
  switch (trigger) {
    case Trigger::AutoLock:
      strcpy(str, "autoLock");
      break;
    case Trigger::Automatic:
      strcpy(str, "automatic");
      break;
    case Trigger::Button:
      strcpy(str, "button");
      break;
    case Trigger::Manual:
      strcpy(str, "manual");
      break;
    case Trigger::System:
      strcpy(str, "system");
      break;
    default:
      strcpy(str, "undefined");
      break;
  }
}

inline void completionStatusToString(const CompletionStatus status, char* str) {
  switch (status) {
    case CompletionStatus::Success:
      strcpy(str, "success");
      break;
    case CompletionStatus::Busy:
      strcpy(str, "busy");
      break;
    case CompletionStatus::Canceled:
      strcpy(str, "canceled");
      break;
    case CompletionStatus::ClutchFailure:
      strcpy(str, "clutchFailure");
      break;
    case CompletionStatus::IncompleteFailure:
      strcpy(str, "incompleteFailure");
      break;
    case CompletionStatus::Failure:
      strcpy(str, "failure");
      break;
    case CompletionStatus::InvalidCode:
      strcpy(str, "invalidCode");
      break;
    case CompletionStatus::LowMotorVoltage:
      strcpy(str, "lowMotorVoltage");
      break;
    case CompletionStatus::MotorBlocked:
      strcpy(str, "motorBlocked");
      break;
    case CompletionStatus::MotorPowerFailure:
      strcpy(str, "motorPowerFailure");
      break;
    case CompletionStatus::OtherError:
      strcpy(str, "otherError");
      break;
    case CompletionStatus::TooRecent:
      strcpy(str, "tooRecent");
      break;
    case CompletionStatus::Unknown:
      strcpy(str, "unknown");
      break;
    default:
      strcpy(str, "undefined");
      break;

  }
}

inline void doorSensorStateToString(const DoorSensorState state, char* str) {
  switch (state) {
    case DoorSensorState::Unavailable:
      strcpy(str, "unavailable");
      break;
    case DoorSensorState::Deactivated:
      strcpy(str, "deactivated");
      break;
    case DoorSensorState::DoorClosed:
      strcpy(str, "doorClosed");
      break;
    case DoorSensorState::DoorOpened:
      strcpy(str, "doorOpened");
      break;
    case DoorSensorState::DoorStateUnknown:
      strcpy(str, "doorStateUnknown");
      break;
    case DoorSensorState::Calibrating:
      strcpy(str, "calibrating");
      break;
    default:
      strcpy(str, "undefined");
      break;
  }
}

inline void loggingTypeToString(const LoggingType state, char* str) {
  switch (state) {
    case LoggingType::LoggingEnabled:
      strcpy(str, "LoggingEnabled");
      break;
    case LoggingType::LockAction:
      strcpy(str, "LockAction");
      break;
    case LoggingType::Calibration:
      strcpy(str, "Calibration");
      break;
    case LoggingType::InitializationRun:
      strcpy(str, "InitializationRun");
      break;
    case LoggingType::KeypadAction:
      strcpy(str, "KeypadAction");
      break;
    case LoggingType::DoorSensor:
      strcpy(str, "DoorSensor");
      break;
    case LoggingType::DoorSensorLoggingEnabled:
      strcpy(str, "DoorSensorLoggingEnabled");
      break;
    default:
      strcpy(str, "Unknown");
      break;
  }
}

} // namespace Nuki
