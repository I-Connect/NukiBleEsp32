/**
 * @file NukiBle.cpp
 *
 * Created: 2022
 * License: GNU GENERAL PUBLIC LICENSE (see LICENSE)
 *
 * This library implements the communication from an ESP32 via BLE to a Nuki smart lock.
 * Based on the Nuki Smart Lock API V2.2.1
 * https://developer.nuki.io/page/nuki-smart-lock-api-2/2/
 *
 */

#include "NukiBle.h"
#include "NukiLockUtils.h"
#include "NukiUtils.h"
#include "string.h"
#include "sodium/crypto_scalarmult.h"
#include "sodium/crypto_core_hsalsa20.h"
#include "sodium/crypto_auth_hmacsha256.h"
#include "sodium/crypto_secretbox.h"
#include "sodium/crypto_box.h"
#include "NimBLEBeacon.h"

#define NUKI_SEMAPHORE_TIMEOUT 1000

namespace Nuki {

const char* NUKI_SEMAPHORE_OWNER = "Nuki";

NukiBle::NukiBle(const std::string& deviceName,
                 const uint32_t deviceId,
                 const NimBLEUUID pairingServiceUUID,
                 const NimBLEUUID deviceServiceUUID,
                 const NimBLEUUID gdioUUID,
                 const NimBLEUUID userDataUUID,
                 const std::string preferencedId)
  : deviceName(deviceName),
    deviceId(deviceId),
    pairingServiceUUID(pairingServiceUUID),
    deviceServiceUUID(deviceServiceUUID),
    gdioUUID(gdioUUID),
    userDataUUID(userDataUUID),
    preferencesId(preferencedId) {
}

NukiBle::~NukiBle() {
  if (bleScanner != nullptr) {
    bleScanner->unsubscribe(this);
    bleScanner = nullptr;
  }
}

void NukiBle::initialize() {
  preferences.begin(preferencesId.c_str(), false);
  if (!BLEDevice::getInitialized()) {
    BLEDevice::init(deviceName);
  }

  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(this);
  pClient->setConnectTimeout(connectTimeoutSec);

  isPaired = retrieveCredentials();
}

void NukiBle::registerBleScanner(BleScanner::Publisher* bleScanner) {
  this->bleScanner = bleScanner;
  bleScanner->subscribe(this);
}

PairingResult NukiBle::pairNuki(AuthorizationIdType idType) {
  authorizationIdType = idType;

  if (retrieveCredentials()) {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Allready paired");
    #endif
    isPaired = true;
    return PairingResult::Success;
  }
  PairingResult result = PairingResult::Pairing;

  if (pairingServiceAvailable && bleAddress != BLEAddress("")) {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Nuki in pairing mode found");
    #endif
    if (connectBle(bleAddress)) {
      crypto_box_keypair(myPublicKey, myPrivateKey);

      PairingState nukiPairingState = PairingState::InitPairing;
      do {
        nukiPairingState = pairStateMachine(nukiPairingState);
        delay(50);
      } while ((nukiPairingState != PairingState::Success) && (nukiPairingState != PairingState::Timeout));

      if (nukiPairingState == PairingState::Success) {
        saveCredentials();
        result = PairingResult::Success;
        lastHeartbeat = millis();
      } else {
        result = PairingResult::Timeout;
      }
      extendDisonnectTimeout();
    }
  } else {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("No nuki in pairing mode found");
    #endif
  }

  #ifdef DEBUG_NUKI_CONNECT
  log_d("pairing result %d", result);
  #endif

  isPaired = (result == PairingResult::Success);
  return result;
}

void NukiBle::unPairNuki() {
  deleteCredentials();
  isPaired = false;
  #ifdef DEBUG_NUKI_CONNECT
  log_d("[%s] Credentials deleted", deviceName.c_str());
  #endif
}

bool NukiBle::connectBle(const BLEAddress bleAddress) {
  connecting = true;
  bleScanner->enableScanning(false);
  if (!pClient->isConnected()) {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("connecting within: %s", pcTaskGetTaskName(xTaskGetCurrentTaskHandle()));
    #endif

    uint8_t connectRetry = 0;
    pClient->setConnectTimeout(connectTimeoutSec);
    while (connectRetry < connectRetries) {
      #ifdef DEBUG_NUKI_CONNECT
      log_d("connection attemnpt %d", connectRetry);
      #endif
      if (pClient->connect(bleAddress, true)) {
        if (pClient->isConnected() && registerOnGdioChar() && registerOnUsdioChar()) {  //doublecheck if is connected otherwise registiring gdio crashes esp
          bleScanner->enableScanning(true);
          connecting = false;
          return true;
        } else {
          log_w("BLE register on pairing or data Service/Char failed");
        }
      } else {
        pClient->disconnect();
        log_w("BLE Connect failed, %d retries left", connectRetries - connectRetry - 1);
      }
      connectRetry++;
      esp_task_wdt_reset();
      delay(10);
    }
  } else {
    bleScanner->enableScanning(true);
    connecting = false;
    return true;
  }
  bleScanner->enableScanning(true);
  connecting = false;
  log_w("BLE Connect failed");
  return false;
}

void NukiBle::updateConnectionState() {
  if (connecting) {
    lastStartTimeout = 0;
  }

  if (lastStartTimeout != 0 && (millis() - lastStartTimeout > timeoutDuration)) {
    if (pClient && pClient->isConnected()) {
      pClient->disconnect();
      #ifdef DEBUG_NUKI_CONNECT
      log_d("disconnecting BLE on timeout");
      #endif
    }
  }
}

void NukiBle::setDisonnectTimeout(uint32_t timeoutMs) {
  timeoutDuration = timeoutMs;
}

void NukiBle::setConnectTimeout(uint8_t timeout) {
  connectTimeoutSec = timeout;
}

void NukiBle::setConnectRetries(uint8_t retries) {
  connectRetries = retries;
}

void NukiBle::extendDisonnectTimeout() {
  lastStartTimeout = millis();
}

void NukiBle::onResult(BLEAdvertisedDevice* advertisedDevice) {
  if (isPaired) {
    if (bleAddress == advertisedDevice->getAddress()) {
      rssi = advertisedDevice->getRSSI();
      lastReceivedBeaconTs = millis();

      std::string manufacturerData = advertisedDevice->getManufacturerData();
      uint8_t* manufacturerDataPtr = (uint8_t*)manufacturerData.data();
      char* pHex = BLEUtils::buildHexData(nullptr, manufacturerDataPtr, manufacturerData.length());

      bool isKeyTurnerUUID = true;
      std::string serviceUUID = deviceServiceUUID.toString();
      size_t len = serviceUUID.length();
      int offset = 0;
      for (int i = 0; i < len; i++) {
        if (serviceUUID[i + offset] == '-') {
          ++offset;
          --len;
        }

        if (pHex[i + 8] != serviceUUID.at(i + offset)) {
          isKeyTurnerUUID = false;
        }
      }
      free(pHex);

      if (isKeyTurnerUUID) {
        #ifdef DEBUG_NUKI_CONNECT
        log_d("Nuki Advertising: %s", advertisedDevice->toString().c_str());
        #endif

        uint8_t cManufacturerData[100];
        manufacturerData.copy((char*)cManufacturerData, manufacturerData.length(), 0);

        if (manufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00) {
          BLEBeacon oBeacon = BLEBeacon();
          oBeacon.setData(manufacturerData);
          #ifdef DEBUG_NUKI_CONNECT
          log_d("iBeacon ID: %04X Major: %d Minor: %d UUID: %s Power: %d\n", oBeacon.getManufacturerId(),
                ENDIAN_CHANGE_U16(oBeacon.getMajor()), ENDIAN_CHANGE_U16(oBeacon.getMinor()),
                oBeacon.getProximityUUID().toString().c_str(), oBeacon.getSignalPower());
          #endif
          lastHeartbeat = millis();
          if ((oBeacon.getSignalPower() & 0x01) > 0) {
            if (eventHandler) {
              eventHandler->notify(EventType::KeyTurnerStatusUpdated);
            }
          }
        }
      }
    }
  } else {
    if (advertisedDevice->haveServiceData()) {
      if (advertisedDevice->getServiceData(pairingServiceUUID) != "") {
        #ifdef DEBUG_NUKI_CONNECT
        log_d("Found nuki in pairing state: %s addr: %s", std::string(advertisedDevice->getName()).c_str(), std::string(advertisedDevice->getAddress()).c_str());
        #endif
        bleAddress = advertisedDevice->getAddress();
        pairingServiceAvailable = true;
      } else {
        pairingServiceAvailable = false;
      }
    }
  }
}

Nuki::CmdResult NukiBle::retrieveKeypadEntries(const uint16_t offset, const uint16_t count) {
  NukiLock::Action action;
  unsigned char payload[4] = {0};
  memcpy(payload, &offset, 2);
  memcpy(&payload[2], &count, 2);

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::RequestKeypadCodes;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  listOfKeyPadEntries.clear();
  nrOfReceivedKeypadCodes = 0;
  keypadCodeCountReceived = false;

  uint32_t timeNow = millis();
  Nuki::CmdResult result = executeAction(action);

  if (result == Nuki::CmdResult::Success) {
    //wait for return of Keypad Code Count (0x0044)
    while (!keypadCodeCountReceived) {
      if (millis() - timeNow > GENERAL_TIMEOUT) {
        log_w("Receive keypad count timeout");
        return CmdResult::TimeOut;
      }
      delay(10);
    }
    #ifdef DEBUG_NUKI_COMMAND
    log_d("Keypad code count %d", getKeypadEntryCount());
    #endif

    //wait for return of Keypad Codes (0x0045)
    timeNow = millis();
    while (nrOfReceivedKeypadCodes < getKeypadEntryCount()) {
      if (millis() - timeNow > GENERAL_TIMEOUT) {
        log_w("Receive keypadcodes timeout");
        return CmdResult::TimeOut;
      }
      delay(10);
    }
    #ifdef DEBUG_NUKI_COMMAND
    log_d("%d codes received", nrOfReceivedKeypadCodes);
    #endif
  } else {
    log_w("Retreive keypad codes from lock failed");
  }
  return result;
}

Nuki::CmdResult NukiBle::addKeypadEntry(NewKeypadEntry newKeypadEntry) {
  //TODO verify data validity, ie check for invalid chars in name
  NukiLock::Action action;

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::AddKeypadCode;
  memcpy(action.payload, &newKeypadEntry, sizeof(NewKeypadEntry));
  action.payloadLen = sizeof(NewKeypadEntry);

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addKeyPadEntry, payloadlen: %d", sizeof(NewKeypadEntry));
    printBuffer(action.payload, sizeof(NewKeypadEntry), false, "addKeyPadCode content: ");
    NukiLock::logNewKeypadEntry(newKeypadEntry);
    #endif
  }
  return result;
}

Nuki::CmdResult NukiBle::updateKeypadEntry(UpdatedKeypadEntry updatedKeyPadEntry) {
  //TODO verify data validity
  NukiLock::Action action;

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::UpdateKeypadCode;
  memcpy(action.payload, &updatedKeyPadEntry, sizeof(UpdatedKeypadEntry));
  action.payloadLen = sizeof(UpdatedKeypadEntry);

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addKeyPadEntry, payloadlen: %d", sizeof(UpdatedKeypadEntry));
    printBuffer(action.payload, sizeof(UpdatedKeypadEntry), false, "updatedKeypad content: ");
    NukiLock::logUpdatedKeypadEntry(updatedKeyPadEntry);
    #endif
  }
  return result;
}

void NukiBle::getKeypadEntries(std::list<KeypadEntry>* requestedKeypadCodes) {
  requestedKeypadCodes->clear();
  std::list<KeypadEntry>::iterator it = listOfKeyPadEntries.begin();
  while (it != listOfKeyPadEntries.end()) {
    requestedKeypadCodes->push_back(*it);
    it++;
  }
}

uint16_t NukiBle::getKeypadEntryCount() {
  return nrOfKeypadCodes;
}

CmdResult NukiBle::deleteKeypadEntry(uint16_t id) {
  NukiLock::Action action;
  unsigned char payload[2] = {0};
  memcpy(payload, &id, 2);

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::RemoveKeypadCode;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  return executeAction(action);
}

Nuki::CmdResult NukiBle::retrieveAuthorizationEntries(const uint16_t offset, const uint16_t count) {
  NukiLock::Action action;
  unsigned char payload[4] = {0};
  memcpy(payload, &offset, 2);
  memcpy(&payload[2], &count, 2);

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::RequestAuthorizationEntries;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  listOfAuthorizationEntries.clear();

  return executeAction(action);
}

void NukiBle::getAuthorizationEntries(std::list<AuthorizationEntry>* requestedAuthorizationEntries) {
  requestedAuthorizationEntries->clear();
  std::list<AuthorizationEntry>::iterator it = listOfAuthorizationEntries.begin();
  while (it != listOfAuthorizationEntries.end()) {
    requestedAuthorizationEntries->push_back(*it);
    it++;
  }
}

Nuki::CmdResult NukiBle::addAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry) {
  //TODO verify data validity
  NukiLock::Action action;
  unsigned char payload[sizeof(NewAuthorizationEntry)] = {0};
  memcpy(payload, &newAuthorizationEntry, sizeof(NewAuthorizationEntry));

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::AuthorizationDatInvite;
  memcpy(action.payload, &payload, sizeof(NewAuthorizationEntry));
  action.payloadLen = sizeof(NewAuthorizationEntry);

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addAuthorizationEntry, payloadlen: %d", sizeof(NewAuthorizationEntry));
    printBuffer(action.payload, sizeof(NewAuthorizationEntry), false, "addAuthorizationEntry content: ");
    NukiLock::logNewAuthorizationEntry(newAuthorizationEntry);
    #endif
  }
  return result;
}

Nuki::CmdResult NukiBle::updateAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry) {
  //TODO verify data validity
  NukiLock::Action action;
  unsigned char payload[sizeof(UpdatedAuthorizationEntry)] = {0};
  memcpy(payload, &updatedAuthorizationEntry, sizeof(UpdatedAuthorizationEntry));

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::UpdateAuthorization;
  memcpy(action.payload, &payload, sizeof(UpdatedAuthorizationEntry));
  action.payloadLen = sizeof(UpdatedAuthorizationEntry);

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addAuthorizationEntry, payloadlen: %d", sizeof(UpdatedAuthorizationEntry));
    printBuffer(action.payload, sizeof(UpdatedAuthorizationEntry), false, "updatedKeypad content: ");
    NukiLock::logUpdatedAuthorizationEntry(updatedAuthorizationEntry);
    #endif
  }
  return result;
}

uint16_t NukiBle::getLogEntryCount() {
  return logEntryCount;
}

Nuki::CmdResult NukiBle::setSecurityPin(const uint16_t newSecurityPin) {
  NukiLock::Action action;
  unsigned char payload[2] = {0};
  memcpy(payload, &newSecurityPin, 2);

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::SetSecurityPin;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    pinCode = newSecurityPin;
    saveCredentials();
  }
  return result;
}

Nuki::CmdResult NukiBle::verifySecurityPin() {
  NukiLock::Action action;

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::VerifySecurityPin;
  action.payloadLen = 0;

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Verify security pin code success");
    #endif
  }
  return result;
}

Nuki::CmdResult NukiBle::requestCalibration() {
  NukiLock::Action action;

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::RequestCalibration;
  action.payloadLen = 0;

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Calibration executed");
    #endif
  }
  return result;
}

Nuki::CmdResult NukiBle::requestReboot() {
  NukiLock::Action action;

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::RequestReboot;
  action.payloadLen = 0;

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Reboot executed");
    #endif
  }
  return result;
}

Nuki::CmdResult NukiBle::updateTime(TimeValue time) {
  NukiLock::Action action;
  unsigned char payload[sizeof(TimeValue)] = {0};
  memcpy(payload, &time, sizeof(TimeValue));

  action.cmdType = Nuki::CommandType::CommandWithChallengeAndPin;
  action.command = Command::UpdateTime;
  memcpy(action.payload, &payload, sizeof(TimeValue));
  action.payloadLen = sizeof(TimeValue);

  Nuki::CmdResult result = executeAction(action);
  if (result == Nuki::CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Time set: %d-%d-%d %d:%d:%d", time.year, time.month, time.day, time.hour, time.minute, time.second);
    #endif
  }
  return result;
}

bool NukiBle::saveSecurityPincode(const uint16_t pinCode) {
  if (preferences.putBytes(SECURITY_PINCODE_STORE_NAME, &pinCode, 2) == 2) {
    this->pinCode = pinCode;
    return true;
  }
  return false;
}

void NukiBle::saveCredentials() {
  unsigned char currentBleAddress[6];
  unsigned char storedBleAddress[6];
  uint16_t defaultPincode = 0;
  currentBleAddress[0] = bleAddress.getNative()[5];
  currentBleAddress[1] = bleAddress.getNative()[4];
  currentBleAddress[2] = bleAddress.getNative()[3];
  currentBleAddress[3] = bleAddress.getNative()[2];
  currentBleAddress[4] = bleAddress.getNative()[1];
  currentBleAddress[5] = bleAddress.getNative()[0];

  preferences.getBytes(BLE_ADDRESS_STORE_NAME, storedBleAddress, 6);

  if (compareCharArray(currentBleAddress, storedBleAddress, 6)) {
    //only store earlier retreived pin code if address is the same
    //otherwise it is a different/new lock
    preferences.putBytes(SECURITY_PINCODE_STORE_NAME, &pinCode, 2);
  } else {
    preferences.putBytes(SECURITY_PINCODE_STORE_NAME, &defaultPincode, 2);
  }

  if ((preferences.putBytes(BLE_ADDRESS_STORE_NAME, currentBleAddress, 6) == 6)
      && (preferences.putBytes(SECRET_KEY_STORE_NAME, secretKeyK, 32) == 32)
      && (preferences.putBytes(AUTH_ID_STORE_NAME, authorizationId, 4) == 4)
     ) {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Credentials saved:");
    printBuffer(secretKeyK, sizeof(secretKeyK), false, SECRET_KEY_STORE_NAME);
    printBuffer(currentBleAddress, 6, false, BLE_ADDRESS_STORE_NAME);
    printBuffer(authorizationId, sizeof(authorizationId), false, AUTH_ID_STORE_NAME);
    log_d("pincode: %d", pinCode);
    #endif
  } else {
    log_w("ERROR saving credentials");
  }
}

uint16_t NukiBle::getSecurityPincode() {

  if (takeNukiBleSemaphore("retr pincode cred")) {
    uint16_t storedPincode = 0000;
    if ((preferences.getBytes(SECURITY_PINCODE_STORE_NAME, &storedPincode, 2) > 0)) {
      giveNukiBleSemaphore();
      return storedPincode;
    }
    giveNukiBleSemaphore();
  }
  return 0;
}

void NukiBle::getMacAddress(char* macAddress) {
  unsigned char buf[6];
  if (takeNukiBleSemaphore("retr pincode cred")) {
    if ((preferences.getBytes(BLE_ADDRESS_STORE_NAME, buf, 6) > 0)) {
      BLEAddress address = BLEAddress(buf);
      sprintf(macAddress, "%d", address.toString().c_str());
      giveNukiBleSemaphore();
    }
    giveNukiBleSemaphore();
  }
}

bool NukiBle::retrieveCredentials() {
  //TODO check on empty (invalid) credentials?
  unsigned char buff[6];

  if (takeNukiBleSemaphore("retr cred")) {
    if ((preferences.getBytes(BLE_ADDRESS_STORE_NAME, buff, 6) > 0)
        && (preferences.getBytes(SECURITY_PINCODE_STORE_NAME, &pinCode, 2) > 0)
        && (preferences.getBytes(SECRET_KEY_STORE_NAME, secretKeyK, 32) > 0)
        && (preferences.getBytes(AUTH_ID_STORE_NAME, authorizationId, 4) > 0)
       ) {
      bleAddress = BLEAddress(buff);

      #ifdef DEBUG_NUKI_CONNECT
      log_d("[%s] Credentials retrieved :", deviceName.c_str());
      printBuffer(secretKeyK, sizeof(secretKeyK), false, SECRET_KEY_STORE_NAME);
      log_d("bleAddress: %s", bleAddress.toString().c_str());
      printBuffer(authorizationId, sizeof(authorizationId), false, AUTH_ID_STORE_NAME);
      #endif

      if (isCharArrayEmpty(secretKeyK, sizeof(secretKeyK)) || isCharArrayEmpty(authorizationId, sizeof(authorizationId))) {
        log_w("secret key OR authorizationId is empty: not paired");
        giveNukiBleSemaphore();
        return false;
      }

      if (pinCode == 0) {
        log_w("Pincode is 000000, probably not defined");
      }

    } else {
      log_e("Error getting data from NVS");
      giveNukiBleSemaphore();
      return false;
    }
    giveNukiBleSemaphore();
  }

  return true;
}

void NukiBle::deleteCredentials() {
  if (takeNukiBleSemaphore("del cred")) {
    unsigned char emptySecretKeyK[32] = {0x00};
    unsigned char emptyAuthorizationId[4] = {0x00};
    preferences.putBytes(SECRET_KEY_STORE_NAME, emptySecretKeyK, 32);
    preferences.putBytes(AUTH_ID_STORE_NAME, emptyAuthorizationId, 4);
    // preferences.remove(SECRET_KEY_STORE_NAME);
    // preferences.remove(AUTH_ID_STORE_NAME);
    giveNukiBleSemaphore();
  }
  #ifdef DEBUG_NUKI_CONNECT
  log_d("Credentials deleted");
  #endif
}

PairingState NukiBle::pairStateMachine(const PairingState nukiPairingState) {
  switch (nukiPairingState) {
    case PairingState::InitPairing: {
      memset(challengeNonceK, 0, sizeof(challengeNonceK));
      memset(remotePublicKey, 0, sizeof(remotePublicKey));
      receivedStatus = 0xff;
      timeNow = millis();
      nukiPairingResultState = PairingState::ReqRemPubKey;
    }
    case PairingState::ReqRemPubKey: {
      //Request remote public key (Sent message should be 0100030027A7)
      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### REQUEST REMOTE PUBLIC KEY #########################");
      #endif
      unsigned char buff[sizeof(Command)];
      uint16_t cmd = (uint16_t)Command::PublicKey;
      memcpy(buff, &cmd, sizeof(Command));
      sendPlainMessage(Command::RequestData, buff, sizeof(Command));
      nukiPairingResultState = PairingState::RecRemPubKey;
    }
    case PairingState::RecRemPubKey: {
      if (isCharArrayNotEmpty(remotePublicKey, sizeof(remotePublicKey))) {
        nukiPairingResultState = PairingState::SendPubKey;
      }
      break;
    }
    case PairingState::SendPubKey: {
      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### SEND CLIENT PUBLIC KEY #########################");
      #endif
      sendPlainMessage(Command::PublicKey, myPublicKey, sizeof(myPublicKey));
      nukiPairingResultState = PairingState::GenKeyPair;
    }
    case PairingState::GenKeyPair: {
      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### CALCULATE DH SHARED KEY s #########################");
      #endif
      unsigned char sharedKeyS[32] = {0x00};
      crypto_scalarmult_curve25519(sharedKeyS, myPrivateKey, remotePublicKey);
      printBuffer(sharedKeyS, sizeof(sharedKeyS), false, "Shared key s");

      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### DERIVE LONG TERM SHARED SECRET KEY k #########################");
      #endif
      unsigned char in[16];
      memset(in, 0, 16);
      unsigned char sigma[] = "expand 32-byte k";
      crypto_core_hsalsa20(secretKeyK, in, sharedKeyS, sigma);
      printBuffer(secretKeyK, sizeof(secretKeyK), false, "Secret key k");
      nukiPairingResultState = PairingState::CalculateAuth;
    }
    case PairingState::CalculateAuth: {
      if (isCharArrayNotEmpty(challengeNonceK, sizeof(challengeNonceK))) {
        #ifdef DEBUG_NUKI_CONNECT
        log_d("##################### CALCULATE/VERIFY AUTHENTICATOR #########################");
        #endif
        //concatenate local public key, remote public key and receive challenge data
        unsigned char hmacPayload[96];
        memcpy(&hmacPayload[0], myPublicKey, sizeof(myPublicKey));
        memcpy(&hmacPayload[32], remotePublicKey, sizeof(remotePublicKey));
        memcpy(&hmacPayload[64], challengeNonceK, sizeof(challengeNonceK));
        printBuffer((byte*)hmacPayload, sizeof(hmacPayload), false, "Concatenated data r");
        crypto_auth_hmacsha256(authenticator, hmacPayload, sizeof(hmacPayload), secretKeyK);
        printBuffer(authenticator, sizeof(authenticator), false, "HMAC 256 result");
        memset(challengeNonceK, 0, sizeof(challengeNonceK));
        nukiPairingResultState = PairingState::SendAuth;
      }
      break;
    }
    case PairingState::SendAuth: {
      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### SEND AUTHENTICATOR #########################");
      #endif
      sendPlainMessage(Command::AuthorizationAuthenticator, authenticator, sizeof(authenticator));
      nukiPairingResultState = PairingState::SendAuthData;
    }
    case PairingState::SendAuthData: {
      if (isCharArrayNotEmpty(challengeNonceK, sizeof(challengeNonceK))) {
        #ifdef DEBUG_NUKI_CONNECT
        log_d("##################### SEND AUTHORIZATION DATA #########################");
        #endif
        unsigned char authorizationData[101] = {};
        unsigned char authorizationDataIdType[1] = {(unsigned char)authorizationIdType };
        unsigned char authorizationDataId[4] = {};
        unsigned char authorizationDataName[32] = {};
        unsigned char authorizationDataNonce[32] = {};
        authorizationDataId[0] = (deviceId >> (8 * 0)) & 0xff;
        authorizationDataId[1] = (deviceId >> (8 * 1)) & 0xff;
        authorizationDataId[2] = (deviceId >> (8 * 2)) & 0xff;
        authorizationDataId[3] = (deviceId >> (8 * 3)) & 0xff;
        memcpy(authorizationDataName, deviceName.c_str(), deviceName.size());
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

        memset(challengeNonceK, 0, sizeof(challengeNonceK));
        sendPlainMessage(Command::AuthorizationData, authorizationDataMessage, sizeof(authorizationDataMessage));
        nukiPairingResultState = PairingState::SendAuthIdConf;
      }
      break;
    }
    case PairingState::SendAuthIdConf: {
      if (isCharArrayNotEmpty(authorizationId, sizeof(authorizationId))) {
        #ifdef DEBUG_NUKI_CONNECT
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
        sendPlainMessage(Command::AuthorizationIdConfirmation, confirmationDataMessage, sizeof(confirmationDataMessage));
        nukiPairingResultState = PairingState::RecStatus;
      }
      break;
    }
    case PairingState::RecStatus: {
      if (receivedStatus == 0) {
        #ifdef DEBUG_NUKI_CONNECT
        log_d("####################### PAIRING DONE ###############################################");
        #endif
        nukiPairingResultState = PairingState::Success;
      }
      break;
    }
    default: {
      log_e("Unknown pairing status");
      nukiPairingResultState = PairingState::Timeout;
    }
  }

  if (millis() - timeNow > PAIRING_TIMEOUT) {
    log_w("Pairing timeout");
    nukiPairingResultState = PairingState::Timeout;
  }

  return nukiPairingResultState;
}

bool NukiBle::sendEncryptedMessage(Command commandIdentifier, const unsigned char* payload, const uint8_t payloadLen) {
  /*
  #     ADDITIONAL DATA (not encr)      #                    PLAIN DATA (encr)                             #
  #  nonce  # auth identifier # msg len # authorization identifier # command identifier # payload #  crc   #
  # 24 byte #    4 byte       # 2 byte  #      4 byte              #       2 byte       #  n byte # 2 byte #
  */

  //compose plain data
  unsigned char plainData[6 + payloadLen] = {};
  unsigned char plainDataWithCrc[8 + payloadLen] = {};

  memcpy(&plainData[0], &authorizationId, sizeof(authorizationId));
  memcpy(&plainData[4], &commandIdentifier, sizeof(commandIdentifier));
  memcpy(&plainData[6], payload, payloadLen);

  //get crc over plain data
  uint16_t dataCrc = calculateCrc((uint8_t*)plainData, 0, sizeof(plainData));

  memcpy(&plainDataWithCrc[0], &plainData, sizeof(plainData));
  memcpy(&plainDataWithCrc[sizeof(plainData)], &dataCrc, sizeof(dataCrc));

  #ifdef DEBUG_NUKI_HEX_DATA
  log_d("payloadlen: %d", payloadLen);
  log_d("sizeof(plainData): %d", sizeof(plainData));
  log_d("CRC: %0.2x", dataCrc);
  #endif
  printBuffer((byte*)plainDataWithCrc, sizeof(plainDataWithCrc), false, "Plain data with CRC: ");

  //compose additional data
  unsigned char additionalData[30] = {};
  generateNonce(sentNonce, sizeof(sentNonce));

  memcpy(&additionalData[0], sentNonce, sizeof(sentNonce));
  memcpy(&additionalData[24], authorizationId, sizeof(authorizationId));

  //Encrypt plain data
  unsigned char plainDataEncr[ sizeof(plainDataWithCrc) + crypto_secretbox_MACBYTES] = {0};
  int encrMsgLen = encode(plainDataEncr, plainDataWithCrc, sizeof(plainDataWithCrc), sentNonce, secretKeyK);

  if (encrMsgLen >= 0) {
    int16_t length = sizeof(plainDataEncr);
    memcpy(&additionalData[28], &length, 2);

    printBuffer((byte*)additionalData, 30, false, "Additional data: ");
    printBuffer((byte*)secretKeyK, sizeof(secretKeyK), false, "Encryption key (secretKey): ");
    printBuffer((byte*)plainDataEncr, sizeof(plainDataEncr), false, "Plain data encrypted: ");

    //compose complete message
    unsigned char dataToSend[sizeof(additionalData) + sizeof(plainDataEncr)] = {};
    memcpy(&dataToSend[0], additionalData, sizeof(additionalData));
    memcpy(&dataToSend[30], plainDataEncr, sizeof(plainDataEncr));

    if (connectBle(bleAddress)) {
      printBuffer((byte*)dataToSend, sizeof(dataToSend), false, "Sending encrypted message");
      return pUsdioCharacteristic->writeValue((uint8_t*)dataToSend, sizeof(dataToSend), true);
    } else {
      log_w("Send encr msg failed due to unable to connect");
    }
  } else {
    log_w("Send msg failed due to encryption fail");
  }
  return false;
}

bool NukiBle::sendPlainMessage(Command commandIdentifier, const unsigned char* payload, const uint8_t payloadLen) {
  /*
  #                PLAIN DATA                   #
  #command identifier  #   payload   #   crc    #
  #      2 byte        #   n byte    #  2 byte  #
  */

  //compose data
  char dataToSend[200];
  memcpy(&dataToSend, &commandIdentifier, sizeof(commandIdentifier));
  memcpy(&dataToSend[2], payload, payloadLen);
  uint16_t dataCrc = calculateCrc((uint8_t*)dataToSend, 0, payloadLen + 2);

  memcpy(&dataToSend[2 + payloadLen], &dataCrc, sizeof(dataCrc));
  printBuffer((byte*)dataToSend, payloadLen + 4, false, "Sending plain message");
  #ifdef DEBUG_NUKI_HEX_DATA
  log_d("Command identifier: %02x, CRC: %04x", (uint32_t)commandIdentifier, dataCrc);
  #endif

  uint32_t beforeConnectBle = millis();
  uint32_t afterConnectBle = 0;
  if (connectBle(bleAddress)) {
    return pGdioCharacteristic->writeValue((uint8_t*)dataToSend, payloadLen + 4, true);
  } else {
    log_w("Send plain msg failed due to unable to connect");
  }
  return false;
}

bool NukiBle::registerOnGdioChar() {
  // Obtain a reference to the KeyTurner Pairing service
  pKeyturnerPairingService = pClient->getService(pairingServiceUUID);
  if (pKeyturnerPairingService != nullptr) {
    //Obtain reference to GDIO char
    pGdioCharacteristic = pKeyturnerPairingService->getCharacteristic(gdioUUID);
    if (pGdioCharacteristic != nullptr) {
      if (pGdioCharacteristic->canIndicate()) {

        using namespace std::placeholders;
        notify_callback callback = std::bind(&NukiBle::notifyCallback, this, _1, _2, _3, _4);
        pGdioCharacteristic->subscribe(false, callback, true); //false = indication, true = notification
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("GDIO characteristic registered");
        #endif
        delay(100);
        return true;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("GDIO characteristic canIndicate false, stop connecting");
        #endif
        return false;
      }
    } else {
      log_w("Unable to get GDIO characteristic");
      return false;
    }
  } else {
    log_w("Unable to get keyturner pairing service");
    return false;
  }
  return false;
}

bool NukiBle::registerOnUsdioChar() {
  // Obtain a reference to the KeyTurner service
  pKeyturnerDataService = pClient->getService(deviceServiceUUID);
  if (pKeyturnerDataService != nullptr) {
    //Obtain reference to NDIO char
    pUsdioCharacteristic = pKeyturnerDataService->getCharacteristic(userDataUUID);
    if (pUsdioCharacteristic != nullptr) {
      if (pUsdioCharacteristic->canIndicate()) {

        using namespace std::placeholders;
        notify_callback callback = std::bind(&NukiBle::notifyCallback, this, _1, _2, _3, _4);

        pUsdioCharacteristic->subscribe(false, callback, true); //false = indication, true = notification
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("USDIO characteristic registered");
        #endif
        delay(100);
        return true;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("USDIO characteristic canIndicate false, stop connecting");
        #endif
        return false;
      }
    } else {
      log_w("Unable to get USDIO characteristic");
      return false;
    }
  } else {
    log_w("Unable to get keyturner data service");
    return false;
  }

  return false;
}

void NukiBle::notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* recData, size_t length, bool isNotify) {

  #ifdef DEBUG_NUKI_COMMUNICATION
  log_d(" Notify callback for characteristic: %s of length: %d", pBLERemoteCharacteristic->getUUID().toString().c_str(), length);
  #endif
  printBuffer((byte*)recData, length, false, "Received data");

  if (pBLERemoteCharacteristic->getUUID() == gdioUUID) {
    //handle not encrypted msg
    uint16_t returnCode = ((uint16_t)recData[1] << 8) | recData[0];
    crcCheckOke = crcValid(recData, length);
    if (crcCheckOke) {
      unsigned char plainData[200];
      memcpy(plainData, &recData[2], length - 4);
      handleReturnMessage((Command)returnCode, plainData, length - 4);
    }
  } else if (pBLERemoteCharacteristic->getUUID() == userDataUUID) {
    //handle encrypted msg
    unsigned char recNonce[crypto_secretbox_NONCEBYTES];
    unsigned char recAuthorizationId[4];
    unsigned char recMsgLen[2];
    memcpy(recNonce, &recData[0], crypto_secretbox_NONCEBYTES);
    memcpy(recAuthorizationId, &recData[crypto_secretbox_NONCEBYTES], 4);
    memcpy(recMsgLen, &recData[crypto_secretbox_NONCEBYTES + 4], 2);
    uint16_t encrMsgLen = 0;
    memcpy(&encrMsgLen, recMsgLen, 2);
    unsigned char encrData[encrMsgLen];
    memcpy(&encrData, &recData[crypto_secretbox_NONCEBYTES + 6], encrMsgLen);

    unsigned char decrData[encrMsgLen - crypto_secretbox_MACBYTES];
    decode(decrData, encrData, encrMsgLen, recNonce, secretKeyK);

    #ifdef DEBUG_NUKI_COMMUNICATION
    log_d("Received encrypted msg, len: %d", encrMsgLen);
    #endif
    printBuffer(recNonce, sizeof(recNonce), false, "received nonce");
    printBuffer(recAuthorizationId, sizeof(recAuthorizationId), false, "Received AuthorizationId");
    printBuffer(encrData, sizeof(encrData), false, "Rec encrypted data");
    printBuffer(decrData, sizeof(decrData), false, "Decrypted data");

    crcCheckOke = crcValid(decrData, sizeof(decrData));
    if (crcCheckOke) {
      uint16_t returnCode = 0;
      memcpy(&returnCode, &decrData[4], 2);
      unsigned char payload[sizeof(decrData) - 8];
      memcpy(&payload, &decrData[6], sizeof(payload));
      handleReturnMessage((Command)returnCode, payload, sizeof(payload));
    }
  }
}

void NukiBle::handleReturnMessage(Command returnCode, unsigned char* data, uint16_t dataLen) {
  switch (returnCode) {
    case Command::RequestData : {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("requestData");
      #endif
      break;
    }
    case Command::PublicKey : {
      memcpy(remotePublicKey, data, 32);
      printBuffer(remotePublicKey, sizeof(remotePublicKey), false,  "Remote public key");
      break;
    }
    case Command::Challenge : {
      memcpy(challengeNonceK, data, 32);
      printBuffer((byte*)data, dataLen, false, "Challenge");
      break;
    }
    case Command::AuthorizationAuthenticator : {
      printBuffer((byte*)data, dataLen, false, "authorizationAuthenticator");
      break;
    }
    case Command::AuthorizationData : {
      printBuffer((byte*)data, dataLen, false, "authorizationData");
      break;
    }
    case Command::AuthorizationId : {
      unsigned char lockId[16];
      printBuffer((byte*)data, dataLen, false, "authorizationId data");
      memcpy(authorizationId, &data[32], 4);
      memcpy(lockId, &data[36], sizeof(lockId));
      memcpy(challengeNonceK, &data[52], sizeof(challengeNonceK));
      printBuffer(authorizationId, sizeof(authorizationId), false, AUTH_ID_STORE_NAME);
      printBuffer(lockId, sizeof(lockId), false, "lockId");
      break;
    }
    case Command::AuthorizationEntry : {
      printBuffer((byte*)data, dataLen, false, "authorizationEntry");
      AuthorizationEntry authEntry;
      memcpy(&authEntry, data, sizeof(authEntry));
      listOfAuthorizationEntries.push_back(authEntry);
      #ifdef DEBUG_NUKI_READABLE_DATA
      NukiLock::logAuthorizationEntry(authEntry);
      #endif
      break;
    }

    case Command::Status : {
      printBuffer((byte*)data, dataLen, false, "status");
      receivedStatus = data[0];
      #ifdef DEBUG_NUKI_COMMUNICATION
      if (receivedStatus == 0) {
        log_d("command COMPLETE");
      } else if (receivedStatus == 1) {
        log_d("command ACCEPTED");
      }
      #endif
      break;
    }
    case Command::OpeningsClosingsSummary : {
      printBuffer((byte*)data, dataLen, false, "openingsClosingsSummary");
      log_w("NOT IMPLEMENTED ONLY FOR NUKI v1"); //command is not available on Nuki v2 (only on Nuki v1)
      break;
    }

    case Command::ErrorReport : {
      log_e("Error: %02x for command: %02x:%02x", data[0], data[2], data[1]);
      memcpy(&errorCode, &data[0], sizeof(errorCode));
      logErrorCode(data[0]);
      break;
    }
    case Command::AuthorizationIdConfirmation : {
      printBuffer((byte*)data, dataLen, false, "authorizationIdConfirmation");
      break;
    }
    case Command::AuthorizationIdInvite : {
      printBuffer((byte*)data, dataLen, false, "authorizationIdInvite");
      break;
    }
    case Command::AuthorizationEntryCount : {
      printBuffer((byte*)data, dataLen, false, "authorizationEntryCount");
      uint16_t count = 0;
      memcpy(&count, data, 2);
      log_d("authorizationEntryCount: %d", count);
      break;
    }
    case Command::LogEntryCount : {
      memcpy(&loggingEnabled, data, sizeof(logEntryCount));
      memcpy(&logEntryCount, &data[1], sizeof(logEntryCount));
      #ifdef DEBUG_NUKI_READABLE_DATA
      log_d("Logging enabled: %d, total nr of log entries: %d", loggingEnabled, logEntryCount);
      #endif
      printBuffer((byte*)data, dataLen, false, "logEntryCount");
      break;
    }
    case Command::TimeControlEntryCount : {
      printBuffer((byte*)data, dataLen, false, "timeControlEntryCount");
      break;
    }
    case Command::KeypadCodeId : {
      printBuffer((byte*)data, dataLen, false, "keypadCodeId");
      break;
    }
    case Command::KeypadCodeCount : {
      memcpy(&nrOfKeypadCodes, data, 2);
      keypadCodeCountReceived = true;
      printBuffer((byte*)data, dataLen, false, "keypadCodeCount");
      #ifdef DEBUG_NUKI_READABLE_DATA
      uint16_t count = 0;
      memcpy(&count, data, 2);
      log_d("keyPadCodeCount: %d", count);
      #endif

      break;
    }
    case Command::KeypadCode : {
      KeypadEntry keypadEntry;
      memcpy(&keypadEntry, data, sizeof(KeypadEntry));
      listOfKeyPadEntries.push_back(keypadEntry);
      nrOfReceivedKeypadCodes++;

      printBuffer((byte*)data, dataLen, false, "keypadCode");
      #ifdef DEBUG_NUKI_READABLE_DATA
      NukiLock::logKeypadEntry(keypadEntry);
      #endif
      break;
    }
    case Command::KeypadAction : {
      printBuffer((byte*)data, dataLen, false, "keypadAction");
      break;
    }
    default:
      log_e("UNKNOWN RETURN COMMAND: %04x", returnCode);
  }
}

void NukiBle::onConnect(BLEClient*) {
  #ifdef DEBUG_NUKI_CONNECT
  log_d("BLE connected");
  #endif
};

void NukiBle::onDisconnect(BLEClient*) {
  #ifdef DEBUG_NUKI_CONNECT
  log_d("BLE disconnected");
  #endif
};

void NukiBle::setEventHandler(SmartlockEventHandler* handler) {
  eventHandler = handler;
}

const bool NukiBle::isPairedWithLock() const {
  return isPaired;
};

bool NukiBle::takeNukiBleSemaphore(std::string taker) {
  bool result = xSemaphoreTake(nukiBleSemaphore, NUKI_SEMAPHORE_TIMEOUT / portTICK_PERIOD_MS) == pdTRUE;

  if (!result) {
    log_d("%s FAILED to take Nuki semaphore. Owner %s", taker.c_str(), owner.c_str());
  } else {
    owner = taker;
  }

  return result;
}

void NukiBle::giveNukiBleSemaphore() {
  owner = "free";
  xSemaphoreGive(nukiBleSemaphore);
}

int NukiBle::getRssi() const {
  return rssi;
}

unsigned long  NukiBle::getLastReceivedBeaconTs() const {
  return lastReceivedBeaconTs;
}

uint32_t NukiBle::getLastHeartbeat() {
  return lastHeartbeat;
}

const BLEAddress NukiBle::getBleAddress() const {
  return bleAddress;
}

} // namespace Nuki
