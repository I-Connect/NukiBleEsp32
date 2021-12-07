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

void printBuffer(const byte* buff, const uint8_t size, const boolean asChars, const char* header);

class NukiBle : public BLEClientCallbacks {
  public:
    NukiBle(std::string& bleAddress, uint32_t deviceId, uint8_t* deviceName);
    virtual ~NukiBle();

    QueueHandle_t  nukiBleIsrFlagQueue;

    void onConnect(BLEClient*) override;
    void onDisconnect(BLEClient*) override;

    virtual void initialize();
    bool connect();
    bool isPaired = false;
    void pairedStatus(bool status) {
      isPaired = status;
    }

    bool executeLockAction(lockAction action);
    void sendEncryptedMessage(nukiCommand commandIdentifier, char* payload, uint8_t payloadLen);
    static int encode(unsigned char* input, unsigned char* output, unsigned int len, unsigned char* nonce,  unsigned char* keyS);

  private:
    TaskHandle_t TaskHandleNukiBle;
    BaseType_t xHigherPriorityTaskWoken;
    void startNukiBleXtask();
    void pushNotificationToQueue();

    bool registerOnGdioChar();
    void sendPlainMessage(nukiCommand commandIdentifier, char* payload, uint8_t payloadLen);
    // void sendEncryptedMessage(nukiCommand commandIdentifier, char* payload, uint8_t payloadLen);
    static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
    static void my_gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t* param);
    static void handleErrorCode(uint8_t errorCode);
    static void handleReturnMessage(uint16_t returnCode, char* data, uint8_t dataLen);

    unsigned char secretKeyK[32];

    std::string bleAddress = "";
    uint32_t deviceId;            //The ID of the Nuki App, Nuki Bridge or Nuki Fob to be authorized.
    uint8_t deviceName[32];       //The name to be displayed for this authorization.
    BLEClient* pClient;
    BLERemoteService* pKeyturnerPairingService = nullptr;
    BLERemoteCharacteristic* pGdioCharacteristic = nullptr;

    // void keyGen(uint8_t *key, uint8_t keyLen, uint8_t seedPin);
    void generateNonce(unsigned char* hexArray, uint8_t nrOfBytes);

    // uint8_t localPrivateKey[32];
    // uint8_t localPublicKey[32];
};