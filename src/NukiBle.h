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

#include "NimBLEDevice.h"
#include "NukiConstants.h"
#include "NukiDataTypes.h"
#include "Arduino.h"
#include <Preferences.h>
#include <esp_task_wdt.h>
#include <BleInterfaces.h>
#include "sodium/crypto_secretbox.h"

#define GENERAL_TIMEOUT 3000
#define CMD_TIMEOUT 10000
#define PAIRING_TIMEOUT 30000
#define HEARTBEAT_TIMEOUT 30000

namespace Nuki {
class NukiBle : public BLEClientCallbacks, public BleScanner::Subscriber {
  public:
    NukiBle(const std::string& deviceName,
            const uint32_t deviceId,
            const NimBLEUUID pairingServiceUUID,
            const NimBLEUUID deviceServiceUUID,
            const NimBLEUUID gdioUUID,
            const NimBLEUUID userDataUUID,
            const std::string preferencedId);
    virtual ~NukiBle();

    /**
     * @brief Set the Event Handler object
     *
     * @param handler method to handle the event
     */
    void setEventHandler(Nuki::SmartlockEventHandler* handler);

    /**
     * @brief Checks if credentials are stored in preferences, if not initiate pairing
     *
     * @return
     */
    Nuki::PairingResult pairNuki(AuthorizationIdType idType = AuthorizationIdType::Bridge);

    /**
     * @brief Delete stored credentials
     */
    void unPairNuki();

    /**
     * @brief checks the time past after last connect/communication sent, if the time past > timeout
     * it will disconnect the BLE connection with the lock so that lock will start sending advertisements.
     *
     * This method is optional as the lock will also disconnect automaticlally after ~20 sec.
     * If used this method should be run in loop or a task.
     *
     */
    void updateConnectionState();

    /**
     * @brief Set the BLE Disonnect Timeout, if longer than ~20 sec the lock will disconnect by itself
     * if there is no BLE communication
     *
     * @param timeoutMs
     */
    void setDisonnectTimeout(uint32_t timeoutMs);

    /**
     * @brief Returns pairing state (if credentials are stored or not)
     */
    const bool isPairedWithLock() const;

    /**
     * @brief Returns the log entry count. Only available after executing retreiveLogEntries.
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
    Nuki::CmdResult addKeypadEntry(NewKeypadEntry newKeypadEntry);

    /**
     * @brief Send an updated keypad entry to the lock via BLE
     *
     * @param updatedKeypadEntry Nuki api based datatype to be sent
     * Keypad Codes that start with 12 are not allowed
     * 0 is not allowed
     * Duplicates are not allowed
     */
    Nuki::CmdResult updateKeypadEntry(UpdatedKeypadEntry updatedKeyPadEntry);

    /**
    * @brief Returns the keypad entry count.
    * Only available after executing retreiveKeypadEntries.
    */
    uint16_t getKeypadEntryCount();

    /**
     * @brief Request the lock via BLE to send the existing keypad entries
     *
     * @param offset The start offset to be read.
     * @param count The number of entries to be read, starting at the specified offset.
     */
    Nuki::CmdResult retrieveKeypadEntries(const uint16_t offset, const uint16_t count);

    /**
     * @brief Get the Keypad Entries stored on the esp (after executing retreieveLogKeypadEntries)
     *
     * @param requestedKeyPadEntries list to store the returned Keypad entries
     */
    void getKeypadEntries(std::list<KeypadEntry>* requestedKeyPadEntries);

    /**
    * @brief Delete a Keypad Entry
    *
    * @param id Id to be deleted
    */
    CmdResult deleteKeypadEntry(uint16_t id);

    /**
     * @brief Request the lock via BLE to send the existing authorizationentries
     *
     * @param offset The start offset to be read.
     * @param count The number of entries to be read, starting at the specified offset.
     */
    Nuki::CmdResult retrieveAuthorizationEntries(const uint16_t offset, const uint16_t count);

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
    Nuki::CmdResult addAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry);

    /**
     * @brief Sends an updated authorization entry to the lock via BLE
     *
     * @param updatedAuthorizationEntry Nuki api based datatype to send
     */
    Nuki::CmdResult updateAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry);

    /**
     * @brief Sends an calibration (mechanical) request to the lock via BLE
     */
    Nuki::CmdResult requestCalibration();

    /**
     * @brief Sends an reboot request to the lock via BLE
     *
     */
    Nuki::CmdResult requestReboot();

    /**
     * @brief Sends the time to be set to the lock via BLE
     *
     * @param time Nuki api based datatype to send
     */
    Nuki::CmdResult updateTime(TimeValue time);

    /**
     * @brief Saves the pincode on the esp. This pincode is used for sending/setting config via BLE to the lock
     * by other methods and needs to be the same pincode as stored in the lock
     *
     * @param pinCode
     * @return true if stored successfully
     */
    bool saveSecurityPincode(const uint16_t pinCode);

    /**
     * @brief Gets the pincode stored on the esp. This pincode is used for sending/setting config via BLE to the lock
     * by other methods and needs to be the same pincode as stored in the lock
     *
     * @return pincode
     */
    uint16_t getSecurityPincode();

    /**
     * @brief Send the new pincode command to the lock via BLE
     * (this command uses the earlier by saveSecurityPincode() stored pincode which needs to be the same as
     * the pincode stored in the lock)
     *
     * @param newSecurityPin
     * @return Nuki::CmdResult
     */
    Nuki::CmdResult setSecurityPin(const uint16_t newSecurityPin);

    /**
     * @brief Send the verify pincode command via BLE to the lock.
     * This command uses the earlier by saveSecurityPincode() stored pincode
     *
     * @return Nuki::CmdResult returns success when the pin code is correct (same as stored in the lock)
     */
    Nuki::CmdResult verifySecurityPin();

    /**
     * @brief Gets the ble mac address of the paired lock stored on the esp.
     *
     * @return 18 byte Char array with mac address
     */
    void getMacAddress(char* macAddress);

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

    /**
    * @brief Returns the RSSI of the last received ble beacon broadcast
    *
     * @return RSSI value
    */
    int getRssi();

    /**
    * @brief Returns the timestamp in milliseconds when the last ble beacon has been received from the device
    *
    * @return Timestamp in milliseconds
    */
    int getLastReceivedBeaconTs();

    /**
    * @brief Returns the timestamp (millis) of the last received BLE beacon from the lock.
    *
    * @return Last heartbeat value
    */
    uint32_t getLastHeartbeat();

  protected:
    bool connectBle(const BLEAddress bleAddress);
    void extendDisonnectTimeout();

    template <typename TDeviceAction>
    Nuki::CmdResult executeAction(const TDeviceAction action);

    template <typename TDeviceAction>
    Nuki::CmdResult cmdStateMachine(const TDeviceAction action);

    template <typename TDeviceAction>
    Nuki::CmdResult cmdChallStateMachine(const TDeviceAction action, const bool sendPinCode = false);

    template <typename TDeviceAction>
    Nuki::CmdResult cmdChallAccStateMachine(const TDeviceAction action);

  protected:
    virtual void handleReturnMessage(Command returnCode, unsigned char* data, uint16_t dataLen);
    virtual void logErrorCode(uint8_t errorCode) = 0;
    uint8_t errorCode;
    uint8_t lockBusyRetryAttempt = 0;
    Command lastMsgCodeReceived = Command::Empty;

  private:
    SemaphoreHandle_t nukiBleSemaphore = xSemaphoreCreateMutex();
    bool takeNukiBleSemaphore(std::string taker);
    std::string owner = "free";
    void giveNukiBleSemaphore();

    bool connecting = false;
    uint32_t lastStartTimeout = 0;
    uint16_t timeoutDuration = 1000;
    void onConnect(BLEClient*) override;
    void onDisconnect(BLEClient*) override;
    void onResult(BLEAdvertisedDevice* advertisedDevice) override;
    bool registerOnGdioChar();
    bool registerOnUsdioChar();

    bool sendPlainMessage(Command commandIdentifier, const unsigned char* payload, const uint8_t payloadLen);
    bool sendEncryptedMessage(Command commandIdentifier, const unsigned char* payload, const uint8_t payloadLen);

    void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
    void saveCredentials();
    bool retrieveCredentials();
    void deleteCredentials();
    Nuki::PairingState pairStateMachine(const Nuki::PairingState nukiPairingState);
    Nuki::PairingState nukiPairingResultState = Nuki::PairingState::InitPairing;

    unsigned char authenticator[32];
    Preferences preferences;

    BLEAddress bleAddress = BLEAddress("");
    bool pairingServiceAvailable = false;
    std::string deviceName;       //The name to be displayed for this authorization and used for storing preferences
    uint32_t deviceId;            //The ID of the Nuki App, Nuki Bridge or Nuki Fob to be authorized.
    BLEClient* pClient = nullptr;

//Keyturner Pairing Service
    const NimBLEUUID pairingServiceUUID;
//Keyturner Service
    const NimBLEUUID deviceServiceUUID;
//Keyturner pairing Data Input Output characteristic
    const NimBLEUUID gdioUUID;
//User-Specific Data Input Output characteristic
    const NimBLEUUID userDataUUID;

    const std::string preferencesId;

    BLERemoteService* pKeyturnerPairingService = nullptr;
    BLERemoteCharacteristic* pGdioCharacteristic = nullptr;
    BLERemoteService* pKeyturnerDataService = nullptr;
    BLERemoteCharacteristic* pUsdioCharacteristic = nullptr;

    Nuki::CommandState nukiCommandState = Nuki::CommandState::Idle;

    uint32_t timeNow = 0;
    uint32_t lastHeartbeat = 0;

    BleScanner::Publisher* bleScanner = nullptr;
    bool isPaired = false;

    Nuki::SmartlockEventHandler* eventHandler;

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

    uint16_t nrOfKeypadCodes = 0;
    uint8_t nrOfReceivedKeypadCodes = 0;
    bool keypadCodeCountReceived = false;
    uint16_t logEntryCount = 0;
    bool loggingEnabled = false;
    int rssi = 0;
    unsigned long lastReceivedBeaconTs = 0;
    std::list<KeypadEntry> listOfKeyPadEntries;
    std::list<AuthorizationEntry> listOfAuthorizationEntries;
    AuthorizationIdType authorizationIdType = AuthorizationIdType::Bridge;

};

} // namespace Nuki

#include "NukiBle.hpp"
