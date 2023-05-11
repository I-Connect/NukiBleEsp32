#include "NukiConstants.h"
#include "NukiDataTypes.h"

namespace Nuki {
template<typename TDeviceAction>
Nuki::CmdResult NukiBle::executeAction(const TDeviceAction action) {
  if (millis() - lastHeartbeat > HEARTBEAT_TIMEOUT) {
    log_e("Lock Heartbeat timeout, command failed");
    return Nuki::CmdResult::Error;
  }

  #ifdef DEBUG_NUKI_CONNECT
  log_d("************************ CHECK PAIRED ************************");
  #endif
  if (isPaired) {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Lock is paired, ready for commands");
    #endif
  } else {
    #ifdef DEBUG_NUKI_CONNECT
    log_d("Lock not paired, first pair with the lock");
    #endif
    return Nuki::CmdResult::NotPaired;
  }

  if (takeNukiBleSemaphore("exec Action")) {
    #ifdef DEBUG_NUKI_COMMUNICATION
    log_d("Start executing: %02x ", action.command);
    #endif
    if (action.cmdType == Nuki::CommandType::Command) {
      while (1) {
        Nuki::CmdResult result = cmdStateMachine(action);
        if (result != Nuki::CmdResult::Working) {
          giveNukiBleSemaphore();
          extendDisonnectTimeout();
          return result;
        }
        esp_task_wdt_reset();
        delay(10);
      }
    } else if (action.cmdType == Nuki::CommandType::CommandWithChallenge) {
      while (1) {
        Nuki::CmdResult result = cmdChallStateMachine(action);
        if (result != Nuki::CmdResult::Working) {
          giveNukiBleSemaphore();
          extendDisonnectTimeout();
          return result;
        }
        esp_task_wdt_reset();
        delay(10);
      }
    } else if (action.cmdType == Nuki::CommandType::CommandWithChallengeAndAccept) {
      while (1) {
        Nuki::CmdResult result = cmdChallAccStateMachine(action);
        if (result != Nuki::CmdResult::Working) {
          giveNukiBleSemaphore();
          extendDisonnectTimeout();
          return result;
        }
        esp_task_wdt_reset();
        delay(10);
      }
    } else if (action.cmdType == Nuki::CommandType::CommandWithChallengeAndPin) {
      while (1) {
        Nuki::CmdResult result = cmdChallStateMachine(action, true);
        if (result != Nuki::CmdResult::Working) {
          giveNukiBleSemaphore();
          extendDisonnectTimeout();
          return result;
        }
        esp_task_wdt_reset();
        delay(10);
      }
    } else {
      log_w("Unknown cmd type");
    }
    giveNukiBleSemaphore();
  }
  return Nuki::CmdResult::Failed;
}

template <typename TDeviceAction>
Nuki::CmdResult NukiBle::cmdStateMachine(const TDeviceAction action) {
  switch (nukiCommandState) {
    case CommandState::Idle: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING COMMAND [%d] ************************", action.command);
      #endif
      lastMsgCodeReceived = Command::Empty;

      if (sendEncryptedMessage(Command::RequestData, action.payload, action.payloadLen)) {
        timeNow = millis();
        nukiCommandState = CommandState::CmdSent;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ SENDING COMMAND FAILED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      }
      break;
    }
    case CommandState::CmdSent: {
      if (millis() - timeNow > CMD_TIMEOUT) {
        log_w("************************ COMMAND FAILED TIMEOUT************************");
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived != Command::ErrorReport && lastMsgCodeReceived != Command::Empty) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND DONE ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Success;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode != 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode == 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED LOCK BUSY ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Lock_Busy;
      }
    }
    break;
    default: {
      log_w("Unknown request command state");
      return Nuki::CmdResult::Failed;
      break;
    }
  }
  return Nuki::CmdResult::Working;
}

template <typename TDeviceAction>
Nuki::CmdResult NukiBle::cmdChallStateMachine(const TDeviceAction action, const bool sendPinCode) {
  switch (nukiCommandState) {
    case CommandState::Idle: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING CHALLENGE ************************");
      #endif
      lastMsgCodeReceived = Command::Empty;
      unsigned char payload[sizeof(Command)] = {0x04, 0x00};  //challenge

      if (sendEncryptedMessage(Command::RequestData, payload, sizeof(Command))) {
        timeNow = millis();
        nukiCommandState = CommandState::ChallengeSent;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ SENDING CHALLENGE FAILED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      }
      break;
    }
    case CommandState::ChallengeSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING CHALLENGE RESPONSE************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        log_w("************************ COMMAND FAILED TIMEOUT ************************");
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::Challenge) {
        nukiCommandState = CommandState::ChallengeRespReceived;
        lastMsgCodeReceived = Command::Empty;
      }
      break;
    }
    case CommandState::ChallengeRespReceived: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING COMMAND [%d] ************************", action.command);
      #endif
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
        timeNow = millis();
        nukiCommandState = CommandState::CmdSent;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ SENDING COMMAND FAILED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      }
      break;
    }
    case CommandState::CmdSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING DATA ************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        log_w("************************ COMMAND FAILED TIMEOUT ************************");
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode != 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode == 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED LOCK BUSY ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Lock_Busy;
      } else if (crcCheckOke) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ DATA RECEIVED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::Success;
      }
      break;
    }
    default:
      log_w("Unknown request command state");
      return Nuki::CmdResult::Failed;
      break;
  }
  return Nuki::CmdResult::Working;
}

template <typename TDeviceAction>
Nuki::CmdResult NukiBle::cmdChallAccStateMachine(const TDeviceAction action) {
  switch (nukiCommandState) {
    case CommandState::Idle: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING CHALLENGE ************************");
      #endif
      lastMsgCodeReceived = Command::Empty;
      unsigned char payload[sizeof(Command)] = {0x04, 0x00};  //challenge

      if (sendEncryptedMessage(Command::RequestData, payload, sizeof(Command))) {
        timeNow = millis();
        nukiCommandState = CommandState::ChallengeSent;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ SENDING CHALLENGE FAILED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      }
      break;
    }
    case CommandState::ChallengeSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING CHALLENGE RESPONSE************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        log_w("************************ COMMAND FAILED TIMEOUT ************************");
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::Challenge) {
        nukiCommandState = CommandState::ChallengeRespReceived;
        lastMsgCodeReceived = Command::Empty;
      }
      break;
    }
    case CommandState::ChallengeRespReceived: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING COMMAND [%d] ************************", action.command);
      #endif
      lastMsgCodeReceived = Command::Empty;
      //add received challenge nonce to payload
      uint8_t payloadLen = action.payloadLen + sizeof(challengeNonceK);
      unsigned char payload[payloadLen];
      memcpy(payload, action.payload, action.payloadLen);
      memcpy(&payload[action.payloadLen], challengeNonceK, sizeof(challengeNonceK));

      if (sendEncryptedMessage(action.command, payload, action.payloadLen + sizeof(challengeNonceK))) {
        timeNow = millis();
        nukiCommandState = CommandState::CmdSent;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ SENDING COMMAND FAILED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      }
      break;
    }
    case CommandState::CmdSent: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING ACCEPT ************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        log_w("************************ ACCEPT FAILED TIMEOUT ************************");
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::Status && (CommandStatus)receivedStatus == CommandStatus::Accepted) {
        timeNow = millis();
        nukiCommandState = CommandState::CmdAccepted;
        lastMsgCodeReceived = Command::Empty;
      } else if (lastMsgCodeReceived == Command::Status && (CommandStatus)receivedStatus == CommandStatus::Complete) {
        //accept was skipped on lock because ie unlock command when lock allready unlocked?
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND SUCCESS (SKIPPED) ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Success;
      }
      break;
    }
    case CommandState::CmdAccepted: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING COMPLETE ************************");
      #endif
      if (millis() - timeNow > CMD_TIMEOUT) {
        log_w("************************ COMMAND FAILED TIMEOUT ************************");
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode != 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode == 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED LOCK BUSY ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Lock_Busy;
      } else if ((CommandStatus)lastMsgCodeReceived == CommandStatus::Complete) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND SUCCESS ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Success;
      }
      break;
    }
    default:
      log_w("Unknown request command state");
      return Nuki::CmdResult::Failed;
      break;
  }
  return Nuki::CmdResult::Working;
}
}
