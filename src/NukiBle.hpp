#include "NukiConstants.h"
#include "NukiDataTypes.h"

namespace Nuki {
template<typename TDeviceAction>
Nuki::CmdResult NukiBle::executeAction(const TDeviceAction action) {
  #ifndef NUKI_64BIT_TIME
  if (millis() - lastHeartbeat > HEARTBEAT_TIMEOUT) {
  #else
  if ((esp_timer_get_time() / 1000) - lastHeartbeat > HEARTBEAT_TIMEOUT) {
  #endif
    log_e("Lock Heartbeat timeout, command failed");
    return Nuki::CmdResult::Error;
  }

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
    return Nuki::CmdResult::NotPaired;
  }

  if (takeNukiBleSemaphore("exec Action")) {
    #ifdef DEBUG_NUKI_COMMUNICATION
    log_d("Start executing: %02x ", action.command);
    #endif

    while (1) {
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
        log_w("Unknown cmd type");
        giveNukiBleSemaphore();
        return Nuki::CmdResult::Failed;
      }
      if (result != Nuki::CmdResult::Working) {
        giveNukiBleSemaphore();

        #ifdef NUKI_ALT_CONNECT
        if (result == Nuki::CmdResult::Error)
        {
          disconnect();
        }
        else
        {
          extendDisconnectTimeout();
        }
        #else
        extendDisconnectTimeout(); 
        #endif
        
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
  switch (nukiCommandState) {
    case CommandState::Idle: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ SENDING COMMAND [%d] ************************", action.command);
      #endif
      lastMsgCodeReceived = Command::Empty;

      if (sendEncryptedMessage(Command::RequestData, action.payload, action.payloadLen)) {
        #ifndef NUKI_64BIT_TIME
        timeNow = millis();
        #else
        timeNow = (esp_timer_get_time() / 1000);
        #endif
        nukiCommandState = CommandState::CmdSent;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ SENDING COMMAND FAILED ************************");
        #endif
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
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
        log_w("************************ COMMAND FAILED TIMEOUT************************");
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived != Command::ErrorReport && lastMsgCodeReceived != Command::Empty) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND DONE ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        extendDisconnectTimeout();
        return Nuki::CmdResult::Success;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode != 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode == 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED LOCK BUSY ************************");
        #endif
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Lock_Busy;
      }
    }
    break;
    default: {
      log_w("Unknown request command state");
      #ifdef NUKI_ALT_CONNECT
      disconnect();
      #endif
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
        #ifndef NUKI_64BIT_TIME
        timeNow = millis();
        #else
        timeNow = (esp_timer_get_time() / 1000);
        #endif
        nukiCommandState = CommandState::ChallengeSent;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ SENDING CHALLENGE FAILED ************************");
        #endif
        #ifdef NUKI_ALT_CONNECT
        disconnect();
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
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > CMD_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > CMD_TIMEOUT) {
      #endif
        log_w("************************ COMMAND FAILED TIMEOUT ************************");
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
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
        #ifndef NUKI_64BIT_TIME
        timeNow = millis();
        #else
        timeNow = (esp_timer_get_time() / 1000);
        #endif
        nukiCommandState = CommandState::CmdSent;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ SENDING COMMAND FAILED ************************");
        #endif
        #ifdef NUKI_ALT_CONNECT
        disconnect();
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
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > CMD_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > CMD_TIMEOUT) {
      #endif
        log_w("************************ COMMAND FAILED TIMEOUT ************************");
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode != 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode == 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED LOCK BUSY ************************");
        #endif
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Lock_Busy;
      } else if (crcCheckOke) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ DATA RECEIVED ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        extendDisconnectTimeout();
        return Nuki::CmdResult::Success;
      }
      break;
    }
    default:
      log_w("Unknown request command state");
      #ifdef NUKI_ALT_CONNECT
      disconnect();
      #endif
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
        #ifndef NUKI_64BIT_TIME
        timeNow = millis();
        #else
        timeNow = (esp_timer_get_time() / 1000);
        #endif
        nukiCommandState = CommandState::ChallengeSent;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ SENDING CHALLENGE FAILED ************************");
        #endif
        #ifdef NUKI_ALT_CONNECT
        disconnect();
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
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > CMD_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > CMD_TIMEOUT) {
      #endif
        log_w("************************ COMMAND FAILED TIMEOUT ************************");
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
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
        #ifndef NUKI_64BIT_TIME
        timeNow = millis();
        #else
        timeNow = (esp_timer_get_time() / 1000);
        #endif
        nukiCommandState = CommandState::CmdSent;
      } else {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ SENDING COMMAND FAILED ************************");
        #endif
        #ifdef NUKI_ALT_CONNECT
        disconnect();
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
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > CMD_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > CMD_TIMEOUT) {
      #endif
        log_w("************************ ACCEPT FAILED TIMEOUT ************************");
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
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
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND SUCCESS (SKIPPED) ************************");
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        extendDisconnectTimeout();
        return Nuki::CmdResult::Success;
      }
      break;
    }
    case CommandState::CmdAccepted: {
      #ifdef DEBUG_NUKI_COMMUNICATION
      log_d("************************ RECEIVING COMPLETE ************************");
      #endif
      #ifndef NUKI_64BIT_TIME
      if (millis() - timeNow > CMD_TIMEOUT) {
      #else
      if ((esp_timer_get_time() / 1000) - timeNow > CMD_TIMEOUT) {
      #endif
        log_w("************************ COMMAND FAILED TIMEOUT ************************");
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
        nukiCommandState = CommandState::Idle;
        return Nuki::CmdResult::TimeOut;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode != 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED ************************");
        #endif
        #ifdef NUKI_ALT_CONNECT
        disconnect();
        #endif
        nukiCommandState = CommandState::Idle;
        lastMsgCodeReceived = Command::Empty;
        return Nuki::CmdResult::Failed;
      } else if (lastMsgCodeReceived == Command::ErrorReport && errorCode == 69) {
        #ifdef DEBUG_NUKI_COMMUNICATION
        log_d("************************ COMMAND FAILED LOCK BUSY ************************");
        #endif
        #ifdef NUKI_ALT_CONNECT
        disconnect();
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
        extendDisconnectTimeout();
        return Nuki::CmdResult::Success;
      }
      break;
    }
    default:
      log_w("Unknown request command state");
      #ifdef NUKI_ALT_CONNECT
      disconnect();
      #endif
      return Nuki::CmdResult::Failed;
      break;
  }
  return Nuki::CmdResult::Working;
}
}