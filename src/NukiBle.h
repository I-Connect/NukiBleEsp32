#pragma once
/*
 * nukiBle.h
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "NimBLEDevice.h"
#include "NukiConstants.h"
#include "NukiDataTypes.h"
#include "Arduino.h"
#include <Preferences.h>
#include <esp_task_wdt.h>
#include <BleScanner.h>

#define GENERAL_TIMEOUT 10000
#define CMD_TIMEOUT 10000
#define PAIRING_TIMEOUT 30000


class NukiBle : public BLEClientCallbacks, BLEScannerSubscriber {
  public:
    NukiBle(const std::string& deviceName, const uint32_t deviceId);
    virtual ~NukiBle();

    void setEventHandler(NukiSmartlockEventHandler* handler);

    bool pairNuki();
    void unPairNuki();
    bool savePincode(uint16_t pinCode);

    uint8_t requestKeyTurnerState(KeyTurnerState* retreivedKeyTurnerState);
    void retreiveKeyTunerState(KeyTurnerState* retreivedKeyTurnerState);
    uint8_t lockAction(LockAction lockAction, uint32_t nukiAppId, uint8_t flags = 0, unsigned char* nameSuffix = nullptr);

    uint8_t requestConfig(Config* retreivedConfig);
    uint8_t requestAdvancedConfig(AdvancedConfig* retreivedAdvancedConfig);

    uint8_t requestBatteryReport(BatteryReport* retreivedBatteryReport);
    bool batteryCritical();
    bool batteryIsCharging();
    uint8_t getBatteryPerc();
    uint8_t retreiveLogEntries(uint32_t startIndex, uint16_t count, uint8_t sortOrder, bool totalCount);
    void getLogEntries(std::list<LogEntry>* requestedLogEntries);

    uint8_t addKeypadEntry(NewKeypadEntry newKeypadEntry);
    uint8_t updateKeypadEntry(UpdatedKeypadEntry updatedKeyPadEntry);
    uint8_t retreiveKeypadEntries(uint16_t offset, uint16_t count);
    void getKeypadEntries(std::list<KeypadEntry>* requestedKeyPadEntries);

    uint8_t retreiveAuthorizationEntries(uint16_t offset, uint16_t count);
    void getAuthorizationEntries(std::list<AuthorizationEntry>* requestedAuthorizationEntries);
    uint8_t addAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry);
    uint8_t updateAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry);

    uint8_t requestCalibration();
    uint8_t requestReboot();

    uint8_t updateTime(TimeValue time);

    uint8_t addTimeControlEntry(NewTimeControlEntry newTimecontrolEntry);
    uint8_t updateTimeControlEntry(TimeControlEntry TimeControlEntry);
    uint8_t removeTimeControlEntry(uint8_t entryId);
    uint8_t retreiveTimeControlEntries();
    void getTimeControlEntries(std::list<TimeControlEntry>* timeControlEntries);

    uint8_t setSecurityPin(uint16_t newSecurityPin);
    uint8_t verifySecurityPin();

    //basic config changes
    uint8_t setName(std::string name);
    uint8_t enablePairing(bool enable);
    uint8_t enableButton(bool enable);
    uint8_t enableLedFlash(bool enable);
    uint8_t setLedBrightness(uint8_t level);
    uint8_t enableSingleLock(bool enable);
    uint8_t setAdvertisingMode(AdvertisingMode mode);
    uint8_t enableDst(bool enable);
    uint8_t setTimeZoneOffset(int16_t minutes);
    uint8_t setTimeZoneId(TimeZoneId timeZoneId);

    //advanced config changes
    uint8_t setSingleButtonPressAction(ButtonPressAction action);
    uint8_t setDoubleButtonPressAction(ButtonPressAction action);
    uint8_t setBatteryType(BatteryType type);
    uint8_t enableAutoBatteryTypeDetection(bool enable);
    uint8_t disableAutoUnlock(bool disable);
    uint8_t enableAutoLock(bool enable);
    uint8_t enableImmediateAutoLock(bool enable);
    uint8_t enableAutoUpdate(bool enable);

    virtual void initialize();
    void update();

  private:

    bool connectBle(BLEAddress bleAddress);
    void onConnect(BLEClient*) override;
    void onDisconnect(BLEClient*) override;
    void onResult(BLEAdvertisedDevice* advertisedDevice) override;
    bool registerOnGdioChar();
    bool registerOnUsdioChar();

    void sendPlainMessage(NukiCommand commandIdentifier, unsigned char* payload, uint8_t payloadLen);
    void sendEncryptedMessage(NukiCommand commandIdentifier, unsigned char* payload, uint8_t payloadLen);

    static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
    static void handleReturnMessage(NukiCommand returnCode, unsigned char* data, uint16_t dataLen);
    void saveCredentials();
    bool retreiveCredentials();
    void deleteCredentials();
    uint8_t pairStateMachine();
    static bool crcValid(uint8_t* pData, uint16_t length);

    uint8_t setConfig(NewConfig newConfig);
    uint8_t setAdvancedConfig(NewAdvancedConfig newAdvancedConfig);
    void createNewConfig(Config* oldConfig, NewConfig* newConfig);
    void createNewAdvancedConfig(AdvancedConfig* oldConfig, NewAdvancedConfig* newConfig);

    //TODO generate public and private keys?
    const unsigned char myPrivateKey[32] = {0x8C, 0xAA, 0x54, 0x67, 0x23, 0x07, 0xBF, 0xFD, 0xF5, 0xEA, 0x18, 0x3F, 0xC6, 0x07, 0x15, 0x8D, 0x20, 0x11, 0xD0, 0x08, 0xEC, 0xA6, 0xA1, 0x08, 0x86, 0x14, 0xFF, 0x08, 0x53, 0xA5, 0xAA, 0x07};
    unsigned char myPublicKey[32] = {0xF8, 0x81, 0x27, 0xCC, 0xF4, 0x80, 0x23, 0xB5, 0xCB, 0xE9, 0x10, 0x1D, 0x24, 0xBA, 0xA8, 0xA3, 0x68, 0xDA, 0x94, 0xE8, 0xC2, 0xE3, 0xCD, 0xE2, 0xDE, 0xD2, 0x9C, 0xE9, 0x6A, 0xB5, 0x0C, 0x15};
    unsigned char authenticator[32];
    Preferences preferences;

    BLEAddress bleAddress;
    std::string deviceName;       //The name to be displayed for this authorization and used for storing preferences
    uint32_t deviceId;            //The ID of the Nuki App, Nuki Bridge or Nuki Fob to be authorized.
    BLEClient* pClient;
    BLEScan* pBLEScan;
    BLERemoteService* pKeyturnerPairingService = nullptr;
    BLERemoteCharacteristic* pGdioCharacteristic = nullptr;

    BLERemoteService* pKeyturnerDataService = nullptr;
    BLERemoteCharacteristic* pUsdioCharacteristic = nullptr;

    NukiCmdResult cmdStateMachine(NukiAction action);
    uint8_t executeAction(NukiAction action);
    NukiCmdResult cmdChallStateMachine(NukiAction action, bool sendPinCode = false);
    NukiCmdResult cmdChallAccStateMachine(NukiAction action);

    NukiPairingState nukiPairingState = NukiPairingState::initPairing;
    NukiCommandState nukiCommandState = NukiCommandState::idle;

    uint32_t timeNow = 0;

    BleScanner bleScanner;
    bool isPaired = false;
    std::string keyTurnerUUIDString;

    NukiSmartlockEventHandler* eventHandler;
};