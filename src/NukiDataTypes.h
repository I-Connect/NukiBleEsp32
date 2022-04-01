#pragma once

#include "Arduino.h"
#include "NukiConstants.h"

enum class NukiEventType {
  KeyTurnerStatusUpdated
};

class NukiSmartlockEventHandler {
  public:
    virtual ~NukiSmartlockEventHandler() {};
    virtual void notify(NukiEventType eventType) = 0;
};

enum NukiCmdResult : uint8_t {
  success   = 1,
  failed    = 2,
  timeOut   = 3,
  working   = 4,
  notPaired = 5
};

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


