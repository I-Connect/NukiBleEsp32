/*
 * NukiBle.cpp
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "NukiBle.h"
#include "Crc16.h"
#include "string.h"
#include "sodium/crypto_scalarmult.h"
#include "sodium/crypto_core_hsalsa20.h"
#include "sodium/crypto_auth_hmacsha256.h"
#include "sodium/crypto_secretbox_xsalsa20poly1305.h"


unsigned char remotePublicKey[32] = {0x00, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char challengeNonceK[32] = {0x00, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char authorizationId[4] = {0x00, 0x00, 0x0, 0x00};
unsigned char lockId[16];

void printBuffer(const byte* buff, const uint8_t size, const boolean asChars, const char* header) {
  #ifdef DEBUG_NUKI
  delay(100); //delay otherwise first part of print will not be shown
  char tmp[16];

  if (strlen(header) > 0) {
    Serial.print(header);
    Serial.print(": ");
  }
  for (int i = 0; i < size; i++) {
    if (asChars) {
      Serial.print((char)buff[i]);
    } else {
      sprintf(tmp, "%02x", buff[i]);
      Serial.print(tmp);
      Serial.print(" ");
    }
  }
  Serial.println();
  #endif
}

//task to retrieve messages from BLE when a notification occurrs
void nukiBleTask(void* pvParameters) {
  #ifdef DEBUG_NUKI
  log_d("TASK: Nuki BLE task started");
  #endif
  NukiBle* nukiBleObj = reinterpret_cast<NukiBle*>(pvParameters);
  uint8_t notification;

  while (1) {
    xQueueReceive(nukiBleObj->nukiBleIsrFlagQueue, &notification, portMAX_DELAY );

  }
}


NukiBle::NukiBle(std::string& bleAddress, uint32_t deviceId, uint8_t* aDeviceName): bleAddress(bleAddress), deviceId(deviceId) {
  memcpy(deviceName, aDeviceName, sizeof(aDeviceName));
}

NukiBle::~NukiBle() {}

void NukiBle::initialize() {
  #ifdef DEBUG_NUKI
  log_d("Initializing Nuki");
  BLEDevice::setCustomGattcHandler(my_gattc_event_handler);
  #endif
  BLEDevice::init("ESP32_test");

  // keyGen(localPrivate_key, 32, 34);
  // keyGen(localPublic_key, 32, 34);
}

void NukiBle::startNukiBleXtask() {
  nukiBleIsrFlagQueue = xQueueCreate(10, sizeof(uint8_t));
  TaskHandleNukiBle = NULL;
  xTaskCreatePinnedToCore(&nukiBleTask, "nukiBleTask", 4096, this, 1, &TaskHandleNukiBle, 1);
}

bool NukiBle::connect() {
  #ifdef DEBUG_NUKI
  log_d("Connecting with: %s ", bleAddress.c_str());
  #endif

  pClient  = BLEDevice::createClient();
  pClient->setClientCallbacks(this);

  if (!pClient->connect(bleAddress)) {
    log_w("BLE Connect failed");
    return false;
  }
  if (!registerOnGdioChar()) {
    log_w("BLE register on pairing Service/Char failed");
    return false;
  }

  //Request remote public key (Sent message should be 0100030027A7)
  #ifdef DEBUG_NUKI
  log_d("##################### REQUEST REMOTE PUBLIC KEY #########################");
  #endif
  uint16_t payload = (uint16_t)nukiCommand::publicKey;
  sendPlainMessage(nukiCommand::requestData, (char*)&payload, sizeof(payload));

  //TODO generate public and private keys
  static unsigned char myPrivateKey[32] = {0x8C, 0xAA, 0x54, 0x67, 0x23, 0x07, 0xBF, 0xFD, 0xF5, 0xEA, 0x18, 0x3F, 0xC6, 0x07, 0x15, 0x8D, 0x20, 0x11, 0xD0, 0x08, 0xEC, 0xA6, 0xA1, 0x08, 0x86, 0x14, 0xFF, 0x08, 0x53, 0xA5, 0xAA, 0x07};
  static unsigned char myPublicKey[32] = {0xF8, 0x81, 0x27, 0xCC, 0xF4, 0x80, 0x23, 0xB5, 0xCB, 0xE9, 0x10, 0x1D, 0x24, 0xBA, 0xA8, 0xA3, 0x68, 0xDA, 0x94, 0xE8, 0xC2, 0xE3, 0xCD, 0xE2, 0xDE, 0xD2, 0x9C, 0xE9, 0x6A, 0xB5, 0x0C, 0x15};

  //simulated test keys
  // static unsigned char remotePublicKeySim[32] = {0x39, 0xE7, 0x5F, 0xB9, 0x4B, 0xBE, 0xC0, 0x31, 0x7F, 0x8C, 0xFB, 0x88, 0x09, 0xEB, 0x50, 0x97, 0x51, 0x45, 0x29, 0x83, 0x35, 0xC0, 0xE1, 0x26, 0xCD, 0x48, 0xCB, 0x64, 0xB5, 0x7C, 0x8C, 0x26};
  // static unsigned char challengeNonceKSim[32] = {0xe8, 0x8d, 0xf6, 0x06, 0x0d, 0x05, 0x72, 0x71, 0xec, 0x70, 0x27, 0x90, 0xc7, 0x77, 0x0c, 0xf5, 0xa1, 0xa4, 0x56, 0x68, 0xe1, 0xc2, 0x6d, 0xb3, 0xfd, 0x2e, 0x50, 0xbd, 0x2c, 0x25, 0x6b, 0xfb};
  #ifdef DEBUG_NUKI
  log_d("##################### SEND CLIENT PUBLIC KEY #########################");
  #endif
  sendPlainMessage(nukiCommand::publicKey, (char*)&myPublicKey, sizeof(myPublicKey));

  #ifdef DEBUG_NUKI
  log_d("##################### CALCULATE DH SHARED KEY s #########################");
  #endif
  unsigned char sharedKeyS[32];
  crypto_scalarmult_curve25519(sharedKeyS, myPrivateKey, remotePublicKey);
  printBuffer(sharedKeyS, sizeof(sharedKeyS), false, "Shared key s");

  #ifdef DEBUG_NUKI
  log_d("##################### DERIVE LONG TERM SHARED SECRET KEY k #########################");
  #endif

  unsigned char _0[16];
  memset(_0, 0, 16);
  unsigned char sigma[] = "expand 32-byte k";
  crypto_core_hsalsa20(secretKeyK, _0, sharedKeyS, sigma);
  printBuffer(secretKeyK, sizeof(secretKeyK), false, "Secret key k");

  #ifdef DEBUG_NUKI
  log_d("##################### CALCULATE/VERIFY AUTHENTICATOR #########################");
  #endif
  //concatenate local public key, remote public key and receive challenge data
  unsigned char authenticator[32];
  unsigned char hmacPayload[96];
  memcpy(&hmacPayload[0], myPublicKey, sizeof(myPublicKey));
  memcpy(&hmacPayload[32], remotePublicKey, sizeof(remotePublicKey));
  memcpy(&hmacPayload[64], challengeNonceK, sizeof(challengeNonceK));
  printBuffer((byte*)hmacPayload, sizeof(hmacPayload), false, "Concatenated data r");
  crypto_auth_hmacsha256(authenticator, hmacPayload, sizeof(hmacPayload), secretKeyK);
  printBuffer(authenticator, sizeof(authenticator), false, "HMAC 256 result");

  #ifdef DEBUG_NUKI
  log_d("##################### SEND AUTHENTICATOR #########################");
  #endif
  sendPlainMessage(nukiCommand::authorizationAuthenticator, (char*)&authenticator, sizeof(authenticator));

  #ifdef DEBUG_NUKI
  log_d("##################### SEND AUTHORIZATION DATA #########################");
  #endif
  unsigned char authorizationData[101] = {};
  unsigned char authorizationDataIdType[1] = {0x01}; //0 = App, 1 = Bridge, 2 = Fob, 3 = Keypad
  unsigned char authorizationDataId[4] = {};
  unsigned char authorizationDataName[32] = {};
  unsigned char authorizationDataNonce[32] = {};
  authorizationDataId[0] = (deviceId >> (8 * 0)) & 0xff;
  authorizationDataId[1] = (deviceId >> (8 * 1)) & 0xff;
  authorizationDataId[2] = (deviceId >> (8 * 2)) & 0xff;
  authorizationDataId[3] = (deviceId >> (8 * 3)) & 0xff;
  memcpy(authorizationDataName, deviceName, sizeof(deviceName));
  generateNonce(authorizationDataNonce, sizeof(authorizationDataNonce));

  //calculate authenticator of message to send
  memcpy(&authorizationData[0], authorizationDataIdType, sizeof(authorizationDataIdType));
  memcpy(&authorizationData[1], authorizationDataId, sizeof(authorizationDataId));
  memcpy(&authorizationData[5], authorizationDataName, sizeof(authorizationDataName));
  memcpy(&authorizationData[37], authorizationDataNonce, sizeof(authorizationDataNonce));
  memcpy(&authorizationData[69], challengeNonceK, sizeof(challengeNonceK));
  crypto_auth_hmacsha256(authenticator, authorizationData, sizeof(authorizationData), secretKeyK);

  //compose and send message
  unsigned char authorizationDataMessage[101];
  memcpy(&authorizationDataMessage[0], authenticator, sizeof(authenticator));
  memcpy(&authorizationDataMessage[32], authorizationDataIdType, sizeof(authorizationDataIdType));
  memcpy(&authorizationDataMessage[33], authorizationDataId, sizeof(authorizationDataId));
  memcpy(&authorizationDataMessage[37], authorizationDataName, sizeof(authorizationDataName));
  memcpy(&authorizationDataMessage[69], authorizationDataNonce, sizeof(authorizationDataNonce));
  sendPlainMessage(nukiCommand::authorizationData, (char*)&authorizationDataMessage, sizeof(authorizationDataMessage));

  #ifdef DEBUG_NUKI
  log_d("##################### SEND AUTHORIZATION ID confirmation #########################");
  #endif
  unsigned char confirmationData[36] = {};

  //calculate authenticator of message to send
  memcpy(&confirmationData[0], authorizationId, sizeof(authorizationId));
  memcpy(&confirmationData[4], challengeNonceK, sizeof(challengeNonceK));
  crypto_auth_hmacsha256(authenticator, confirmationData, sizeof(confirmationData), secretKeyK);

  //compose and send message
  unsigned char confirmationDataMessage[36];
  memcpy(&confirmationDataMessage[0], authenticator, sizeof(authenticator));
  memcpy(&confirmationDataMessage[32], authorizationId, sizeof(authorizationId));
  // memcpy(&confirmationDataMessage[36], challengeNonceK, sizeof(challengeNonceK));
  sendPlainMessage(nukiCommand::authorizationIdConfirmation, (char*)&confirmationDataMessage, sizeof(confirmationDataMessage));
  #ifdef DEBUG_NUKI
  log_d("####################### CONNECT DONE ###############################################");
  #endif

  return true;
}

void NukiBle::sendEncryptedMessage(nukiCommand commandIdentifier, char* payload, uint8_t payloadLen) {
  /*
  #     ADDITIONAL DATA (not encr)      #                    PLAIN DATA (encr)                             #
  #  nonce  # auth identifier # msg len # authorization identifier # command identifier # payload #  crc   #
  # 24 byte #    4 byte       # 2 byte  #      4 byte              #       2 byte       #  n byte # 2 byte #
  */


  //compose plain data
  unsigned char plainData[6 + payloadLen] = {};
  unsigned char plainDataWithCrc[8 + payloadLen] = {};

  Crc16 crcObj;
  uint16_t dataCrc;

  memcpy(&plainData[0], &authorizationId, sizeof(authorizationId));
  memcpy(&plainData[4], &commandIdentifier, sizeof(commandIdentifier));
  memcpy(&plainData[6], payload, payloadLen);

  //get crc over plain data
  crcObj.clearCrc();
  // CCITT-False:	width=16 poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1
  dataCrc = crcObj.fastCrc((uint8_t*)plainData, 0, sizeof(plainData), false, false, 0x1021, 0xffff, 0x0000, 0x8000, 0xffff);
  memcpy(&plainDataWithCrc[0], &plainData, sizeof(plainData));
  memcpy(&plainDataWithCrc[sizeof(plainData)], &dataCrc, sizeof(dataCrc));
  #ifdef DEBUG_NUKI
  printBuffer((byte*)plainDataWithCrc, sizeof(plainDataWithCrc), false, "Plain data with CRC: ");
  #endif

  //compose additional data
  unsigned char additionalData[30] = {};
  unsigned char nonce[24] = {};
  uint8_t msgLen = 0;
  generateNonce(nonce, sizeof(nonce));
  msgLen = sizeof(plainDataWithCrc);
  log_d("msgLen: %d", msgLen);

  memcpy(&additionalData[0], nonce, sizeof(nonce));
  memcpy(&additionalData[24], authorizationId, sizeof(authorizationId));
  memcpy(&additionalData[28], &msgLen, sizeof(msgLen));
  #ifdef DEBUG_NUKI
  printBuffer((byte*)additionalData, 30, false, "Additional data: ");
  #endif

  //Encrypt plain data
  unsigned char plainDataEncr[26] = {};
  encode(plainDataWithCrc, plainDataEncr, sizeof(plainDataWithCrc), nonce, secretKeyK);

  #ifdef DEBUG_NUKI
  printBuffer((byte*)plainDataEncr, sizeof(plainDataEncr), false, "Plain data encrypted: ");
  #endif

  //compose complete message
  unsigned char dataToSend[sizeof(additionalData) + sizeof(plainDataEncr)] = {};
  memcpy(&dataToSend[0], additionalData, sizeof(additionalData));
  memcpy(&dataToSend[30], plainDataEncr, sizeof(plainDataEncr));

  #ifdef DEBUG_NUKI
  printBuffer((byte*)dataToSend, sizeof(dataToSend), false, "Sending encrypted message");
  #endif

  pGdioCharacteristic->writeValue((uint8_t*)dataToSend, sizeof(dataToSend), true);
  delay(1000); //wait for response via BLE char
}

int NukiBle::encode(unsigned char* input, unsigned char* output, unsigned int len, unsigned char* nonce, unsigned char* keyS) {
  //https://cpp.hotexamples.com/examples/-/-/crypto_box_curve25519xsalsa20poly1305_afternm/cpp-crypto_box_curve25519xsalsa20poly1305_afternm-function-examples.html
  if ((len + crypto_secretbox_xsalsa20poly1305_ZEROBYTES - crypto_secretbox_xsalsa20poly1305_BOXZEROBYTES) > 200)
  {
    log_d("Encryption failed (packet length %i is above MAX_BUFFER_SIZE %i)\n", (len + crypto_secretbox_xsalsa20poly1305_ZEROBYTES - crypto_secretbox_xsalsa20poly1305_BOXZEROBYTES), 200);
    return -1;
  }

  unsigned char tempbuffer[len + crypto_secretbox_xsalsa20poly1305_ZEROBYTES], tempbufferinput[len + crypto_secretbox_xsalsa20poly1305_ZEROBYTES];

  memset(tempbufferinput, 0, crypto_secretbox_xsalsa20poly1305_ZEROBYTES);
  memcpy(tempbufferinput + crypto_secretbox_xsalsa20poly1305_ZEROBYTES, input, len);

  int result = crypto_secretbox_xsalsa20poly1305(
                 tempbuffer,
                 tempbufferinput,
                 len + crypto_secretbox_xsalsa20poly1305_ZEROBYTES,
                 nonce,
                 keyS
               );

  if (result)
  {
    log_d("Encryption failed (length %i, given result %i)\n", len, result);
    return -1;
  }

  memcpy(output, tempbuffer + crypto_secretbox_xsalsa20poly1305_BOXZEROBYTES, len + crypto_secretbox_xsalsa20poly1305_ZEROBYTES - crypto_secretbox_xsalsa20poly1305_BOXZEROBYTES);

  return len + crypto_secretbox_xsalsa20poly1305_ZEROBYTES - crypto_secretbox_xsalsa20poly1305_BOXZEROBYTES;
}

void NukiBle::sendPlainMessage(nukiCommand commandIdentifier, char* payload, uint8_t payloadLen) {
  /*
  #                PLAIN DATA                   #
  #command identifier  #   payload   #   crc    #
  #      2 byte        #   n byte    #  2 byte  #
  */

  Crc16 crcObj;
  uint16_t dataCrc;

  //compose data
  char dataToSend[200];
  memcpy(&dataToSend, &commandIdentifier, sizeof(commandIdentifier));
  memcpy(&dataToSend[2], payload, payloadLen);

  //get crc over data (data is both command identifier and payload)
  crcObj.clearCrc();
  // CCITT-False:	width=16 poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1
  dataCrc = crcObj.fastCrc((uint8_t*)dataToSend, 0, payloadLen + 2, false, false, 0x1021, 0xffff, 0x0000, 0x8000, 0xffff);

  memcpy(&dataToSend[2 + payloadLen], &dataCrc, sizeof(dataCrc));
  printBuffer((byte*)dataToSend, payloadLen + 4, false, "Sending plain message");
  #ifdef DEBUG_NUKI
  log_d("Command identifier: %02x, CRC: %04x", (uint32_t)commandIdentifier, dataCrc);
  #endif
  pGdioCharacteristic->writeValue((uint8_t*)dataToSend, payloadLen + 4, true);
  delay(1000); //wait for response via BLE char
}

bool NukiBle::registerOnGdioChar() {
  // Obtain a reference to the KeyTurner Pairing service
  pKeyturnerPairingService = pClient->getService(STRING(keyturnerPairingServiceUUID));
  //Obtain reference to GDIO char
  pGdioCharacteristic = pKeyturnerPairingService->getCharacteristic(STRING(keyturnerGdioUUID));
  if (pGdioCharacteristic->canIndicate()) {
    pGdioCharacteristic->registerForNotify(notifyCallback, false); //false = indication, true = notification
    delay(100);
    return true;
  }
  else {
    #ifdef DEBUG_NUKI
    log_d("GDIO characteristic canIndicate false, stop connecting");
    #endif
    return false;
  }
  return false;
}



bool NukiBle::executeLockAction(lockAction aLockAction) {
  #ifdef DEBUG_NUKI
  log_d("Executing lock action: %d", aLockAction);
  #endif
  return true;
}

void NukiBle::notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  #ifdef DEBUG_NUKI
  log_d(" Notify callback for characteristic: %s of length: %d", pBLERemoteCharacteristic->getUUID().toString().c_str(), length);
  printBuffer((byte*)pData, length, false, "Received data");
  #endif

  //check CRC
  uint16_t receivedCrc = ((uint16_t)pData[length - 1] << 8) | pData[length - 2];
  Crc16 crcObj;
  uint16_t dataCrc;
  crcObj.clearCrc();
  dataCrc = crcObj.fastCrc(pData, 0, length - 2, false, false, 0x1021, 0xffff, 0x0000, 0x8000, 0xffff);
  // log_d("Received CRC: %d, calculated CRC: %d", receivedCrc, dataCrc);
  uint16_t returnCode = ((uint16_t)pData[1] << 8) | pData[0];
  // log_d("Return code: %d", returnCode);
  if (!(receivedCrc == dataCrc)) {
    log_e("CRC CHECK FAILED!");
    //TODO retry last communications
  }
  else {
    #ifdef DEBUG_NUKI
    log_d("CRC CHECK OKE");
    #endif
    if (returnCode == (uint16_t)nukiCommand::errorReport) {
      handleErrorCode(pData[2]);
    }
    else {
      char data[200];
      memcpy(data, &pData[2], length - 4);
      handleReturnMessage(returnCode, data, length - 4);
    }
  }
}

void NukiBle::handleErrorCode(uint8_t errorCode) {

  switch (errorCode) {
    case (uint8_t)nukiErrorCode::ERROR_BAD_CRC :
      log_e("ERROR_BAD_CRC");
      break;
    case (uint8_t)nukiErrorCode::ERROR_BAD_LENGTH :
      log_e("ERROR_BAD_LENGTH");
      break;
    case (uint8_t)nukiErrorCode::ERROR_UNKNOWN :
      log_e("ERROR_UNKNOWN");
      break;
    case (uint8_t)nukiErrorCode::P_ERROR_NOT_PAIRING :
      log_e("P_ERROR_NOT_PAIRING");
      break;
    case (uint8_t)nukiErrorCode::P_ERROR_BAD_AUTHENTICATOR :
      log_e("P_ERROR_BAD_AUTHENTICATOR");
      break;
    case (uint8_t)nukiErrorCode::P_ERROR_BAD_PARAMETER :
      log_e("P_ERROR_BAD_PARAMETER");
      break;
    case (uint8_t)nukiErrorCode::P_ERROR_MAX_USER :
      log_e("P_ERROR_MAX_USER");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_AUTO_UNLOCK_TOO_RECENT :
      log_e("K_ERROR_AUTO_UNLOCK_TOO_RECENT");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_BAD_NONCE :
      log_e("K_ERROR_BAD_NONCE");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_BAD_PARAMETER :
      log_e("K_ERROR_BAD_PARAMETER");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_BAD_PIN :
      log_e("K_ERROR_BAD_PIN");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_BUSY :
      log_e("K_ERROR_BUSY");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CANCELED :
      log_e("K_ERROR_CANCELED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CLUTCH_FAILURE :
      log_e("K_ERROR_CLUTCH_FAILURE");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CLUTCH_POWER_FAILURE :
      log_e("K_ERROR_CLUTCH_POWER_FAILURE");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CODE_ALREADY_EXISTS :
      log_e("K_ERROR_CODE_ALREADY_EXISTS");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CODE_INVALID :
      log_e("K_ERROR_CODE_INVALID");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_1 :
      log_e("K_ERROR_CODE_INVALID_TIMEOUT_1");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_2 :
      log_e("K_ERROR_CODE_INVALID_TIMEOUT_2");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_3 :
      log_e("K_ERROR_CODE_INVALID_TIMEOUT_3");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_DISABLED :
      log_e("K_ERROR_DISABLED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_FIRMWARE_UPDATE_NEEDED :
      log_e("K_ERROR_FIRMWARE_UPDATE_NEEDED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_INVALID_AUTH_ID :
      log_e("K_ERROR_INVALID_AUTH_ID");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_MOTOR_BLOCKED :
      log_e("K_ERROR_MOTOR_BLOCKED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_MOTOR_LOW_VOLTAGE :
      log_e("K_ERROR_MOTOR_LOW_VOLTAGE");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_MOTOR_POSITION_LIMIT :
      log_e("K_ERROR_MOTOR_POSITION_LIMIT");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_MOTOR_POWER_FAILURE :
      log_e("K_ERROR_MOTOR_POWER_FAILURE");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_MOTOR_TIMEOUT :
      log_e("K_ERROR_MOTOR_TIMEOUT");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_NOT_AUTHORIZED :
      log_e("K_ERROR_NOT_AUTHORIZED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_NOT_CALIBRATED :
      log_e("K_ERROR_NOT_CALIBRATED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_POSITION_UNKNOWN :
      log_e("K_ERROR_POSITION_UNKNOWN");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_REMOTE_NOT_ALLOWED :
      log_e("K_ERROR_REMOTE_NOT_ALLOWED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_TIME_NOT_ALLOWED :
      log_e("K_ERROR_TIME_NOT_ALLOWED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_TOO_MANY_ENTRIES :
      log_e("K_ERROR_TOO_MANY_ENTRIES");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_TOO_MANY_PIN_ATTEMPTS :
      log_e("K_ERROR_TOO_MANY_PIN_ATTEMPTS");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_VOLTAGE_TOO_LOW :
      log_e("K_ERROR_VOLTAGE_TOO_LOW");
      break;
    default:
      log_e("UNDEFINED ERROR");
  }
}

void NukiBle::handleReturnMessage(uint16_t returnCode, char* data, uint8_t dataLen) {

  switch (returnCode) {
    case (uint16_t)nukiCommand::requestData :
      log_d("requestData");
      break;
    case (uint16_t)nukiCommand::publicKey :
      memcpy(remotePublicKey, data, 32);
      printBuffer(remotePublicKey, sizeof(remotePublicKey), false,  "Remote public key");
      break;
    case (uint16_t)nukiCommand::challenge :
      memcpy(challengeNonceK, data, 32);
      printBuffer((byte*)data, dataLen, false, "Challenge");
      break;
    case (uint16_t)nukiCommand::authorizationAuthenticator :
      printBuffer((byte*)data, dataLen, false, "authorizationAuthenticator");
      break;
    case (uint16_t)nukiCommand::authorizationData :
      printBuffer((byte*)data, dataLen, false, "authorizationData");
      break;
    case (uint16_t)nukiCommand::authorizationId :
      printBuffer((byte*)data, dataLen, false, "send authorizationId data");
      memcpy(authorizationId, &data[32], 4);
      memcpy(lockId, &data[36], sizeof(lockId));
      memcpy(challengeNonceK, &data[52], sizeof(challengeNonceK));
      printBuffer(authorizationId, sizeof(authorizationId), false, "authorizationId");
      printBuffer(lockId, sizeof(lockId), false, "lockId");
      break;
    case (uint16_t)nukiCommand::removeUserAuthorization :
      printBuffer((byte*)data, dataLen, false, "removeUserAuthorization");
      break;
    case (uint16_t)nukiCommand::requestAuthorizationEntries :
      printBuffer((byte*)data, dataLen, false, "requestAuthorizationEntries");
      break;
    case (uint16_t)nukiCommand::authorizationEntry :
      printBuffer((byte*)data, dataLen, false, "authorizationEntry");
      break;
    case (uint16_t)nukiCommand::authorizationDatInvite :
      printBuffer((byte*)data, dataLen, false, "authorizationDatInvite");
      break;
    case (uint16_t)nukiCommand::keyturnerStates :
      printBuffer((byte*)data, dataLen, false, "keyturnerStates");
      break;
    case (uint16_t)nukiCommand::lockAction :
      printBuffer((byte*)data, dataLen, false, "lockAction");
      break;
    case (uint16_t)nukiCommand::status :
      printBuffer((byte*)data, dataLen, false, "status");
      break;
    case (uint16_t)nukiCommand::mostRecentCommand :
      printBuffer((byte*)data, dataLen, false, "mostRecentCommand");
      break;
    case (uint16_t)nukiCommand::openingsClosingsSummary :
      printBuffer((byte*)data, dataLen, false, "openingsClosingsSummary");
      break;
    case (uint16_t)nukiCommand::batteryReport :
      printBuffer((byte*)data, dataLen, false, "batteryReport");
      break;
    case (uint16_t)nukiCommand::errorReport :
      printBuffer((byte*)data, dataLen, false, "errorReport");
      break;
    case (uint16_t)nukiCommand::setConfig :
      printBuffer((byte*)data, dataLen, false, "setConfig");
      break;
    case (uint16_t)nukiCommand::requestConfig :
      printBuffer((byte*)data, dataLen, false, "requestConfig");
      break;
    case (uint16_t)nukiCommand::config :
      printBuffer((byte*)data, dataLen, false, "config");
      break;
    case (uint16_t)nukiCommand::setSecurityPin :
      printBuffer((byte*)data, dataLen, false, "setSecurityPin");
      break;
    case (uint16_t)nukiCommand::requestCalibration :
      printBuffer((byte*)data, dataLen, false, "requestCalibration");
      break;
    case (uint16_t)nukiCommand::requestReboot :
      printBuffer((byte*)data, dataLen, false, "requestReboot");
      break;
    case (uint16_t)nukiCommand::authorizationIdConfirmation :
      printBuffer((byte*)data, dataLen, false, "authorizationIdConfirmation");
      break;
    case (uint16_t)nukiCommand::authorizationIdInvite :
      printBuffer((byte*)data, dataLen, false, "authorizationIdInvite");
      break;
    case (uint16_t)nukiCommand::verifySecurityPin :
      printBuffer((byte*)data, dataLen, false, "verifySecurityPin");
      break;
    case (uint16_t)nukiCommand::updateTime :
      printBuffer((byte*)data, dataLen, false, "updateTime");
      break;
    case (uint16_t)nukiCommand::updateUserAuthorization :
      printBuffer((byte*)data, dataLen, false, "updateUserAuthorization");
      break;
    case (uint16_t)nukiCommand::authorizationEntryCount :
      printBuffer((byte*)data, dataLen, false, "authorizationEntryCount");
      break;
    case (uint16_t)nukiCommand::requestLogEntries :
      printBuffer((byte*)data, dataLen, false, "requestLogEntries");
      break;
    case (uint16_t)nukiCommand::logEntry :
      printBuffer((byte*)data, dataLen, false, "logEntry");
      break;
    case (uint16_t)nukiCommand::logEntryCount :
      printBuffer((byte*)data, dataLen, false, "logEntryCount");
      break;
    case (uint16_t)nukiCommand::enableLogging :
      printBuffer((byte*)data, dataLen, false, "enableLogging");
      break;
    case (uint16_t)nukiCommand::setAdvancedConfig :
      printBuffer((byte*)data, dataLen, false, "setAdvancedConfig");
      break;
    case (uint16_t)nukiCommand::requestAdvancedConfig :
      printBuffer((byte*)data, dataLen, false, "requestAdvancedConfig");
      break;
    case (uint16_t)nukiCommand::advancedConfig :
      printBuffer((byte*)data, dataLen, false, "advancedConfig");
      break;
    case (uint16_t)nukiCommand::addTimeControlEntry :
      printBuffer((byte*)data, dataLen, false, "addTimeControlEntry");
      break;
    case (uint16_t)nukiCommand::timeControlEntryId :
      printBuffer((byte*)data, dataLen, false, "timeControlEntryId");
      break;
    case (uint16_t)nukiCommand::removeTimeControlEntry :
      printBuffer((byte*)data, dataLen, false, "removeTimeControlEntry");
      break;
    case (uint16_t)nukiCommand::requestTimeControlEntries :
      printBuffer((byte*)data, dataLen, false, "requestTimeControlEntries");
      break;
    case (uint16_t)nukiCommand::timeControlEntryCount :
      printBuffer((byte*)data, dataLen, false, "timeControlEntryCount");
      break;
    case (uint16_t)nukiCommand::timeControlEntry :
      printBuffer((byte*)data, dataLen, false, "timeControlEntry");
      break;
    case (uint16_t)nukiCommand::updateTimeControlEntry :
      printBuffer((byte*)data, dataLen, false, "updateTimeControlEntry");
      break;
    case (uint16_t)nukiCommand::addKeypadCode :
      printBuffer((byte*)data, dataLen, false, "addKeypadCode");
      break;
    case (uint16_t)nukiCommand::keypadCodeId :
      printBuffer((byte*)data, dataLen, false, "keypadCodeId");
      break;
    case (uint16_t)nukiCommand::requestKeypadCodes :
      printBuffer((byte*)data, dataLen, false, "requestKeypadCodes");
      break;
    case (uint16_t)nukiCommand::keypadCodeCount :
      printBuffer((byte*)data, dataLen, false, "keypadCodeCount");
      break;
    case (uint16_t)nukiCommand::keypadCode :
      printBuffer((byte*)data, dataLen, false, "keypadCode");
      break;
    case (uint16_t)nukiCommand::updateKeypadCode :
      printBuffer((byte*)data, dataLen, false, "updateKeypadCode");
      break;
    case (uint16_t)nukiCommand::removeKeypadCode :
      printBuffer((byte*)data, dataLen, false, "removeKeypadCode");
      break;
    case (uint16_t)nukiCommand::keypadAction :
      printBuffer((byte*)data, dataLen, false, "keypadAction");
      break;
    case (uint16_t)nukiCommand::simpleLockAction :
      printBuffer((byte*)data, dataLen, false, "simpleLockAction");
      break;
    default:
      log_e("UNKNOWN RETURN COMMAND");
  }
}

void NukiBle::pushNotificationToQueue() {
  bool notification = true;
  xQueueSendFromISR(nukiBleIsrFlagQueue, &notification, &xHigherPriorityTaskWoken);
}

void NukiBle::my_gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t* param) {
  ESP_LOGW(LOG_TAG, "custom gattc event handler, event: %d", (uint8_t)event);
  if (event == ESP_GATTC_DISCONNECT_EVT) {
    Serial.print("Disconnect reason: ");
    Serial.println((int)param->disconnect.reason);
  }
}

void NukiBle::onConnect(BLEClient*) {
  #ifdef DEBUG_NUKI
  log_d("BLE connected");
  #endif
};

void NukiBle::onDisconnect(BLEClient*) {
  #ifdef DEBUG_NUKI
  log_d("BLE disconnected");
  #endif
};

// void NukiBle::keyGen(uint8_t *key, uint8_t keyLen, uint8_t seedPin){
//   randomSeed(analogRead(seedPin));
//   for(int i=0; i < keyLen; i++){
//     key[i] = random(1, 255);
//   }
//   printBuffer((byte*)key, keyLen, false, "Generated key");
// }

void NukiBle::generateNonce(unsigned char* hexArray, uint8_t nrOfBytes) {

  for (int i = 0 ; i < nrOfBytes ; i++) {
    randomSeed(millis());
    hexArray[i] = random(0, 65500);
  }
  printBuffer((byte*)hexArray, nrOfBytes, false, "Nonce");
}