#pragma once

//Keyturner initialization service
#define keyturnerInitServiceUUID           a92ee000-5501-11e4-916c-0800200c9a66
//Keyturner Pairing Service
#define keyturnerPairingServiceUUID        a92ee100-5501-11e4-916c-0800200c9a66
//Keyturner pairing Data Input Output characteristic
#define keyturnerGdioUUID                  a92ee101-5501-11e4-916c-0800200c9a66
//Keyturner Service
#define keyturnerServiceUUID               a92ee200-5501-11e4-916c-0800200c9a66
//Keyturner Data Input Output characteristic
#define keyturnerDataUUID                  a92ee201-5501-11e4-916c-0800200c9a66
//User-Specific Data Input Output characteristic
#define userDataUUID                       a92ee202-5501-11e4-916c-0800200c9a66

#define STRINGIFY_(x) #x
#define STRING(VAR) STRINGIFY_(VAR)

enum class NukiCommand : uint16_t {
  empty                         = 0x0000,
  requestData	                  = 0x0001,
  publicKey	                    = 0x0003,
  challenge	                    = 0x0004,
  authorizationAuthenticator	  = 0x0005,
  authorizationData	            = 0x0006,
  authorizationId	              = 0x0007,
  removeUserAuthorization	      = 0x0008,
  requestAuthorizationEntries	  = 0x0009,
  authorizationEntry	          = 0x000A,
  authorizationDatInvite	      = 0x000B,
  keyturnerStates	              = 0x000C,
  lockAction	                  = 0x000D,
  status	                      = 0x000E,
  mostRecentCommand	            = 0x000F,
  openingsClosingsSummary	      = 0x0010,
  batteryReport	                = 0x0011,
  errorReport	                  = 0x0012,
  setConfig	                    = 0x0013,
  requestConfig	                = 0x0014,
  config	                      = 0x0015,
  setSecurityPin	              = 0x0019,
  requestCalibration	          = 0x001A,
  requestReboot	                = 0x001D,
  authorizationIdConfirmation	  = 0x001E,
  authorizationIdInvite	        = 0x001F,
  verifySecurityPin	            = 0x0020,
  updateTime	                  = 0x0021,
  updateUserAuthorization	      = 0x0025,
  authorizationEntryCount	      = 0x0027,
  requestLogEntries	            = 0x0031,
  logEntry	                    = 0x0032,
  logEntryCount	                = 0x0033,
  enableLogging	                = 0x0034,
  setAdvancedConfig	            = 0x0035,
  requestAdvancedConfig	        = 0x0036,
  advancedConfig	              = 0x0037,
  addTimeControlEntry	          = 0x0039,
  timeControlEntryId	          = 0x003A,
  removeTimeControlEntry	      = 0x003B,
  requestTimeControlEntries	    = 0x003C,
  timeControlEntryCount	        = 0x003D,
  timeControlEntry	            = 0x003E,
  updateTimeControlEntry	      = 0x003F,
  addKeypadCode	                = 0x0041,
  keypadCodeId	                = 0x0042,
  requestKeypadCodes	          = 0x0043,
  keypadCodeCount	              = 0x0044,
  keypadCode	                  = 0x0045,
  updateKeypadCode	            = 0x0046,
  removeKeypadCode	            = 0x0047,
  keypadAction	                = 0x0048,
  simpleLockAction	            = 0x0100
};

enum class NukiErrorCode : uint8_t {
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

enum class CommandStatus : uint8_t {
  complete        = 0x00,
  accepted        = 0x01
};

enum class NukiState : uint8_t {
  uninitialized   = 0x00,
  pairingMode     = 0x01,
  doorMode        = 0x02,
  maintenanceMode = 0x04
};

enum class LockState : uint8_t {
  uncalibrated    = 0x00,
  locked          = 0x01,
  unlocking       = 0x02,
  unlocked        = 0x03,
  locking         = 0x04,
  unlatched       = 0x05,
  unlockedLnga    = 0x06,
  unlatching      = 0x07,
  calibration     = 0xFC,
  bootRun         = 0xFD,
  motorBlocked    = 0xFE,
  undefined       = 0xFF
};

enum class NukiTrigger : uint8_t {
  system          = 0x00,
  manual          = 0x01,
  button          = 0x02,
  automatic       = 0x03,
  autoLock        = 0x06
};

enum class CriticalBatteryState : uint8_t {
  oke             = 0x00,
  critical        = 0x01
};

enum class DoorSensorState : uint8_t {
  unavailable       = 0x00,
  deactivated       = 0x01,
  doorClosed        = 0x02,
  doorOpened        = 0x03,
  doorStateUnknown  = 0x04,
  calibrating       = 0x05
};

enum class LockAction : uint8_t {
  unlock          = 0x01,
  lock            = 0x02,
  unlatch         = 0x03,
  lockNgo         = 0x04,
  lockNgoUnlatch  = 0x05,
  fullLock        = 0x06,
  fobAction1      = 0x81,
  fobAction2      = 0x82,
  fobAction3      = 0x83
};

enum class CompletionStatus : uint8_t {
  success           = 0x00,
  motorBlocked      = 0x01,
  canceled          = 0x02,
  tooRecent         = 0x03,
  busy              = 0x04,
  lowMotorVoltage   = 0x05,
  clutchFailure     = 0x06,
  motorPowerFailure = 0x07,
  incompleteFailure = 0x08,
  invalidCode       = 0xE0,
  otherError        = 0xFE,
  unknown           = 0xFF
};

enum class ButtonPressAction : uint8_t {
  noAction          = 0x00,
  intelligent       = 0x01,
  unlock            = 0x02,
  lock              = 0x03,
  unlatch           = 0x04,
  lockNgo           = 0x05,
  showStatus        = 0x06
};

enum class BatteryType : uint8_t {
  alkali            = 0x00,
  accumulators      = 0x01,
  lithium           = 0x02
};

enum class LoggingType : uint8_t {
  loggingEnabled            = 0x01,
  lockAction                = 0x02,
  calibration               = 0x03,
  initializationRun         = 0x04,
  keypadAction              = 0x05,
  doorSensor                = 0x06,
  doorSensorLoggingEnabled  = 0x07
};

struct __attribute__((packed)) NewKeypadEntry {
  uint32_t code;  //needs to be 6 digits
  uint8_t name[20];
  uint8_t timeLimited;
  uint16_t allowedFromYear;
  uint8_t allowedFromMonth;
  uint8_t allowedFromDay;
  uint8_t allowedFromHour;
  uint8_t allowedFromMin;
  uint8_t allowedFromSec;
  uint16_t allowedUntillYear;
  uint8_t allowedUntillMonth;
  uint8_t allowedUntillDay;
  uint8_t allowedUntillHour;
  uint8_t allowedUntillMin;
  uint8_t allowedUntillSec;
  uint8_t allowedWeekdays;
  uint8_t allowedFromTimeHour;
  uint8_t allowedFromTimeMin;
  uint8_t allowedUntillTimeHour;
  uint8_t allowedUntillTimeMin;
};

struct __attribute__((packed)) KeypadEntry {
  uint16_t codeId;
  uint32_t code;
  uint8_t name[20];
  uint8_t enabled;
  uint16_t dateCreatedYear;
  uint8_t dateCreatedMonth;
  uint8_t dateCreatedDay;
  uint8_t dateCreatedHour;
  uint8_t dateCreatedMin;
  uint8_t dateCreatedSec;
  uint16_t dateLastActiveYear;
  uint8_t dateLastActiveMonth;
  uint8_t dateLastActiveDay;
  uint8_t dateLastActiveHour;
  uint8_t dateLastActiveMin;
  uint8_t dateLastActiveSec;
  uint16_t lockCount;
  uint8_t timeLimited;
  uint16_t allowedFromYear;
  uint8_t allowedFromMonth;
  uint8_t allowedFromDay;
  uint8_t allowedFromHour;
  uint8_t allowedFromMin;
  uint8_t allowedFromSec;
  uint16_t allowedUntillYear;
  uint8_t allowedUntillMonth;
  uint8_t allowedUntillDay;
  uint8_t allowedUntillHour;
  uint8_t allowedUntillMin;
  uint8_t allowedUntillSec;
  uint8_t allowedWeekdays;
  uint8_t allowedFromTimeHour;
  uint8_t allowedFromTimeMin;
  uint8_t allowedUntillTimeHour;
  uint8_t allowedUntillTimeMin;
};

struct __attribute__((packed)) KeyTurnerState {
  NukiState nukiState;
  LockState lockState;
  NukiTrigger trigger;
  uint16_t currentTimeYear;
  uint8_t currentTimeMonth;
  uint8_t currentTimeDay;
  uint8_t currentTimeHour;
  uint8_t currentTimeMinute;
  uint8_t currentTimeSecond;
  int16_t timeZoneOffset;
  CriticalBatteryState criticalBatteryState;
  uint8_t configUpdateCount;
  bool lockNgoTimer;
  LockAction lastLockAction;
  NukiTrigger lastLockActionTrigger;
  CompletionStatus lastLockActionCompletionStatus;
  DoorSensorState doorSensorState;
};

struct __attribute__((packed)) Config {
  uint32_t nukiId;
  unsigned char name[32];
  float latitide;
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
  int16_t timeZOneOffset;
  uint8_t dstMode;
  uint8_t  hasFob;
  uint8_t  fobAction1;
  uint8_t  fobAction2;
  uint8_t  fobAction3;
  uint8_t  singleLock;
  uint8_t advertisingMode;
  uint8_t hasKeypad;
  unsigned char firmwareVersion[3];
  unsigned char hardwareRevision[2];
  uint8_t homeKitStatus;
  uint16_t timeZoneId;
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
  uint8_t autoLockDisabled;
  uint8_t nightModeEnabled;
  unsigned char nightModeStartTime[2];
  unsigned char nightModeEndTime[2];
  uint8_t nightModeAutoLockEnabled;
  uint8_t nightModeAutoUnlockDisabled;
  uint8_t  nightModeImmediateLockOnStart;
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
