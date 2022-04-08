#pragma once

#include "NimBLEUUID.h"

namespace Nuki {

//Keyturner initialization service
const NimBLEUUID keyturnerInitServiceUUID = NimBLEUUID("a92ee000-5501-11e4-916c-0800200c9a66");
//Keyturner Pairing Service
const NimBLEUUID keyturnerPairingServiceUUID  = NimBLEUUID("a92ee100-5501-11e4-916c-0800200c9a66");
//Keyturner Service
const NimBLEUUID keyturnerServiceUUID  = NimBLEUUID("a92ee200-5501-11e4-916c-0800200c9a66");
//Keyturner pairing Data Input Output characteristic
const NimBLEUUID keyturnerGdioUUID  = NimBLEUUID("a92ee101-5501-11e4-916c-0800200c9a66");
//Keyturner Data Input Output characteristic
const NimBLEUUID keyturnerDataUUID  = NimBLEUUID("a92ee201-5501-11e4-916c-0800200c9a66");
//User-Specific Data Input Output characteristic
const NimBLEUUID userDataUUID  = NimBLEUUID("a92ee202-5501-11e4-916c-0800200c9a66");

const char BLE_ADDRESS_STORE_NAME[]       = "bleAddress";
const char SECURITY_PINCODE_STORE_NAME[]  = "securityPinCode";
const char SECRET_KEY_STORE_NAME[]        = "secretKeyK";
const char AUTH_ID_STORE_NAME[]           = "authorizationId";

enum class Command : uint16_t {
  Empty                         = 0x0000,
  RequestData	                  = 0x0001,
  PublicKey	                    = 0x0003,
  Challenge	                    = 0x0004,
  AuthorizationAuthenticator	  = 0x0005,
  AuthorizationData	            = 0x0006,
  AuthorizationId	              = 0x0007,
  RemoveUserAuthorization	      = 0x0008,
  RequestAuthorizationEntries	  = 0x0009,
  AuthorizationEntry	          = 0x000A,
  AuthorizationDatInvite	      = 0x000B,
  KeyturnerStates	              = 0x000C,
  LockAction	                  = 0x000D,
  Status	                      = 0x000E,
  MostRecentCommand	            = 0x000F,
  OpeningsClosingsSummary	      = 0x0010,
  BatteryReport	                = 0x0011,
  ErrorReport	                  = 0x0012,
  SetConfig	                    = 0x0013,
  RequestConfig	                = 0x0014,
  Config	                      = 0x0015,
  SetSecurityPin	              = 0x0019,
  RequestCalibration	          = 0x001A,
  RequestReboot	                = 0x001D,
  AuthorizationIdConfirmation	  = 0x001E,
  AuthorizationIdInvite	        = 0x001F,
  VerifySecurityPin	            = 0x0020,
  UpdateTime	                  = 0x0021,
  UpdateAuthorization	          = 0x0025,
  AuthorizationEntryCount	      = 0x0027,
  RequestLogEntries	            = 0x0031,
  LogEntry	                    = 0x0032,
  LogEntryCount	                = 0x0033,
  EnableLogging	                = 0x0034,
  SetAdvancedConfig	            = 0x0035,
  RequestAdvancedConfig	        = 0x0036,
  AdvancedConfig	              = 0x0037,
  AddTimeControlEntry	          = 0x0039,
  TimeControlEntryId	          = 0x003A,
  RemoveTimeControlEntry	      = 0x003B,
  RequestTimeControlEntries	    = 0x003C,
  TimeControlEntryCount	        = 0x003D,
  TimeControlEntry	            = 0x003E,
  UpdateTimeControlEntry	      = 0x003F,
  AddKeypadCode	                = 0x0041,
  KeypadCodeId	                = 0x0042,
  RequestKeypadCodes	          = 0x0043,
  KeypadCodeCount	              = 0x0044,
  KeypadCode	                  = 0x0045,
  UpdateKeypadCode	            = 0x0046,
  RemoveKeypadCode	            = 0x0047,
  KeypadAction	                = 0x0048,
  SimpleLockAction	            = 0x0100
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

enum class CommandStatus : uint8_t {
  Complete        = 0x00,
  Accepted        = 0x01
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

enum class DoorSensorState : uint8_t {
  Unavailable       = 0x00,
  Deactivated       = 0x01,
  DoorClosed        = 0x02,
  DoorOpened        = 0x03,
  DoorStateUnknown  = 0x04,
  Calibrating       = 0x05
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
  InvalidCode       = 0xE0,
  OtherError        = 0xFE,
  Unknown           = 0xFF
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

enum class BatteryType : uint8_t {
  Alkali            = 0x00,
  Accumulators      = 0x01,
  Lithium           = 0x02
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

enum class AdvertisingMode : uint8_t {
  Automatic                 = 0x00,
  Normal                    = 0x01,
  Slow                      = 0x02,
  Slowest                   = 0x03
};

enum class TimeZoneId : uint16_t {
  Africa_Cairo = 0,  // UTC+2 EET dst: no
  Africa_Lagos = 1,  // UTC+1 WAT dst: no
  Africa_Maputo = 2,  // UTC+2 CAT, SAST dst: no
  Africa_Nairobi = 3,  // UTC+3 EAT dst: no
  America_Anchorage = 4,  // UTC-9/-8 AKDT dst: yes
  America_Argentina_Buenos_Aires = 5,  // UTC-3 ART, UYT dst: no
  America_Chicago = 6,  // UTC-6/-5 CDT dst: yes
  America_Denver = 7,  // UTC-7/-6 MDT dst: yes
  America_Halifax = 8,  // UTC-4/-3 ADT dst: yes
  America_Los_Angeles = 9,  // UTC-8/-7 PDT dst: yes
  America_Manaus = 10,  // UTC-4 AMT, BOT, VET, AST, GYT dst: no
  America_Mexico_City = 11,  // UTC-6/-5 CDT dst: yes
  America_New_York = 12,  // UTC-5/-4 EDT dst: yes
  America_Phoenix = 13,  // UTC-7 MST dst: no
  America_Regina = 14,  // UTC-6 CST dst: no
  America_Santiago = 15,  // UTC-4/-3 CLST, AMST, WARST, PYST dst: yes
  America_Sao_Paulo = 16,  // UTC-3 BRT dst: no
  America_St_Johns = 17,  // UTC-3½/ -2½ NDT dst: yes
  Asia_Bangkok = 18,  // UTC+7 ICT, WIB dst: no
  Asia_Dubai = 19,  // UTC+4 SAMT, GET, AZT, GST, MUT, RET, SCT, AMT-Arm dst: no
  Asia_Hong_Kong = 20,  // UTC+8 HKT dst: no
  Asia_Jerusalem = 21,  // UTC+2/+3 IDT dst: yes
  Asia_Karachi = 22,  // UTC+5 PKT, YEKT, TMT, UZT, TJT, ORAT dst: no
  Asia_Kathmandu = 23,  // UTC+5¾ NPT dst: no
  Asia_Kolkata = 24,  // UTC+5½ IST dst: no
  Asia_Riyadh = 25,  // UTC+3 AST-Arabia dst: no
  Asia_Seoul = 26,  // UTC+9 KST dst: no
  Asia_Shanghai = 27,  // UTC+8 CST, ULAT, IRKT, PHT, BND, WITA dst: no
  Asia_Tehran = 28,  // UTC+3½ ARST dst: no
  Asia_Tokyo = 29,  // UTC+9 JST, WIT, PWT, YAKT dst: no
  Asia_Yangon = 30,  // UTC+6½ MMT dst: no
  Australia_Adelaide = 31,  // UTC+9½/10½ ACDT dst: yes
  Australia_Brisbane = 32,  // UTC+10 AEST, PGT, VLAT dst: no
  Australia_Darwin = 33,  // UTC+9½ ACST dst: no
  Australia_Hobart = 34,  // UTC+10/+11 AEDT dst: yes
  Australia_Perth = 35,  // UTC+8 AWST dst: no
  Australia_Sydney = 36,  // UTC+10/+11 AEDT dst: yes
  Europe_Berlin = 37,  // UTC+1/+2 CEST dst: yes
  Europe_Helsinki = 38,  // UTC+2/+3 EEST dst: yes
  Europe_Istanbul = 39,  // UTC+3 TRT dst: no
  Europe_London = 40,  // UTC+0/+1 BST, IST dst: yes
  Europe_Moscow = 41,  // UTC+3 MSK dst: no
  Pacific_Auckland = 42,  // UTC+12/+13 NZDT dst: yes
  Pacific_Guam = 43,  // UTC+10 ChST dst: no
  Pacific_Honolulu = 44,  // UTC-10 H(A)ST dst: no
  Pacific_Pago_Pago = 45,  // UTC-11 SST dst: no
  None = 65535,  //

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
  // bit 7  6  5  4  3  2  1  0
  //     -  M  T  W  T  F  S  S
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

  // bit 7  6  5  4  3  2  1  0
  //     -  M  T  W  T  F  S  S
  uint8_t allowedWeekdays;
  uint8_t allowedFromTimeHour;
  uint8_t allowedFromTimeMin;
  uint8_t allowedUntillTimeHour;
  uint8_t allowedUntillTimeMin;
};

struct __attribute__((packed)) UpdatedKeypadEntry {
  uint16_t codeId;
  uint32_t code;
  uint8_t name[20];
  uint8_t enabled;
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
  // bit 7  6  5  4  3  2  1  0
  //     -  M  T  W  T  F  S  S
  uint8_t allowedWeekdays;
  uint8_t allowedFromTimeHour;
  uint8_t allowedFromTimeMin;
  uint8_t allowedUntillTimeHour;
  uint8_t allowedUntillTimeMin;
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
};

struct __attribute__((packed)) NewConfig {
  unsigned char name[32];
  float latitide;
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

struct __attribute__((packed)) AuthorizationEntry {
  uint32_t authId;
  uint8_t idType;
  uint8_t name[32];
  uint8_t enabled;
  uint8_t remoteAllowed;
  uint16_t createdYear;
  uint8_t createdMonth;
  uint8_t createdDay;
  uint8_t createdHour;
  uint8_t createdMinute;
  uint8_t createdSecond;
  uint16_t lastActYear;
  uint8_t lastActMonth;
  uint8_t lastActDay;
  uint8_t lastActHour;
  uint8_t lastActMinute;
  uint8_t lastActSecond;
  uint16_t lockCount;
  uint8_t timeLimited;
  uint16_t allowedFromYear;
  uint8_t allowedFromMonth;
  uint8_t allowedFromDay;
  uint8_t allowedFromHour;
  uint8_t allowedFromMinute;
  uint8_t allowedFromSecond;
  uint16_t allowedUntilYear;
  uint8_t allowedUntilMonth;
  uint8_t allowedUntilDay;
  uint8_t allowedUntilHour;
  uint8_t allowedUntilMinute;
  uint8_t allowedUntilSecond;
  uint8_t allowedWeekdays;
  uint8_t allowedFromTimeHour;
  uint8_t allowedFromTimeMin;
  uint8_t allowedUntillTimeHour;
  uint8_t allowedUntillTimeMin;
};

struct __attribute__((packed)) NewAuthorizationEntry {
  uint8_t name[32];
  uint8_t idType;
  uint8_t sharedKey[32];  //TODO: add shared key within class
  uint8_t remoteAllowed;
  uint8_t timeLimited;
  uint16_t allowedFromYear;
  uint8_t allowedFromMonth;
  uint8_t allowedFromDay;
  uint8_t allowedFromHour;
  uint8_t allowedFromMinute;
  uint8_t allowedFromSecond;
  uint16_t allowedUntilYear;
  uint8_t allowedUntilMonth;
  uint8_t allowedUntilDay;
  uint8_t allowedUntilHour;
  uint8_t allowedUntilMinute;
  uint8_t allowedUntilSecond;
  uint8_t allowedWeekdays;
  uint8_t allowedFromTimeHour;
  uint8_t allowedFromTimeMin;
  uint8_t allowedUntillTimeHour;
  uint8_t allowedUntillTimeMin;
};

struct __attribute__((packed)) UpdatedAuthorizationEntry {
  uint32_t authId;
  uint8_t name[32];
  uint8_t enabled;
  uint8_t remoteAllowed;
  uint8_t timeLimited;
  uint16_t allowedFromYear;
  uint8_t allowedFromMonth;
  uint8_t allowedFromDay;
  uint8_t allowedFromHour;
  uint8_t allowedFromMinute;
  uint8_t allowedFromSecond;
  uint16_t allowedUntilYear;
  uint8_t allowedUntilMonth;
  uint8_t allowedUntilDay;
  uint8_t allowedUntilHour;
  uint8_t allowedUntilMinute;
  uint8_t allowedUntilSecond;
  uint8_t allowedWeekdays;
  uint8_t allowedFromTimeHour;
  uint8_t allowedFromTimeMin;
  uint8_t allowedUntillTimeHour;
  uint8_t allowedUntillTimeMin;
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

struct __attribute__((packed)) TimeValue {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint16_t second;
};

inline void lockstateToString(const LockState state, char* str) {
  switch (state) {
    case LockState::Uncalibrated:
      strcpy(str, "uncalibrated");
      break;
    case LockState::Locked:
      strcpy(str, "locked");
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

} // namespace Nuki