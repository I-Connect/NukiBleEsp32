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
#include "sodium/crypto_secretbox.h"

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

    NukiErrorCode getLastError() const;
    
    bool savePincode(uint16_t pinCode);

    NukiCmdResult requestKeyTurnerState(KeyTurnerState* retreivedKeyTurnerState);
    void retreiveKeyTunerState(KeyTurnerState* retreivedKeyTurnerState);
    NukiCmdResult lockAction(LockAction lockAction, uint32_t nukiAppId, uint8_t flags = 0, unsigned char* nameSuffix = nullptr);

    NukiCmdResult requestConfig(Config* retreivedConfig);
    NukiCmdResult requestAdvancedConfig(AdvancedConfig* retreivedAdvancedConfig);

    NukiCmdResult requestBatteryReport(BatteryReport* retreivedBatteryReport);
    bool batteryCritical();
    bool batteryIsCharging();
    uint8_t getBatteryPerc();
    NukiCmdResult retreiveLogEntries(uint32_t startIndex, uint16_t count, uint8_t sortOrder, bool totalCount);
    void getLogEntries(std::list<LogEntry>* requestedLogEntries);

    NukiCmdResult addKeypadEntry(NewKeypadEntry newKeypadEntry);
    NukiCmdResult updateKeypadEntry(UpdatedKeypadEntry updatedKeyPadEntry);
    NukiCmdResult retreiveKeypadEntries(uint16_t offset, uint16_t count);
    void getKeypadEntries(std::list<KeypadEntry>* requestedKeyPadEntries);

    NukiCmdResult retreiveAuthorizationEntries(uint16_t offset, uint16_t count);
    void getAuthorizationEntries(std::list<AuthorizationEntry>* requestedAuthorizationEntries);
    NukiCmdResult addAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry);
    NukiCmdResult updateAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry);

    NukiCmdResult requestCalibration();
    NukiCmdResult requestReboot();

    NukiCmdResult updateTime(TimeValue time);

    NukiCmdResult addTimeControlEntry(NewTimeControlEntry newTimecontrolEntry);
    NukiCmdResult updateTimeControlEntry(TimeControlEntry TimeControlEntry);
    NukiCmdResult removeTimeControlEntry(uint8_t entryId);
    NukiCmdResult retrieveTimeControlEntries();
    void getTimeControlEntries(std::list<TimeControlEntry>* timeControlEntries);

    NukiCmdResult setSecurityPin(uint16_t newSecurityPin);
    NukiCmdResult verifySecurityPin();

    //basic config changes
    NukiCmdResult enablePairing(bool enable);
    NukiCmdResult enableButton(bool enable);
    NukiCmdResult enableLedFlash(bool enable);
    NukiCmdResult setLedBrightness(uint8_t level);
    NukiCmdResult enableSingleLock(bool enable);
    NukiCmdResult setAdvertisingMode(AdvertisingMode mode);
    NukiCmdResult setName(std::string name);
    NukiCmdResult enableDst(bool enable);
    NukiCmdResult setTimeZoneOffset(int16_t minutes);
    NukiCmdResult setTimeZoneId(TimeZoneId timeZoneId);

    //advanced config changes
    NukiCmdResult setSingleButtonPressAction(ButtonPressAction action);
    NukiCmdResult setDoubleButtonPressAction(ButtonPressAction action);
    NukiCmdResult setBatteryType(BatteryType type);
    NukiCmdResult enableAutoBatteryTypeDetection(bool enable);
    NukiCmdResult disableAutoUnlock(bool disable);
    NukiCmdResult enableAutoLock(bool enable);
    NukiCmdResult enableImmediateAutoLock(bool enable);
    NukiCmdResult enableAutoUpdate(bool enable);

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

    void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
    void handleReturnMessage(NukiCommand returnCode, unsigned char* data, uint16_t dataLen);
    void saveCredentials();
    bool retrieveCredentials();
    void deleteCredentials();
    NukiPairingState pairStateMachine(const NukiPairingState nukiPairingState);

    NukiCmdResult setConfig(NewConfig newConfig);
    NukiCmdResult setAdvancedConfig(NewAdvancedConfig newAdvancedConfig);
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
    NukiCmdResult executeAction(NukiAction action);
    NukiCmdResult cmdChallStateMachine(NukiAction action, bool sendPinCode = false);
    NukiCmdResult cmdChallAccStateMachine(NukiAction action);

    NukiCommandState nukiCommandState = NukiCommandState::Idle;

    uint32_t timeNow = 0;

    BleScanner bleScanner;
    bool isPaired = false;

    NukiSmartlockEventHandler* eventHandler;

    uint8_t receivedStatus;
    bool crcCheckOke;

    unsigned char remotePublicKey[32] = {0x00};
    unsigned char challengeNonceK[32] = {0x00};
    unsigned char authorizationId[4] = {0x00};
    uint16_t pinCode = 0000;
    unsigned char lockId[16];
    unsigned char secretKeyK[32] = {0x00};

    unsigned char sentNonce[crypto_secretbox_NONCEBYTES] = {};

    KeyTurnerState keyTurnerState;
    Config config;
    AdvancedConfig advancedConfig;
    BatteryReport batteryReport;
    NukiErrorCode errorCode;
    NukiCommand lastMsgCodeReceived = NukiCommand::Empty;
    uint16_t nrOfKeypadCodes = 0;
    uint16_t logEntryCount = 0;
    bool loggingEnabled = false;
    std::list<LogEntry> listOfLogEntries;
    std::list<KeypadEntry> listOfKeyPadEntries;
    std::list<AuthorizationEntry> listOfAuthorizationEntries;
    std::list<TimeControlEntry> listOfTimeControlEntries;
};