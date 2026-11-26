#!/usr/bin/env python3
"""Runs the command state machines of NukiBle.hpp on this computer, against a scripted device.

    python3 test/command_state_machine/run.py

Needs g++, no ESP32 and no Nuki device. NukiBle.hpp is compiled as it is; the few functions of
NukiBle.cpp that belong to it (asking for a challenge ahead, what follows a refusal) are taken
from the source by name, so the test runs the code that ships.
"""
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"

FROM_CPP = ["setChallengePrefetch", "challengePrefetchActive", "prefetchChallenge",
            "requestChallengeAhead", "afterRefusal"]


def function(text, name):
    match = re.search(r"^[\w:<> ]+ NukiBle::%s\([^)]*\)( const)? \{\n.*?^\}\n" % name, text, re.S | re.M)
    assert match, name
    return match.group(0)


STUBS = {
    "Arduino.h": r'''
#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
typedef uint8_t byte;
extern int64_t nowMs;
inline unsigned long millis() { return (unsigned long) nowMs; }
inline int64_t esp_timer_get_time() { return nowMs * 1000; }
void delay(int ms);
inline void esp_task_wdt_reset() {}
typedef void* SemaphoreHandle_t;
#define pdMS_TO_TICKS(ms) (ms)
int xSemaphoreTake(SemaphoreHandle_t, int ms);
''',
    "NimBLEUUID.h": r'''
#pragma once
struct NimBLEUUID { NimBLEUUID(const char*) {} NimBLEUUID() {} };
''',
}

HARNESS = r'''
#include <cassert>
#include <deque>
#include <functional>
#include <string>
#include <vector>
#include "Arduino.h"
#include "NukiConstants.h"
#include "NukiDataTypes.h"

int64_t nowMs = 1000;

namespace Nuki {
struct Action {
  CommandType cmdType = CommandType::CommandWithChallengeAndPin;
  Command command = Command::UpdateKeypadCode;
  unsigned char payload[100] = {0};
  uint8_t payloadLen = 1;
};

// The members NukiBle.hpp uses, and a device that answers by script
class NukiBle {
 public:
  template <typename TDeviceAction> CmdResult executeAction(const TDeviceAction action);
  template <typename TDeviceAction> CmdResult cmdStateMachine(const TDeviceAction action);
  template <typename TDeviceAction> CmdResult cmdChallStateMachine(const TDeviceAction action, const bool sendPinCode = false);
  template <typename TDeviceAction> CmdResult cmdChallAccStateMachine(const TDeviceAction action);
  template <typename TDeviceAction> static bool isExpectedResponse(const TDeviceAction& action, const Command received);

  void setChallengePrefetch(bool enable);
  bool challengePrefetchActive() const;
  void prefetchChallenge(bool refresh = false);
  void requestChallengeAhead();
  void afterRefusal();
  static bool isChangeCommand(Command command) {
    switch (command) {
      case Command::AddKeypadCode: case Command::UpdateKeypadCode: case Command::RemoveKeypadCode:
      case Command::UpdateAuthorization: case Command::RemoveUserAuthorization:
      case Command::AddTimeControlEntry: case Command::UpdateTimeControlEntry: case Command::RemoveTimeControlEntry:
        return true;
      default: return false;
    }
  }

  bool isPairedWithLock() { return true; }
  bool retrieveCredentials() { return true; }
  bool isLockUltra() { return false; }
  void extendDisconnectTimeout() {}
  void logMessage(const char*, int = 4) {}
  void logMessageVar(const char*, unsigned int, int = 4) {}
  bool isConnected() { return connected; }
  void disconnect() { connected = false; disconnects++; challengePrefetch = ChallengePrefetch::None; }

  // What goes out: the command, and for a command with a challenge the challenge it carries
  struct Sent { Command command; uint8_t challenge; bool isChallengeRequest; };
  std::vector<Sent> sent;
  bool sendEncryptedMessage(Command command, const unsigned char* payload, const uint8_t payloadLen) {
    if (!challengePrefetchSending && command != Command::RequestData) {
      challengePrefetch = ChallengePrefetch::None;
    }
    connected = true;
    const bool challengeRequest = command == Command::RequestData && payloadLen == 2 && payload[0] == 0x04;
    uint8_t carried = 0;
    if (command != Command::RequestData && payloadLen >= 33) {
      carried = payload[payloadLen - 32 - 2];  // payload, challenge (32), pin (2)
    }
    sent.push_back({command, carried, challengeRequest});
    if (onSend) onSend(sent.back());
    return true;
  }
  std::function<void(const Sent&)> onSend;

  // What NukiBle::notifyCallback() and handleReturnMessage() do with a message
  void receive(Command command, uint8_t value = 0) {
    if (command == Command::Challenge) {
      memset(challengeNonceK, value, sizeof(challengeNonceK));
      if (challengePrefetch == ChallengePrefetch::Sent) challengePrefetch = ChallengePrefetch::Ready;
    } else if (command == Command::ErrorReport) {
      errorCode = value;
      errorReportSeen = true;
    } else if (command == Command::Status) {
      receivedStatus = value;
      if (value == 0) statusCompleteSeen = true; else if (value == 1) statusAcceptedSeen = true;
    }
    lastMsgCodeReceived = command;
    crcCheckOke = true;
  }

  bool debugNukiConnect = false, debugNukiCommunication = false;
  bool connected = false;
  int disconnects = 0;
  bool isPaired = true;
  CommandState nukiCommandState = CommandState::Idle;
  Command lastMsgCodeReceived = Command::Empty;
  volatile bool errorReportSeen = false;
  volatile bool statusAcceptedSeen = false, statusCompleteSeen = false;
  bool linkOkAfterFailure = false;
  uint8_t errorCode = 0;
  uint8_t receivedStatus = 0xff;
  bool crcCheckOke = false;
  unsigned char challengeNonceK[32] = {0};
  uint16_t pinCode = 0;
  uint32_t ultraPinCode = 0;
  int64_t timeNow = 0;
  uint32_t commandTimeoutDuration = 3000;
  uint32_t generalTimeoutDuration = 10000;
  SemaphoreHandle_t responseSignal = (void*) 1;
  enum class ChallengePrefetch : uint8_t { None, Sent, Ready };
  volatile ChallengePrefetch challengePrefetch = ChallengePrefetch::None;
  bool challengePrefetchEnabled = false, challengePrefetchSending = false, challengePrefetchUsed = false;
  uint8_t challengePrefetchStrikes = 0;
  int64_t challengePrefetchAt = 0;
};
}  // namespace Nuki

#define NUKI_64BIT_TIME
#include "NukiBle.hpp"

namespace Nuki {
%(from_cpp)s
}  // namespace Nuki

using namespace Nuki;

// Every wait of a command for the device is one step of the script: a message, or time passing
static std::deque<std::function<void()>> script;
static int waits = 0;
int xSemaphoreTake(SemaphoreHandle_t, int ms) {
  waits++;
  assert(waits < 100000 && "the command never ends");
  if (!script.empty()) {
    auto step = script.front();
    script.pop_front();
    step();
  } else {
    nowMs += ms;
  }
  return 1;
}
void delay(int ms) { xSemaphoreTake(nullptr, ms); }

static NukiBle ble;
static uint8_t nextChallenge = 1;
// A device that answers a challenge request with the next challenge, and a command with `answer`
static void device(std::function<void(const NukiBle::Sent&)> answer) {
  ble.onSend = [answer](const NukiBle::Sent& message) {
    if (message.isChallengeRequest) {
      const uint8_t value = nextChallenge++;
      script.push_back([value] { ble.receive(Command::Challenge, value); });
    } else if (answer) {
      answer(message);
    }
  };
}
static void complete(const NukiBle::Sent&) {
  script.push_back([] { ble.receive(Command::Status, (uint8_t) CommandStatus::Complete); });
}
static std::function<void(const NukiBle::Sent&)> error(uint8_t code) {
  return [code](const NukiBle::Sent&) { script.push_back([code] { ble.receive(Command::ErrorReport, code); }); };
}
static void drain() { while (!script.empty()) xSemaphoreTake(nullptr, 0); }
static size_t challengeRequests() {
  size_t n = 0;
  for (auto& message : ble.sent) n += message.isChallengeRequest;
  return n;
}

int main(int argc, char** argv) {
  assert(argc == 2);
  const std::string scenario = argv[1];
  Action change;
  Action list; list.command = Command::RequestKeypadCodes;
  Action read; read.cmdType = CommandType::Command; read.command = Command::RequestData; read.payload[0] = 0x0C; read.payloadLen = 2;
  Action lockAction; lockAction.cmdType = CommandType::CommandWithChallengeAndAccept; lockAction.command = Command::LockAction;

  if (scenario == "plain") {
    // Without asking ahead: challenge, command, done; nothing sent afterwards
    device(complete);
    assert(ble.executeAction(change) == CmdResult::Success);
    assert(ble.sent.size() == 2 && ble.sent[0].isChallengeRequest && ble.sent[1].challenge == 1);
    assert(ble.executeAction(change) == CmdResult::Success);
    assert(ble.sent.size() == 4 && ble.sent[3].challenge == 2);
    return 0;
  }
  if (scenario == "ahead_ready") {
    // The challenge asked for after the first change is the one the second change carries
    ble.setChallengePrefetch(true);
    device(complete);
    assert(ble.executeAction(change) == CmdResult::Success);
    assert(ble.sent.size() == 3 && ble.sent[2].isChallengeRequest);
    drain();
    assert(ble.challengePrefetch == NukiBle::ChallengePrefetch::Ready);
    assert(ble.executeAction(change) == CmdResult::Success);
    assert(ble.sent[3].command == Command::UpdateKeypadCode && ble.sent[3].challenge == 2);
    assert(challengeRequests() == 3);  // one per command, none twice
    return 0;
  }
  if (scenario == "ahead_on_its_way") {
    // The second change comes before the challenge asked for ahead: it waits for that one
    ble.setChallengePrefetch(true);
    device(complete);
    assert(ble.executeAction(change) == CmdResult::Success);
    assert(ble.challengePrefetch == NukiBle::ChallengePrefetch::Sent);
    assert(ble.executeAction(change) == CmdResult::Success);
    assert(ble.sent[3].command == Command::UpdateKeypadCode && ble.sent[3].challenge == 2);
    return 0;
  }
  if (scenario == "refresh_crosses_command") {
    // A challenge is there, a fresh one is asked for (the keep-alive of a held connection) and
    // a command starts before it arrives: the command must carry the fresh one, the device
    // has forgotten the one before
    ble.setChallengePrefetch(true);
    device(complete);
    assert(ble.executeAction(change) == CmdResult::Success);
    drain();
    ble.prefetchChallenge(true);
    assert(ble.challengePrefetch == NukiBle::ChallengePrefetch::Sent);
    assert(ble.executeAction(change) == CmdResult::Success);
    assert(ble.sent.back().isChallengeRequest);
    const auto& command = ble.sent[ble.sent.size() - 2];
    assert(command.command == Command::UpdateKeypadCode && command.challenge == 3);
    return 0;
  }
  if (scenario == "read_keeps_challenge") {
    // A state read between two changes does not use up the challenge asked for ahead
    ble.setChallengePrefetch(true);
    device([](const NukiBle::Sent& message) {
      if (message.command == Command::RequestData) script.push_back([] { ble.receive(Command::KeyturnerStates); });
      else complete(message);
    });
    assert(ble.executeAction(change) == CmdResult::Success);
    assert(ble.executeAction(read) == CmdResult::Success);  // waits for the challenge on its way first
    assert(ble.challengePrefetch == NukiBle::ChallengePrefetch::Ready);
    assert(ble.executeAction(change) == CmdResult::Success);
    assert(challengeRequests() == 3);
    return 0;
  }
  if (scenario == "no_ahead_after_list") {
    // A list request is followed by its entries: nothing is asked in between
    ble.setChallengePrefetch(true);
    device([](const NukiBle::Sent&) { script.push_back([] { ble.receive(Command::KeypadCodeCount); }); });
    assert(ble.executeAction(list) == CmdResult::Success);
    assert(ble.sent.size() == 2);
    return 0;
  }
  if (scenario == "refused") {
    // The device says no to the code: failed, and the connection stays for the next command
    device(error(0x2A));
    assert(ble.executeAction(change) == CmdResult::Failed);
    assert(ble.disconnects == 0 && ble.connected);
    device(complete);
    assert(ble.executeAction(change) == CmdResult::Success);
    return 0;
  }
  if (scenario == "refused_connection") {
    // An error that says something about the connection closes it
    device(error(0x21));
    assert(ble.executeAction(change) == CmdResult::Failed);
    assert(ble.disconnects >= 1 && !ble.connected);
    return 0;
  }
  if (scenario == "error_then_message") {
    // The error report is followed by another message before the command looks: still failed,
    // not a wait for the timeout
    device([](const NukiBle::Sent&) {
      script.push_back([] { ble.receive(Command::ErrorReport, 0x2A); ble.receive(Command::Challenge, 9); });
    });
    const int64_t started = nowMs;
    assert(ble.executeAction(change) == CmdResult::Failed);
    assert(nowMs - started < 1000);
    return 0;
  }
  if (scenario == "ahead_not_taken") {
    // Two commands refused for their challenge: asking ahead switches itself off
    ble.setChallengePrefetch(true);
    device(complete);
    assert(ble.executeAction(change) == CmdResult::Success);
    drain();
    device(error(0x22));
    assert(ble.executeAction(change) == CmdResult::Failed);
    assert(ble.disconnects >= 1 && ble.challengePrefetchActive());
    device(complete);
    assert(ble.executeAction(change) == CmdResult::Success);
    drain();
    device(error(0x22));
    assert(ble.executeAction(change) == CmdResult::Failed);
    assert(!ble.challengePrefetchActive());
    device(complete);
    const size_t before = ble.sent.size();
    assert(ble.executeAction(change) == CmdResult::Success);
    assert(ble.sent.size() == before + 2);  // challenge and command, nothing ahead any more
    return 0;
  }
  if (scenario == "timeout") {
    device(nullptr);
    assert(ble.executeAction(change) == CmdResult::TimeOut);
    assert(ble.disconnects == 1);
    return 0;
  }
  if (scenario == "action_refused") {
    // A lock action the device refuses right away (busy): the answer, not the timeout
    device(error(69));
    const int64_t started = nowMs;
    assert(ble.executeAction(lockAction) == CmdResult::Lock_Busy);
    assert(nowMs - started < 1000 && ble.disconnects == 0);
    return 0;
  }
  if (scenario == "action_stale_error") {
    // The error of the command before does not fail the action after it
    device(error(0x2A));
    assert(ble.executeAction(change) == CmdResult::Failed);
    device([](const NukiBle::Sent&) {
      script.push_back([] { ble.receive(Command::Status, (uint8_t) CommandStatus::Accepted); });
      script.push_back([] { ble.receive(Command::Status, (uint8_t) CommandStatus::Complete); });
    });
    assert(ble.executeAction(lockAction) == CmdResult::Success);
    return 0;
  }
  if (scenario == "action_complete_right_after_accepted") {
    // "accepted" and "complete" arrive one right after the other: done, not a timeout
    device([](const NukiBle::Sent&) {
      script.push_back([] { ble.receive(Command::Status, (uint8_t) CommandStatus::Accepted); });
      script.push_back([] { ble.receive(Command::Status, (uint8_t) CommandStatus::Complete); });
    });
    const int64_t started = nowMs;
    assert(ble.executeAction(lockAction) == CmdResult::Success);
    assert(nowMs - started < 1000);
    script.clear();
    device([](const NukiBle::Sent&) {
      script.push_back([] {
        ble.receive(Command::Status, (uint8_t) CommandStatus::Accepted);
        ble.receive(Command::Status, (uint8_t) CommandStatus::Complete);
      });
    });
    assert(ble.executeAction(lockAction) == CmdResult::Success);
    return 0;
  }
  if (scenario == "action_accepted_only") {
    // As it always was: an action is done once the device accepted it
    device([](const NukiBle::Sent&) {
      script.push_back([] { ble.receive(Command::Status, (uint8_t) CommandStatus::Accepted); });
    });
    const int64_t started = nowMs;
    assert(ble.executeAction(lockAction) == CmdResult::Success);
    assert(nowMs - started < 1000);
    return 0;
  }
  if (scenario == "action_state_behind_accepted") {
    // The device sends its state right behind "accepted", before the command has looked, and
    // "complete" only seconds later: done at "accepted", not a timeout and a second attempt
    device([](const NukiBle::Sent&) {
      script.push_back([] {
        ble.receive(Command::Status, (uint8_t) CommandStatus::Accepted);
        ble.receive(Command::KeyturnerStates);
      });
    });
    const int64_t started = nowMs;
    assert(ble.executeAction(lockAction) == CmdResult::Success);
    assert(nowMs - started < 1000 && ble.disconnects == 0);
    return 0;
  }
  if (scenario == "action_error_before_it_went_out") {
    // A late error of the action before arrives while this one waits for its challenge: it
    // is not this action's answer
    ble.onSend = [](const NukiBle::Sent& message) {
      if (message.isChallengeRequest) {
        script.push_back([] { ble.receive(Command::ErrorReport, 69); ble.receive(Command::Challenge, 5); });
      } else {
        script.push_back([] { ble.receive(Command::Status, (uint8_t) CommandStatus::Accepted); });
      }
    };
    assert(ble.executeAction(lockAction) == CmdResult::Success);
    return 0;
  }
  if (scenario == "no_wait_between_steps") {
    // With the challenge there, the command goes out without a wait in between
    ble.setChallengePrefetch(true);
    device(complete);
    assert(ble.executeAction(change) == CmdResult::Success);
    drain();
    waits = 0;
    assert(ble.executeAction(change) == CmdResult::Success);
    assert(waits == 1);  // the one for the device's answer
    return 0;
  }
  assert(false && "unknown scenario");
}
'''

SCENARIOS = ["plain", "ahead_ready", "ahead_on_its_way", "refresh_crosses_command", "read_keeps_challenge",
             "no_ahead_after_list", "refused", "refused_connection", "error_then_message", "ahead_not_taken",
             "timeout", "action_refused", "action_stale_error", "action_complete_right_after_accepted",
             "action_accepted_only", "action_state_behind_accepted", "action_error_before_it_went_out",
             "no_wait_between_steps"]


def main():
    cpp = (SRC / "NukiBle.cpp").read_text().replace("\r\n", "\n")
    from_cpp = "\n".join(function(cpp, name) for name in FROM_CPP)
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        for name, text in STUBS.items():
            (tmp / name).write_text(text)
        for name in ("NukiConstants.h", "NukiDataTypes.h", "NukiBle.hpp"):
            (tmp / name).write_text((SRC / name).read_text().replace("\r\n", "\n"))
        (tmp / "main.cpp").write_text(HARNESS % {"from_cpp": from_cpp})
        build = subprocess.run(["g++", "-std=c++17", "-g", "-I", str(tmp), str(tmp / "main.cpp"), "-o", str(tmp / "test")],
                               capture_output=True, text=True)
        if build.returncode != 0:
            print(build.stderr[-6000:])
            return 1
        failed = 0
        for scenario in SCENARIOS:
            run = subprocess.run([str(tmp / "test"), scenario], capture_output=True, text=True)
            ok = run.returncode == 0
            failed += not ok
            print(("ok    " if ok else "FAILED"), scenario, "" if ok else run.stderr.strip().splitlines()[-1:])
        print("%d of %d passed" % (len(SCENARIOS) - failed, len(SCENARIOS)))
        return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
