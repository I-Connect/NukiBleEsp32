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

    const char BLE_ADDRESS_STORE_NAME[]       = "BleAddress";
    const char SECURITY_PINCODE_STORE_NAME[]  = "SecurityPinCode";
    const char SECRET_KEY_STORE_NAME[]        = "SecretKeyK";
    const char AUTH_ID_STORE_NAME[]           = "AuthorizationId";

    enum class LockAction: uint8_t
    {
        ActivateRTO = 0x01,
        DeactivateRTO = 0x02,
        ElectricStrikeActuation = 0x03,

        ActivateCM = 0x04,
        DeactivateCM = 0x05,

        FobAction1 = 0x81,
        FobAction2 = 0x82,
        FobAction3 = 0x83
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

    enum class CommandStatus : uint8_t {
        Complete        = 0x00,
        Accepted        = 0x01
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
        uint8_t ringToOpenTimer;
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
        Nuki::BatteryType batteryType;
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

} // namespace Nuki