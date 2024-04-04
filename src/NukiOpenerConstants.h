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

namespace NukiOpener {
using namespace Nuki;

const NimBLEUUID openerPairingServiceUUID  = NimBLEUUID("a92ae100-5501-11e4-916c-0800200c9a66");
const NimBLEUUID openerServiceUUID  = NimBLEUUID("a92ae200-5501-11e4-916c-0800200c9a66");
const NimBLEUUID openerGdioUUID  = NimBLEUUID("a92ae101-5501-11e4-916c-0800200c9a66");
const NimBLEUUID openerUserDataUUID  = NimBLEUUID("a92ae202-5501-11e4-916c-0800200c9a66");

enum class LockAction : uint8_t {
  ActivateRTO = 0x01,
  DeactivateRTO = 0x02,
  ElectricStrikeActuation = 0x03,

  ActivateCM = 0x04,
  DeactivateCM = 0x05,

  FobAction1 = 0x81,
  FobAction2 = 0x82,
  FobAction3 = 0x83
};


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

enum class CompletionStatus : uint8_t {
  Success = 0x00,
  Canceled = 0x02,
  TooRecent = 0x03,
  Busy = 0x04,
  Incomplete = 0x08,
  OtherError = 0xFE,
  Unknown = 0xFF
};


enum class State : uint8_t {
  Uninitialized   = 0x00,
  PairingMode     = 0x01,
  DoorMode        = 0x02,
  ContinuousMode  = 0x03,
  MaintenanceMode = 0x04
};

enum class LockState : uint8_t {
  Uncalibrated = 0x00,
  Locked = 0x01,
  RTOactive = 0x03,
  Open = 0x05,
  Opening = 0x07,
  Undefined = 0xFF
};

enum class Trigger : uint8_t {
  System          = 0x00,
  Manual          = 0x01,
  Button          = 0x02,
  Automatic       = 0x03
};

enum class ButtonPressAction : uint8_t {
  NoAction = 0x00,
  ToggleRTO = 0x01,
  ActivateRTO = 0x02,
  DeactivateRTO = 0x03,
  ToggleCM = 0x04,
  ActivateCM = 0x05,
  DectivateCM = 0x06,
  Open = 0x07
};

struct __attribute__((packed)) OpenerState {
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
  uint8_t ringToOpenTimer;
  LockAction lastLockAction;
  Trigger lastLockActionTrigger;
  CompletionStatus lastLockActionCompletionStatus;
  DoorSensorState doorSensorState = DoorSensorState::Unavailable;
};

struct __attribute__((packed)) Config {
  uint32_t nukiId;
  unsigned char name[32];
  float latitude;
  float longitude;
  uint8_t capabilities;
  uint8_t pairingEnabled;
  uint8_t buttonEnabled;
  uint8_t ledFlashEnabled;
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
  uint8_t  operatingMode;
  AdvertisingMode advertisingMode;
  uint8_t hasKeypad;
  unsigned char firmwareVersion[3];
  unsigned char hardwareRevision[2];
  TimeZoneId timeZoneId;
  uint8_t undocumented1;
  uint8_t undocumented2;
  uint8_t hasKeypadV2;
};

struct __attribute__((packed)) NewConfig {
  unsigned char name[32];
  float latitude;
  float longitude;
  uint8_t capabilities;
  uint8_t pairingEnabled;
  uint8_t buttonEnabled;
  uint8_t ledFlashEnabled;
  int16_t timeZoneOffset;
  uint8_t dstMode;
  uint8_t  fobAction1;
  uint8_t  fobAction2;
  uint8_t  fobAction3;
  uint8_t  operatingMode;
  AdvertisingMode advertisingMode;
  TimeZoneId timeZoneId;
};

struct __attribute__((packed)) AdvancedConfig {
  uint16_t intercomID;
  uint8_t busModeSwitch;
  uint16_t shortCircuitDuration;
  uint16_t electricStrikeDelay;
  uint8_t randomElectricStrikeDelay;
  uint16_t electricStrikeDuration;
  uint8_t disableRtoAfterRing;
  uint8_t rtoTimeout;
  uint8_t unknown;
  uint8_t doorbellSuppression;
  uint16_t doorbellSuppressionDuration;
  uint8_t soundRing;
  uint8_t soundOpen;
  uint8_t soundRto;
  uint8_t soundCm;
  uint8_t soundConfirmation;
  uint8_t soundLevel;
  ButtonPressAction singleButtonPressAction;
  ButtonPressAction doubleButtonPressAction;
  Nuki::BatteryType batteryType;
  uint8_t automaticBatteryTypeDetection;
};

struct __attribute__((packed)) NewAdvancedConfig {
  uint16_t intercomID;
  uint8_t busModeSwitch;
  uint16_t shortCircuitDuration;
  uint16_t electricStrikeDelay;
  uint8_t randomElectricStrikeDelay;
  uint16_t electricStrikeDuration;
  uint8_t disableRtoAfterRing;
  uint8_t rtoTimeout;
  uint8_t unknown;
  uint8_t doorbellSuppression;
  uint16_t doorbellSuppressionDuration;
  uint8_t soundRing;
  uint8_t soundOpen;
  uint8_t soundRto;
  uint8_t soundCm;
  uint8_t soundConfirmation;
  uint8_t soundLevel;
  ButtonPressAction singleButtonPressAction;
  ButtonPressAction doubleButtonPressAction;
  Nuki::BatteryType batteryType;
  uint8_t automaticBatteryTypeDetection;
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
  KeypadAction              = 0x05,
  DoorbellRecognition       = 0x06
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
  uint8_t data[8];
};

inline void lockactionToString(const LockAction action, char* str) {
  switch (action) {
    case LockAction::ActivateRTO:
      strcpy(str, "ActivateRTO");
      break;
    case LockAction::DeactivateRTO:
      strcpy(str, "DeactivateRTO");
      break;
    case LockAction::ElectricStrikeActuation:
      strcpy(str, "ElectricStrikeActuation");
      break;
    case LockAction::ActivateCM:
      strcpy(str, "ActivateCM");
      break;
    case LockAction::DeactivateCM:
      strcpy(str, "DeactivateCM");
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
    case LockState::RTOactive:
      strcpy(str, "RTOactive");
      break;
    case LockState::Open:
      strcpy(str, "open");
      break;
    case LockState::Opening:
      strcpy(str, "opening");
      break;
    default:
      strcpy(str, "undefined");
      break;
  }
}


inline void triggerToString(const Trigger trigger, char* str) {
  switch (trigger) {
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
    case CompletionStatus::OtherError:
      strcpy(str, "otherError");
      break;
    case CompletionStatus::TooRecent:
      strcpy(str, "tooRecent");
      break;
    case CompletionStatus::Incomplete:
      strcpy(str, "incomplete");
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
    case LoggingType::KeypadAction:
      strcpy(str, "KeypadAction");
      break;
    case LoggingType::DoorbellRecognition:
      strcpy(str, "DoorbellRecognition");
      break;
    default:
      strcpy(str, "Unknown");
      break;
  }
}

} // namespace Nuki