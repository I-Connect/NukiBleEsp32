/*
 * NukiBle.cpp
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "NukiBle.h"
#include "NukiUtills.h"
#include "string.h"
#include "sodium/crypto_scalarmult.h"
#include "sodium/crypto_core_hsalsa20.h"
#include "sodium/crypto_auth_hmacsha256.h"
#include "sodium/crypto_secretbox.h"
#include "NimBLEBeacon.h"

// #define crypto_secretbox_KEYBYTES 32
#define crypto_box_NONCEBYTES 24
// #define crypto_secretbox_MACBYTES 16
#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

uint8_t receivedStatus;
bool crcCheckOke;

//TODO, these need to move to the class
unsigned char remotePublicKey[32] = {0x00, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char challengeNonceK[32] = {0x00, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char authorizationId[4] = {0x00, 0x00, 0x0, 0x00};
uint16_t pinCode = 0000;
unsigned char lockId[16];
unsigned char secretKeyK[32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char sharedKeyS[32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char sentNonce[crypto_secretbox_NONCEBYTES] = {};

//TODO, these need to move to the class
KeyTurnerState keyTurnerState;
Config config;
AdvancedConfig advancedConfig;
BatteryReport batteryReport;
NukiErrorCode errorCode;
NukiCommand lastMsgCodeReceived = NukiCommand::empty;
uint16_t nrOfKeypadCodes = 0;
uint16_t logEntryCount = 0;
bool loggingEnabled = false;
std::list<LogEntry> listOfLogEntries;
std::list<KeypadEntry> listOfKeyPadEntries;
std::list<AuthorizationEntry> listOfAuthorizationEntries;
std::list<TimeControlEntry> listOfTimeControlEntries;

NukiBle::NukiBle(const std::string& deviceName, const uint32_t deviceId)
  : deviceName(deviceName),
    deviceId(deviceId) {
  keyTurnerUUIDString = STRING(keyturnerServiceUUID);
}

NukiBle::~NukiBle() {}

void NukiBle::initialize() {

  preferences.begin(deviceName.c_str(), false);
  BLEDevice::init("ESP32_test");

  pClient  = BLEDevice::createClient();
  pClient->setClientCallbacks(this);

  bleScanner.initialize(deviceName);
  bleScanner.subscribe(this);

  // TODO
  //disable auto update to prevent unchecked updates in case lock connects with app
  //disable pairing when lock is paired with C-Sense to prevent user to pair with phone?
}

void NukiBle::update() {
  bleScanner.update();
}

bool NukiBle::pairNuki() {
  if (retreiveCredentials()) {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Allready paired");
    #endif
    isPaired = true;
    return true;
  }
  bool result = false;

  if (bleAddress != BLEAddress("") ) {
    if (connectBle(bleAddress)) {
      while (pairStateMachine() == 99) {
        //running pair state machine, it has a timeout
      }
      if (nukiPairingState == NukiPairingState::success) {
        saveCredentials();
        result = true;
      }
    }
  } else {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("No nuki in pairing mode found");
    #endif
  }
  isPaired = result;
  return result;
}

void NukiBle::unPairNuki() {
  // TODO: unpair selected nuki based on deviceName
  #ifdef DEBUG_NUKI_CONNECT
  log_d("[%s] Credentials deleted", deviceName.c_str());
  #endif
  deleteCredentials();
}

bool NukiBle::connectBle(BLEAddress bleAddress) {
  if (!pClient->isConnected()) {
    uint8_t connectRetry = 0;
    while (connectRetry < 5) {
      if (pClient->connect(bleAddress, true)) {
        if (pClient->isConnected() && registerOnGdioChar() && registerOnUsdioChar()) {  //doublecheck if is connected otherwise registiring gdio crashes esp
          return true;
        } else {
          log_w("BLE register on pairing or data Service/Char failed");
        }
      } else {
        log_w("BLE Connect failed");
      }
      connectRetry++;
      esp_task_wdt_reset();
      delay(200);
    }
  } else {
    return true;
  }

  return false;
}

void NukiBle::onResult(BLEAdvertisedDevice* advertisedDevice) {
  if (isPaired) {
    if (bleAddress == advertisedDevice->getAddress()) {
      std::string manufacturerData = advertisedDevice->getManufacturerData();
      uint8_t* manufacturerDataPtr = (uint8_t*)manufacturerData.data();
      char* pHex = BLEUtils::buildHexData(nullptr, manufacturerDataPtr, manufacturerData.length());

      bool isKeyTurnerUUID = true;
      size_t len = keyTurnerUUIDString.length();
      int offset = 0;
      for (int i = 0; i < len; i++) {
        if (keyTurnerUUIDString[i + offset] == '-') {
          ++offset;
          --len;
        }

        if (pHex[i + 8] != keyTurnerUUIDString.at(i + offset)) {
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
          // #ifdef DEBUG_NUKI_CONNECT
          log_d("iBeacon ID: %04X Major: %d Minor: %d UUID: %s Power: %d\n", oBeacon.getManufacturerId(),
                ENDIAN_CHANGE_U16(oBeacon.getMajor()), ENDIAN_CHANGE_U16(oBeacon.getMinor()),
                oBeacon.getProximityUUID().toString().c_str(), oBeacon.getSignalPower());
          // #endif
          if (oBeacon.getSignalPower() == -59) {
            if (eventHandlerSet) {
              eventHandler->notify(NukiEventType::KeyTurnerStatusUpdated);
            }
          }
        }
      }
    }
  } else {
    if (advertisedDevice->haveServiceData()) {
      if (advertisedDevice->getServiceData(NimBLEUUID(STRING(keyturnerPairingServiceUUID))) != "") {
        #ifdef DEBUG_NUKI_CONNECT
        log_d("Found nuki in pairing state: %s addr: %s", std::string(advertisedDevice->getName()).c_str(), std::string(advertisedDevice->getAddress()).c_str());
        #endif
        bleAddress = advertisedDevice->getAddress();
      }
    }
  }
}

uint8_t NukiBle::executeAction(NukiAction action) {
  #ifdef DEBUG_NUKI_CONNECT
  log_d("************************ CHECK PAIRED ************************");
  #endif
  if ( retreiveCredentials() ) {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Credentials retreived from preferences, ready for commands");
    #endif
  } else {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Credentials NOT retreived from preferences, first pair with the lock");
    #endif
    return NukiCmdResult::notPaired;
  }

  #ifdef DEBUG_NUKI_COMMUNICATION
  log_d("Start executing: %02x ", action.command);
  #endif
  if (action.cmdType == NukiCommandType::command) {
    while (1) {
      NukiCmdResult result = cmdStateMachine(action);
      if (result != NukiCmdResult::working) {
        return result;
      }
      esp_task_wdt_reset();
      delay(10);
    }

  } else if (action.cmdType == NukiCommandType::commandWithChallenge) {
    while (1) {
      NukiCmdResult result = cmdChallStateMachine(action);
      if (result != NukiCmdResult::working) {
        return result;
      }
      esp_task_wdt_reset();
      delay(10);
    }
  } else if (action.cmdType == NukiCommandType::commandWithChallengeAndAccept) {
    while (1) {
      NukiCmdResult result = cmdChallAccStateMachine(action);
      if (result != NukiCmdResult::working) {
        return result;
      }
      esp_task_wdt_reset();
      delay(10);
    }
  } else if (action.cmdType == NukiCommandType::commandWithChallengeAndPin) {
    while (1) {
      NukiCmdResult result = cmdChallStateMachine(action, true);
      if (result != NukiCmdResult::working) {
        return result;
      }
      esp_task_wdt_reset();
      delay(10);
    }
  } else {
    log_w("Unknown cmd type");
  }
  return false;
}

NukiBle::NukiCmdResult NukiBle::cmdStateMachine(NukiAction action) {
  switch (nukiCommandState) {
    case NukiCommandState::idle: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING COMMAND ************************");
      #endif
      lastMsgCodeReceived = NukiCommand::empty;
      timeNow = millis();
      sendEncryptedMessage(NukiCommand::requestData, action.payload, action.payloadLen);
      nukiCommandState = NukiCommandState::cmdSent;
      break;
    }
    case NukiCommandState::cmdSent: {
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving command response");
        nukiCommandState = NukiCommandState::idle;
        return NukiCmdResult::timeOut;
      } else if (lastMsgCodeReceived != NukiCommand::errorReport && lastMsgCodeReceived != NukiCommand::empty) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND DONE ************************");
        #endif
        nukiCommandState = NukiCommandState::idle;
        lastMsgCodeReceived = NukiCommand::empty;
        return NukiCmdResult::success;
      } else if (lastMsgCodeReceived == NukiCommand::errorReport) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        nukiCommandState = NukiCommandState::idle;
        lastMsgCodeReceived = NukiCommand::empty;
        return NukiCmdResult::failed;
      }
      break;
    }
    default: {
      log_w("Unknown request command state");
      return NukiCmdResult::failed;
      break;
    }
  }
  return NukiCmdResult::working;
}

NukiBle::NukiCmdResult NukiBle::cmdChallStateMachine(NukiAction action, bool sendPinCode) {
  switch (nukiCommandState) {
    case NukiCommandState::idle: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING CHALLENGE ************************");
      #endif
      lastMsgCodeReceived = NukiCommand::empty;
      timeNow = millis();
      unsigned char payload[sizeof(NukiCommand)] = {0x04, 0x00};  //challenge
      sendEncryptedMessage(NukiCommand::requestData, payload, sizeof(NukiCommand));
      nukiCommandState = NukiCommandState::challengeSent;
      break;
    }
    case NukiCommandState::challengeSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING CHALLENGE RESPONSE************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving challenge response");
        nukiCommandState = NukiCommandState::idle;
        return NukiCmdResult::timeOut;
      } else if (lastMsgCodeReceived == NukiCommand::challenge) {
        log_d("last msg code: %d, compared with: %d", lastMsgCodeReceived, NukiCommand::challenge);
        nukiCommandState = NukiCommandState::challengeRespReceived;
        lastMsgCodeReceived = NukiCommand::empty;
      }
      delay(50);
      break;
    }
    case NukiCommandState::challengeRespReceived: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING COMMAND ************************");
      #endif
      lastMsgCodeReceived = NukiCommand::empty;
      timeNow = millis();
      crcCheckOke = false;
      //add received challenge nonce to payload
      uint8_t payloadLen = action.payloadLen + sizeof(challengeNonceK);
      if (sendPinCode) {
        payloadLen = payloadLen + 2;
      }
      unsigned char payload[payloadLen];
      memcpy(payload, action.payload, action.payloadLen);
      memcpy(&payload[action.payloadLen], challengeNonceK, sizeof(challengeNonceK));
      if (sendPinCode) {
        memcpy(&payload[action.payloadLen + sizeof(challengeNonceK)], &pinCode, 2);
      }
      sendEncryptedMessage(action.command, payload, payloadLen);
      nukiCommandState = NukiCommandState::cmdSent;
      break;
    }
    case NukiCommandState::cmdSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING DATA ************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving data");
        nukiCommandState = NukiCommandState::idle;
        return NukiCmdResult::timeOut;
      } else if (lastMsgCodeReceived == NukiCommand::errorReport) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        nukiCommandState = NukiCommandState::idle;
        lastMsgCodeReceived = NukiCommand::empty;
        return NukiCmdResult::failed;
      } else if (crcCheckOke) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ DATA RECEIVED ************************");
        #endif
        nukiCommandState = NukiCommandState::idle;
        return NukiCmdResult::success;
      }
      delay(50);
      break;
    }
    default:
      log_w("Unknown request command state");
      return NukiCmdResult::failed;
      break;
  }
  return NukiCmdResult::working;
}

NukiBle::NukiCmdResult NukiBle::cmdChallAccStateMachine(NukiAction action) {
  switch (nukiCommandState) {
    case NukiCommandState::idle: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING CHALLENGE ************************");
      #endif
      lastMsgCodeReceived = NukiCommand::empty;
      timeNow = millis();
      unsigned char payload[sizeof(NukiCommand)] = {0x04, 0x00};  //challenge
      sendEncryptedMessage(NukiCommand::requestData, payload, sizeof(NukiCommand));
      nukiCommandState = NukiCommandState::challengeSent;
      break;
    }
    case NukiCommandState::challengeSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING CHALLENGE RESPONSE************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving challenge response");
        nukiCommandState = NukiCommandState::idle;
        return NukiCmdResult::timeOut;
      } else if (lastMsgCodeReceived == NukiCommand::challenge) {
        log_d("last msg code: %d, compared with: %d", lastMsgCodeReceived, NukiCommand::challenge);
        nukiCommandState = NukiCommandState::challengeRespReceived;
        lastMsgCodeReceived = NukiCommand::empty;
      }
      delay(50);
      break;
    }
    case NukiCommandState::challengeRespReceived: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING COMMAND ************************");
      #endif
      lastMsgCodeReceived = NukiCommand::empty;
      timeNow = millis();
      //add received challenge nonce to payload
      uint8_t payloadLen = action.payloadLen + sizeof(challengeNonceK);
      unsigned char payload[payloadLen];
      memcpy(payload, action.payload, action.payloadLen);
      memcpy(&payload[action.payloadLen], challengeNonceK, sizeof(challengeNonceK));
      sendEncryptedMessage(action.command, payload, action.payloadLen + sizeof(challengeNonceK));
      nukiCommandState = NukiCommandState::cmdSent;
      break;
    }
    case NukiCommandState::cmdSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING ACCEPT ************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving accept response");
        nukiCommandState = NukiCommandState::idle;
        return NukiCmdResult::timeOut;
      } else if ((CommandStatus)lastMsgCodeReceived == CommandStatus::accepted) {
        nukiCommandState = NukiCommandState::cmdAccepted;
        lastMsgCodeReceived = NukiCommand::empty;
      }
      delay(50);
      break;
    }
    case NukiCommandState::cmdAccepted: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING COMPLETE ************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving complete response");
        nukiCommandState = NukiCommandState::idle;
        return NukiCmdResult::timeOut;
      } else if (lastMsgCodeReceived == NukiCommand::errorReport) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        nukiCommandState = NukiCommandState::idle;
        lastMsgCodeReceived = NukiCommand::empty;
        return NukiCmdResult::failed;
      } else if ((CommandStatus)lastMsgCodeReceived == CommandStatus::complete) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND SUCCESS ************************");
        #endif
        nukiCommandState = NukiCommandState::idle;
        lastMsgCodeReceived = NukiCommand::empty;
        return NukiCmdResult::success;
      }
      delay(50);
      break;
    }
    default:
      log_w("Unknown request command state");
      return NukiCmdResult::failed;
      break;
  }
  return NukiCmdResult::working;
}

uint8_t NukiBle::requestKeyTurnerState(KeyTurnerState* retreivedKeyTurnerState) {
  NukiAction action;
  uint16_t payload = (uint16_t)NukiCommand::keyturnerStates;

  action.cmdType = NukiCommandType::command;
  action.command = NukiCommand::requestData;
  memcpy(&action.payload[0], &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    memcpy(retreivedKeyTurnerState, &keyTurnerState, sizeof(KeyTurnerState));
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

void NukiBle::retreiveKeyTunerState(KeyTurnerState* retreivedKeyTurnerState) {
  memcpy(retreivedKeyTurnerState, &keyTurnerState, sizeof(KeyTurnerState));
}

bool NukiBle::batteryCritical() {
  //MSB/LSB!
  return keyTurnerState.criticalBatteryState & (1 << 7);
}

bool NukiBle::batteryIsCharging() {
  //MSB/LSB!
  return keyTurnerState.criticalBatteryState & (1 << 6);
}

uint8_t NukiBle::getBatteryPerc() {
  // Note: Bits 2-7 represent the battery load state from 0-100% in steps of two (e.g. 50% is represented as 25).
  //MSB/LSB!

  uint8_t value = keyTurnerState.criticalBatteryState & 0xFC;

  uint8_t result = value & 1; // perc will be reversed bits of value; first get LSB of value
  uint8_t s = sizeof(value);

  for (value >>= 1; value; value >>= 1) {
    result <<= 1;
    result |= value & 1;
    s--;
  }
  return 2 * result;
}

uint8_t NukiBle::requestBatteryReport(BatteryReport* retreivedBatteryReport) {
  NukiAction action;
  uint16_t payload = (uint16_t)NukiCommand::batteryReport;

  action.cmdType = NukiCommandType::command;
  action.command = NukiCommand::requestData;
  memcpy(&action.payload[0], &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  uint8_t result = executeAction(action);
  if ( result == NukiCmdResult::success) {
    memcpy(retreivedBatteryReport, &batteryReport, sizeof(batteryReport));
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::lockAction(LockAction lockAction, uint32_t nukiAppId, uint8_t flags, unsigned char* nameSuffix) {
  NukiAction action;
  unsigned char payload[26] = {0};
  memcpy(payload, &lockAction, sizeof(LockAction));
  memcpy(&payload[sizeof(LockAction)], &nukiAppId, 4);
  memcpy(&payload[sizeof(LockAction) + 4], &flags, 1);
  uint8_t payloadLen = 0;
  if (nameSuffix) {
    memcpy(&payload[sizeof(LockAction) + 4 + 1], &nameSuffix, sizeof(nameSuffix));
    payloadLen = sizeof(LockAction) + 4 + 1 + sizeof(nameSuffix);
  } else {
    payloadLen = sizeof(LockAction) + 4 + 1;
  }

  action.cmdType = NukiCommandType::commandWithChallengeAndAccept;
  action.command = NukiCommand::lockAction;
  memcpy(action.payload, &payload, payloadLen);
  action.payloadLen = payloadLen;

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::retreiveKeypadEntries(uint16_t offset, uint16_t count) {
  NukiAction action;
  unsigned char payload[4] = {0};
  memcpy(payload, &offset, 2);
  memcpy(&payload[2], &count, 2);

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::requestKeypadCodes;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  listOfKeyPadEntries.clear();

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::addKeypadEntry(NewKeypadEntry newKeypadEntry) {
  //TODO verify data validity
  NukiAction action;
  unsigned char payload[sizeof(NewKeypadEntry)] = {0};
  memcpy(payload, &newKeypadEntry, sizeof(NewKeypadEntry));

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::addKeypadCode;
  memcpy(action.payload, &payload, sizeof(NewKeypadEntry));
  action.payloadLen = sizeof(NewKeypadEntry);

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addKeyPadEntry, payloadlen: %d", sizeof(NewKeypadEntry));
    printBuffer(action.payload, sizeof(NewKeypadEntry), false, "addKeyPadCode content: ");
    logNewKeypadEntry(newKeypadEntry);
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::updateKeypadEntry(UpdatedKeypadEntry updatedKeyPadEntry) {
  //TODO verify data validity
  NukiAction action;
  unsigned char payload[sizeof(UpdatedKeypadEntry)] = {0};
  memcpy(payload, &updatedKeyPadEntry, sizeof(UpdatedKeypadEntry));

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::updateKeypadCode;
  memcpy(action.payload, &payload, sizeof(UpdatedKeypadEntry));
  action.payloadLen = sizeof(UpdatedKeypadEntry);

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addKeyPadEntry, payloadlen: %d", sizeof(UpdatedKeypadEntry));
    printBuffer(action.payload, sizeof(UpdatedKeypadEntry), false, "updatedKeypad content: ");
    logUpdatedKeypadEntry(updatedKeyPadEntry);
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

void NukiBle::getKeypadEntries(std::list<KeypadEntry>* requestedKeypadCodes) {
  requestedKeypadCodes->clear();
  std::list<KeypadEntry>::iterator it = listOfKeyPadEntries.begin();
  while (it != listOfKeyPadEntries.end()) {
    requestedKeypadCodes->push_back(*it);
    it++;
  }
}

uint8_t NukiBle::retreiveAuthorizationEntries(uint16_t offset, uint16_t count) {
  NukiAction action;
  unsigned char payload[4] = {0};
  memcpy(payload, &offset, 2);
  memcpy(&payload[2], &count, 2);

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::requestAuthorizationEntries;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  listOfAuthorizationEntries.clear();

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

void NukiBle::getAuthorizationEntries(std::list<AuthorizationEntry>* requestedAuthorizationEntries) {
  requestedAuthorizationEntries->clear();
  std::list<AuthorizationEntry>::iterator it = listOfAuthorizationEntries.begin();
  while (it != listOfAuthorizationEntries.end()) {
    requestedAuthorizationEntries->push_back(*it);
    it++;
  }
}

uint8_t NukiBle::addAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry) {
  //TODO verify data validity
  NukiAction action;
  unsigned char payload[sizeof(NewAuthorizationEntry)] = {0};
  memcpy(payload, &newAuthorizationEntry, sizeof(NewAuthorizationEntry));

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::authorizationDatInvite;
  memcpy(action.payload, &payload, sizeof(NewAuthorizationEntry));
  action.payloadLen = sizeof(NewAuthorizationEntry);

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addAuthorizationEntry, payloadlen: %d", sizeof(NewAuthorizationEntry));
    printBuffer(action.payload, sizeof(NewAuthorizationEntry), false, "addAuthorizationEntry content: ");
    logNewAuthorizationEntry(newAuthorizationEntry);
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::updateAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry) {
  //TODO verify data validity
  NukiAction action;
  unsigned char payload[sizeof(UpdatedAuthorizationEntry)] = {0};
  memcpy(payload, &updatedAuthorizationEntry, sizeof(UpdatedAuthorizationEntry));

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::updateAuthorization;
  memcpy(action.payload, &payload, sizeof(UpdatedAuthorizationEntry));
  action.payloadLen = sizeof(UpdatedAuthorizationEntry);

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addAuthorizationEntry, payloadlen: %d", sizeof(UpdatedAuthorizationEntry));
    printBuffer(action.payload, sizeof(UpdatedAuthorizationEntry), false, "updatedKeypad content: ");
    logUpdatedAuthorizationEntry(updatedAuthorizationEntry);
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::retreiveLogEntries(uint32_t startIndex, uint16_t count, uint8_t sortOrder, bool totalCount) {
  NukiAction action;
  unsigned char payload[8] = {0};
  memcpy(payload, &startIndex, 4);
  memcpy(&payload[4], &count, 2);
  memcpy(&payload[6], &sortOrder, 1);
  memcpy(&payload[7], &totalCount, 1);

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::requestLogEntries;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  listOfLogEntries.clear();

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

void NukiBle::getLogEntries(std::list<LogEntry>* requestedLogEntries) {
  requestedLogEntries->clear();
  std::list<LogEntry>::iterator it = listOfLogEntries.begin();
  while (it != listOfLogEntries.end()) {
    requestedLogEntries->push_back(*it);
    it++;
  }
}

uint8_t NukiBle::requestConfig(Config* retreivedConfig) {
  NukiAction action;

  action.cmdType = NukiCommandType::commandWithChallenge;
  action.command = NukiCommand::requestConfig;

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    memcpy(retreivedConfig, &config, sizeof(Config));
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::requestAdvancedConfig(AdvancedConfig* retreivedAdvancedConfig) {
  NukiAction action;

  action.cmdType = NukiCommandType::commandWithChallenge;
  action.command = NukiCommand::requestAdvancedConfig;

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    memcpy(retreivedAdvancedConfig, &advancedConfig, sizeof(AdvancedConfig));
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::setConfig(NewConfig newConfig) {
  NukiAction action;
  unsigned char payload[sizeof(NewConfig)] = {0};
  memcpy(payload, &newConfig, sizeof(NewConfig));

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::setConfig;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::setAdvancedConfig(NewAdvancedConfig newAdvancedConfig) {
  NukiAction action;
  unsigned char payload[sizeof(NewAdvancedConfig)] = {0};
  memcpy(payload, &newAdvancedConfig, sizeof(NewAdvancedConfig));

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::setAdvancedConfig;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::addTimeControlEntry(NewTimeControlEntry newTimeControlEntry) {
//TODO verify data validity
  NukiAction action;
  unsigned char payload[sizeof(NewTimeControlEntry)] = {0};
  memcpy(payload, &newTimeControlEntry, sizeof(NewTimeControlEntry));

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::addTimeControlEntry;
  memcpy(action.payload, &payload, sizeof(NewTimeControlEntry));
  action.payloadLen = sizeof(NewTimeControlEntry);

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addTimeControlEntry, payloadlen: %d", sizeof(NewTimeControlEntry));
    printBuffer(action.payload, sizeof(NewTimeControlEntry), false, "new time control content: ");
    logNewTimeControlEntry(newTimeControlEntry);
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::updateTimeControlEntry(TimeControlEntry TimeControlEntry) {
  //TODO verify data validity
  NukiAction action;
  unsigned char payload[sizeof(TimeControlEntry)] = {0};
  memcpy(payload, &TimeControlEntry, sizeof(TimeControlEntry));

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::updateTimeControlEntry;
  memcpy(action.payload, &payload, sizeof(TimeControlEntry));
  action.payloadLen = sizeof(TimeControlEntry);

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addTimeControlEntry, payloadlen: %d", sizeof(TimeControlEntry));
    printBuffer(action.payload, sizeof(TimeControlEntry), false, "updated time control content: ");
    logTimeControlEntry(TimeControlEntry);
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::removeTimeControlEntry(uint8_t entryId) {
//TODO verify data validity
  NukiAction action;
  unsigned char payload[1] = {0};
  memcpy(payload, &entryId, 1);

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::removeTimeControlEntry;
  memcpy(action.payload, &payload, 1);
  action.payloadLen = 1;

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Removed time control id:%d", entryId);
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::retreiveTimeControlEntries() {
  NukiAction action;

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::requestTimeControlEntries;
  action.payloadLen = 0;

  listOfTimeControlEntries.clear();

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Retreive time control entries success");
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

void NukiBle::getTimeControlEntries(std::list<TimeControlEntry>* requestedTimeControlEntries) {
  requestedTimeControlEntries->clear();
  std::list<TimeControlEntry>::iterator it = listOfTimeControlEntries.begin();
  while (it != listOfTimeControlEntries.end()) {
    requestedTimeControlEntries->push_back(*it);
    it++;
  }
}

uint8_t NukiBle::setSecurityPin(uint16_t newSecurityPin) {
  NukiAction action;
  unsigned char payload[2] = {0};
  memcpy(payload, &newSecurityPin, 2);

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::setSecurityPin;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    pinCode = newSecurityPin;
    saveCredentials();
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::verifySecurityPin() {
  NukiAction action;

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::verifySecurityPin;
  action.payloadLen = 0;

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Verify security pin code success");
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::requestCalibration() {
  NukiAction action;

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::requestCalibration;
  action.payloadLen = 0;

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Calibration executed");
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::requestReboot() {
  NukiAction action;

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::requestReboot;
  action.payloadLen = 0;

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Reboot executed");
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

uint8_t NukiBle::updateTime(TimeValue time) {
  NukiAction action;
  unsigned char payload[sizeof(TimeValue)] = {0};
  memcpy(payload, &time, sizeof(TimeValue));

  action.cmdType = NukiCommandType::commandWithChallengeAndPin;
  action.command = NukiCommand::updateTime;
  memcpy(action.payload, &payload, sizeof(TimeValue));
  action.payloadLen = sizeof(TimeValue);

  uint8_t result = executeAction(action);
  if (result == NukiCmdResult::success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Time set: %d-%d-%d %d:%d:%d", time.year, time.month, time.day, time.hour, time.minute, time.second);
    #endif
    return true;
  } else if (result == NukiCmdResult::timeOut) {
    return NukiCmdResult::timeOut;
  }
  return uint8_t(errorCode);
}

void NukiBle::createNewConfig(Config* oldConfig, NewConfig* newConfig) {
  memcpy(newConfig->name, oldConfig->name, sizeof(newConfig->name));
  newConfig->latitide = oldConfig->latitide;
  newConfig->longitude = oldConfig->longitude;
  newConfig->autoUnlatch = oldConfig->autoUnlatch;
  newConfig->pairingEnabled = oldConfig->pairingEnabled;
  newConfig->buttonEnabled = oldConfig->buttonEnabled;
  newConfig->ledEnabled = oldConfig->ledEnabled;
  newConfig->ledBrightness = oldConfig->ledBrightness;
  newConfig->timeZoneOffset = oldConfig->timeZoneOffset;
  newConfig->dstMode = oldConfig->dstMode;
  newConfig->fobAction1 = oldConfig->fobAction1;
  newConfig->fobAction2 = oldConfig->fobAction2;
  newConfig->fobAction3 = oldConfig->fobAction3;
  newConfig->singleLock = oldConfig->singleLock;
  newConfig->advertisingMode = oldConfig->advertisingMode;
  newConfig->timeZoneId = oldConfig->timeZoneId;
}

//basic config change methods
uint8_t NukiBle::setName(std::string name) {
  Config oldConfig;
  NewConfig newConfig;
  uint8_t result;

  if (name.length() <= 32) {
    result = requestConfig(&oldConfig);
    if (result == 1) {
      memcpy(oldConfig.name, name.c_str(), name.length());
      createNewConfig(&oldConfig, &newConfig);
      result = setConfig(newConfig);
      if ( result == 1) {
        return true;
      } else {
        return result;
      }
    } else {
      return result;
    }
  } else {
    log_w("setName, too long (max32)");
    return false;
  }
}

uint8_t NukiBle::enablePairing(bool enable) {
  Config oldConfig;
  NewConfig newConfig;
  uint8_t result;

  result = requestConfig(&oldConfig);
  if (result == 1) {
    oldConfig.pairingEnabled = enable;
    createNewConfig(&oldConfig, &newConfig);
    result = setConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::enableButton(bool enable) {
  Config oldConfig;
  NewConfig newConfig;
  uint8_t result;

  result = requestConfig(&oldConfig);
  if (result == 1) {
    oldConfig.buttonEnabled = enable;
    createNewConfig(&oldConfig, &newConfig);
    result = setConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::enableLedFlash(bool enable) {
  Config oldConfig;
  NewConfig newConfig;
  uint8_t result;

  result = requestConfig(&oldConfig);
  if (result == 1) {
    oldConfig.ledEnabled = enable;
    createNewConfig(&oldConfig, &newConfig);
    result = setConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::setLedBrightness(uint8_t level) {
  //level is from 0 (off) to 5(max)
  Config oldConfig;
  NewConfig newConfig;
  uint8_t result;

  if (level > 5) {
    level = 5;
  }

  result = requestConfig(&oldConfig);
  if (result == 1) {
    oldConfig.ledBrightness = level;
    createNewConfig(&oldConfig, &newConfig);
    result = setConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::enableSingleLock(bool enable) {
  Config oldConfig;
  NewConfig newConfig;
  uint8_t result;

  result = requestConfig(&oldConfig);
  if (result == 1) {
    oldConfig.singleLock = enable;
    createNewConfig(&oldConfig, &newConfig);
    result = setConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::setAdvertisingMode(AdvertisingMode mode) {
  Config oldConfig;
  NewConfig newConfig;
  uint8_t result;

  result = requestConfig(&oldConfig);
  if (result == 1) {
    oldConfig.advertisingMode = mode;
    createNewConfig(&oldConfig, &newConfig);
    result = setConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::enableDst(bool enable) {
  Config oldConfig;
  NewConfig newConfig;
  uint8_t result;

  result = requestConfig(&oldConfig);
  if (result == 1) {
    oldConfig.dstMode = enable;
    createNewConfig(&oldConfig, &newConfig);
    result = setConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::setTimeZoneOffset(int16_t minutes) {
  Config oldConfig;
  NewConfig newConfig;
  uint8_t result;

  result = requestConfig(&oldConfig);
  if (result == 1) {
    oldConfig.timeZoneOffset = minutes;
    createNewConfig(&oldConfig, &newConfig);
    result = setConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::setTimeZoneId(TimeZoneId timeZoneId) {
  Config oldConfig;
  NewConfig newConfig;
  uint8_t result;

  result = requestConfig(&oldConfig);
  if (result == 1) {
    oldConfig.timeZoneId = timeZoneId;
    createNewConfig(&oldConfig, &newConfig);
    result = setConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

void NukiBle::createNewAdvancedConfig(AdvancedConfig* oldConfig, NewAdvancedConfig* newConfig) {
  newConfig->unlockedPositionOffsetDegrees = oldConfig->unlockedPositionOffsetDegrees;
  newConfig->lockedPositionOffsetDegrees = oldConfig->lockedPositionOffsetDegrees;
  newConfig->singleLockedPositionOffsetDegrees = oldConfig->singleLockedPositionOffsetDegrees;
  newConfig->unlockedToLockedTransitionOffsetDegrees = oldConfig->unlockedToLockedTransitionOffsetDegrees;
  newConfig->lockNgoTimeout = oldConfig->lockNgoTimeout;
  newConfig->singleButtonPressAction = oldConfig->singleButtonPressAction;
  newConfig->doubleButtonPressAction = oldConfig->doubleButtonPressAction;
  newConfig->detachedCylinder = oldConfig->detachedCylinder;
  newConfig->batteryType = oldConfig->batteryType;
  newConfig->automaticBatteryTypeDetection = oldConfig->automaticBatteryTypeDetection;
  newConfig->unlatchDuration = oldConfig->unlatchDuration;
  newConfig->autoLockTimeOut = oldConfig->autoLockTimeOut;
  newConfig->autoUnLockDisabled = oldConfig->autoUnLockDisabled;
  newConfig->nightModeEnabled = oldConfig->nightModeEnabled;
  memcpy(newConfig->nightModeStartTime, oldConfig->nightModeStartTime, sizeof(newConfig->nightModeStartTime));
  memcpy(newConfig->nightModeEndTime, oldConfig->nightModeEndTime, sizeof(newConfig->nightModeEndTime));
  newConfig->nightModeAutoLockEnabled = oldConfig->nightModeAutoLockEnabled;
  newConfig->nightModeAutoUnlockDisabled = oldConfig->nightModeAutoUnlockDisabled;
  newConfig->nightModeImmediateLockOnStart = oldConfig->nightModeImmediateLockOnStart;
  newConfig->autoLockEnabled = oldConfig->autoLockEnabled;
  newConfig->immediateAutoLockEnabled = oldConfig->immediateAutoLockEnabled;
  newConfig->autoUpdateEnabled = oldConfig->autoUpdateEnabled;

}

//advanced config change methods
uint8_t NukiBle::setSingleButtonPressAction(ButtonPressAction action) {
  AdvancedConfig oldConfig;
  NewAdvancedConfig newConfig;
  uint8_t result;

  result = requestAdvancedConfig(&oldConfig);
  if (result == 1) {
    oldConfig.singleButtonPressAction = action;
    createNewAdvancedConfig(&oldConfig, &newConfig);
    result = 1;
    result = setAdvancedConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::setDoubleButtonPressAction(ButtonPressAction action) {
  AdvancedConfig oldConfig;
  NewAdvancedConfig newConfig;
  uint8_t result;

  result = requestAdvancedConfig(&oldConfig);
  if (result == 1) {
    oldConfig.doubleButtonPressAction = action;
    createNewAdvancedConfig(&oldConfig, &newConfig);
    result = setAdvancedConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::setBatteryType(BatteryType type) {
  AdvancedConfig oldConfig;
  NewAdvancedConfig newConfig;
  uint8_t result;

  result = requestAdvancedConfig(&oldConfig);
  if (result == 1) {
    oldConfig.batteryType = type;
    createNewAdvancedConfig(&oldConfig, &newConfig);
    result = setAdvancedConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::enableAutoBatteryTypeDetection(bool enable) {
  AdvancedConfig oldConfig;
  NewAdvancedConfig newConfig;
  uint8_t result;

  result = requestAdvancedConfig(&oldConfig);
  if (result == 1) {
    oldConfig.automaticBatteryTypeDetection = enable;
    createNewAdvancedConfig(&oldConfig, &newConfig);
    result = setAdvancedConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::disableAutoUnlock(bool disable) {
  AdvancedConfig oldConfig;
  NewAdvancedConfig newConfig;
  uint8_t result;

  result = requestAdvancedConfig(&oldConfig);
  if (result == 1) {
    oldConfig.autoUnLockDisabled = disable;
    createNewAdvancedConfig(&oldConfig, &newConfig);
    result = setAdvancedConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::enableAutoLock(bool enable) {
  AdvancedConfig oldConfig;
  NewAdvancedConfig newConfig;
  uint8_t result;

  result = requestAdvancedConfig(&oldConfig);
  if (result == 1) {
    oldConfig.autoLockEnabled = enable;
    createNewAdvancedConfig(&oldConfig, &newConfig);
    result = setAdvancedConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::enableImmediateAutoLock(bool enable) {
  AdvancedConfig oldConfig;
  NewAdvancedConfig newConfig;
  uint8_t result;

  result = requestAdvancedConfig(&oldConfig);
  if (result == 1) {
    oldConfig.immediateAutoLockEnabled = enable;
    createNewAdvancedConfig(&oldConfig, &newConfig);
    result = setAdvancedConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

uint8_t NukiBle::enableAutoUpdate(bool enable) {
  AdvancedConfig oldConfig;
  NewAdvancedConfig newConfig;
  uint8_t result;

  result = requestAdvancedConfig(&oldConfig);
  if (result == 1) {
    oldConfig.autoUpdateEnabled = enable;
    createNewAdvancedConfig(&oldConfig, &newConfig);
    result = setAdvancedConfig(newConfig);
    if ( result == 1) {
      return true;
    } else {
      return result;
    }
  } else {
    return result;
  }
}

bool NukiBle::savePincode(uint16_t pinCode) {
  return (preferences.putBytes("securityPinCode", &pinCode, 2) == 2);
}

void NukiBle::saveCredentials() {
  unsigned char buff[6];
  buff[0] = bleAddress.getNative()[5];
  buff[1] = bleAddress.getNative()[4];
  buff[2] = bleAddress.getNative()[3];
  buff[3] = bleAddress.getNative()[2];
  buff[4] = bleAddress.getNative()[1];
  buff[5] = bleAddress.getNative()[0];

  if ( (preferences.putBytes("secretKeyK", secretKeyK, 32) == 32 )
       && ( preferences.putBytes("bleAddress", buff, 6) == 6 )
       && ( preferences.putBytes("authorizationId", authorizationId, 4) == 4 )
       && preferences.putBytes("securityPinCode", &pinCode, 2) == 2) {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Credentials saved:");
    printBuffer(secretKeyK, sizeof(secretKeyK), false, "secretKeyK");
    printBuffer(buff, 6, false, "bleAddress");
    printBuffer(authorizationId, sizeof(authorizationId), false, "authorizationId");
    log_d("pincode: %d", pinCode);
    #endif
  } else {
    log_w("ERROR saving credentials");
  }
}

bool NukiBle::retreiveCredentials() {
  //TODO check on empty (invalid) credentials?
  unsigned char buff[6];
  if ( (preferences.getBytes("secretKeyK", secretKeyK, 32) > 0)
       && (preferences.getBytes("bleAddress", buff, 6) > 0)
       && (preferences.getBytes("authorizationId", authorizationId, 4) > 0)
       && (preferences.getBytes("securityPinCode", &pinCode, 2) > 0) ) {
    bleAddress = BLEAddress(buff);
    #ifdef DEBUG_NUKI_CONNECT
    log_d("[%s] Credentials retreived :", deviceName.c_str());
    printBuffer(secretKeyK, sizeof(secretKeyK), false, "secretKeyK");
    log_d("bleAddress: %s", bleAddress.toString().c_str());
    printBuffer(authorizationId, sizeof(authorizationId), false, "authorizationId");
    log_d("PinCode: %d", pinCode);
    #endif

  } else {
    log_w("ERROR retreiving credentials");
    return false;
  }
  return true;
}

void NukiBle::deleteCredentials() {
  preferences.remove("secretKeyK");
  preferences.remove("bleAddress");
  preferences.remove("authorizationId");
  #ifdef DEBUG_NUKI_CONNECT
  log_d("Credentials deleted");
  #endif
}

uint8_t NukiBle::pairStateMachine() {
  switch (nukiPairingState) {
    case NukiPairingState::initPairing: {

      memset(challengeNonceK, 0, sizeof(challengeNonceK));
      memset(remotePublicKey, 0, sizeof(remotePublicKey));
      receivedStatus = 0xff;
      nukiPairingState = NukiPairingState::reqRemPubKey;
      break;
    }
    case NukiPairingState::reqRemPubKey: {
      //Request remote public key (Sent message should be 0100030027A7)
      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### REQUEST REMOTE PUBLIC KEY #########################");
      #endif
      unsigned char buff[sizeof(NukiCommand)];
      uint16_t cmd = (uint16_t)NukiCommand::publicKey;
      memcpy(buff, &cmd, sizeof(NukiCommand));
      sendPlainMessage(NukiCommand::requestData, buff, sizeof(NukiCommand));
      timeNow = millis();
      nukiPairingState = NukiPairingState::recRemPubKey;
      break;
    }
    case NukiPairingState::recRemPubKey: {
      if (checkCharArrayEmpty(remotePublicKey, sizeof(remotePublicKey))) {
        nukiPairingState = NukiPairingState::sendPubKey;
      } else if (millis() - timeNow > GENERAL_TIMEOUT) {
        log_w("Remote public key receive timeout");
        return false;
      }
      delay(50);
      break;
    }
    case NukiPairingState::sendPubKey: {
      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### SEND CLIENT PUBLIC KEY #########################");
      #endif
      //TODO generate public and private keys?
      sendPlainMessage(NukiCommand::publicKey, myPublicKey, sizeof(myPublicKey));
      nukiPairingState = NukiPairingState::genKeyPair;
      break;
    }
    case NukiPairingState::genKeyPair: {
      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### CALCULATE DH SHARED KEY s #########################");
      #endif
      crypto_scalarmult_curve25519(sharedKeyS, myPrivateKey, remotePublicKey);
      printBuffer(sharedKeyS, sizeof(sharedKeyS), false, "Shared key s");

      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### DERIVE LONG TERM SHARED SECRET KEY k #########################");
      #endif
      unsigned char _0[16];
      memset(_0, 0, 16);
      unsigned char sigma[] = "expand 32-byte k";
      crypto_core_hsalsa20(secretKeyK, _0, sharedKeyS, sigma);
      printBuffer(secretKeyK, sizeof(secretKeyK), false, "Secret key k");
      timeNow = millis();
      nukiPairingState = NukiPairingState::calculateAuth;
      break;
    }
    case NukiPairingState::calculateAuth: {
      if (checkCharArrayEmpty(challengeNonceK, sizeof(challengeNonceK))) {
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
        nukiPairingState = NukiPairingState::sendAuth;
      } else if (millis() - timeNow > GENERAL_TIMEOUT) {
        log_w("Challenge 1 receive timeout");
        return false;
      }
      delay(50);
      break;
    }
    case NukiPairingState::sendAuth: {
      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### SEND AUTHENTICATOR #########################");
      #endif
      sendPlainMessage(NukiCommand::authorizationAuthenticator, authenticator, sizeof(authenticator));
      timeNow = millis();
      nukiPairingState = NukiPairingState::sendAuthData;
      break;
    }
    case NukiPairingState::sendAuthData: {
      if (checkCharArrayEmpty(challengeNonceK, sizeof(challengeNonceK))) {
        #ifdef DEBUG_NUKI_CONNECT
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
        sendPlainMessage(NukiCommand::authorizationData, authorizationDataMessage, sizeof(authorizationDataMessage));
        timeNow = millis();
        nukiPairingState = NukiPairingState::sendAuthIdConf;
      } else if (millis() - timeNow > GENERAL_TIMEOUT) {
        log_w("Challenge 2 receive timeout");
        return false;
      }
      delay(50);
      break;
    }
    case NukiPairingState::sendAuthIdConf: {
      if (checkCharArrayEmpty(authorizationId, sizeof(authorizationId))) {
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
        sendPlainMessage(NukiCommand::authorizationIdConfirmation, confirmationDataMessage, sizeof(confirmationDataMessage));
        timeNow = millis();
        nukiPairingState = NukiPairingState::recStatus;
      } else if (millis() - timeNow > GENERAL_TIMEOUT) {
        log_w("Authorization id receive timeout");
        return false;
      }
      delay(50);
      break;
    }
    case NukiPairingState::recStatus: {
      if (receivedStatus == 0) {
        #ifdef DEBUG_NUKI_CONNECT
        log_d("####################### PAIRING DONE ###############################################");
        #endif
        nukiPairingState = NukiPairingState::success;
        return true;
      } else if (millis() - timeNow > GENERAL_TIMEOUT) {
        log_w("pairing FAILED");
        return false;
      }
      delay(50);
      break;
    }
    default: {
      log_e("Unknown pairing status");
      break;
    }
  }

  if (millis() - timeNow > PAIRING_TIMEOUT) {
    log_w("Pairing timeout");
    return 0;
  }
  return 99;
}

void NukiBle::sendEncryptedMessage(NukiCommand commandIdentifier, unsigned char* payload, uint8_t payloadLen) {
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
    memcpy(&additionalData[28], &length, 2 );

    printBuffer((byte*)additionalData, 30, false, "Additional data: ");
    printBuffer((byte*)secretKeyK, sizeof(secretKeyK), false, "Encryption key (secretKey): ");
    printBuffer((byte*)plainDataEncr, sizeof(plainDataEncr), false, "Plain data encrypted: ");

    //compose complete message
    unsigned char dataToSend[sizeof(additionalData) + sizeof(plainDataEncr)] = {};
    memcpy(&dataToSend[0], additionalData, sizeof(additionalData));
    memcpy(&dataToSend[30], plainDataEncr, sizeof(plainDataEncr));

    printBuffer((byte*)dataToSend, sizeof(dataToSend), false, "Sending encrypted message");

    if (connectBle(bleAddress)) {
      pUsdioCharacteristic->writeValue((uint8_t*)dataToSend, sizeof(dataToSend), true);
    } else {
      log_w("Send encr msg failed due to unable to connect");
    }
  } else {
    log_w("Send msg failed due to encryption fail");
  }

}

void NukiBle::sendPlainMessage(NukiCommand commandIdentifier, unsigned char* payload, uint8_t payloadLen) {
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
  #ifdef DEBUG_NUKI_HEX_DATA
  log_d("Command identifier: %02x, CRC: %04x", (uint32_t)commandIdentifier, dataCrc);
  #endif

  if (connectBle(bleAddress)) {
    pGdioCharacteristic->writeValue((uint8_t*)dataToSend, payloadLen + 4, true);
  } else {
    log_w("Send plain msg failed due to unable to connect");
  }
}

bool NukiBle::registerOnGdioChar() {
  // Obtain a reference to the KeyTurner Pairing service
  pKeyturnerPairingService = pClient->getService(STRING(keyturnerPairingServiceUUID));
  if (pKeyturnerPairingService != nullptr) {
    //Obtain reference to GDIO char
    pGdioCharacteristic = pKeyturnerPairingService->getCharacteristic(STRING(keyturnerGdioUUID));
    if (pGdioCharacteristic != nullptr) {
      if (pGdioCharacteristic->canIndicate()) {
        pGdioCharacteristic->registerForNotify(notifyCallback, false); //false = indication, true = notification
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
  pKeyturnerDataService = pClient->getService(STRING(keyturnerServiceUUID));
  if (pKeyturnerDataService != nullptr) {
    //Obtain reference to NDIO char
    pUsdioCharacteristic = pKeyturnerDataService->getCharacteristic(STRING(userDataUUID));
    if (pUsdioCharacteristic != nullptr) {
      if (pUsdioCharacteristic->canIndicate()) {
        pUsdioCharacteristic->registerForNotify(notifyCallback, false); //false = indication, true = notification
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

  std::string gdioUuid = STRING(keyturnerGdioUUID);
  std::string udioUuid = STRING(userDataUUID);

  if (pBLERemoteCharacteristic->getUUID().toString() ==  gdioUuid) {
    //handle not encrypted msg
    uint16_t returnCode = ((uint16_t)recData[1] << 8) | recData[0];
    if (crcValid(recData, length)) {
      unsigned char plainData[200];
      memcpy(plainData, &recData[2], length - 4);
      handleReturnMessage((NukiCommand)returnCode, plainData, length - 4);
    }
  } else if (pBLERemoteCharacteristic->getUUID().toString() == udioUuid) {
    //handle encrypted msg
    unsigned char recNonce[crypto_secretbox_NONCEBYTES];
    unsigned char recAuthorizationId[4];
    unsigned char recMsgLen[2];
    memcpy(recNonce, &recData[0], 24);
    memcpy(recAuthorizationId, &recData[24], 4);
    memcpy(recMsgLen, &recData[28], 2);
    uint16_t encrMsgLen = 0;
    memcpy(&encrMsgLen, recMsgLen, 2);
    unsigned char encrData[encrMsgLen];
    memcpy(&encrData, &recData[30], encrMsgLen);

    unsigned char decrData[encrMsgLen - crypto_secretbox_MACBYTES];
    decode(decrData, encrData, encrMsgLen, recNonce, secretKeyK);

    #ifdef DEBUG_NUKI_COMMUNICATION
    log_d("Received encrypted msg, len: %d", encrMsgLen);
    #endif
    printBuffer(recNonce, sizeof(recNonce), false, "received nonce");
    printBuffer(recAuthorizationId, sizeof(recAuthorizationId), false, "Received AuthorizationId");
    printBuffer(encrData, sizeof(encrData), false, "Rec encrypted data");
    printBuffer(decrData, sizeof(decrData), false, "Decrypted data");

    if (crcValid(decrData, sizeof(decrData))) {
      uint16_t returnCode = 0;
      memcpy(&returnCode, &decrData[4], 2);
      unsigned char payload[sizeof(decrData) - 8];
      memcpy(&payload, &decrData[6], sizeof(payload));
      handleReturnMessage((NukiCommand)returnCode, payload, sizeof(payload));
    }
  }
}

void NukiBle::handleReturnMessage(NukiCommand returnCode, unsigned char* data, uint16_t dataLen) {
  lastMsgCodeReceived = returnCode;

  switch (returnCode) {
    case NukiCommand::requestData : {
      log_d("requestData");
      break;
    }
    case NukiCommand::publicKey : {
      memcpy(remotePublicKey, data, 32);
      printBuffer(remotePublicKey, sizeof(remotePublicKey), false,  "Remote public key");
      break;
    }
    case NukiCommand::challenge : {
      memcpy(challengeNonceK, data, 32);
      printBuffer((byte*)data, dataLen, false, "Challenge");
      break;
    }
    case NukiCommand::authorizationAuthenticator : {
      printBuffer((byte*)data, dataLen, false, "authorizationAuthenticator");
      break;
    }
    case NukiCommand::authorizationData : {
      printBuffer((byte*)data, dataLen, false, "authorizationData");
      break;
    }
    case NukiCommand::authorizationId : {
      printBuffer((byte*)data, dataLen, false, "authorizationId data");
      memcpy(authorizationId, &data[32], 4);
      memcpy(lockId, &data[36], sizeof(lockId));
      memcpy(challengeNonceK, &data[52], sizeof(challengeNonceK));
      printBuffer(authorizationId, sizeof(authorizationId), false, "authorizationId");
      printBuffer(lockId, sizeof(lockId), false, "lockId");
      break;
    }
    case NukiCommand::authorizationEntry : {
      printBuffer((byte*)data, dataLen, false, "authorizationEntry");
      AuthorizationEntry authEntry;
      memcpy(&authEntry, data, sizeof(authEntry));
      listOfAuthorizationEntries.push_back(authEntry);
      #ifdef DEBUG_NUKI_READABLE_DATA
      logAuthorizationEntry(authEntry);
      #endif
      break;
    }
    case NukiCommand::keyturnerStates : {
      printBuffer((byte*)data, dataLen, false, "keyturnerStates");
      memcpy(&keyTurnerState, data, sizeof(keyTurnerState));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logKeyturnerState(keyTurnerState);
      #endif
      break;
    }
    case NukiCommand::status : {
      printBuffer((byte*)data, dataLen, false, "status");
      receivedStatus = data[0];
      #ifdef DEBUG_NUKI_READABLE_DATA
      if (receivedStatus == 0) {
        log_d("command COMPLETE");
      } else if (receivedStatus == 1) {
        log_d("command ACCEPTED");
      }
      #endif
      break;
    }
    case NukiCommand::openingsClosingsSummary : {
      printBuffer((byte*)data, dataLen, false, "openingsClosingsSummary");
      log_w("NOT IMPLEMENTED ONLY FOR NUKI v1"); //command is not available on Nuki v2 (only on Nuki v1)
      break;
    }
    case NukiCommand::batteryReport : {
      printBuffer((byte*)data, dataLen, false, "batteryReport");
      memcpy(&batteryReport, data, sizeof(batteryReport));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logBatteryReport(batteryReport);
      #endif
      break;
    }
    case NukiCommand::errorReport : {
      log_e("Error: %02x for command: %02x:%02x", data[0], data[2], data[1]);
      memcpy(&errorCode, &data[0], sizeof(errorCode));
      logErrorCode(data[0]);
      break;
    }
    case NukiCommand::config : {
      memcpy(&config, data, sizeof(config));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logConfig(config);
      #endif
      printBuffer((byte*)data, dataLen, false, "config");
      break;
    }
    case NukiCommand::authorizationIdConfirmation : {
      printBuffer((byte*)data, dataLen, false, "authorizationIdConfirmation");
      break;
    }
    case NukiCommand::authorizationIdInvite : {
      printBuffer((byte*)data, dataLen, false, "authorizationIdInvite");
      break;
    }
    case NukiCommand::authorizationEntryCount : {
      printBuffer((byte*)data, dataLen, false, "authorizationEntryCount");
      uint16_t count = 0;
      memcpy(&count, data, 2);
      log_d("authorizationEntryCount: %d", count);
      break;
    }
    case NukiCommand::logEntry : {
      printBuffer((byte*)data, dataLen, false, "logEntry");
      LogEntry logEntry;
      memcpy(&logEntry, data, sizeof(logEntry));
      listOfLogEntries.push_back(logEntry);
      #ifdef DEBUG_NUKI_READABLE_DATA
      logLogEntry(logEntry);
      #endif
      break;
    }
    case NukiCommand::logEntryCount : {
      memcpy(&loggingEnabled, data, sizeof(logEntryCount));
      memcpy(&logEntryCount, &data[1], sizeof(logEntryCount));
      #ifdef DEBUG_NUKI_READABLE_DATA
      log_d("Logging enabled: %d, total nr of log entries: %d", loggingEnabled, logEntryCount);
      #endif
      printBuffer((byte*)data, dataLen, false, "logEntryCount");
      break;
    }
    case NukiCommand::advancedConfig : {
      memcpy(&advancedConfig, data, sizeof(advancedConfig));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logAdvancedConfig(advancedConfig);
      #endif
      printBuffer((byte*)data, dataLen, false, "advancedConfig");
      break;
    }
    case NukiCommand::timeControlEntryCount : {
      printBuffer((byte*)data, dataLen, false, "timeControlEntryCount");
      break;
    }
    case NukiCommand::timeControlEntry : {
      printBuffer((byte*)data, dataLen, false, "timeControlEntry");
      TimeControlEntry timeControlEntry;
      memcpy(&timeControlEntry, data, sizeof(timeControlEntry));
      listOfTimeControlEntries.push_back(timeControlEntry);
      break;
    }
    case NukiCommand::keypadCodeId : {
      printBuffer((byte*)data, dataLen, false, "keypadCodeId");
      break;
    }
    case NukiCommand::keypadCodeCount : {
      printBuffer((byte*)data, dataLen, false, "keypadCodeCount");
      #ifdef DEBUG_NUKI_READABLE_DATA
      uint16_t count = 0;
      memcpy(&count, data, 2);
      log_d("keyPadCodeCount: %d", count);
      #endif
      memcpy(&nrOfKeypadCodes, data, 2);
      break;
    }
    case NukiCommand::keypadCode : {
      printBuffer((byte*)data, dataLen, false, "keypadCode");
      #ifdef DEBUG_NUKI_READABLE_DATA
      KeypadEntry keypadEntry;
      memcpy(&keypadEntry, data, sizeof(KeypadEntry));
      listOfKeyPadEntries.push_back(keypadEntry);
      logKeypadEntry(keypadEntry);
      #endif
      break;
    }
    case NukiCommand::keypadAction : {
      printBuffer((byte*)data, dataLen, false, "keypadAction");
      break;
    }
    default:
      log_e("UNKNOWN RETURN COMMAND: %02x", returnCode);
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

bool NukiBle::crcValid(uint8_t* pData, uint16_t length) {
  uint16_t receivedCrc = ((uint16_t)pData[length - 1] << 8) | pData[length - 2];
  Crc16 crcObj;
  uint16_t dataCrc;
  crcObj.clearCrc();
  dataCrc = crcObj.fastCrc(pData, 0, length - 2, false, false, 0x1021, 0xffff, 0x0000, 0x8000, 0xffff);

  if (!(receivedCrc == dataCrc)) {
    log_e("CRC CHECK FAILED!");
    crcCheckOke = false;
    return false;
  }
  #ifdef DEBUG_NUKI_COMMUNICATION
  log_d("CRC CHECK OKE");
  #endif
  crcCheckOke = true;
  return true;
}

void NukiBle::setEventHandler(NukiSmartlockEventHandler* handler) {
  eventHandler = handler;
  eventHandlerSet = true;
}