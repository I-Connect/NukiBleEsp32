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

    const ErrorCode getLastError() const;
    const bool isPairedWithLock() const;

    CmdResult requestKeyTurnerState(KeyTurnerState* retrievedKeyTurnerState);
    void retrieveKeyTunerState(KeyTurnerState* retrievedKeyTurnerState);
    CmdResult lockAction(const LockAction lockAction, const uint32_t nukiAppId = 1, const uint8_t flags = 0, const char* nameSuffix = nullptr);

    CmdResult requestConfig(Config* retrievedConfig);
    CmdResult requestAdvancedConfig(AdvancedConfig* retrievedAdvancedConfig);

    CmdResult requestBatteryReport(BatteryReport* retrievedBatteryReport);
    bool isBatteryCritical();
    bool isBatteryCharging();
    uint8_t getBatteryPerc();
    CmdResult retrieveLogEntries(const uint32_t startIndex, const uint16_t count, const uint8_t sortOrder, const bool totalCount);
    void getLogEntries(std::list<LogEntry>* requestedLogEntries);

    CmdResult addKeypadEntry(NewKeypadEntry newKeypadEntry);
    CmdResult updateKeypadEntry(UpdatedKeypadEntry updatedKeyPadEntry);
    CmdResult retrieveKeypadEntries(const uint16_t offset, const uint16_t count);
    void getKeypadEntries(std::list<KeypadEntry>* requestedKeyPadEntries);

    CmdResult retrieveAuthorizationEntries(const uint16_t offset, const uint16_t count);
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

    bool saveSecurityPincode(const uint16_t pinCode);
    CmdResult setSecurityPin(const uint16_t newSecurityPin);
    CmdResult verifySecurityPin();

    //basic config changes
    CmdResult enablePairing(const bool enable);
    CmdResult enableButton(const bool enable);
    CmdResult enableLedFlash(const bool enable);
    CmdResult setLedBrightness(const uint8_t level);
    CmdResult enableSingleLock(const bool enable);
    CmdResult setAdvertisingMode(const AdvertisingMode mode);
    CmdResult setName(const std::string& name);
    CmdResult enableDst(const bool enable);
    CmdResult setTimeZoneOffset(const int16_t minutes);
    CmdResult setTimeZoneId(const TimeZoneId timeZoneId);

    //advanced config changes
    CmdResult setSingleButtonPressAction(const ButtonPressAction action);
    CmdResult setDoubleButtonPressAction(const ButtonPressAction action);
    CmdResult setBatteryType(const BatteryType type);
    CmdResult enableAutoBatteryTypeDetection(const bool enable);
    CmdResult disableAutoUnlock(const bool disable);
    CmdResult enableAutoLock(const bool enable);
    CmdResult enableImmediateAutoLock(const bool enable);
    CmdResult enableAutoUpdate(const bool enable);

    void initialize();
    void registerBleScanner(BLEScannerPublisher* bleScanner);

  private:

    bool connectBle(const BLEAddress bleAddress);
    void onConnect(BLEClient*) override;
    void onDisconnect(BLEClient*) override;
    void onResult(BLEAdvertisedDevice* advertisedDevice) override;
    bool registerOnGdioChar();
    bool registerOnUsdioChar();

    void sendPlainMessage(Command commandIdentifier, const unsigned char* payload, const uint8_t payloadLen);
    void sendEncryptedMessage(Command commandIdentifier, const unsigned char* payload, const uint8_t payloadLen);

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

    unsigned char authenticator[32];
    Preferences preferences;

    BLEAddress bleAddress = BLEAddress("");
    std::string deviceName;       //The name to be displayed for this authorization and used for storing preferences
    uint32_t deviceId;            //The ID of the Nuki App, Nuki Bridge or Nuki Fob to be authorized.
    BLEClient* pClient;

    BLERemoteService* pKeyturnerPairingService = nullptr;
    BLERemoteCharacteristic* pGdioCharacteristic = nullptr;
    BLERemoteService* pKeyturnerDataService = nullptr;
    BLERemoteCharacteristic* pUsdioCharacteristic = nullptr;

    CmdResult cmdStateMachine(const Action action);
    CmdResult executeAction(const Action action);
    CmdResult cmdChallStateMachine(const Action action, const bool sendPinCode = false);
    CmdResult cmdChallAccStateMachine(const Action action);

    CommandState nukiCommandState = CommandState::Idle;

    uint32_t timeNow = 0;

    BLEScannerPublisher* bleScanner = nullptr;
    bool isPaired = false;

    SmartlockEventHandler* eventHandler;

    uint8_t receivedStatus;
    bool crcCheckOke;

    unsigned char remotePublicKey[32] = {0x00};
    unsigned char challengeNonceK[32] = {0x00};
    unsigned char authorizationId[4] = {0x00};
    unsigned char myPublicKey[32] = {0x00};
    unsigned char myPrivateKey[32] = {0x00};
    uint16_t pinCode = 0000;
    unsigned char secretKeyK[32] = {0x00};

    unsigned char sentNonce[crypto_secretbox_NONCEBYTES] = {};

    KeyTurnerState keyTurnerState;
    Config config;
    AdvancedConfig advancedConfig;
    BatteryReport batteryReport;
    ErrorCode errorCode;
    Command lastMsgCodeReceived = Command::Empty;
    uint16_t nrOfKeypadCodes = 0; // TODO : Unused?
    uint16_t logEntryCount = 0; // TODO: Unused?
    bool loggingEnabled = false;
    std::list<LogEntry> listOfLogEntries;
    std::list<KeypadEntry> listOfKeyPadEntries;
    std::list<AuthorizationEntry> listOfAuthorizationEntries;
    std::list<TimeControlEntry> listOfTimeControlEntries;
};

} // namespace Nuki