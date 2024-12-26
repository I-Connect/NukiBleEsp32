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
#include <algorithm>
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
    preferencesId(preferencedId)
{
  rssi = 0;
  lastReceivedBeaconTs = 0;
  lastHeartbeat = 0;

  #ifdef DEBUG_NUKI_CONNECT
  debugNukiConnect = true;
  #endif
  #ifdef DEBUG_NUKI_COMMUNICATION
  debugNukiCommunication = true;
  #endif
  #ifdef DEBUG_NUKI_READABLE_DATA
  debugNukiReadableData = true;
  #endif
  #ifdef DEBUG_NUKI_HEX_DATA
  debugNukiHexData = true;
  #endif
  #ifdef DEBUG_NUKI_COMMAND
  debugNukiCommand = true;
  #endif

}

NukiBle::~NukiBle() {
  if (bleScanner != nullptr) {
    bleScanner->unsubscribe(this);
    bleScanner = nullptr;
  }
}

void NukiBle::initialize(bool initAltConnect) {
  preferences.begin(preferencesId.c_str(), false);
  #ifdef NUKI_USE_LATEST_NIMBLE
  if (!NimBLEDevice::isInitialized())
  #else
  if (!NimBLEDevice::getInitialized())
  #endif
  {
    NimBLEDevice::init(deviceName);
  }

  if (!initAltConnect) {
    pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(this);
    #ifdef NUKI_USE_LATEST_NIMBLE
    pClient->setConnectTimeout(connectTimeoutSec * 1000);
    #else
    pClient->setConnectTimeout(connectTimeoutSec);
    #endif
  }
  else
  {
    altConnect = true;
  }
  isPaired = retrieveCredentials();
}

#ifdef NUKI_USE_LATEST_NIMBLE
void NukiBle::setPower(esp_power_level_t powerLevel) {
  if (!NimBLEDevice::isInitialized()) {
    NimBLEDevice::init(deviceName);
  }

  int power = 9;

  switch(powerLevel)
  {
    case ESP_PWR_LVL_N12:
      power = -12;
      break;
    case ESP_PWR_LVL_N9:
      power = -9;
      break;
    case ESP_PWR_LVL_N6:
      power = -6;
      break;
    case ESP_PWR_LVL_N0:
      power = 0;
      break;
    case ESP_PWR_LVL_P3:
      power = 3;
      break;
    case ESP_PWR_LVL_P6:
      power = 6;
      break;
    case ESP_PWR_LVL_P9:
      power = 9;
      break;
    default:
      power = 9;
      break;
  }

  NimBLEDevice::setPower(power);
}
#else
void NukiBle::setPower(esp_power_level_t powerLevel) {
  if (!NimBLEDevice::getInitialized()) {
    NimBLEDevice::init(deviceName);
  }

  NimBLEDevice::setPower(powerLevel);
}
#endif

void NukiBle::registerBleScanner(BleScanner::Publisher* bleScanner) {
  this->bleScanner = bleScanner;
  bleScanner->subscribe(this);
}

PairingResult NukiBle::pairNuki(AuthorizationIdType idType) {
  authorizationIdType = idType;

  if (retrieveCredentials()) {
    if (debugNukiConnect) {
      logMessage("Already paired");
    }
    isPaired = true;
    return PairingResult::Success;
  }
  PairingResult result = PairingResult::Pairing;

  #ifndef NUKI_64BIT_TIME
  if (pairingLastSeen < millis() - 2000) pairingServiceAvailable = false;
  #else
  if (pairingLastSeen < (esp_timer_get_time() / 1000) - 2000) pairingServiceAvailable = false;
  #endif

  if (pairingServiceAvailable && bleAddress != BLEAddress("", 0)) {
    pairingServiceAvailable = false;
    if (debugNukiConnect) {
      logMessage("Nuki in pairing mode found");
    }
    if (connectBle(bleAddress, true)) {
      crypto_box_keypair(myPublicKey, myPrivateKey);

      PairingState nukiPairingState = PairingState::InitPairing;
      do {
        nukiPairingState = pairStateMachine(nukiPairingState);
        extendDisconnectTimeout();
        delay(50);
      } while ((nukiPairingState != PairingState::Success) && (nukiPairingState != PairingState::Timeout));

      if (nukiPairingState == PairingState::Success) {
        saveCredentials();
        result = PairingResult::Success;
        #ifndef NUKI_64BIT_TIME
        lastHeartbeat = millis();
        #else
        lastHeartbeat = (esp_timer_get_time() / 1000);
        #endif
      } else {
        result = PairingResult::Timeout;
      }
      extendDisconnectTimeout();
    }
  } else {
    if (debugNukiConnect) {
      logMessage("No nuki in pairing mode found");
    }
  }

  if (debugNukiConnect) {
    logMessageVar("pairing result %d", (unsigned int)result);
  }

  isPaired = (result == PairingResult::Success);
  return result;
}

void NukiBle::unPairNuki() {
  deleteCredentials();
  isPaired = false;
  if (debugNukiConnect) {
    logMessageVar("[%s] Credentials deleted", deviceName.c_str());
  }
}

void NukiBle::resetHost() {
  if (debugNukiConnect) {
    logMessageVar("[%s] Resetting BLE host", deviceName.c_str());
  }
  
  ble_hs_sched_reset(0);
}

bool NukiBle::connectBle(const BLEAddress bleAddress, bool pairing) {
  if (altConnect) {
    connecting = true;
    bleScanner->enableScanning(false);
    pClient = nullptr;

    if (debugNukiConnect) {
      #if (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0))
      logMessageVar("connecting within: %s", pcTaskGetTaskName(xTaskGetCurrentTaskHandle()));
      #else
      logMessageVar("connecting within: %s", pcTaskGetName(xTaskGetCurrentTaskHandle()));
      #endif
    }

    uint8_t connectRetry = 0;

    while (connectRetry < connectRetries) {
      #ifdef NUKI_USE_LATEST_NIMBLE
      if(NimBLEDevice::getCreatedClientCount())
      #else
      if(NimBLEDevice::getClientListSize())
      #endif
      {
        pClient = NimBLEDevice::getClientByPeerAddress(bleAddress);
        if(pClient){
          if(!pClient->isConnected()) {
            if(!pClient->connect(bleAddress, refreshServices)) {
              if (debugNukiConnect) {
                logMessageVar("[%s] Reconnect failed", deviceName.c_str());
              }
              connectRetry++;
              #ifndef NUKI_NO_WDT_RESET
              esp_task_wdt_reset();
              #endif
              delay(10);
              continue;
            } else {
              refreshServices = false;
            }
            if (debugNukiConnect) {
              logMessageVar("[%s] Reconnect success", deviceName.c_str());
            }
          }
        }
      }

      if(!pClient) {
        #ifdef NUKI_USE_LATEST_NIMBLE
        if(NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS)
        #else
        if(NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS)
        #endif
        {
          if (debugNukiConnect) {
            logMessageVar("[%s] Max clients reached - no more connections available", deviceName.c_str());
          }
          connectRetry++;
          #ifndef NUKI_NO_WDT_RESET
          esp_task_wdt_reset();
          #endif
          delay(10);
          continue;
        }

        pClient = NimBLEDevice::createClient();
        pClient->setClientCallbacks(this);
        pClient->setConnectionParams(12,12,0,600,64,64);
        #ifndef NUKI_USE_LATEST_NIMBLE
        if (logger == nullptr) {
          log_d("[%s] Connect timeout %d s", deviceName.c_str(), connectTimeoutSec);
        }
        else
        {
          logger->printf("[%s] Connect timeout %d s\r\n", deviceName.c_str(), connectTimeoutSec);
        }
        pClient->setConnectTimeout(connectTimeoutSec);
        #else
        if (logger == nullptr) {
          log_d("[%s] Connect timeout %d ms", deviceName.c_str(), connectTimeoutSec * 1000);
        }
        else
        {
          logger->printf("[%s] Connect timeout %d ms\r\n", deviceName.c_str(), connectTimeoutSec * 1000);
        }
        pClient->setConnectTimeout(connectTimeoutSec * 1000);
        #endif

        delay(300);

        int loopCreateClient = 0;

        while(!pClient && loopCreateClient < 50) {
          delay(100);
          loopCreateClient++;
        }

        if (!pClient) {
          if (debugNukiConnect) {
            logMessageVar("[%s] Failed to create client", deviceName.c_str());
          }
          connectRetry++;
          #ifndef NUKI_NO_WDT_RESET
          esp_task_wdt_reset();
          #endif
          delay(10);
          continue;
        }
      }

      if(!pClient->isConnected()) {
        if (!pClient->connect(bleAddress, refreshServices)) {
          if (debugNukiConnect) {
            logMessageVar("[%s] Failed to connect", deviceName.c_str());
          }
          connectRetry++;
          #ifndef NUKI_NO_WDT_RESET
          esp_task_wdt_reset();
          #endif
          delay(10);
          continue;
        } else {
          refreshServices = false;
        }
      }

      if (debugNukiConnect) {
        if (logger == nullptr) {
          log_d("[%s] Connected to: %s RSSI: %d", deviceName.c_str(), pClient->getPeerAddress().toString().c_str(), pClient->getRssi());
        }
        else
        {
          logger->printf("[%s] Connected to: %s RSSI: %d\r\n", deviceName.c_str(), pClient->getPeerAddress().toString().c_str(), pClient->getRssi());
        }
      }

      if(pairing) {
        if (!registerOnGdioChar()) {
          if (debugNukiConnect) {
            logMessageVar("[%s] Failed to connect on registering GDIO", deviceName.c_str());
          }
          connectRetry++;
          #ifndef NUKI_NO_WDT_RESET
          esp_task_wdt_reset();
          #endif
          delay(10);
          continue;
        }
      } else {
        if (!registerOnUsdioChar()) {
          if (debugNukiConnect) {
            logMessageVar("[%s] Failed to connect on registering USDIO", deviceName.c_str());
          }
          connectRetry++;
          #ifndef NUKI_NO_WDT_RESET
          esp_task_wdt_reset();
          #endif
          delay(10);
          continue;
        }
      }

      bleScanner->enableScanning(true);
      connecting = false;
      return true;
    }

    bleScanner->enableScanning(true);
    connecting = false;
    return false;
  }
  else
  {
    connecting = true;
    bleScanner->enableScanning(false);
    if (!pClient->isConnected()) {
      if (debugNukiConnect) {
        #if (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0))
        logMessageVar("connecting within: %s", pcTaskGetTaskName(xTaskGetCurrentTaskHandle()));
        #else
        logMessageVar("connecting within: %s", pcTaskGetName(xTaskGetCurrentTaskHandle()));
        #endif
      }

      uint8_t connectRetry = 0;
      #ifdef NUKI_USE_LATEST_NIMBLE
      pClient->setConnectTimeout(connectTimeoutSec * 1000);
      #else
      pClient->setConnectTimeout(connectTimeoutSec);
      #endif
      while (connectRetry < connectRetries) {
        if (debugNukiConnect) {
          logMessageVar("connection attempt %d", connectRetry);
        }
        if (pClient->connect(bleAddress, true)) {
          if (pClient->isConnected() && registerOnGdioChar() && registerOnUsdioChar()) {  //doublecheck if is connected otherwise registering gdio crashes esp
            bleScanner->enableScanning(true);
            connecting = false;
            return true;
          } else {
            logMessage("BLE register on pairing or data Service/Char failed", 2);
          }
        } else {
          pClient->disconnect();
          logMessageVar("BLE Connect failed, %d retries left", connectRetries - connectRetry - 1, 2);
        }
        connectRetry++;
        #ifndef NUKI_NO_WDT_RESET
        esp_task_wdt_reset();
        #endif
        delay(10);
      }
    } else {
      bleScanner->enableScanning(true);
      connecting = false;
      return true;
    }
    bleScanner->enableScanning(true);
    connecting = false;
    logMessage("BLE Connect failed", 2);
    return false;
  }
}

void NukiBle::updateConnectionState() {
  if (connecting) {
    if (altConnect) {
      return;
    }
    lastStartTimeout = 0;
  }

  #ifndef NUKI_64BIT_TIME
  if (lastStartTimeout != 0 && (millis() - lastStartTimeout > timeoutDuration)) {
  #else
  if (lastStartTimeout != 0 && ((esp_timer_get_time() / 1000) - lastStartTimeout > timeoutDuration)) {
  #endif
    if (altConnect) {
      disconnect();
      delay(200);
    }
    else if (pClient && pClient->isConnected()) {
      pClient->disconnect();
      if (debugNukiConnect) {
        logMessage("disconnecting BLE on timeout");
      }
    }
  }
}

void NukiBle::disconnect()
{
  pClient = nullptr;

  #ifdef NUKI_USE_LATEST_NIMBLE
  if(NimBLEDevice::getCreatedClientCount())
  #else
  if(NimBLEDevice::getClientListSize())
  #endif
  {
    pClient = NimBLEDevice::getClientByPeerAddress(bleAddress);
  }

  if (pClient) {
    if (pClient->isConnected()) {
      if (debugNukiConnect) {
        logMessage("Disconnecting BLE");
      }

      countDisconnects++;
      pClient->disconnect();
      int loop = 0;

      while ((countDisconnects > 0 || pClient->isConnected()) && loop < 50) {
        logMessage(".");
        loop++;
        delay(100);
      }

      if (countDisconnects > 0 || pClient->isConnected())
      {
        if (debugNukiConnect) {
          logMessage("Error while disconnecting BLE client");
        }
        eventHandler->notify(EventType::BLE_ERROR_ON_DISCONNECT);
      }
    }
  }
}

void NukiBle::setDisconnectTimeout(uint32_t timeoutMs) {
  timeoutDuration = timeoutMs;
}

void NukiBle::setConnectTimeout(uint8_t timeout) {
  connectTimeoutSec = timeout;
}

void NukiBle::setConnectRetries(uint8_t retries) {
  connectRetries = retries;
}

void NukiBle::extendDisconnectTimeout() {
  #ifndef NUKI_64BIT_TIME
  lastStartTimeout = millis();
  lastHeartbeat = millis();
  #else
  lastStartTimeout = (esp_timer_get_time() / 1000);
  lastHeartbeat = (esp_timer_get_time() / 1000);
  #endif
}

#ifndef NUKI_USE_LATEST_NIMBLE
void NukiBle::onResult(BLEAdvertisedDevice* advertisedDevice) {
#else
void NukiBle::onResult(const BLEAdvertisedDevice* advertisedDevice) {
#endif
  if (isPaired) {
    if (bleAddress == advertisedDevice->getAddress()) {
      rssi = advertisedDevice->getRSSI();
      #ifndef NUKI_64BIT_TIME
      lastReceivedBeaconTs = millis();
      #else
      lastReceivedBeaconTs = (esp_timer_get_time() / 1000);
      #endif

      std::string manufacturerData = advertisedDevice->getManufacturerData();
      uint8_t* manufacturerDataPtr = (uint8_t*)manufacturerData.data();
      bool isKeyTurnerUUID = true;
      std::string serviceUUID = deviceServiceUUID.toString();

      #ifndef NUKI_USE_LATEST_NIMBLE
      char* pHex = BLEUtils::buildHexData(nullptr, manufacturerDataPtr, manufacturerData.length());

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
      #else
      std::string pHex = advertisedDevice->toString();
      serviceUUID.erase(std::remove(serviceUUID.begin(), serviceUUID.end(), '-'), serviceUUID.end());

      if (pHex.find(serviceUUID) == std::string::npos) {
        isKeyTurnerUUID = false;
      }
      #endif

      if (isKeyTurnerUUID) {
        if (debugNukiConnect) {
          logMessageVar("Nuki Advertising: %s", advertisedDevice->toString().c_str());
        }

        uint8_t cManufacturerData[100];
        manufacturerData.copy((char*)cManufacturerData, manufacturerData.length(), 0);

        if (manufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00) {
          BLEBeacon oBeacon = BLEBeacon();

          #ifndef NUKI_USE_LATEST_NIMBLE
          oBeacon.setData(manufacturerData);
          #else
          oBeacon.setData((uint8_t*)manufacturerData.c_str(), (uint8_t)manufacturerData.length());
          #endif
          if (debugNukiConnect) {
            if (logger == nullptr) {
              log_d("iBeacon ID: %04X Major: %d Minor: %d UUID: %s Power: %d", oBeacon.getManufacturerId(),
                  ENDIAN_CHANGE_U16(oBeacon.getMajor()), ENDIAN_CHANGE_U16(oBeacon.getMinor()),
                  oBeacon.getProximityUUID().toString().c_str(), oBeacon.getSignalPower());
            }
            else
            {
              logger->printf("iBeacon ID: %04X Major: %d Minor: %d UUID: %s Power: %d\r\n", oBeacon.getManufacturerId(),
                  ENDIAN_CHANGE_U16(oBeacon.getMajor()), ENDIAN_CHANGE_U16(oBeacon.getMinor()),
                  oBeacon.getProximityUUID().toString().c_str(), oBeacon.getSignalPower());
            }
          }
          #ifndef NUKI_64BIT_TIME
          lastHeartbeat = millis();
          #else
          lastHeartbeat = (esp_timer_get_time() / 1000);
          #endif
          if ((oBeacon.getSignalPower() & 0x01) > 0) {
            if (eventHandler) {
              eventHandler->notify(EventType::KeyTurnerStatusUpdated);
            }

            statusUpdated = true;
          }
          else if (statusUpdated)
          {
            statusUpdated = false;

            if (eventHandler) {
              eventHandler->notify(EventType::KeyTurnerStatusReset);
            }
          }
        }
      }
    }
  } else {
    if (advertisedDevice->haveServiceData()) {
      if (advertisedDevice->getServiceData(pairingServiceUUID) != "") {
        if (debugNukiConnect) {
          if (logger == nullptr) {
            log_d("Found nuki in pairing state: %s addr: %s", std::string(advertisedDevice->getName()).c_str(), std::string(advertisedDevice->getAddress()).c_str());
          }
          else
          {
            logger->printf("Found nuki in pairing state: %s addr: %s\r\n", std::string(advertisedDevice->getName()).c_str(), std::string(advertisedDevice->getAddress()).c_str());
          }
        }
        bleAddress = advertisedDevice->getAddress();
        pairingServiceAvailable = true;
        #ifndef NUKI_64BIT_TIME
        pairingLastSeen = millis();
        #else
        pairingLastSeen = (esp_timer_get_time() / 1000);
        #endif
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

  #ifndef NUKI_64BIT_TIME
  unsigned long timeNow = millis();
  #else
  int64_t timeNow = (esp_timer_get_time() / 1000);
  #endif
  Nuki::CmdResult result = executeAction(action);

  if (result == Nuki::CmdResult::Success) {
    //wait for return of Keypad Code Count (0x0044)
    while (!keypadCodeCountReceived) {
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > GENERAL_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > GENERAL_TIMEOUT) {
      #endif
        logMessage("Receive keypad count timeout", 2);
        return CmdResult::TimeOut;
      }
      delay(10);
    }
    if (debugNukiCommand) {
      logMessageVar("Keypad code count %d", getKeypadEntryCount());
    }

    //wait for return of Keypad Codes (0x0045)
    #ifndef NUKI_64BIT_TIME
    timeNow = millis();
    #else
    timeNow = (esp_timer_get_time() / 1000);
    #endif
    while (nrOfReceivedKeypadCodes < getKeypadEntryCount()) {
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > GENERAL_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > GENERAL_TIMEOUT) {
      #endif
        logMessage("Receive keypadcodes timeout", 2);
        return CmdResult::TimeOut;
      }
      delay(10);
    }
    if (debugNukiCommand) {
      logMessageVar("%d codes received", nrOfReceivedKeypadCodes);
    }
  } else {
    logMessage("Retrieve keypad codes from lock failed", 2);
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
    if (debugNukiReadableData) {
      logMessageVar("addKeyPadEntry, payloadlen: %d", sizeof(NewKeypadEntry));
      printBuffer(action.payload, sizeof(NewKeypadEntry), false, "addKeyPadCode content: ", debugNukiHexData, logger);
      NukiLock::logNewKeypadEntry(newKeypadEntry, debugNukiReadableData, logger);
    }
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
    if (debugNukiReadableData) {
      logMessageVar("addKeyPadEntry, payloadlen: %d", sizeof(UpdatedKeypadEntry));
      printBuffer(action.payload, sizeof(UpdatedKeypadEntry), false, "updatedKeypad content: ", debugNukiHexData, logger);
      NukiLock::logUpdatedKeypadEntry(updatedKeyPadEntry, debugNukiReadableData, logger);
    }
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
    if (debugNukiReadableData) {
      logMessageVar("addAuthorizationEntry, payloadlen: %d", sizeof(NewAuthorizationEntry));
      printBuffer(action.payload, sizeof(NewAuthorizationEntry), false, "addAuthorizationEntry content: ", debugNukiHexData, logger);
      NukiLock::logNewAuthorizationEntry(newAuthorizationEntry, debugNukiReadableData, logger);
    }
  }
  return result;
}

Nuki::CmdResult NukiBle::deleteAuthorizationEntry(uint32_t id) {
  NukiLock::Action action;
  unsigned char payload[4] = {0};
  memcpy(payload, &id, 4);

  action.cmdType = CommandType::CommandWithChallengeAndPin;
  action.command = Command::RemoveUserAuthorization;
  memcpy(action.payload, &payload, sizeof(payload));
  action.payloadLen = sizeof(payload);

  return executeAction(action);
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
    if (debugNukiReadableData) {
      logMessageVar("addAuthorizationEntry, payloadlen: %d", sizeof(UpdatedAuthorizationEntry));
      printBuffer(action.payload, sizeof(UpdatedAuthorizationEntry), false, "updatedKeypad content: ", debugNukiHexData, logger);
      NukiLock::logUpdatedAuthorizationEntry(updatedAuthorizationEntry, debugNukiReadableData, logger);
    }
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
    if (debugNukiReadableData) {
      logMessage("Verify security pin code success");
    }
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
    if (debugNukiReadableData) {
      logMessage("Calibration executed");
    }
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
    if (debugNukiReadableData) {
      logMessage("Reboot executed");
    }
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
    if (debugNukiReadableData) {
      if (logger == nullptr) {
        log_d("Time set: %d-%d-%d %d:%d:%d", time.year, time.month, time.day, time.hour, time.minute, time.second);
      }
      else
      {
        logger->printf("Time set: %d-%d-%d %d:%d:%d\r\n", time.year, time.month, time.day, time.hour, time.minute, time.second);
      }
    }
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
  #ifdef NUKI_USE_LATEST_NIMBLE
  currentBleAddress[0] = bleAddress.getVal()[5];
  currentBleAddress[1] = bleAddress.getVal()[4];
  currentBleAddress[2] = bleAddress.getVal()[3];
  currentBleAddress[3] = bleAddress.getVal()[2];
  currentBleAddress[4] = bleAddress.getVal()[1];
  currentBleAddress[5] = bleAddress.getVal()[0];
  #else
  currentBleAddress[0] = bleAddress.getNative()[5];
  currentBleAddress[1] = bleAddress.getNative()[4];
  currentBleAddress[2] = bleAddress.getNative()[3];
  currentBleAddress[3] = bleAddress.getNative()[2];
  currentBleAddress[4] = bleAddress.getNative()[1];
  currentBleAddress[5] = bleAddress.getNative()[0];
  #endif
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
    if (debugNukiConnect) {
      logMessage("Credentials saved:");
      printBuffer(secretKeyK, sizeof(secretKeyK), false, SECRET_KEY_STORE_NAME, debugNukiHexData, logger);
      printBuffer(currentBleAddress, 6, false, BLE_ADDRESS_STORE_NAME, debugNukiHexData, logger);
      printBuffer(authorizationId, sizeof(authorizationId), false, AUTH_ID_STORE_NAME, debugNukiHexData, logger);
      logMessageVar("pincode: %d", pinCode);
    }
  } else {
    logMessage("ERROR saving credentials", 1);
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
      BLEAddress address = BLEAddress(buf, 0);
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
      bleAddress = BLEAddress(buff, 0);

      if (debugNukiConnect) {
        logMessageVar("[%s] Credentials retrieved :", deviceName.c_str());
        printBuffer(secretKeyK, sizeof(secretKeyK), false, SECRET_KEY_STORE_NAME, debugNukiHexData, logger);
        logMessageVar("bleAddress: %s", bleAddress.toString().c_str());
        printBuffer(authorizationId, sizeof(authorizationId), false, AUTH_ID_STORE_NAME, debugNukiHexData, logger);
      }

      if (isCharArrayEmpty(secretKeyK, sizeof(secretKeyK)) || isCharArrayEmpty(authorizationId, sizeof(authorizationId))) {
        logMessage("secret key OR authorizationId is empty: not paired", 2);
        giveNukiBleSemaphore();
        return false;
      }

      if (pinCode == 0) {
        logMessage("Pincode is 000000, probably not defined", 2);
      }

    } else {
      logMessage("Error getting data from NVS", 1);
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
  if (debugNukiConnect) {
    logMessage("Credentials deleted");
  }
}

PairingState NukiBle::pairStateMachine(const PairingState nukiPairingState) {
  switch (nukiPairingState) {
    case PairingState::InitPairing: {
      memset(challengeNonceK, 0, sizeof(challengeNonceK));
      memset(remotePublicKey, 0, sizeof(remotePublicKey));
      receivedStatus = 0xff;
      #ifndef NUKI_64BIT_TIME
      timeNow = millis();
      #else
      timeNow = (esp_timer_get_time() / 1000);
      #endif
      nukiPairingResultState = PairingState::ReqRemPubKey;
    }
    case PairingState::ReqRemPubKey: {
      //Request remote public key (Sent message should be 0100030027A7)
      if (debugNukiConnect) {
        logMessage("##################### REQUEST REMOTE PUBLIC KEY #########################");
      }
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
      if (debugNukiConnect) {
        logMessage("##################### SEND CLIENT PUBLIC KEY #########################");
      }
      sendPlainMessage(Command::PublicKey, myPublicKey, sizeof(myPublicKey));
      nukiPairingResultState = PairingState::GenKeyPair;
    }
    case PairingState::GenKeyPair: {
      if (debugNukiConnect) {
        logMessage("##################### CALCULATE DH SHARED KEY s #########################");
      }
      unsigned char sharedKeyS[32] = {0x00};
      crypto_scalarmult_curve25519(sharedKeyS, myPrivateKey, remotePublicKey);
      printBuffer(sharedKeyS, sizeof(sharedKeyS), false, "Shared key s", debugNukiHexData, logger);

      if (debugNukiConnect) {
        logMessage("##################### DERIVE LONG TERM SHARED SECRET KEY k #########################");
      }
      unsigned char in[16];
      memset(in, 0, 16);
      unsigned char sigma[] = "expand 32-byte k";
      crypto_core_hsalsa20(secretKeyK, in, sharedKeyS, sigma);
      printBuffer(secretKeyK, sizeof(secretKeyK), false, "Secret key k", debugNukiHexData, logger);
      nukiPairingResultState = PairingState::CalculateAuth;
    }
    case PairingState::CalculateAuth: {
      if (isCharArrayNotEmpty(challengeNonceK, sizeof(challengeNonceK))) {
        if (debugNukiConnect) {
          logMessage("##################### CALCULATE/VERIFY AUTHENTICATOR #########################");
        }
        //concatenate local public key, remote public key and receive challenge data
        unsigned char hmacPayload[96];
        memcpy(&hmacPayload[0], myPublicKey, sizeof(myPublicKey));
        memcpy(&hmacPayload[32], remotePublicKey, sizeof(remotePublicKey));
        memcpy(&hmacPayload[64], challengeNonceK, sizeof(challengeNonceK));
        printBuffer((byte*)hmacPayload, sizeof(hmacPayload), false, "Concatenated data r", debugNukiHexData, logger);
        crypto_auth_hmacsha256(authenticator, hmacPayload, sizeof(hmacPayload), secretKeyK);
        printBuffer(authenticator, sizeof(authenticator), false, "HMAC 256 result", debugNukiHexData, logger);
        memset(challengeNonceK, 0, sizeof(challengeNonceK));
        nukiPairingResultState = PairingState::SendAuth;
      }
      break;
    }
    case PairingState::SendAuth: {
      if (debugNukiConnect) {
        logMessage("##################### SEND AUTHENTICATOR #########################");
      }
      sendPlainMessage(Command::AuthorizationAuthenticator, authenticator, sizeof(authenticator));
      nukiPairingResultState = PairingState::SendAuthData;
    }
    case PairingState::SendAuthData: {
      if (isCharArrayNotEmpty(challengeNonceK, sizeof(challengeNonceK))) {
        if (debugNukiConnect) {
          logMessage("##################### SEND AUTHORIZATION DATA #########################");
        }
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
        generateNonce(authorizationDataNonce, sizeof(authorizationDataNonce), debugNukiHexData);

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
        if (debugNukiConnect) {
          logMessage("##################### SEND AUTHORIZATION ID confirmation #########################");
        }
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
        if (debugNukiConnect) {
          logMessage("####################### PAIRING DONE ###############################################");
        }
        nukiPairingResultState = PairingState::Success;
      }
      break;
    }
    default: {
      logMessage("Unknown pairing status", 1);
      nukiPairingResultState = PairingState::Timeout;
    }
  }

  #ifndef NUKI_64BIT_TIME
  if (millis() - timeNow > PAIRING_TIMEOUT) {
  #else
  if ((esp_timer_get_time() / 1000) - timeNow > PAIRING_TIMEOUT) {
  #endif
    logMessage("Pairing timeout", 2);
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

  if (debugNukiHexData) {
    logMessageVar("payloadlen: %d", payloadLen);
    logMessageVar("sizeof(plainData): %d", sizeof(plainData));
    logMessageVar("CRC: %0.2x", dataCrc);
  }
  printBuffer((byte*)plainDataWithCrc, sizeof(plainDataWithCrc), false, "Plain data with CRC: ", debugNukiHexData, logger);

  //compose additional data
  unsigned char additionalData[30] = {};
  generateNonce(sentNonce, sizeof(sentNonce), debugNukiHexData);

  memcpy(&additionalData[0], sentNonce, sizeof(sentNonce));
  memcpy(&additionalData[24], authorizationId, sizeof(authorizationId));

  //Encrypt plain data
  unsigned char plainDataEncr[ sizeof(plainDataWithCrc) + crypto_secretbox_MACBYTES] = {0};
  int encrMsgLen = encode(plainDataEncr, plainDataWithCrc, sizeof(plainDataWithCrc), sentNonce, secretKeyK, logger);

  if (encrMsgLen >= 0) {
    int16_t length = sizeof(plainDataEncr);
    memcpy(&additionalData[28], &length, 2);

    printBuffer((byte*)additionalData, 30, false, "Additional data: ", debugNukiHexData, logger);
    printBuffer((byte*)secretKeyK, sizeof(secretKeyK), false, "Encryption key (secretKey): ", debugNukiHexData, logger);
    printBuffer((byte*)plainDataEncr, sizeof(plainDataEncr), false, "Plain data encrypted: ", debugNukiHexData, logger);

    //compose complete message
    unsigned char dataToSend[sizeof(additionalData) + sizeof(plainDataEncr)] = {};
    memcpy(&dataToSend[0], additionalData, sizeof(additionalData));
    memcpy(&dataToSend[30], plainDataEncr, sizeof(plainDataEncr));

    if (connectBle(bleAddress, false)) {
      printBuffer((byte*)dataToSend, sizeof(dataToSend), false, "Sending encrypted message", debugNukiHexData, logger);
      return pUsdioCharacteristic->writeValue((uint8_t*)dataToSend, sizeof(dataToSend), true);
    } else {
      logMessage("Send encr msg failed due to unable to connect", 2);
    }
  } else {
    logMessage("Send msg failed due to encryption fail", 2);
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
  printBuffer((byte*)dataToSend, payloadLen + 4, false, "Sending plain message", debugNukiHexData, logger);
  if (debugNukiHexData) {
    if (logger == nullptr) {
      log_d("Command identifier: %02x, CRC: %04x", (uint32_t)commandIdentifier, dataCrc);
    }
    else
    {
      logger->printf("Command identifier: %02x, CRC: %04x\r\n", (uint32_t)commandIdentifier, dataCrc);
    }
  }

  if (connectBle(bleAddress, true)) {
    return pGdioCharacteristic->writeValue((uint8_t*)dataToSend, payloadLen + 4, true);
  } else {
    logMessage("Send plain msg failed due to unable to connect", 2);
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
        #ifdef NUKI_USE_LATEST_NIMBLE
        NimBLERemoteCharacteristic::notify_callback callback = std::bind(&NukiBle::notifyCallback, this, _1, _2, _3, _4);
        #else
        notify_callback callback = std::bind(&NukiBle::notifyCallback, this, _1, _2, _3, _4);
        #endif
        if(!pGdioCharacteristic->subscribe(false, callback, true)) {
          logMessage("Unable to subscribe to GDIO characteristic", 2);
          refreshServices = true;
          disconnect();
          return false;
        }
        if (debugNukiCommunication) {
          logMessage("GDIO characteristic registered");
        }
        delay(100);
        return true;
      } else {
        if (debugNukiCommunication) {
          logMessage("GDIO characteristic canIndicate false, stop connecting");
        }
        refreshServices = true;
        disconnect();
        return false;
      }
    } else {
      logMessage("Unable to get GDIO characteristic", 2);
      refreshServices = true;
      disconnect();
      return false;
    }
  } else {
    logMessage("Unable to get keyturner pairing service", 2);
    refreshServices = true;
    disconnect();
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
        #ifdef NUKI_USE_LATEST_NIMBLE
        NimBLERemoteCharacteristic::notify_callback callback = std::bind(&NukiBle::notifyCallback, this, _1, _2, _3, _4);
        #else
        notify_callback callback = std::bind(&NukiBle::notifyCallback, this, _1, _2, _3, _4);
        #endif
        if(!pUsdioCharacteristic->subscribe(false, callback, true)) {
          logMessage("Unable to subscribe to USDIO characteristic", 2);
          refreshServices = true;
          disconnect();
          return false;
        }
        if (debugNukiCommunication) {
          logMessage("USDIO characteristic registered");
        }
        delay(100);
        return true;
      } else {
        if (debugNukiCommunication) {
          logMessage("USDIO characteristic canIndicate false, stop connecting");
        }
        refreshServices = true;
        disconnect();
        return false;
      }
    } else {
      logMessage("Unable to get USDIO characteristic", 2);
      refreshServices = true;
      disconnect();
      return false;
    }
  } else {
    logMessage("Unable to get keyturner data service", 2);
    refreshServices = true;
    disconnect();
    return false;
  }

  return false;
}

void NukiBle::notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* recData, size_t length, bool isNotify) {
  #ifndef NUKI_64BIT_TIME
  lastHeartbeat = millis();
  #else
  lastHeartbeat = (esp_timer_get_time() / 1000);
  #endif
  if (debugNukiCommunication) {
    if (logger == nullptr) {
      log_d("Notify callback for characteristic: %s of length: %d", pBLERemoteCharacteristic->getUUID().toString().c_str(), length);
    }
    else
    {
      logger->printf("Notify callback for characteristic: %s of length: %d\r\n", pBLERemoteCharacteristic->getUUID().toString().c_str(), length);
    }
  }
  printBuffer((byte*)recData, length, false, "Received data", debugNukiHexData, logger);

  if (pBLERemoteCharacteristic->getUUID() == gdioUUID) {
    //handle not encrypted msg
    uint16_t returnCode = ((uint16_t)recData[1] << 8) | recData[0];
    crcCheckOke = crcValid(recData, length, debugNukiCommunication, logger);
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
    decode(decrData, encrData, encrMsgLen, recNonce, secretKeyK, logger);

    if (debugNukiCommunication) {
      logMessageVar("Received encrypted msg, len: %d", encrMsgLen);
    }
    printBuffer(recNonce, sizeof(recNonce), false, "received nonce", debugNukiHexData, logger);
    printBuffer(recAuthorizationId, sizeof(recAuthorizationId), false, "Received AuthorizationId", debugNukiHexData, logger);
    printBuffer(encrData, sizeof(encrData), false, "Rec encrypted data", debugNukiHexData, logger);
    printBuffer(decrData, sizeof(decrData), false, "Decrypted data", debugNukiHexData, logger);

    crcCheckOke = crcValid(decrData, sizeof(decrData), debugNukiCommunication, logger);
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
      if (debugNukiCommunication) {
        logMessage("requestData");
      }
      break;
    }
    case Command::PublicKey : {
      memcpy(remotePublicKey, data, 32);
      printBuffer(remotePublicKey, sizeof(remotePublicKey), false,  "Remote public key", debugNukiHexData, logger);
      break;
    }
    case Command::Challenge : {
      memcpy(challengeNonceK, data, 32);
      printBuffer((byte*)data, dataLen, false, "Challenge", debugNukiHexData, logger);
      break;
    }
    case Command::AuthorizationAuthenticator : {
      printBuffer((byte*)data, dataLen, false, "authorizationAuthenticator", debugNukiHexData, logger);
      break;
    }
    case Command::AuthorizationData : {
      printBuffer((byte*)data, dataLen, false, "authorizationData", debugNukiHexData, logger);
      break;
    }
    case Command::AuthorizationId : {
      unsigned char lockId[16];
      printBuffer((byte*)data, dataLen, false, "authorizationId data", debugNukiHexData, logger);
      memcpy(authorizationId, &data[32], 4);
      memcpy(lockId, &data[36], sizeof(lockId));
      memcpy(challengeNonceK, &data[52], sizeof(challengeNonceK));
      printBuffer(authorizationId, sizeof(authorizationId), false, AUTH_ID_STORE_NAME);
      printBuffer(lockId, sizeof(lockId), false, "lockId");
      break;
    }
    case Command::AuthorizationEntry : {
      printBuffer((byte*)data, dataLen, false, "authorizationEntry", debugNukiHexData, logger);
      AuthorizationEntry authEntry;
      memcpy(&authEntry, data, sizeof(authEntry));
      listOfAuthorizationEntries.push_back(authEntry);
      if (debugNukiReadableData) {
        NukiLock::logAuthorizationEntry(authEntry, true, logger);
      }
      break;
    }

    case Command::Status : {
      printBuffer((byte*)data, dataLen, false, "status", debugNukiHexData, logger);
      receivedStatus = data[0];
      if (debugNukiCommunication) {
        if (receivedStatus == 0) {
          logMessage("command COMPLETE");
        } else if (receivedStatus == 1) {
          logMessage("command ACCEPTED");
        }
      }
      break;
    }
    case Command::OpeningsClosingsSummary : {
      printBuffer((byte*)data, dataLen, false, "openingsClosingsSummary", debugNukiHexData, logger);
      logMessage("NOT IMPLEMENTED ONLY FOR NUKI v1", 2); //command is not available on Nuki v2 (only on Nuki v1)
      break;
    }

    case Command::ErrorReport : {
      if (logger == nullptr) {
        log_e("Error: %02x for command: %02x:%02x", data[0], data[2], data[1]);
      }
      else
      {
        logger->printf("Error: %02x for command: %02x:%02x\r\n", data[0], data[2], data[1]);
      }
      memcpy(&errorCode, &data[0], sizeof(errorCode));
      logErrorCode(data[0]);
      if ((uint8_t)data[0] == (uint8_t)0x21) {
        if (eventHandler) {
          eventHandler->notify(EventType::ERROR_BAD_PIN);
        }
      }
      break;
    }
    case Command::AuthorizationIdConfirmation : {
      printBuffer((byte*)data, dataLen, false, "authorizationIdConfirmation", debugNukiHexData, logger);
      break;
    }
    case Command::AuthorizationIdInvite : {
      printBuffer((byte*)data, dataLen, false, "authorizationIdInvite", debugNukiHexData, logger);
      break;
    }
    case Command::AuthorizationEntryCount : {
      printBuffer((byte*)data, dataLen, false, "authorizationEntryCount", debugNukiHexData, logger);
      uint16_t count = 0;
      memcpy(&count, data, 2);
      logMessageVar("authorizationEntryCount: %d", count);
      break;
    }
    case Command::LogEntryCount : {
      memcpy(&loggingEnabled, data, sizeof(logEntryCount));
      memcpy(&logEntryCount, &data[1], sizeof(logEntryCount));
      if (debugNukiReadableData) {
        if (logger == nullptr) {
          log_d("Logging enabled: %d, total nr of log entries: %d", loggingEnabled, logEntryCount);
        }
        else
        {
          logger->printf("Logging enabled: %d, total nr of log entries: %d\r\n", loggingEnabled, logEntryCount);
        }
      }
      printBuffer((byte*)data, dataLen, false, "logEntryCount", debugNukiHexData, logger);
      break;
    }
    case Command::TimeControlEntryCount : {
      printBuffer((byte*)data, dataLen, false, "timeControlEntryCount", debugNukiHexData, logger);
      break;
    }
    case Command::KeypadCodeId : {
      printBuffer((byte*)data, dataLen, false, "keypadCodeId", debugNukiHexData, logger);
      break;
    }
    case Command::KeypadCodeCount : {
      memcpy(&nrOfKeypadCodes, data, 2);
      keypadCodeCountReceived = true;
      printBuffer((byte*)data, dataLen, false, "keypadCodeCount", debugNukiHexData, logger);
      if (debugNukiReadableData) {
        uint16_t count = 0;
        memcpy(&count, data, 2);
        logMessageVar("keyPadCodeCount: %d", count);
      }

      break;
    }
    case Command::KeypadCode : {
      KeypadEntry keypadEntry;
      memcpy(&keypadEntry, data, sizeof(KeypadEntry));
      listOfKeyPadEntries.push_back(keypadEntry);
      nrOfReceivedKeypadCodes++;

      printBuffer((byte*)data, dataLen, false, "keypadCode", debugNukiHexData, logger);
      if (debugNukiReadableData) {
        NukiLock::logKeypadEntry(keypadEntry, true, logger);
      }
      break;
    }
    case Command::KeypadAction : {
      printBuffer((byte*)data, dataLen, false, "keypadAction", debugNukiHexData, logger);
      break;
    }
    default:
      logMessageVar("UNKNOWN RETURN COMMAND: %04x", (unsigned int)returnCode, 1);
  }
}

void NukiBle::onConnect(BLEClient*) {
  extendDisconnectTimeout();
  if (debugNukiConnect) {
    logMessage("BLE connected");
  }
};

#ifdef NUKI_USE_LATEST_NIMBLE
void NukiBle::onDisconnect(BLEClient*, int reason)
#else
void NukiBle::onDisconnect(BLEClient*)
#endif
{
  if (debugNukiConnect) {
    logMessage("BLE disconnected");
  }
  countDisconnects = 0;
};

void NukiBle::setEventHandler(SmartlockEventHandler* handler) {
  eventHandler = handler;
}

const bool NukiBle::isPairedWithLock() const {
  return isPaired;
};

bool NukiBle::takeNukiBleSemaphore(std::string taker) {
  #ifndef NUKI_MUTEX_RECURSIVE
  bool result = xSemaphoreTake(nukiBleSemaphore, NUKI_SEMAPHORE_TIMEOUT / portTICK_PERIOD_MS) == pdTRUE;
  #else
  bool result = xSemaphoreTakeRecursive(nukiBleSemaphore, NUKI_SEMAPHORE_TIMEOUT / portTICK_PERIOD_MS) == pdTRUE;
  #endif

  if (!result) {
    if (logger == nullptr) {
      log_d("%s FAILED to take Nuki semaphore. Owner %s", taker.c_str(), owner.c_str());
    }
    else
    {
      logger->printf("%s FAILED to take Nuki semaphore. Owner %s\r\n", taker.c_str(), owner.c_str());
    }
  } else {
    owner = taker;
  }

  return result;
}

void NukiBle::giveNukiBleSemaphore() {
  owner = "free";
  #ifndef NUKI_MUTEX_RECURSIVE
  xSemaphoreGive(nukiBleSemaphore);
  #else
  xSemaphoreGiveRecursive(nukiBleSemaphore);
  #endif
}

int NukiBle::getRssi() const {
  return rssi;
}

#ifndef NUKI_64BIT_TIME
unsigned long NukiBle::getLastReceivedBeaconTs() const {
#else
int64_t NukiBle::getLastReceivedBeaconTs() const {
#endif
  return lastReceivedBeaconTs;
}

#ifndef NUKI_64BIT_TIME
unsigned long NukiBle::getLastHeartbeat() {
#else
int64_t NukiBle::getLastHeartbeat() {
#endif
  return lastHeartbeat;
}

const BLEAddress NukiBle::getBleAddress() const {
  return bleAddress;
}

void NukiBle::setDebugConnect(bool enable) {
  debugNukiConnect = enable;
}

void NukiBle::setDebugCommunication(bool enable) {
  debugNukiCommunication = enable;
}

void NukiBle::setDebugReadableData(bool enable) {
  debugNukiReadableData = enable;
}

void NukiBle::setDebugHexData(bool enable) {
  debugNukiHexData = enable;
}

void NukiBle::setDebugCommand(bool enable) {
  debugNukiCommand = enable;
}

void NukiBle::registerLogger(Print* Log) {
  logger = Log;
}

void NukiBle::logMessage(const char* message, int level) {
  if (logger == nullptr) {
    switch (level) {
      case 1:
        esp_log_write(ESP_LOG_ERROR, "NukiBle", message);
        break;
      case 2:
        esp_log_write(ESP_LOG_WARN, "NukiBle", message);
        break;
      case 3:
        esp_log_write(ESP_LOG_INFO, "NukiBle", message);
        break;
      case 4:
      default:
        esp_log_write(ESP_LOG_DEBUG, "NukiBle", message);
        break;
    }
  }
  else
  {
    logger->println(message);
  }
}

void NukiBle::logMessageVar(const char* message, unsigned int var, int level) {
  if (logger == nullptr) {
    switch (level) {
      case 1:
        esp_log_write(ESP_LOG_ERROR, "NukiBle", message, var);
        break;
      case 2:
        esp_log_write(ESP_LOG_WARN, "NukiBle", message, var);
        break;
      case 3:
        esp_log_write(ESP_LOG_INFO, "NukiBle", message, var);
        break;
      case 4:
      default:
        esp_log_write(ESP_LOG_DEBUG, "NukiBle", message, var);
        break;
    }
  }
  else
  {
    logger->printf(message, var);
    logger->println();
  }
}

void NukiBle::logMessageVar(const char* message, const char* var, int level) {
  if (logger == nullptr) {
    switch (level) {
      case 1:
        esp_log_write(ESP_LOG_ERROR, "NukiBle", message, var);
        break;
      case 2:
        esp_log_write(ESP_LOG_WARN, "NukiBle", message, var);
        break;
      case 3:
        esp_log_write(ESP_LOG_INFO, "NukiBle", message, var);
        break;
      case 4:
      default:
        esp_log_write(ESP_LOG_DEBUG, "NukiBle", message, var);
        break;
    }
  }
  else
  {
    logger->printf(message, var);
    logger->println();
  }
}

} // namespace Nuki