/*
 * NukiBle.cpp
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "NukiBle.h"
#include "NukiUtils.h"
#include "string.h"
#include "sodium/crypto_scalarmult.h"
#include "sodium/crypto_core_hsalsa20.h"
#include "sodium/crypto_auth_hmacsha256.h"
#include "sodium/crypto_secretbox.h"
#include "sodium/crypto_box.h"
#include "NimBLEBeacon.h"

namespace Nuki {

NukiBle::NukiBle(const std::string& deviceName, const uint32_t deviceId)
  : deviceName(deviceName),
    deviceId(deviceId) {
}

NukiBle::~NukiBle() {
  if (bleScanner != nullptr) {
    bleScanner->unsubscribe(this);
    bleScanner = nullptr;
  }
}

void NukiBle::initialize() {

  preferences.begin(deviceName.c_str(), false);
  if (!BLEDevice::getInitialized()) {
    BLEDevice::init(deviceName);
  }

  pClient = BLEDevice::createClient();
  pClient->setConnectionParams(12, 12, 0, 200); //according to recommendations Nuki (15ms, 15ms, 0, 2sec)
  pClient->setClientCallbacks(this);

  isPaired = retrieveCredentials();
}

void NukiBle::registerBleScanner(BLEScannerPublisher* bleScanner) {
  this->bleScanner = bleScanner;
  bleScanner->subscribe(this);
}

bool NukiBle::pairNuki() {
  if (retrieveCredentials()) {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Allready paired");
    #endif
    isPaired = true;
    return true;
  }
  bool result = false;

  if (bleAddress != BLEAddress("")) {
    if (connectBle(bleAddress)) {
      crypto_box_keypair(myPublicKey, myPrivateKey);

      PairingState nukiPairingState = PairingState::InitPairing;
      do {
        nukiPairingState = pairStateMachine(nukiPairingState);
        delay(50);
      } while ((nukiPairingState != PairingState::Success) && (nukiPairingState != PairingState::Timeout));

      if (nukiPairingState == PairingState::Success) {
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
  deleteCredentials();
  isPaired = false;
  #ifdef DEBUG_NUKI_CONNECT
  log_d("[%s] Credentials deleted", deviceName.c_str());
  #endif
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
      std::string serviceUUID = keyturnerServiceUUID.toString();
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
      if (advertisedDevice->getServiceData(keyturnerPairingServiceUUID) != "") {
        #ifdef DEBUG_NUKI_CONNECT
        log_d("Found nuki in pairing state: %s addr: %s", std::string(advertisedDevice->getName()).c_str(), std::string(advertisedDevice->getAddress()).c_str());
        #endif
        bleAddress = advertisedDevice->getAddress();
      }
    }
  }
}

CmdResult NukiBle::executeAction(Action action) {
  #ifdef DEBUG_NUKI_CONNECT
  log_d("************************ CHECK PAIRED ************************");
  #endif
  if (retrieveCredentials()) {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Credentials retrieved from preferences, ready for commands");
    #endif
  } else {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Credentials NOT retrieved from preferences, first pair with the lock");
    #endif
    return CmdResult::NotPaired;
  }

  #ifdef DEBUG_NUKI_COMMUNICATION
  log_d("Start executing: %02x ", action.command);
  #endif
  if (action.cmdType == CommandType::Command) {
    while (1) {
      CmdResult result = cmdStateMachine(action);
      if (result != CmdResult::Working) {
        return result;
      }
      esp_task_wdt_reset();
      delay(10);
    }

  } else if (action.cmdType == CommandType::CommandWithChallenge) {
    while (1) {
      CmdResult result = cmdChallStateMachine(action);
      if (result != CmdResult::Working) {
        return result;
      }
      esp_task_wdt_reset();
      delay(10);
    }
  } else if (action.cmdType == CommandType::CommandWithChallengeAndAccept) {
    while (1) {
      CmdResult result = cmdChallAccStateMachine(action);
      if (result != CmdResult::Working) {
        return result;
      }
      esp_task_wdt_reset();
      delay(10);
    }
  } else if (action.cmdType == CommandType::CommandWithChallengeAndPin) {
    while (1) {
      CmdResult result = cmdChallStateMachine(action, true);
      if (result != CmdResult::Working) {
        return result;
      }
      esp_task_wdt_reset();
      delay(10);
    }
  } else {
    log_w("Unknown cmd type");
  }
  return CmdResult::Failed;
}

CmdResult NukiBle::cmdStateMachine(Action action) {
  switch (nukiCommandState) {
    case CommandState::Idle: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING COMMAND ************************");
      #endif
      lastMsgCodeReceived = Command::Empty;
      timeNow = millis();
      sendEncryptedMessage(Command::RequestData, action.payload, action.payloadLen);
      nukiCommandState = CommandState::CmdSent;
      break;
    }
    case CommandState::CmdSent: {
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving command response");
        nukiCommandState = CommandState::Idle;
        return CmdResult::TimeOut;
      } else if (lastMsgCodeReceived != Command::ErrorReport && lastMsgCodeReceived != Command::Empty) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND DONE ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return CmdResult::Success;
      } else if (lastMsgCodeReceived == Command::ErrorReport) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return CmdResult::Failed;
      }
      break;
    }
    default: {
      log_w("Unknown request command state");
      return CmdResult::Failed;
      break;
    }
  }
  return CmdResult::Working;
}

CmdResult NukiBle::cmdChallStateMachine(Action action, bool sendPinCode) {
  switch (nukiCommandState) {
    case CommandState::Idle: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING CHALLENGE ************************");
      #endif
      lastMsgCodeReceived = Command::Empty;
      timeNow = millis();
      unsigned char payload[sizeof(Command)] = {0x04, 0x00};  //challenge
      sendEncryptedMessage(Command::RequestData, payload, sizeof(Command));
      nukiCommandState = CommandState::ChallengeSent;
      break;
    }
    case CommandState::ChallengeSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING CHALLENGE RESPONSE************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving challenge response");
        nukiCommandState = CommandState::Idle;
        return CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::Challenge) {
        nukiCommandState = CommandState::ChallengeRespReceived;
        lastMsgCodeReceived = Command::Empty;
      }
      delay(50);
      break;
    }
    case CommandState::ChallengeRespReceived: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING COMMAND ************************");
      #endif
      lastMsgCodeReceived = Command::Empty;
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
      nukiCommandState = CommandState::CmdSent;
      break;
    }
    case CommandState::CmdSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING DATA ************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving data");
        nukiCommandState = CommandState::Idle;
        return CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::ErrorReport) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return CmdResult::Failed;
      } else if (crcCheckOke) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ DATA RECEIVED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        return CmdResult::Success;
      }
      delay(50);
      break;
    }
    default:
      log_w("Unknown request command state");
      return CmdResult::Failed;
      break;
  }
  return CmdResult::Working;
}

CmdResult NukiBle::cmdChallAccStateMachine(Action action) {
  switch (nukiCommandState) {
    case CommandState::Idle: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING CHALLENGE ************************");
      #endif
      lastMsgCodeReceived = Command::Empty;
      timeNow = millis();
      unsigned char payload[sizeof(Command)] = {0x04, 0x00};  //challenge
      sendEncryptedMessage(Command::RequestData, payload, sizeof(Command));
      nukiCommandState = CommandState::ChallengeSent;
      break;
    }
    case CommandState::ChallengeSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING CHALLENGE RESPONSE************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving challenge response");
        nukiCommandState = CommandState::Idle;
        return CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::Challenge) {
        log_d("last msg code: %d, compared with: %d", lastMsgCodeReceived, Command::Challenge);
        nukiCommandState = CommandState::ChallengeRespReceived;
        lastMsgCodeReceived = Command::Empty;
      }
      delay(50);
      break;
    }
    case CommandState::ChallengeRespReceived: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING COMMAND ************************");
      #endif
      lastMsgCodeReceived = Command::Empty;
      timeNow = millis();
      //add received challenge nonce to payload
      uint8_t payloadLen = action.payloadLen + sizeof(challengeNonceK);
      unsigned char payload[payloadLen];
      memcpy(payload, action.payload, action.payloadLen);
      memcpy(&payload[action.payloadLen], challengeNonceK, sizeof(challengeNonceK));
      sendEncryptedMessage(action.command, payload, action.payloadLen + sizeof(challengeNonceK));
      nukiCommandState = CommandState::CmdSent;
      break;
    }
    case CommandState::CmdSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING ACCEPT ************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving accept response");
        nukiCommandState = CommandState::Idle;
        return CmdResult::TimeOut;
      } else if ((CommandStatus)lastMsgCodeReceived == CommandStatus::Accepted) {
        nukiCommandState = CommandState::CmdAccepted;
        lastMsgCodeReceived = Command::Empty;
      }
      delay(50);
      break;
    }
    case CommandState::CmdAccepted: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING COMPLETE ************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        timeNow = millis();
        log_w("Timeout receiving complete response");
        nukiCommandState = CommandState::Idle;
        return CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::ErrorReport) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return CmdResult::Failed;
      } else if ((CommandStatus)lastMsgCodeReceived == CommandStatus::Complete) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND SUCCESS ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return CmdResult::Success;
      }
      delay(50);
      break;
    }
    default:
      log_w("Unknown request command state");
      return CmdResult::Failed;
      break;
  }
  return CmdResult::Working;
}

CmdResult NukiBle::requestKeyTurnerState(KeyTurnerState* retrievedKeyTurnerState) {
  Action action;
  uint16_t payload = (uint16_t)Command::KeyturnerStates;

  action.cmdType = CommandType::Command;
  action.command = Command::RequestData;
  memcpy(&action.payload[0], &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    memcpy(retrievedKeyTurnerState, &keyTurnerState, sizeof(KeyTurnerState));
  }
  return result;
}

void NukiBle::retrieveKeyTunerState(KeyTurnerState* retrievedKeyTurnerState) {
  memcpy(retrievedKeyTurnerState, &keyTurnerState, sizeof(KeyTurnerState));
}

bool NukiBle::isBatteryCritical() {
  //MSB/LSB!
  return keyTurnerState.criticalBatteryState & (1 << 7);
}

bool NukiBle::isBatteryCharging() {
  //MSB/LSB!
  return keyTurnerState.criticalBatteryState & (1 << 6);
}

uint8_t NukiBle::getBatteryPerc() {
  return (keyTurnerState.criticalBatteryState & 0b11111100) >> 1;
}

CmdResult NukiBle::requestBatteryReport(BatteryReport* retrievedBatteryReport) {
  Action action;
  uint16_t payload = (uint16_t)Command::BatteryReport;

  action.cmdType = CommandType::Command;
  action.command = Command::RequestData;
  memcpy(&action.payload[0], &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    memcpy(retrievedBatteryReport, &batteryReport, sizeof(batteryReport));
  }
  return result;
}

CmdResult NukiBle::lockAction(LockAction lockAction, uint32_t nukiAppId, uint8_t flags, unsigned char* nameSuffix) {
  Action action;
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

  action.cmdType = CommandType::CommandWithChallengeAndAccept;
  action.command = Command::LockAction;
  memcpy(action.payload, &payload, payloadLen);
  action.payloadLen = payloadLen;

  return executeAction(action);
}

CmdResult NukiBle::retrieveKeypadEntries(uint16_t offset, uint16_t count) {
  Action action;
  unsigned char payload[4] = {0};
  memcpy(payload, &offset, 2);
  memcpy(&payload[2], &count, 2);

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::RequestKeypadCodes;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  listOfKeyPadEntries.clear();

  return executeAction(action);
}

CmdResult NukiBle::addKeypadEntry(NewKeypadEntry newKeypadEntry) {
  //TODO verify data validity
  Action action;

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::AddKeypadCode;
  memcpy(action.payload, &newKeypadEntry, sizeof(NewKeypadEntry));
  action.payloadLen = sizeof(NewKeypadEntry);

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addKeyPadEntry, payloadlen: %d", sizeof(NewKeypadEntry));
    printBuffer(action.payload, sizeof(NewKeypadEntry), false, "addKeyPadCode content: ");
    logNewKeypadEntry(newKeypadEntry);
    #endif
  }
  return result;
}

CmdResult NukiBle::updateKeypadEntry(UpdatedKeypadEntry updatedKeyPadEntry) {
  //TODO verify data validity
  Action action;

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::UpdateKeypadCode;
  memcpy(action.payload, &updatedKeyPadEntry, sizeof(UpdatedKeypadEntry));
  action.payloadLen = sizeof(UpdatedKeypadEntry);

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addKeyPadEntry, payloadlen: %d", sizeof(UpdatedKeypadEntry));
    printBuffer(action.payload, sizeof(UpdatedKeypadEntry), false, "updatedKeypad content: ");
    logUpdatedKeypadEntry(updatedKeyPadEntry);
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

CmdResult NukiBle::retrieveAuthorizationEntries(uint16_t offset, uint16_t count) {
  Action action;
  unsigned char payload[4] = {0};
  memcpy(payload, &offset, 2);
  memcpy(&payload[2], &count, 2);

  action.cmdType = CommandType::CommandWithChallengeAndPin;
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

CmdResult NukiBle::addAuthorizationEntry(NewAuthorizationEntry newAuthorizationEntry) {
  //TODO verify data validity
  Action action;
  unsigned char payload[sizeof(NewAuthorizationEntry)] = {0};
  memcpy(payload, &newAuthorizationEntry, sizeof(NewAuthorizationEntry));

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::AuthorizationDatInvite;
  memcpy(action.payload, &payload, sizeof(NewAuthorizationEntry));
  action.payloadLen = sizeof(NewAuthorizationEntry);

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addAuthorizationEntry, payloadlen: %d", sizeof(NewAuthorizationEntry));
    printBuffer(action.payload, sizeof(NewAuthorizationEntry), false, "addAuthorizationEntry content: ");
    logNewAuthorizationEntry(newAuthorizationEntry);
    #endif
  }
  return result;
}

CmdResult NukiBle::updateAuthorizationEntry(UpdatedAuthorizationEntry updatedAuthorizationEntry) {
  //TODO verify data validity
  Action action;
  unsigned char payload[sizeof(UpdatedAuthorizationEntry)] = {0};
  memcpy(payload, &updatedAuthorizationEntry, sizeof(UpdatedAuthorizationEntry));

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::UpdateAuthorization;
  memcpy(action.payload, &payload, sizeof(UpdatedAuthorizationEntry));
  action.payloadLen = sizeof(UpdatedAuthorizationEntry);

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addAuthorizationEntry, payloadlen: %d", sizeof(UpdatedAuthorizationEntry));
    printBuffer(action.payload, sizeof(UpdatedAuthorizationEntry), false, "updatedKeypad content: ");
    logUpdatedAuthorizationEntry(updatedAuthorizationEntry);
    #endif
  }
  return result;
}

CmdResult NukiBle::retrieveLogEntries(uint32_t startIndex, uint16_t count, uint8_t sortOrder, bool totalCount) {
  Action action;
  unsigned char payload[8] = {0};
  memcpy(payload, &startIndex, 4);
  memcpy(&payload[4], &count, 2);
  memcpy(&payload[6], &sortOrder, 1);
  memcpy(&payload[7], &totalCount, 1);

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::RequestLogEntries;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  listOfLogEntries.clear();

  return executeAction(action);
}

void NukiBle::getLogEntries(std::list<LogEntry>* requestedLogEntries) {
  requestedLogEntries->clear();
  std::list<LogEntry>::iterator it = listOfLogEntries.begin();
  while (it != listOfLogEntries.end()) {
    requestedLogEntries->push_back(*it);
    it++;
  }
}

CmdResult NukiBle::requestConfig(Config* retrievedConfig) {
  Action action;

  action.cmdType = CommandType::CommandWithChallenge;
  action.command = Command::RequestConfig;

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    memcpy(retrievedConfig, &config, sizeof(Config));
  }
  return result;
}

CmdResult NukiBle::requestAdvancedConfig(AdvancedConfig* retrievedAdvancedConfig) {
  Action action;

  action.cmdType = CommandType::CommandWithChallenge;
  action.command = Command::RequestAdvancedConfig;

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    memcpy(retrievedAdvancedConfig, &advancedConfig, sizeof(AdvancedConfig));
  }
  return result;
}

CmdResult NukiBle::setFromConfig(const Config config) {
  NewConfig newConfig;
  createNewConfig(&config, &newConfig);
  return setConfig(newConfig);
}

CmdResult NukiBle::setConfig(NewConfig newConfig) {
  Action action;
  unsigned char payload[sizeof(NewConfig)] = {0};
  memcpy(payload, &newConfig, sizeof(NewConfig));

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::SetConfig;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  return executeAction(action);
}

CmdResult NukiBle::setFromAdvancedConfig(const AdvancedConfig config) {
  NewAdvancedConfig newConfig;
  createNewAdvancedConfig(&config, &newConfig);
  return setAdvancedConfig(newConfig);
}

CmdResult NukiBle::setAdvancedConfig(NewAdvancedConfig newAdvancedConfig) {
  Action action;
  unsigned char payload[sizeof(NewAdvancedConfig)] = {0};
  memcpy(payload, &newAdvancedConfig, sizeof(NewAdvancedConfig));

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::SetAdvancedConfig;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  return executeAction(action);
}

CmdResult NukiBle::addTimeControlEntry(NewTimeControlEntry newTimeControlEntry) {
//TODO verify data validity
  Action action;
  unsigned char payload[sizeof(NewTimeControlEntry)] = {0};
  memcpy(payload, &newTimeControlEntry, sizeof(NewTimeControlEntry));

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::AddTimeControlEntry;
  memcpy(action.payload, &payload, sizeof(NewTimeControlEntry));
  action.payloadLen = sizeof(NewTimeControlEntry);

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addTimeControlEntry, payloadlen: %d", sizeof(NewTimeControlEntry));
    printBuffer(action.payload, sizeof(NewTimeControlEntry), false, "new time control content: ");
    logNewTimeControlEntry(newTimeControlEntry);
    #endif
  }
  return result;
}

CmdResult NukiBle::updateTimeControlEntry(TimeControlEntry TimeControlEntry) {
  //TODO verify data validity
  Action action;
  unsigned char payload[sizeof(TimeControlEntry)] = {0};
  memcpy(payload, &TimeControlEntry, sizeof(TimeControlEntry));

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::UpdateTimeControlEntry;
  memcpy(action.payload, &payload, sizeof(TimeControlEntry));
  action.payloadLen = sizeof(TimeControlEntry);

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("addTimeControlEntry, payloadlen: %d", sizeof(TimeControlEntry));
    printBuffer(action.payload, sizeof(TimeControlEntry), false, "updated time control content: ");
    logTimeControlEntry(TimeControlEntry);
    #endif
  }
  return result;
}

CmdResult NukiBle::removeTimeControlEntry(uint8_t entryId) {
//TODO verify data validity
  Action action;
  unsigned char payload[1] = {0};
  memcpy(payload, &entryId, 1);

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::RemoveTimeControlEntry;
  memcpy(action.payload, &payload, 1);
  action.payloadLen = 1;

  return executeAction(action);
}

CmdResult NukiBle::retrieveTimeControlEntries() {
  Action action;

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::RequestTimeControlEntries;
  action.payloadLen = 0;

  listOfTimeControlEntries.clear();

  return executeAction(action);
}

void NukiBle::getTimeControlEntries(std::list<TimeControlEntry>* requestedTimeControlEntries) {
  requestedTimeControlEntries->clear();
  std::list<TimeControlEntry>::iterator it = listOfTimeControlEntries.begin();
  while (it != listOfTimeControlEntries.end()) {
    requestedTimeControlEntries->push_back(*it);
    it++;
  }
}

CmdResult NukiBle::setSecurityPin(uint16_t newSecurityPin) {
  Action action;
  unsigned char payload[2] = {0};
  memcpy(payload, &newSecurityPin, 2);

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::SetSecurityPin;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    pinCode = newSecurityPin;
    saveCredentials();
  }
  return result;
}

CmdResult NukiBle::verifySecurityPin() {
  Action action;

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::VerifySecurityPin;
  action.payloadLen = 0;

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Verify security pin code success");
    #endif
  }
  return result;
}

CmdResult NukiBle::requestCalibration() {
  Action action;

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::RequestCalibration;
  action.payloadLen = 0;

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Calibration executed");
    #endif
  }
  return result;
}

CmdResult NukiBle::requestReboot() {
  Action action;

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::RequestReboot;
  action.payloadLen = 0;

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Reboot executed");
    #endif
  }
  return result;
}

CmdResult NukiBle::updateTime(TimeValue time) {
  Action action;
  unsigned char payload[sizeof(TimeValue)] = {0};
  memcpy(payload, &time, sizeof(TimeValue));

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::UpdateTime;
  memcpy(action.payload, &payload, sizeof(TimeValue));
  action.payloadLen = sizeof(TimeValue);

  CmdResult result = executeAction(action);
  if (result == CmdResult::Success) {
    #ifdef DEBUG_NUKI_READABLE_DATA
    log_d("Time set: %d-%d-%d %d:%d:%d", time.year, time.month, time.day, time.hour, time.minute, time.second);
    #endif
  }
  return result;
}

void NukiBle::createNewConfig(const Config* oldConfig, NewConfig* newConfig) {
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
CmdResult NukiBle::setName(std::string name) {

  if (name.length() <= 32) {
    Config oldConfig;
    CmdResult result = requestConfig(&oldConfig);
    if (result == CmdResult::Success) {
      memcpy(oldConfig.name, name.c_str(), name.length());
      result = setFromConfig(oldConfig);
    }
    return result;
  } else {
    log_w("setName, too long (max32)");
    return CmdResult::Failed;
  }
}

CmdResult NukiBle::enablePairing(bool enable) {
  Config oldConfig;
  CmdResult result = requestConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.pairingEnabled = enable;
    result = setFromConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::enableButton(bool enable) {
  Config oldConfig;
  CmdResult result = requestConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.buttonEnabled = enable;
    result = setFromConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::enableLedFlash(bool enable) {
  Config oldConfig;
  CmdResult result = requestConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.ledEnabled = enable;
    result = setFromConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::setLedBrightness(uint8_t level) {
  //level is from 0 (off) to 5(max)
  Config oldConfig;
  CmdResult result = requestConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.ledBrightness = level > 5 ? 5 : level;
    result = setFromConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::enableSingleLock(bool enable) {
  Config oldConfig;
  CmdResult result = requestConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.singleLock = enable;
    result = setFromConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::setAdvertisingMode(AdvertisingMode mode) {
  Config oldConfig;
  CmdResult result = requestConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.advertisingMode = mode;
    result = setFromConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::enableDst(bool enable) {
  Config oldConfig;
  CmdResult result = requestConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.dstMode = enable;
    result = setFromConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::setTimeZoneOffset(int16_t minutes) {
  Config oldConfig;
  CmdResult result = requestConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.timeZoneOffset = minutes;
    result = setFromConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::setTimeZoneId(TimeZoneId timeZoneId) {
  Config oldConfig;
  CmdResult result = requestConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.timeZoneId = timeZoneId;
    result = setFromConfig(oldConfig);
  }
  return result;
}

void NukiBle::createNewAdvancedConfig(const AdvancedConfig* oldConfig, NewAdvancedConfig* newConfig) {
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
CmdResult NukiBle::setSingleButtonPressAction(ButtonPressAction action) {
  AdvancedConfig oldConfig;
  CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.singleButtonPressAction = action;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::setDoubleButtonPressAction(ButtonPressAction action) {
  AdvancedConfig oldConfig;
  CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.doubleButtonPressAction = action;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::setBatteryType(BatteryType type) {
  AdvancedConfig oldConfig;
  CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.batteryType = type;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::enableAutoBatteryTypeDetection(bool enable) {
  AdvancedConfig oldConfig;
  CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.automaticBatteryTypeDetection = enable;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::disableAutoUnlock(bool disable) {
  AdvancedConfig oldConfig;
  CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.autoUnLockDisabled = disable;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::enableAutoLock(bool enable) {
  AdvancedConfig oldConfig;
  CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.autoLockEnabled = enable;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::enableImmediateAutoLock(bool enable) {
  AdvancedConfig oldConfig;
  CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.immediateAutoLockEnabled = enable;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

CmdResult NukiBle::enableAutoUpdate(bool enable) {
  AdvancedConfig oldConfig;
  CmdResult result = requestAdvancedConfig(&oldConfig);
  if (result == CmdResult::Success) {
    oldConfig.autoUpdateEnabled = enable;
    result = setFromAdvancedConfig(oldConfig);
  }
  return result;
}

bool NukiBle::saveSecurityPincode(uint16_t pinCode) {
  return (preferences.putBytes(SECURITY_PINCODE_STORE_NAME, &pinCode, 2) == 2);
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

bool NukiBle::retrieveCredentials() {
  //TODO check on empty (invalid) credentials?
  unsigned char buff[6];

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
    log_d("PinCode: %d", pinCode);
    #endif

    if (pinCode == 0) {
      log_w("Pincode is 000000");
    }

  } else {
    return false;
  }
  return true;
}

void NukiBle::deleteCredentials() {
  preferences.remove(SECRET_KEY_STORE_NAME);
  preferences.remove(AUTH_ID_STORE_NAME);
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
      return PairingState::ReqRemPubKey;
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
      timeNow = millis();
      return PairingState::RecRemPubKey;
    }
    case PairingState::RecRemPubKey: {
      if (isCharArrayNotEmpty(remotePublicKey, sizeof(remotePublicKey))) {
        return PairingState::SendPubKey;
      }
      break;
    }
    case PairingState::SendPubKey: {
      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### SEND CLIENT PUBLIC KEY #########################");
      #endif
      sendPlainMessage(Command::PublicKey, myPublicKey, sizeof(myPublicKey));
      return PairingState::GenKeyPair;
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
      timeNow = millis();
      return PairingState::CalculateAuth;
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
        return PairingState::SendAuth;
      }
      break;
    }
    case PairingState::SendAuth: {
      #ifdef DEBUG_NUKI_CONNECT
      log_d("##################### SEND AUTHENTICATOR #########################");
      #endif
      sendPlainMessage(Command::AuthorizationAuthenticator, authenticator, sizeof(authenticator));
      timeNow = millis();
      return PairingState::SendAuthData;
    }
    case PairingState::SendAuthData: {
      if (isCharArrayNotEmpty(challengeNonceK, sizeof(challengeNonceK))) {
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
        sendPlainMessage(Command::AuthorizationData, authorizationDataMessage, sizeof(authorizationDataMessage));
        timeNow = millis();
        return PairingState::SendAuthIdConf;
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
        timeNow = millis();
        return PairingState::RecStatus;
      }
      break;
    }
    case PairingState::RecStatus: {
      if (receivedStatus == 0) {
        #ifdef DEBUG_NUKI_CONNECT
        log_d("####################### PAIRING DONE ###############################################");
        #endif
        return PairingState::Success;
      }
      break;
    }
    default: {
      log_e("Unknown pairing status");
      return PairingState::Timeout;
    }
  }

  if (millis() - timeNow > PAIRING_TIMEOUT) {
    log_w("Pairing timeout");
    return PairingState::Timeout;
  }

  // Nothing happend, returned same state
  return nukiPairingState;
}

void NukiBle::sendEncryptedMessage(Command commandIdentifier, const unsigned char* payload, uint8_t payloadLen) {
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
      pUsdioCharacteristic->writeValue((uint8_t*)dataToSend, sizeof(dataToSend), true);
    } else {
      log_w("Send encr msg failed due to unable to connect");
    }
  } else {
    log_w("Send msg failed due to encryption fail");
  }

}

void NukiBle::sendPlainMessage(Command commandIdentifier, const unsigned char* payload, uint8_t payloadLen) {
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

  if (connectBle(bleAddress)) {
    pGdioCharacteristic->writeValue((uint8_t*)dataToSend, payloadLen + 4, true);
  } else {
    log_w("Send plain msg failed due to unable to connect");
  }
}

bool NukiBle::registerOnGdioChar() {
  // Obtain a reference to the KeyTurner Pairing service
  pKeyturnerPairingService = pClient->getService(keyturnerPairingServiceUUID);
  if (pKeyturnerPairingService != nullptr) {
    //Obtain reference to GDIO char
    pGdioCharacteristic = pKeyturnerPairingService->getCharacteristic(keyturnerGdioUUID);
    if (pGdioCharacteristic != nullptr) {
      if (pGdioCharacteristic->canIndicate()) {

        using namespace std::placeholders;
        notify_callback callback = std::bind(&NukiBle::notifyCallback, this, _1, _2, _3, _4);
        pGdioCharacteristic->subscribe(false, callback); //false = indication, true = notification
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
  pKeyturnerDataService = pClient->getService(keyturnerServiceUUID);
  if (pKeyturnerDataService != nullptr) {
    //Obtain reference to NDIO char
    pUsdioCharacteristic = pKeyturnerDataService->getCharacteristic(userDataUUID);
    if (pUsdioCharacteristic != nullptr) {
      if (pUsdioCharacteristic->canIndicate()) {

        using namespace std::placeholders;
        notify_callback callback = std::bind(&NukiBle::notifyCallback, this, _1, _2, _3, _4);

        pUsdioCharacteristic->subscribe(false, callback); //false = indication, true = notification
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

  if (pBLERemoteCharacteristic->getUUID() == keyturnerGdioUUID) {
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
  lastMsgCodeReceived = returnCode;

  switch (returnCode) {
    case Command::RequestData : {
      log_d("requestData");
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
      logAuthorizationEntry(authEntry);
      #endif
      break;
    }
    case Command::KeyturnerStates : {
      printBuffer((byte*)data, dataLen, false, "keyturnerStates");
      memcpy(&keyTurnerState, data, sizeof(keyTurnerState));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logKeyturnerState(keyTurnerState);
      #endif
      break;
    }
    case Command::Status : {
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
    case Command::OpeningsClosingsSummary : {
      printBuffer((byte*)data, dataLen, false, "openingsClosingsSummary");
      log_w("NOT IMPLEMENTED ONLY FOR NUKI v1"); //command is not available on Nuki v2 (only on Nuki v1)
      break;
    }
    case Command::BatteryReport : {
      printBuffer((byte*)data, dataLen, false, "batteryReport");
      memcpy(&batteryReport, data, sizeof(batteryReport));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logBatteryReport(batteryReport);
      #endif
      break;
    }
    case Command::ErrorReport : {
      log_e("Error: %02x for command: %02x:%02x", data[0], data[2], data[1]);
      memcpy(&errorCode, &data[0], sizeof(errorCode));
      logErrorCode(data[0]);
      break;
    }
    case Command::Config : {
      memcpy(&config, data, sizeof(config));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logConfig(config);
      #endif
      printBuffer((byte*)data, dataLen, false, "config");
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
    case Command::LogEntry : {
      printBuffer((byte*)data, dataLen, false, "logEntry");
      LogEntry logEntry;
      memcpy(&logEntry, data, sizeof(logEntry));
      listOfLogEntries.push_back(logEntry);
      #ifdef DEBUG_NUKI_READABLE_DATA
      logLogEntry(logEntry);
      #endif
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
    case Command::AdvancedConfig : {
      memcpy(&advancedConfig, data, sizeof(advancedConfig));
      #ifdef DEBUG_NUKI_READABLE_DATA
      logAdvancedConfig(advancedConfig);
      #endif
      printBuffer((byte*)data, dataLen, false, "advancedConfig");
      break;
    }
    case Command::TimeControlEntryCount : {
      printBuffer((byte*)data, dataLen, false, "timeControlEntryCount");
      break;
    }
    case Command::TimeControlEntry : {
      printBuffer((byte*)data, dataLen, false, "timeControlEntry");
      TimeControlEntry timeControlEntry;
      memcpy(&timeControlEntry, data, sizeof(timeControlEntry));
      listOfTimeControlEntries.push_back(timeControlEntry);
      break;
    }
    case Command::KeypadCodeId : {
      printBuffer((byte*)data, dataLen, false, "keypadCodeId");
      break;
    }
    case Command::KeypadCodeCount : {
      printBuffer((byte*)data, dataLen, false, "keypadCodeCount");
      #ifdef DEBUG_NUKI_READABLE_DATA
      uint16_t count = 0;
      memcpy(&count, data, 2);
      log_d("keyPadCodeCount: %d", count);
      #endif
      memcpy(&nrOfKeypadCodes, data, 2);
      break;
    }
    case Command::KeypadCode : {
      printBuffer((byte*)data, dataLen, false, "keypadCode");
      #ifdef DEBUG_NUKI_READABLE_DATA
      KeypadEntry keypadEntry;
      memcpy(&keypadEntry, data, sizeof(KeypadEntry));
      listOfKeyPadEntries.push_back(keypadEntry);
      logKeypadEntry(keypadEntry);
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

ErrorCode NukiBle::getLastError() const {
  return errorCode;
};

} // namespace Nuki
