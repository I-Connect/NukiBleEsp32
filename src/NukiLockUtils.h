#pragma once

/**
 * @file NukiUtills.h
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

#include "Arduino.h"
#include "NukiDataTypes.h"
#include "NukiLockConstants.h"
#include <bitset>

namespace NukiLock {

void cmdResultToString(const CmdResult state, char* str);


void logLockErrorCode(uint8_t errorCode, bool debug = false, Print* Log = nullptr);
void logConfig(Config config, bool debug = false, Print* Log = nullptr);
void logNewConfig(NewConfig newConfig, bool debug = false, Print* Log = nullptr);
void logNewKeypadEntry(NewKeypadEntry newKeypadEntry, bool debug = false, Print* Log = nullptr);
void logKeypadEntry(KeypadEntry keypadEntry, bool debug = false, Print* Log = nullptr);
void logUpdatedKeypadEntry(UpdatedKeypadEntry updatedKeypadEntry, bool debug = false, Print* Log = nullptr);
void logAuthorizationEntry(AuthorizationEntry authorizationEntry, bool debug = false, Print* Log = nullptr);
void logNewAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry, bool debug = false, Print* Log = nullptr);
void logUpdatedAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry, bool debug = false, Print* Log = nullptr);
void logNewTimeControlEntry(NewTimeControlEntry newTimeControlEntry, bool debug = false, Print* Log = nullptr);
void logTimeControlEntry(TimeControlEntry timeControlEntry, bool debug = false, Print* Log = nullptr);
void logCompletionStatus(CompletionStatus completionStatus, bool debug = false, Print* Log = nullptr);
void logNukiTrigger(Trigger nukiTrigger, bool debug = false, Print* Log = nullptr);
void logLockAction(LockAction lockAction, bool debug = false, Print* Log = nullptr);
void logKeyturnerState(KeyTurnerState keyTurnerState, bool debug = false, Print* Log = nullptr);
void logBatteryReport(BatteryReport batteryReport, bool debug = false, Print* Log = nullptr);
void logLogEntry(LogEntry logEntry, bool debug = false, Print* Log = nullptr);
void logAdvancedConfig(AdvancedConfig advancedConfig, bool debug = false, Print* Log = nullptr);
void logNewAdvancedConfig(NewAdvancedConfig newAdvancedConfig, bool debug = false, Print* Log = nullptr);
void logMqttConfig(MqttConfig mqttConfig, bool debug = false, Print* Log = nullptr);
void logMqttConfigForMigration(MqttConfigForMigration mqttConfigForMigration, bool debug = false, Print* Log = nullptr);
void logWifiScanEntry(WifiScanEntry wifiScanEntry, bool debug = false, Print* Log = nullptr);
void logKeypad2Config(Keypad2Config keypad2Config, bool debug = false, Print* Log = nullptr);
void logDoorSensorConfig(DoorSensorConfig doorSensorConfig, bool debug = false, Print* Log = nullptr);
void logWifiConfig(WifiConfig wifiConfig, bool debug = false, Print* Log = nullptr);
void logWifiConfigForMigration(WifiConfigForMigration wifiConfigForMigration, bool debug = false, Print* Log = nullptr);
void logGeneralStatistics(GeneralStatistics generalStatistics, bool debug = false, Print* Log = nullptr);
void logDailyStatistics(DailyStatistics dailyStatistics, bool debug = false, Print* Log = nullptr);
void logFingerprintEntry(FingerprintEntry fingerprintEntry, bool debug = false, Print* Log = nullptr);
void logInternalLogEntry(InternalLogEntry internalLogEntry, bool debug = false, Print* Log = nullptr);
void logAccessoryInfo(AccessoryInfo accessoryInfo, bool debug = false, Print* Log = nullptr);
void logMessageVar(const char* message, unsigned int var, Print* Log = nullptr, int level = 4);
void logMessageVar(const char* message, const char* var, Print* Log = nullptr, int level = 4);
void logMessageVar(const char* message, const float var, Print* Log = nullptr, int level = 4);
void logMessage(const char* message, Print* Log = nullptr, int level = 4);

} // namespace Nuki