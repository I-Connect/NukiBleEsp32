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

#include <cstdint>
#include "NimBLEUUID.h"

namespace Nuki {

const char BLE_ADDRESS_STORE_NAME[]       = "bleAddress";
const char SECURITY_PINCODE_STORE_NAME[]  = "securityPinCode";
const char SECRET_KEY_STORE_NAME[]        = "secretKeyK";
const char AUTH_ID_STORE_NAME[]           = "authorizationId";

enum class DoorSensorState : uint8_t {
  Unavailable       = 0x00,
  Deactivated       = 0x01,
  DoorClosed        = 0x02,
  DoorOpened        = 0x03,
  DoorStateUnknown  = 0x04,
  Calibrating       = 0x05
};

enum class BatteryType : uint8_t {
  Alkali            = 0x00,
  Accumulators      = 0x01,
  Lithium           = 0x02
};

enum class AdvertisingMode : uint8_t {
  Automatic                 = 0x00,
  Normal                    = 0x01,
  Slow                      = 0x02,
  Slowest                   = 0x03
};

enum class CommandType {
  Command                       = 0,
  CommandWithChallenge          = 1,
  CommandWithChallengeAndAccept = 2,
  CommandWithChallengeAndPin    = 3
};


enum class CommandStatus : uint8_t {
  Complete        = 0x00,
  Accepted        = 0x01
};

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
  OpeningsClosingsSummary	      = 0x0010,  // Lock only (+ NUKI v1 only)
  BatteryReport	                = 0x0011,
  ErrorReport	                  = 0x0012,
  SetConfig	                    = 0x0013,
  RequestConfig	                = 0x0014,
  Config	                      = 0x0015,
  SetSecurityPin	              = 0x0019,
  RequestCalibration	          = 0x001A, // SetCalibrated for Opener
  RequestReboot	                = 0x001D,
  AuthorizationIdConfirmation	  = 0x001E,
  AuthorizationIdInvite	        = 0x001F,
  VerifySecurityPin	            = 0x0020,
  UpdateTime	                  = 0x0021,
  UpdateAuthorization	          = 0x0025,
  AuthorizationEntryCount	      = 0x0027,
  StartBusSignalRecording       = 0x002F, // Opener only
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
  ContinuousModeAction     	     = 0x0057, // Opener only
  SimpleLockAction	            = 0x0100
};

enum class AuthorizationIdType : uint8_t {
  App = 0,
  Bridge = 1,
  Fob = 2,
  Keypad = 3
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
  uint16_t allowedUntilYear;
  uint8_t allowedUntilMonth;
  uint8_t allowedUntilDay;
  uint8_t allowedUntilHour;
  uint8_t allowedUntilMin;
  uint8_t allowedUntilSec;
  // bit 7  6  5  4  3  2  1  0
  //     -  M  T  W  T  F  S  S
  uint8_t allowedWeekdays;
  uint8_t allowedFromTimeHour;
  uint8_t allowedFromTimeMin;
  uint8_t allowedUntilTimeHour;
  uint8_t allowedUntilTimeMin;
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
  uint16_t allowedUntilYear;
  uint8_t allowedUntilMonth;
  uint8_t allowedUntilDay;
  uint8_t allowedUntilHour;
  uint8_t allowedUntilMin;
  uint8_t allowedUntilSec;

  // bit 7  6  5  4  3  2  1  0
  //     -  M  T  W  T  F  S  S
  uint8_t allowedWeekdays;
  uint8_t allowedFromTimeHour;
  uint8_t allowedFromTimeMin;
  uint8_t allowedUntilTimeHour;
  uint8_t allowedUntilTimeMin;
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
  uint16_t allowedUntilYear;
  uint8_t allowedUntilMonth;
  uint8_t allowedUntilDay;
  uint8_t allowedUntilHour;
  uint8_t allowedUntilMin;
  uint8_t allowedUntilSec;
  // bit 7  6  5  4  3  2  1  0
  //     -  M  T  W  T  F  S  S
  uint8_t allowedWeekdays;
  uint8_t allowedFromTimeHour;
  uint8_t allowedFromTimeMin;
  uint8_t allowedUntilTimeHour;
  uint8_t allowedUntilTimeMin;
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
  uint8_t allowedUntilTimeHour;
  uint8_t allowedUntilTimeMin;
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
  uint8_t allowedUntilTimeHour;
  uint8_t allowedUntilTimeMin;
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
  uint8_t allowedUntilTimeHour;
  uint8_t allowedUntilTimeMin;
};

struct __attribute__((packed)) TimeValue {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};
} // namespace Nuki
