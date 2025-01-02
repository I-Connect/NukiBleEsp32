#include "NukiConstants.h"
#include "NukiDataTypes.h"

namespace Nuki {
template<typename TDeviceAction>
Nuki::CmdResult NukiBle::executeAction(const TDeviceAction action) {
  if (!altConnect) {
    #ifndef NUKI_64BIT_TIME
    if (millis() - lastHeartbeat > HEARTBEAT_TIMEOUT) {
    #else
    if ((esp_timer_get_time() / 1000) - lastHeartbeat > HEARTBEAT_TIMEOUT) {
    #endif
      logMessage("Lock Heartbeat timeout, command failed", 1);
      return Nuki::CmdResult::Error;
    }
  }
  if (debugNukiConnect) {
    logMessage("************************ CHECK PAIRED ************************");
  }
  if (retrieveCredentials()) {
    if (debugNukiConnect) {
      logMessage("Credentials retrieved from preferences, ready for commands");
    }
  } else {
    if (debugNukiConnect) {
      logMessage("Credentials NOT retrieved from preferences, first pair with the lock");
    }
    return Nuki::CmdResult::NotPaired;
  }

  if (takeNukiBleSemaphore("exec Action")) {
    if (debugNukiCommunication) {
      logMessageVar("Start executing: %02x ", (unsigned int)action.command);
    }

    while (1) {
      extendDisconnectTimeout();

      Nuki::CmdResult result;
      if (action.cmdType == Nuki::CommandType::Command) {
        result = cmdStateMachine(action);
      }
      else if (action.cmdType == Nuki::CommandType::CommandWithChallenge) {
        result = cmdChallStateMachine(action);
      }
      else if (action.cmdType == Nuki::CommandType::CommandWithChallengeAndAccept) {
        result = cmdChallAccStateMachine(action);
      }
      else if (action.cmdType == Nuki::CommandType::CommandWithChallengeAndPin) {
        result = cmdChallStateMachine(action, true);
      }
      else {
        logMessage("Unknown cmd type", 2);
        giveNukiBleSemaphore();
        disconnect();
        return Nuki::CmdResult::Failed;
      }
      if (result != Nuki::CmdResult::Working) {
        giveNukiBleSemaphore();

        if (altConnect && (result == Nuki::CmdResult::Error || result == Nuki::CmdResult::Failed)) {
          disconnect();
        }
        return result;
      }
      #ifndef NUKI_NO_WDT_RESET
      esp_task_wdt_reset();
      #endif
      delay(10);
    }
  }
  return Nuki::CmdResult::Failed;
}

template <typename TDeviceAction>
Nuki::CmdResult NukiBle::cmdStateMachine(const TDeviceAction action) {
  extendDisconnectTimeout();
  delay(10);

  switch (nukiCommandState) {
    case CommandState::Idle: {
      if (debugNukiCommunication) {
        logMessageVar("************************ SENDING COMMAND [%d] ************************", (unsigned int)action.command);
      }
      lastMsgCodeReceived = Command::Empty;

      if (sendEncryptedMessage(Command::RequestData, action.payload, action.payloadLen)) {
        #ifndef NUKI_64BIT_TIME
        timeNow = millis();
        #else
        timeNow = (esp_timer_get_time() / 1000);
        #endif
        nukiCommandState = CommandState::CmdSent;
      } else {
        if (debugNukiCommunication) {
          logMessage("************************ SENDING COMMAND FAILED ************************");
        }
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      }
      break;
    }
    case CommandState::CmdSent: {
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > CMD_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > CMD_TIMEOUT) {
      #endif
        logMessage("************************ COMMAND FAILED TIMEOUT************************", 2);
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived != Command::ErrorReport && lastMsgCodeReceived != Command::Empty) {
        if (debugNukiCommunication) {
          logMessage("************************ COMMAND DONE ************************");
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Success;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode != 69) {
        if (debugNukiCommunication) {
          logMessage("************************ COMMAND FAILED ************************");
        }
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode == 69) {
        if (debugNukiCommunication) {
          logMessage("************************ COMMAND FAILED LOCK BUSY ************************");
        }
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Lock_Busy;
      }
    }
    break;
    default: {
      logMessage("Unknown request command state", 2);
      if (altConnect) {
        disconnect();
      }
      return Nuki::CmdResult::Failed;
      break;
    }
  }
  return Nuki::CmdResult::Working;
}

template <typename TDeviceAction>
Nuki::CmdResult NukiBle::cmdChallStateMachine(const TDeviceAction action, const bool sendPinCode) {
  extendDisconnectTimeout();
  delay(10);

  switch (nukiCommandState) {
    case CommandState::Idle: {
      if (debugNukiCommunication) {
        logMessage("************************ SENDING CHALLENGE ************************");
      }
      lastMsgCodeReceived = Command::Empty;
      unsigned char payload[sizeof(Command)] = {0x04, 0x00};  //challenge

      if (sendEncryptedMessage(Command::RequestData, payload, sizeof(Command))) {
        #ifndef NUKI_64BIT_TIME
        timeNow = millis();
        #else
        timeNow = (esp_timer_get_time() / 1000);
        #endif
        nukiCommandState = CommandState::ChallengeSent;
      } else {
        if (debugNukiCommunication) {
          logMessage("************************ SENDING CHALLENGE FAILED ************************");
        }
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      }
      break;
    }
    case CommandState::ChallengeSent: {
      if (debugNukiCommunication) {
        logMessage("************************ RECEIVING CHALLENGE RESPONSE************************");
      }
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > CMD_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > CMD_TIMEOUT) {
      #endif
        logMessage("************************ COMMAND FAILED TIMEOUT ************************", 2);
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::Challenge) {
        nukiCommandState = CommandState::ChallengeRespReceived;
        lastMsgCodeReceived = Command::Empty;
      }
      break;
    }
    case CommandState::ChallengeRespReceived: {
      if (debugNukiCommunication) {
        logMessageVar("************************ SENDING COMMAND [%d] ************************", (unsigned int)action.command);
      }
      lastMsgCodeReceived = Command::Empty;
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

      if (sendEncryptedMessage(action.command, payload, payloadLen)) {
        #ifndef NUKI_64BIT_TIME
        timeNow = millis();
        #else
        timeNow = (esp_timer_get_time() / 1000);
        #endif
        nukiCommandState = CommandState::CmdSent;
      } else {
        if (debugNukiCommunication) {
          logMessage("************************ SENDING COMMAND FAILED ************************");
        }
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      }
      break;
    }
    case CommandState::CmdSent: {
      if (debugNukiCommunication) {
        logMessage("************************ RECEIVING DATA ************************");
      }
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > CMD_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > CMD_TIMEOUT) {
      #endif
        logMessage("************************ COMMAND FAILED TIMEOUT ************************", 2);
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode != 69) {
        if (debugNukiCommunication) {
          logMessage("************************ COMMAND FAILED ************************");
        }
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode == 69) {
        if (debugNukiCommunication) {
          logMessage("************************ COMMAND FAILED LOCK BUSY ************************");
        }
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Lock_Busy;
      } else if (crcCheckOke) {
        if (debugNukiCommunication) {
          logMessage("************************ DATA RECEIVED ************************");
        }
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::Success;
      }
      break;
    }
    default:
      logMessage("Unknown request command state", 2);
      if (altConnect) {
        disconnect();
      }
      return Nuki::CmdResult::Failed;
      break;
  }
  return Nuki::CmdResult::Working;
}

template <typename TDeviceAction>
Nuki::CmdResult NukiBle::cmdChallAccStateMachine(const TDeviceAction action) {
  extendDisconnectTimeout();
  delay(10);

  switch (nukiCommandState) {
    case CommandState::Idle: {
      if (debugNukiCommunication) {
        logMessage("************************ SENDING CHALLENGE ************************");
      }
      lastMsgCodeReceived = Command::Empty;
      unsigned char payload[sizeof(Command)] = {0x04, 0x00};  //challenge

      if (sendEncryptedMessage(Command::RequestData, payload, sizeof(Command))) {
        #ifndef NUKI_64BIT_TIME
        timeNow = millis();
        #else
        timeNow = (esp_timer_get_time() / 1000);
        #endif
        nukiCommandState = CommandState::ChallengeSent;
      } else {
        if (debugNukiCommunication) {
          logMessage("************************ SENDING CHALLENGE FAILED ************************");
        }
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      }
      break;
    }
    case CommandState::ChallengeSent: {
      if (debugNukiCommunication) {
        logMessage("************************ RECEIVING CHALLENGE RESPONSE************************");
      }
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > CMD_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > CMD_TIMEOUT) {
      #endif
        logMessage("************************ COMMAND FAILED TIMEOUT ************************", 2);
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::Challenge) {
        nukiCommandState = CommandState::ChallengeRespReceived;
        lastMsgCodeReceived = Command::Empty;
      }
      break;
    }
    case CommandState::ChallengeRespReceived: {
      if (debugNukiCommunication) {
        logMessageVar("************************ SENDING COMMAND [%d] ************************", (unsigned int)action.command);
      }
      lastMsgCodeReceived = Command::Empty;
      //add received challenge nonce to payload
      uint8_t payloadLen = action.payloadLen + sizeof(challengeNonceK);
      unsigned char payload[payloadLen];
      memcpy(payload, action.payload, action.payloadLen);
      memcpy(&payload[action.payloadLen], challengeNonceK, sizeof(challengeNonceK));

      if (sendEncryptedMessage(action.command, payload, action.payloadLen + sizeof(challengeNonceK))) {
        #ifndef NUKI_64BIT_TIME
        timeNow = millis();
        #else
        timeNow = (esp_timer_get_time() / 1000);
        #endif
        nukiCommandState = CommandState::CmdSent;
      } else {
        if (debugNukiCommunication) {
          logMessage("************************ SENDING COMMAND FAILED ************************");
        }
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      }
      break;
    }
    case CommandState::CmdSent: {
      if (debugNukiCommunication) {
        logMessage("************************ RECEIVING ACCEPT ************************");
      }
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > CMD_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > CMD_TIMEOUT) {
      #endif
        logMessage("************************ ACCEPT FAILED TIMEOUT ************************", 2);
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::Status && (CommandStatus)receivedStatus == CommandStatus::Accepted) {
        #ifndef NUKI_64BIT_TIME
        timeNow = millis();
        #else
        timeNow = (esp_timer_get_time() / 1000);
        #endif
        nukiCommandState = CommandState::CmdAccepted;
        lastMsgCodeReceived = Command::Empty;
      } else if (lastMsgCodeReceived == Command::Status && (CommandStatus)receivedStatus == CommandStatus::Complete) {
        //accept was skipped on lock because ie unlock command when lock allready unlocked?
        if (debugNukiCommunication) {
          logMessage("************************ COMMAND SUCCESS (SKIPPED) ************************");
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Success;
      }
      break;
    }
    case CommandState::CmdAccepted: {
      if (debugNukiCommunication) {
        logMessage("************************ RECEIVING COMPLETE ************************");
      }
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > CMD_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > CMD_TIMEOUT) {
      #endif
        logMessage("************************ COMMAND FAILED TIMEOUT ************************", 2);
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode != 69) {
        if (debugNukiCommunication) {
          logMessage("************************ COMMAND FAILED ************************");
        }
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode == 69) {
        if (debugNukiCommunication) {
          logMessage("************************ COMMAND FAILED LOCK BUSY ************************");
        }
        if (altConnect) {
          disconnect();
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Lock_Busy;
      } else if ((CommandStatus)lastMsgCodeReceived == CommandStatus::Complete) {
        if (debugNukiCommunication) {
          logMessage("************************ COMMAND SUCCESS ************************");
        }
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Success;
      }
      break;
    }
    default:
      logMessage("Unknown request command state", 2);
      if (altConnect) {
        disconnect();
      }
      return Nuki::CmdResult::Failed;
      break;
  }
  return Nuki::CmdResult::Working;
}
}