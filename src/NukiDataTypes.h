#pragma once
/**
 * @file NukiDataTypes.cpp
 * Definitions of data types
 *
 * Created on: 2022
 * License: GNU GENERAL PUBLIC LICENSE (see LICENSE)
 *
 * This library implements the communication from an ESP32 via BLE to a Nuki smart lock.
 * Based on the Nuki Smart Lock API V2.2.1
 * https://developer.nuki.io/page/nuki-smart-lock-api-2/2/
 *
 */

#include "Arduino.h"
#include "NukiConstants.h"

namespace Nuki {

enum class EventType {
  KeyTurnerStatusUpdated
};

class SmartlockEventHandler {
  public:
    virtual ~SmartlockEventHandler() {};
    virtual void notify(EventType eventType) = 0;
};

enum CmdResult : uint8_t {
  Success   = 1,
  Failed    = 2,
  TimeOut   = 3,
  Working   = 4,
  NotPaired = 5,
  Error     = 99
};

enum class PairingResult : uint8_t {
  Pairing,
  Success,
  Timeout
};

enum class PairingState {
  InitPairing       = 0,
  ReqRemPubKey      = 1,
  RecRemPubKey      = 2,
  SendPubKey        = 3,
  GenKeyPair        = 4,
  CalculateAuth     = 5,
  SendAuth          = 6,
  SendAuthData      = 7,
  SendAuthIdConf    = 8,
  RecStatus         = 9,
  Success           = 10,
  Timeout           = 99
};

enum class CommandState {
  Idle                  = 0,
  CmdReceived           = 1,
  ChallengeSent         = 2,
  ChallengeRespReceived = 3,
  CmdSent               = 4,
  CmdAccepted           = 5,
  TimeOut               = 6
};

enum class CommandType {
  Command                       = 0,
  CommandWithChallenge          = 1,
  CommandWithChallengeAndAccept = 2,
  CommandWithChallengeAndPin    = 3
};

struct Action {
  CommandType cmdType;
  Command command;
  unsigned char payload[100] {0};
  uint8_t payloadLen = 0;
};

} // namespace Nuki
