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

namespace Nuki {

class NukiBle : public BLEClientCallbacks, BLEScannerSubscriber {
  public:
    NukiBle(const std::string& deviceName, const uint32_t deviceId);
    virtual ~NukiBle();

    void setEventHandler(SmartlockEventHandler* handler);

    bool pairNuki();
    void unPairNuki();

    ErrorCode getLastError() const;

    bool savePincode(uint16_t pinCode);

    CmdResult requestKeyTurnerState(KeyTurnerState* retrievedKeyTurnerState);
    void retrieveKeyTunerState(KeyTurnerState* retrievedKeyTurnerState);
    CmdResult lockAction(LockAction lockAction, uint32_t nukiAppId, uint8_t flags = 0, unsigned char* nameSuffix = nullptr);

    CmdResult requestConfig(Config* retrievedConfig);
    CmdResult requestAdvancedConfig(AdvancedConfig* retrievedAdvancedConfig);

    CmdResult requestBatteryReport(BatteryReport* retrievedBatteryReport);
    bool batteryCritical();
    bool batteryIsCharging();
    uint8_t getBatteryPerc();
    CmdResult retrieveLogEntries(uint32_t startIndex, uint16_t count, uint8_t sortOrder, bool totalCount);
    void getLogEntries(std::list<LogEntry>* requestedLogEntries);

    CmdResult addKeypadEntry(NewKeypadEntry newKeypadEntry);
    CmdResult updateKeypadEntry(UpdatedKeypadEntry updatedKeyPadEntry);
    CmdResult retrieveKeypadEntries(uint16_t offset, uint16_t count);
    void getKeypadEntries(std::list<KeypadEntry>* requestedKeyPadEntries);

    CmdResult retrieveAuthorizationEntries(uint16_t offset, uint16_t count);
    void getAuthorizationEntries(std::list<AuthorizationEntry>* requestedAuthorizationEntries);
    CmdResult addAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry);
    CmdResult updateAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry);

    CmdResult requestCalibration();
    CmdResult requestReboot();

    CmdResult updateTime(TimeValue time);

    CmdResult addTimeControlEntry(NewTimeControlEntry newTimecontrolEntry);
    CmdResult updateTimeControlEntry(TimeControlEntry TimeControlEntry);
    CmdResult removeTimeControlEntry(uint8_t entryId);
    CmdResult retrieveTimeControlEntries();
    void getTimeControlEntries(std::list<TimeControlEntry>* timeControlEntries);

    CmdResult setSecurityPin(uint16_t newSecurityPin);
    CmdResult verifySecurityPin();

    //basic config changes
    CmdResult enablePairing(bool enable);
    CmdResult enableButton(bool enable);
    CmdResult enableLedFlash(bool enable);
    CmdResult setLedBrightness(uint8_t level);
    CmdResult enableSingleLock(bool enable);
    CmdResult setAdvertisingMode(AdvertisingMode mode);
    CmdResult setName(std::string name);
    CmdResult enableDst(bool enable);
    CmdResult setTimeZoneOffset(int16_t minutes);
    CmdResult setTimeZoneId(TimeZoneId timeZoneId);

    //advanced config changes
    CmdResult setSingleButtonPressAction(ButtonPressAction action);
    CmdResult setDoubleButtonPressAction(ButtonPressAction action);
    CmdResult setBatteryType(BatteryType type);
    CmdResult enableAutoBatteryTypeDetection(bool enable);
    CmdResult disableAutoUnlock(bool disable);
    CmdResult enableAutoLock(bool enable);
    CmdResult enableImmediateAutoLock(bool enable);
    CmdResult enableAutoUpdate(bool enable);

    virtual void initialize();
    void update();

  private:

    bool connectBle(BLEAddress bleAddress);
    void onConnect(BLEClient*) override;
    void onDisconnect(BLEClient*) override;
    void onResult(BLEAdvertisedDevice* advertisedDevice) override;
    bool registerOnGdioChar();
    bool registerOnUsdioChar();

    void sendPlainMessage(Command commandIdentifier, unsigned char* payload, uint8_t payloadLen);
    void sendEncryptedMessage(Command commandIdentifier, unsigned char* payload, uint8_t payloadLen);

    void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
    void handleReturnMessage(Command returnCode, unsigned char* data, uint16_t dataLen);
    void saveCredentials();
    bool retrieveCredentials();
    void deleteCredentials();
    PairingState pairStateMachine(const PairingState nukiPairingState);

    CmdResult setConfig(NewConfig newConfig);
    CmdResult setFromConfig(const Config config);
    CmdResult setAdvancedConfig(NewAdvancedConfig newAdvancedConfig);
    void createNewConfig(const Config* oldConfig, NewConfig* newConfig);
    void createNewAdvancedConfig(const AdvancedConfig* oldConfig, NewAdvancedConfig* newConfig);
    CmdResult setFromAdvancedConfig(const AdvancedConfig config);

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

    CmdResult cmdStateMachine(Action action);
    CmdResult executeAction(Action action);
    CmdResult cmdChallStateMachine(Action action, bool sendPinCode = false);
    CmdResult cmdChallAccStateMachine(Action action);

    CommandState nukiCommandState = CommandState::Idle;

    uint32_t timeNow = 0;

    BleScanner bleScanner;
    bool isPaired = false;

    SmartlockEventHandler* eventHandler;

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
    ErrorCode errorCode;
    Command lastMsgCodeReceived = Command::Empty;
    uint16_t nrOfKeypadCodes = 0;
    uint16_t logEntryCount = 0;
    bool loggingEnabled = false;
    std::list<LogEntry> listOfLogEntries;
    std::list<KeypadEntry> listOfKeyPadEntries;
    std::list<AuthorizationEntry> listOfAuthorizationEntries;
    std::list<TimeControlEntry> listOfTimeControlEntries;
};

} // namespace Nuki