#pragma once

#include "Arduino.h"
#include "NukiDataTypes.h"

namespace Nuki {

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00)>>8) + (((x)&0xFF)<<8))

void printBuffer(const byte* buff, const uint8_t size, const boolean asChars, const char* header);
bool isCharArrayNotEmpty(unsigned char* array, uint16_t len);
bool compareCharArray(unsigned char* a, unsigned char* b, uint8_t len);
int encode(unsigned char* output, unsigned char* input, unsigned long long len, unsigned char* nonce, unsigned char* keyS);
int decode(unsigned char* output, unsigned char* input, unsigned long long len, unsigned char* nonce, unsigned char* keyS);
void generateNonce(unsigned char* hexArray, uint8_t nrOfBytes);

unsigned int calculateCrc(uint8_t data[], uint8_t start, uint16_t length);
bool crcValid(uint8_t* pData, uint16_t length);

void logErrorCode(uint8_t errorCode);
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