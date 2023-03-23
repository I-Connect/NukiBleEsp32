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


void logLockErrorCode(uint8_t errorCode);
void logConfig(Config config);
void logNewConfig(NewConfig newConfig);
void logNewKeypadEntry(NewKeypadEntry newKeypadEntry);
void logKeypadEntry(KeypadEntry keypadEntry);
void logUpdatedKeypadEntry(UpdatedKeypadEntry updatedKeypadEntry);
void logAuthorizationEntry(AuthorizationEntry authorizationEntry);
void logNewAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry);
void logUpdatedAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry);
void logNewTimeControlEntry(NewTimeControlEntry newTimeControlEntry);
void logTimeControlEntry(TimeControlEntry timeControlEntry);
void logCompletionStatus(CompletionStatus completionStatus);
void logNukiTrigger(Trigger nukiTrigger);
void logLockAction(LockAction lockAction);
void logKeyturnerState(KeyTurnerState keyTurnerState);
void logBatteryReport(BatteryReport batteryReport);
void logLogEntry(LogEntry logEntry);
void logAdvancedConfig(AdvancedConfig advancedConfig);
void logNewAdvancedConfig(NewAdvancedConfig newAdvancedConfig);

} // namespace Nuki