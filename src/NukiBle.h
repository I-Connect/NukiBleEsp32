#pragma once
/**
 * @file NukiBle.h
 *
 * Created: 2022
 * License: GNU GENERAL PUBLIC LICENSE (see LICENSE)
 *
 * This library implements the communication from an ESP32 via BLE to a Nuki smart lock.
 * Based on the Nuki Smart Lock API V2.2.1
 * https://developer.nuki.io/page/nuki-smart-lock-api-2/2/
 *
 */

#include "RTOS.h"
#include "NimBLEDevice.h"
#include "NukiConstants.h"
#include "NukiDataTypes.h"
#include "Arduino.h"
#include <Preferences.h>
#include <esp_task_wdt.h>
#include <BleInterfaces.h>
#include "sodium/crypto_secretbox.h"

#define GENERAL_TIMEOUT 10000
#define CMD_TIMEOUT 10000
#define PAIRING_TIMEOUT 30000

namespace Nuki {

class NukiBle : public BLEClientCallbacks, public BleScanner::Subscriber {
  public:
    NukiBle(const std::string& deviceName, const uint32_t deviceId);
    virtual ~NukiBle();

    /**
     * @brief Set the Event Handler object
     *
     * @param handler method to handle the event
     */
    void setEventHandler(SmartlockEventHandler* handler);

    /**
     * @brief Checks if credentials are stored in preferences, if not initiate pairing
     *
     * @return
     */
    PairingResult pairNuki();

    /**
     * @brief Delete stored credentials
     */
    void unPairNuki();

    /**
     * @brief Get the Last Error code received from the lock
     */
    const ErrorCode getLastError() const;

    /**
     * @brief Returns pairing state (if credentials are stored or not)
     */
    const bool isPairedWithLock() const;

    /**
     * @brief Requests keyturner state from Lock via BLE
     *
     * @param retrievedKeyTurnerState Nuki api based datatype to store the retrieved keyturnerstate
     */
    CmdResult requestKeyTurnerState(KeyTurnerState* retrievedKeyTurnerState);

    /**
     * @brief Gets the last keyturner state stored on the esp
     *
     * @param retrievedKeyTurnerState Nuki api based datatype to store the retrieved keyturnerstate
     */
    void retrieveKeyTunerState(KeyTurnerState* retrievedKeyTurnerState);

    /**
     * @brief Sends lock action cmd via BLE to the lock
     *
     * @param lockAction
     * @param nukiAppId 0 = App, 1 = Bridge, 2 = Fob, 3 = Keypad
     * @param flags optional
     * @param nameSuffix optional
     * @param nameSuffixLen len of nameSuffix if used
     * @return CmdResult
     */
    CmdResult lockAction(const LockAction lockAction, const uint32_t nukiAppId = 1, const uint8_t flags = 0,
                         const char* nameSuffix = nullptr, const uint8_t nameSuffixLen = 0);

    /**
     * @brief Requests config from Lock via BLE
     *
     * @param retrievedConfig Nuki api based datatype to store the retrieved config
     */
    CmdResult requestConfig(Config* retrievedConfig);

    /**
     * @brief Requests advanced config from Lock via BLE
     *
     * @param retrievedAdvancedConfig Nuki api based datatype to store the retrieved advanced config
     */
    CmdResult requestAdvancedConfig(AdvancedConfig* retrievedAdvancedConfig);

    /**
     * @brief Requests battery status from Lock via BLE
     *
     * @param retrievedBatteryReport Nuki api based datatype to store the retrieved battery status
     */
    CmdResult requestBatteryReport(BatteryReport* retrievedBatteryReport);

    /**
     * @brief Returns battery critical state parsed from the battery state byte (battery critical byte)
     *
     * Note that `retrieveKeyTunerState()` needs to be called first to retrieve the needed data
     *
     * @return true if critical
     */
    bool isBatteryCritical();

    /**
     * @brief Returns battery charging state parsed from the battery state byte (battery critical byte)
     *
     * Note that `retrieveKeyTunerState()` needs to be called first to retrieve the needed data
     *
     * @return true if charging
     */
    bool isBatteryCharging();

    /**
     * @brief Returns battery charge percentage state parsed from the battery state byte (battery critical byte)
     *
     * Note that `retrieveKeyTunerState()` needs to be called first to retrieve the needed data
     *
     * @return percentage
     */
    uint8_t getBatteryPerc();

    /**
     * @brief Request the lock via BLE to send the log entries
     *
     * @param startIndex Startindex of first log msg to be send
     * @param count The number of log entries to be read, starting at the specified start index.
     * @param sortOrder The desired sort order
     * @param totalCount true if a Log Entry Count is requested from teh lock
     */
    CmdResult retrieveLogEntries(const uint32_t startIndex, const uint16_t count, const uint8_t sortOrder,
                                 const bool totalCount);

    /**
     * @brief Get the Log Entries stored on the esp. Only available after executing retreieveLogEntries.
     *
     * @param requestedLogEntries list to store the returned log entries
     */
    void getLogEntries(std::list<LogEntry>* requestedLogEntries);

    /**
     * @brief Returns the log entry count. Only available after executing retreieveLogEntries.
     */
    uint16_t getLogEntryCount();

    /**
     * @brief Send a new keypad entry to the lock via BLE
     *
     * @param newKeypadEntry Nuki api based datatype to be sent
     * Keypad Codes that start with 12 are not allowed
     * 0 is not allowed
     * Duplicates are not allowed
     */
    CmdResult addKeypadEntry(NewKeypadEntry newKeypadEntry);

    /**
     * @brief Send an updated keypad entry to the lock via BLE
     *
     * @param updatedKeypadEntry Nuki api based datatype to be sent
     * Keypad Codes that start with 12 are not allowed
     * 0 is not allowed
     * Duplicates are not allowed
     */
    CmdResult updateKeypadEntry(UpdatedKeypadEntry updatedKeyPadEntry);

    /**
     * @brief Request the lock via BLE to send the existing keypad entries
     *
     * @param offset The start offset to be read.
     * @param count The number of entries to be read, starting at the specified offset.
     */
    CmdResult retrieveKeypadEntries(const uint16_t offset, const uint16_t count);

    /**
     * @brief Get the Keypad Entries stored on the esp (after executing retreieveLogKeypadEntries)
     *
     * @param requestedKeyPadEntries list to store the returned Keypad entries
     */
    void getKeypadEntries(std::list<KeypadEntry>* requestedKeyPadEntries);

    /**
     * @brief Request the lock via BLE to send the existing authorizationentries
     *
     * @param offset The start offset to be read.
     * @param count The number of entries to be read, starting at the specified offset.
     */
    CmdResult retrieveAuthorizationEntries(const uint16_t offset, const uint16_t count);

    /**
     * @brief Get the Authorization Entries stored on the esp (after executing retreiveAuthorizationEntries)
     *
     * @param requestedAuthorizationEntries list to store the returned Authorization entries
     */
    void getAuthorizationEntries(std::list<AuthorizationEntry>* requestedAuthorizationEntries);

    /**
     * @brief Sends a new authorization entry to the lock via BLE
     *
     * @param newAuthorizationEntry Nuki api based datatype to send
     */
    CmdResult addAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry);

    /**
     * @brief Sends an updated authorization entry to the lock via BLE
     *
     * @param updatedAuthorizationEntry Nuki api based datatype to send
     */
    CmdResult updateAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry);

    /**
     * @brief Sends an calibration (mechanical) request to the lock via BLE
     */
    CmdResult requestCalibration();

    /**
     * @brief Sends an reboot request to the lock via BLE
     *
     */
    CmdResult requestReboot();

    /**
     * @brief Sends the time to be set to the lock via BLE
     *
     * @param time Nuki api based datatype to send
     */
    CmdResult updateTime(TimeValue time);

    /**
     * @brief Sends a new time(d) control entry via BLE to the lock.
     * This entry is independant of keypad or authorization entries, it will execute the
     * defined action at the defined time in the newTimeControlEntry
     *
     * @param newTimecontrolEntry Nuki api based datatype to send
     */
    CmdResult addTimeControlEntry(NewTimeControlEntry newTimecontrolEntry);

    /**
     * @brief Sends an updated time(d) control entry via BLE to the lock.
     * (see addTimeControlEntry())
     *
     * @param TimeControlEntry Nuki api based datatype to send.
     * The ID can be retrieved via retrieveTimeControlEntries()
     */
    CmdResult updateTimeControlEntry(TimeControlEntry TimeControlEntry);

    /**
     * @brief Deletes a time(d) control entry via BLE to the lock.
     * (see addTimeControlEntry())
     *
     * @param entryId The ID to be deleted, can be retrieved via retrieveTimeControlEntries()
     */
    CmdResult removeTimeControlEntry(uint8_t entryId);

    /**
     * @brief Request the lock via BLE to send the existing time control entries
     *
     */
    CmdResult retrieveTimeControlEntries();

    /**
     * @brief Get the time control entries stored on the esp (after executing retrieveTimeControlEntries())
     *
     * @param timeControlEntries list to store the returned time control entries
     */
    void getTimeControlEntries(std::list<TimeControlEntry>* timeControlEntries);

    /**
     * @brief Saves the pincode on the esp. This pincode is used for sending/setting config via BLE to the lock
     * by other methods and needs to be the same pincode as stored in the lock
     *
     * @param pinCode
     * @return true if stored successfully
     */
    bool saveSecurityPincode(const uint16_t pinCode);

    /**
     * @brief Send the new pincode command to the lock via BLE
     * (this command uses the earlier by saveSecurityPincode() stored pincode which needs to be the same as
     * the pincode stored in the lock)
     *
     * @param newSecurityPin
     * @return CmdResult
     */
    CmdResult setSecurityPin(const uint16_t newSecurityPin);

    /**
     * @brief Send the verify pincode command via BLE to the lock.
     * This command uses the earlier by saveSecurityPincode() stored pincode
     *
     * @return CmdResult returns success when the pin code is correct (same as stored in the lock)
     */
    CmdResult verifySecurityPin();

    /**
     * @brief Sets the lock ability to pair with other devices (can be used to prevent unauthorized pairing)
     * Gets the current config from the lock, updates the pairing parameter and sends the new config to the lock via BLE
     * (CAUTION: if pairing is set to false and credentials are deleted a factory reset of the lock needs to be performed
     * before pairing is possible again)
     *
     * @param enable true if allowed to pair with other devices
     */
    CmdResult enablePairing(const bool enable);

    /**
     * @brief Gets the current config from the lock, updates the whether or not the flashing
     * LED should be enabled to signal an unlocked door. And sends the new config to the lock via BLE
     *
     * @param enable true if led enabled
     */
    CmdResult enableLedFlash(const bool enable);

    /**
     * @brief Gets the current config from the lock, updates the LED brightness parameter and
     * sends the new config to the lock via BLE
     *
     * @param level The LED brightness level. Possible values are 0 to 5 0 = off, …, 5 = max
     */
    CmdResult setLedBrightness(const uint8_t level);

    /**
     * @brief Gets the current config from the lock, updates the LED brightness parameter
     * and sends the new config to the lock via BLE
     *
     * @param enable true if only a single lock should be performed
     */
    CmdResult enableSingleLock(const bool enable);

    /**
     * @brief Gets the current config from the lock, updates the advertising frequency parameter
     * and sends the new config to the lock via BLE
     *
     * @param mode 0x00 Automatic, 0x01 Normal, 0x02 Slow, 0x03 Slowest (~400ms till ~1s)
     */
    CmdResult setAdvertisingMode(const AdvertisingMode mode);

    /**
     * @brief Gets the current config from the lock, updates the name parameter and sends the
     * new config to the lock via BLE
     *
     * @param name max 32 character name
     */
    CmdResult setName(const std::string& name);

    /**
     * @brief Gets the current config from the lock, updates the dst parameter and sends the new
     * config to the lock via BLE
     *
     * @param enable The desired daylight saving time mode. false disabled, true european
     */
    CmdResult enableDst(const bool enable);

    /**
     * @brief Gets the current config from the lock, updates the timezone offset parameter and
     * sends the new config to the lock via BLE
     *
     * @param minutes The timezone offset (UTC) in minutes
     */
    CmdResult setTimeZoneOffset(const int16_t minutes);

    /**
     * @brief Gets the current config from the lock, updates the timezone id parameter and sends the
     * new config to the lock via BLE
     *
     * @param timeZoneId 	The id of the current timezone or 0xFFFF if timezones are not supported
     */
    CmdResult setTimeZoneId(const TimeZoneId timeZoneId);

    /**
     * @brief Gets the current config from the lock, updates the enable button parameter and sends the
     * new config to the lock via BLE
     *
     * @param enable true if button enabled
     */
    CmdResult enableButton(const bool enable);

    /**
     * @brief Gets the current advanced config from the lock, updates the single button press action
     * parameter and sends the new advanced config to the lock via BLE
     *
     * @param action the deired action for a single button press
     */
    CmdResult setSingleButtonPressAction(const ButtonPressAction action);

    /**
     * @brief Gets the current advanced config from the lock, updates the double button press action
     * parameter and sends the new advanced config to the lock via BLE
     *
     * @param action the deired action for a double button press
     */
    CmdResult setDoubleButtonPressAction(const ButtonPressAction action);

    /**
     * @brief Gets the current advanced config from the lock, updates the battery type parameter and
     * sends the new advanced config to the lock via BLE
     *
     * @param type 	The type of the batteries present in the smart lock.
     */
    CmdResult setBatteryType(const BatteryType type);

    /**
     * @brief Gets the current advanced config from the lock, updates the enable battery type
     * detection parameter and sends the new advanced config to the lock via BLE
     *
     * @param enable true if the automatic detection of the battery type is enabled
     */
    CmdResult enableAutoBatteryTypeDetection(const bool enable);

    /**
     * @brief Gets the current advanced config from the lock, updates the disable autounlock
     * parameter and sends the new advanced config to the lock via BLE
     *
     * @param disable true if auto unlock should be disabled in general.
     */
    CmdResult disableAutoUnlock(const bool disable);

    /**
     * @brief Gets the current advanced config from the lock, updates the enable autolock
     * parameter and sends the new advanced config to the lock via BLE
     *
     * @param enable true if auto lock should be enabled in general.
     */
    CmdResult enableAutoLock(const bool enable);

    /**
     * @brief Gets the current advanced config from the lock, updates the enable immediate
     * autolock parameter and sends the new advanced config to the lock via BLE
     *
     * @param enable true if auto lock should be performed immediately after the door has
     * been closed (requires active door sensor)
     */
    CmdResult enableImmediateAutoLock(const bool enable);

    /**
     * @brief Gets the current advanced config from the lock, updates the enable auto update
     * parameter and sends the new advanced config to the lock via BLE
     * (Updating the firmware requires the Nuki app. CAUTION: updating FW could cause breaking changes)
     *
     * @param enable true if automatic firmware updates should be enabled
     */
    CmdResult enableAutoUpdate(const bool enable);

    /**
     * @brief Initializes stored preferences based on the devicename passed in the constructor,
     * creates the BLE client, sets the BLE callback and checks if the lock is paired
     * (if credentials are stored in preferences)
     */
    void initialize();

    /**
     * @brief Registers the BLE scanner to be used for scanning for advertisements from the lock.
     * BleScanner::Publisher is defined in dependent library https://github.com/I-Connect/BleScanner.git
     *
     * @param bleScanner the publisher of the BLE scanner
     */
    void registerBleScanner(BleScanner::Publisher* bleScanner);

  private:
    FreeRTOS::Semaphore nukiBleSemaphore;
    bool takeNukiBleSemaphore(std::string owner);
    void giveNukiBleSemaphore();

    bool connectBle(const BLEAddress bleAddress);
    void onConnect(BLEClient*) override;
    void onDisconnect(BLEClient*) override;
    void onResult(BLEAdvertisedDevice* advertisedDevice) override;
    bool registerOnGdioChar();
    bool registerOnUsdioChar();

    bool sendPlainMessage(Command commandIdentifier, const unsigned char* payload, const uint8_t payloadLen);
    bool sendEncryptedMessage(Command commandIdentifier, const unsigned char* payload, const uint8_t payloadLen);

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

    BleScanner::Publisher* bleScanner = nullptr;
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