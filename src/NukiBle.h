#pragma once
/*
 * nukiBle.h
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "NimBLEDevice.h"
#include "NukiConstants.h"
#include "Arduino.h"
#include <Preferences.h>
#include <esp_task_wdt.h>

#define GENERAL_TIMEOUT 10000
#define CMD_TIMEOUT 3000
#define PAIRING_TIMEOUT 30000

void printBuffer(const byte* buff, const uint8_t size, const boolean asChars, const char* header);
bool checkCharArrayEmpty(unsigned char* array, uint16_t len);

class NukiSmartlockEventHandler {
  public:
    virtual ~NukiSmartlockEventHandler() {};
    virtual void handleEvent() = 0;
};

class NukiBle : public BLEClientCallbacks, BLEAdvertisedDeviceCallbacks {
  public:
    NukiBle(const std::string& deviceName, const uint32_t deviceId);
    virtual ~NukiBle();

    void setEventHandler(NukiSmartlockEventHandler* handler);

    bool pairNuki();
    void unPairNuki();

    void updateKeyTurnerState();
    void lockAction(LockAction lockAction, uint32_t nukiAppId, uint8_t flags = 0, unsigned char* nameSuffix = nullptr);
    void requestConfig(bool advanced);
    void requestBatteryReport();
    void requestOpeningsClosingsSummary();
    void requestAuthorizationEntryCount();

    void requestKeyPadCodes(uint16_t offset, uint16_t nrToBeRead);
    void addKeypadEntry(NewKeypadEntry newKeypadEntry);

    virtual void initialize();
    void runStateMachine();

  private:
    TaskHandle_t TaskHandleNukiBle;
    BaseType_t xHigherPriorityTaskWoken;
    QueueHandle_t  nukiBleRequestQueue;
    void startNukiBleXtask();

    bool connectBle(BLEAddress bleAddress);
    void onConnect(BLEClient*) override;
    void onDisconnect(BLEClient*) override;
    void onResult(BLEAdvertisedDevice* advertisedDevice) override;
    bool bleConnected = false;
    bool registerOnGdioChar();
    bool registerOnUsdioChar();

    void sendPlainMessage(NukiCommand commandIdentifier, unsigned char* payload, uint8_t payloadLen);
    void sendEncryptedMessage(NukiCommand commandIdentifier, unsigned char* payload, uint8_t payloadLen);
    static int encode(unsigned char* output, unsigned char* input, unsigned long long len, unsigned char* nonce,  unsigned char* keyS);
    static int decode(unsigned char* output, unsigned char* input,  unsigned long long len, unsigned char* nonce, unsigned char* keyS);

    static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
    static void logErrorCode(uint8_t errorCode);
    static void handleReturnMessage(NukiCommand returnCode, unsigned char* data, uint16_t dataLen);
    int scanForPairingNuki();
    void saveCredentials();
    bool retreiveCredentials();
    void deleteCredentials();
    uint8_t pairStateMachine();
    static bool crcValid(uint8_t* pData, uint16_t length);

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

    // void keyGen(uint8_t *key, uint8_t keyLen, uint8_t seedPin);
    void generateNonce(unsigned char* hexArray, uint8_t nrOfBytes);

    enum class NukiConnectionState {
      checkPaired         = 1,
      paired              = 2
    };
    NukiConnectionState nukiConnectionState = NukiConnectionState::checkPaired;

    enum class NukiPairingState {
      initPairing       = 0,
      reqRemPubKey      = 1,
      recRemPubKey      = 2,
      sendPubKey        = 3,
      genKeyPair        = 4,
      calculateAuth     = 5,
      sendAuth          = 6,
      sendAuthData      = 7,
      sendAuthIdConf    = 8,
      recStatus         = 9,
      success           = 10
    };

    enum class NukiCommandState {
      idle                  = 0,
      cmdReceived           = 1,
      challengeSent         = 2,
      challengeRespReceived = 3,
      cmdSent               = 4,
      cmdAccepted           = 5,
      timeOut               = 6
    };

    enum class NukiCommandType {
      command                       = 0,
      commandWithChallenge          = 1,
      commandWithChallengeAndAccept = 2,
      commandWithChallengeAndPin    = 3
    };

    struct NukiAction {
      NukiCommandType cmdType;
      NukiCommand command;
      unsigned char payload[100] {0};
      uint8_t payloadLen = 0;
    };

    void addActionToQueue(NukiAction action);
    bool cmdStateMachine(NukiAction action);
    bool cmdChallStateMachine(NukiAction action, bool sendPinCode = false);
    bool cmdChallAccStateMachine(NukiAction action);

    NukiPairingState nukiPairingState = NukiPairingState::initPairing;
    NukiCommandState nukiCommandState = NukiCommandState::idle;

    uint32_t timeNow = 0;

    NukiSmartlockEventHandler* eventHandler;
};