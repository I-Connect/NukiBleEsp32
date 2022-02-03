#pragma once
/*
 * nukiBle.h
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "BLEDevice.h"
#include "nukiConstants.h"
#include "Arduino.h"
#include <Preferences.h>

void printBuffer(const byte* buff, const uint8_t size, const boolean asChars, const char* header);
bool checkCharArrayEmpty(unsigned char* array, uint16_t len);

class NukiBle : public BLEClientCallbacks {
  public:
    NukiBle(std::string& bleAddress, uint32_t deviceId, uint8_t* deviceName);
    virtual ~NukiBle();

    QueueHandle_t  nukiBleIsrFlagQueue;

    void onConnect(BLEClient*) override;
    void onDisconnect(BLEClient*) override;

    virtual void initialize();
    void runStateMachine();

    bool executeLockAction(lockAction action);
    void sendEncryptedMessage(nukiCommand commandIdentifier, char* payload, uint8_t payloadLen);
    static int encode(unsigned char* input, unsigned char* output, unsigned int len, unsigned char* nonce,  unsigned char* keyS);

  private:
    TaskHandle_t TaskHandleNukiBle;
    BaseType_t xHigherPriorityTaskWoken;
    void startNukiBleXtask();

    bool registerOnGdioChar();
    bool registerOnUsdioChar();
    void sendPlainMessage(nukiCommand commandIdentifier, char* payload, uint8_t payloadLen);
    // void sendEncryptedMessage(nukiCommand commandIdentifier, char* payload, uint8_t payloadLen);
    static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
    static void my_gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t* param);
    static void logErrorCode(uint8_t errorCode);
    static void handleReturnMessage(uint16_t returnCode, char* data, uint8_t dataLen);
    void saveCredentials();
    void deleteCredentials();
    uint8_t connectStateMachine();

    unsigned char sharedKeyS[32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char secretKeyK[32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //TODO generate public and private keys?
    unsigned char myPrivateKey[32] = {0x8C, 0xAA, 0x54, 0x67, 0x23, 0x07, 0xBF, 0xFD, 0xF5, 0xEA, 0x18, 0x3F, 0xC6, 0x07, 0x15, 0x8D, 0x20, 0x11, 0xD0, 0x08, 0xEC, 0xA6, 0xA1, 0x08, 0x86, 0x14, 0xFF, 0x08, 0x53, 0xA5, 0xAA, 0x07};
    unsigned char myPublicKey[32] = {0xF8, 0x81, 0x27, 0xCC, 0xF4, 0x80, 0x23, 0xB5, 0xCB, 0xE9, 0x10, 0x1D, 0x24, 0xBA, 0xA8, 0xA3, 0x68, 0xDA, 0x94, 0xE8, 0xC2, 0xE3, 0xCD, 0xE2, 0xDE, 0xD2, 0x9C, 0xE9, 0x6A, 0xB5, 0x0C, 0x15};
    unsigned char authenticator[32];
    Preferences preferences;

    std::string bleAddress = "";
    uint32_t deviceId;            //The ID of the Nuki App, Nuki Bridge or Nuki Fob to be authorized.
    uint8_t deviceName[32];       //The name to be displayed for this authorization.
    BLEClient* pClient;
    BLERemoteService* pKeyturnerPairingService = nullptr;
    BLERemoteCharacteristic* pGdioCharacteristic = nullptr;

    BLERemoteService* pKeyturnerDataService = nullptr;
    BLERemoteCharacteristic* pUsdioCharacteristic = nullptr;

    // void keyGen(uint8_t *key, uint8_t keyLen, uint8_t seedPin);
    void generateNonce(unsigned char* hexArray, uint8_t nrOfBytes);

    enum class NukiState {
      startUp             = 0,
      startPairing        = 1,
      pairing             = 2,
      paired              = 3,
      connected           = 4
    };
    NukiState nukiState = NukiState::startUp;

    enum class NukiConnectState {
      initConnect       = 0,
      reqRemPubKey      = 1,
      recRemPubKey      = 2,
      sendPubKey        = 3,
      genKeyPair        = 4,
      calculateAuth     = 5,
      sendAuth          = 6,
      sendAuthData      = 7,
      sendAuthIdConf    = 8,
      recStatus         = 9
    };
    NukiConnectState nukiConnectState = NukiConnectState::initConnect;

    uint32_t timeNow = 0;
    uint32_t timeWait10Sec = 10000;

};