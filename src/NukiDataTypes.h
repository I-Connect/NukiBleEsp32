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
  Success   = 1,
  Failed    = 2,
  TimeOut   = 3,
  Working   = 4,
  NotPaired = 5
};

enum class NukiPairingState {
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
  Success           = 10
};

enum class NukiCommandState {
  Idle                  = 0,
  CmdReceived           = 1,
  ChallengeSent         = 2,
  ChallengeRespReceived = 3,
  CmdSent               = 4,
  CmdAccepted           = 5,
  TimeOut               = 6
};

enum class NukiCommandType {
  Command                       = 0,
  CommandWithChallenge          = 1,
  CommandWithChallengeAndAccept = 2,
  CommandWithChallengeAndPin    = 3
};

struct NukiAction {
  NukiCommandType cmdType;
  NukiCommand command;
  unsigned char payload[100] {0};
  uint8_t payloadLen = 0;
};


