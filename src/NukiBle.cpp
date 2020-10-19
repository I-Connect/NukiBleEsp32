/*
 * NukiBle.cpp
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "NukiBle.h"
#include "Crc16.h"
#include "string.h"
#include "endian.h"

void printBuffer(const byte* buff, const uint8_t size, const boolean asChars, const char* header) {
  if (strlen(header) > 0) {
    Serial.print(header);
    Serial.print(": ");
  }
  for (int i = 0; i < size; i++) {
    if (asChars) {
      Serial.print((char)buff[i]);
    } else {
      Serial.print(buff[i], HEX);
      Serial.print(" ");
    }
  }
  Serial.println();
}

//task to retrieve messages from BLE when a notification occurrs
void nukiBleTask(void * pvParameters) {
	log_d("TASK: Nuki BLE task started");
	NukiBle* nukiBleObj = reinterpret_cast<NukiBle*>(pvParameters);
	uint8_t notification;
  
	while (1){
		xQueueReceive(nukiBleObj->nukiBleIsrFlagQueue,&notification, portMAX_DELAY );
	
  }
}


NukiBle::NukiBle(std::string &bleAddress): bleAddress(bleAddress){}

NukiBle::~NukiBle() {}

void NukiBle::initialize() {
  log_d("Initializing Nuki");
  #ifdef BLE_DEBUG
    BLEDevice::setCustomGattcHandler(my_gattc_event_handler);
  #endif
  BLEDevice::init("ESP32_test");

  //generate public and private keys
  keyGen(private_key, 32, 34);
  keyGen(public_key, 32, 34);
}

void NukiBle::startNukiBleXtask(){
  nukiBleIsrFlagQueue=xQueueCreate(10,sizeof(uint8_t));
  TaskHandleNukiBle = NULL;
  xTaskCreatePinnedToCore(&nukiBleTask, "nukiBleTask", 4096, this, 1, &TaskHandleNukiBle, 1);
}

bool NukiBle::connect() {
  log_d("Connecting with: %s ", bleAddress.c_str());
      
  pClient  = BLEDevice::createClient();
  pClient->setClientCallbacks(this);

  if(!pClient->connect(bleAddress)){
    log_w("BLE Connect failed");
    return false;
  }
  if(!registerOnGdioChar()){
    log_w("BLE register on pairing Service/Char failed");
    return false;
  }

  //send public key command (Sent message should be 0100030027A7)
  uint16_t payload = (uint16_t)nukiCommand::publicKey;
  sendPlainMessage(nukiCommand::requestData, (char*)&payload, sizeof(payload));

  log_d("BLE connect and pairing success");
  return true;
}

bool NukiBle::registerOnGdioChar(){
  // Obtain a reference to the KeyTurner Pairing service
  pKeyturnerPairingService = pClient->getService(STRING(keyturnerPairingServiceUUID));
  //Obtain reference to GDIO char
  pGdioCharacteristic = pKeyturnerPairingService->getCharacteristic(STRING(keyturnerGdioUUID));
  if(pGdioCharacteristic->canIndicate()){
    pGdioCharacteristic->registerForNotify(notifyCallback, false); //false = indication, true = notification
    delay(100);
    return true;
  }
  else{
    log_d("GDIO characteristic canIndicate false, stop connecting");
    return false;
  }
  return false;
}

void NukiBle::sendPlainMessage(nukiCommand commandIdentifier, char* payload, uint8_t payloadLen) {
  Crc16 crcObj;
  uint16_t dataCrc;
  
  //get crc over data (data is both command identifier and payload)
  char dataToSend[200];
  memcpy(&dataToSend, &commandIdentifier, sizeof(commandIdentifier));
  memcpy(&dataToSend[2], payload, payloadLen);
 
  crcObj.clearCrc();
  // CCITT-False:	width=16 poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1
  dataCrc = crcObj.fastCrc((uint8_t*)dataToSend, 0, payloadLen + 2, false, false, 0x1021, 0xffff, 0x0000, 0x8000, 0xffff);
  
  memcpy(&dataToSend[2+payloadLen], &dataCrc, sizeof(dataCrc));
  log_d("Sending plain message %02x%02x%02x%02x%02x%02x", dataToSend[0], dataToSend[1], dataToSend[2], dataToSend[3], dataToSend[4] , dataToSend[5]);
  log_d("Command identifier: %02x, CRC: %04x", (uint32_t)commandIdentifier, dataCrc);
  pGdioCharacteristic->writeValue((uint8_t*)dataToSend, payloadLen + 4, true);
}

bool NukiBle::executeLockAction(lockAction aLockAction) {
  log_d("Executing lock action: %d", aLockAction);
  return true;
}

void NukiBle::notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  // log_d(" Notify callback for characteristic: %s of length: %d", pBLERemoteCharacteristic->getUUID().toString().c_str(), length);
  delay(100);
  printBuffer((byte*)pData, length, false, "Received data");

  //check CRC
  uint16_t receivedCrc = ((uint16_t)pData[length - 1] << 8) | pData[length - 2];
  Crc16 crcObj;
  uint16_t dataCrc;
  crcObj.clearCrc();
  dataCrc = crcObj.fastCrc(pData, 0, length - 2 , false, false, 0x1021, 0xffff, 0x0000, 0x8000, 0xffff);
  // log_d("Received CRC: %d, calculated CRC: %d", receivedCrc, dataCrc);
  uint16_t returnCode = ((uint16_t)pData[1] << 8) | pData[0];
  log_d("Return code: %d", returnCode);
  if(!(receivedCrc == dataCrc)){
    log_e("CRC CHECK FAILED!");
  }
  else{
    log_d("CRC CHECK OKE");
  }

  if(returnCode == (uint16_t)nukiCommand::errorReport){
    // log_e("ERROR: %02x", pData[2]);
    handleErrorCode(pData[2]);
  }
  else{
    handleReturnCode(returnCode);
  }
}

void NukiBle::handleErrorCode(uint8_t errorCode){
  
  switch(errorCode) {
    case (uint8_t)nukiErrorCode::ERROR_BAD_CRC :
      log_e("ERROR_BAD_CRC");
      break;
    case (uint8_t)nukiErrorCode::ERROR_BAD_LENGTH :
      log_e("ERROR_BAD_LENGTH");
      break;
    case (uint8_t)nukiErrorCode::ERROR_UNKNOWN :
      log_e("ERROR_UNKNOWN");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_AUTO_UNLOCK_TOO_RECENT :
      log_e("K_ERROR_AUTO_UNLOCK_TOO_RECENT");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_BAD_NONCE :
      log_e("K_ERROR_BAD_NONCE");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_BAD_PARAMETER :
      log_e("K_ERROR_BAD_PARAMETER");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_BAD_PIN :
      log_e("K_ERROR_BAD_PIN");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_BUSY :
      log_e("K_ERROR_BUSY");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CANCELED :
      log_e("K_ERROR_CANCELED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CLUTCH_FAILURE :
      log_e("K_ERROR_CLUTCH_FAILURE");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CLUTCH_POWER_FAILURE :
      log_e("K_ERROR_CLUTCH_POWER_FAILURE");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CODE_ALREADY_EXISTS :
      log_e("K_ERROR_CODE_ALREADY_EXISTS");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CODE_INVALID :
      log_e("K_ERROR_CODE_INVALID");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_1 :
      log_e("K_ERROR_CODE_INVALID_TIMEOUT_1");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_2 :
      log_e("K_ERROR_CODE_INVALID_TIMEOUT_2");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_CODE_INVALID_TIMEOUT_3 :
      log_e("K_ERROR_CODE_INVALID_TIMEOUT_3");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_DISABLED :
      log_e("K_ERROR_DISABLED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_FIRMWARE_UPDATE_NEEDED :
      log_e("K_ERROR_FIRMWARE_UPDATE_NEEDED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_INVALID_AUTH_ID :
      log_e("K_ERROR_INVALID_AUTH_ID");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_MOTOR_BLOCKED :
      log_e("K_ERROR_MOTOR_BLOCKED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_MOTOR_LOW_VOLTAGE :
      log_e("K_ERROR_MOTOR_LOW_VOLTAGE");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_MOTOR_POSITION_LIMIT :
      log_e("K_ERROR_MOTOR_POSITION_LIMIT");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_MOTOR_POWER_FAILURE :
      log_e("K_ERROR_MOTOR_POWER_FAILURE");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_MOTOR_TIMEOUT :
      log_e("K_ERROR_MOTOR_TIMEOUT");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_NOT_AUTHORIZED :
      log_e("K_ERROR_NOT_AUTHORIZED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_NOT_CALIBRATED :
      log_e("K_ERROR_NOT_CALIBRATED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_POSITION_UNKNOWN :
      log_e("K_ERROR_POSITION_UNKNOWN");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_REMOTE_NOT_ALLOWED :
      log_e("K_ERROR_REMOTE_NOT_ALLOWED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_TIME_NOT_ALLOWED :
      log_e("K_ERROR_TIME_NOT_ALLOWED");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_TOO_MANY_ENTRIES :
      log_e("K_ERROR_TOO_MANY_ENTRIES");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_TOO_MANY_PIN_ATTEMPTS :
      log_e("K_ERROR_TOO_MANY_PIN_ATTEMPTS");
      break;
    case (uint8_t)nukiErrorCode::K_ERROR_VOLTAGE_TOO_LOW :
      log_e("K_ERROR_VOLTAGE_TOO_LOW");
      break;
    case (uint8_t)nukiErrorCode::P_ERROR_BAD_AUTHENTICATOR :
      log_e("P_ERROR_BAD_AUTHENTICATOR");
      break;
    case (uint8_t)nukiErrorCode::P_ERROR_BAD_PARAMETER :
      log_e("P_ERROR_BAD_PARAMETER");
      break;
    case (uint8_t)nukiErrorCode::P_ERROR_MAX_USER :
      log_e("P_ERROR_MAX_USER");
      break;
    case (uint8_t)nukiErrorCode::P_ERROR_NOT_PAIRING :
      log_e("P_ERROR_NOT_PAIRING");
      break;
    default:
      log_e("UNKNOWN ERROR");
    }
}

void NukiBle::handleReturnCode(uint16_t returnCode){
  
  switch(returnCode) {
    case (uint16_t)nukiCommand::requestData :
      log_e("requestData");
      break;
    case (uint16_t)nukiCommand::publicKey :
      log_e("publicKey");
      break;
    case (uint16_t)nukiCommand::challenge :
      log_e("challenge");
      break;
    case (uint16_t)nukiCommand::authorizationAuthenticator :
      log_e("authorizationAuthenticator");
      break;
    case (uint16_t)nukiCommand::authorizationData :
      log_e("authorizationData");
      break;
    case (uint16_t)nukiCommand::authorizationId :
      log_e("authorizationId");
      break;
    case (uint16_t)nukiCommand::removeUserAuthorization :
      log_e("removeUserAuthorization");
      break;
    case (uint16_t)nukiCommand::requestAuthorizationEntries :
      log_e("requestAuthorizationEntries");
      break;
    case (uint16_t)nukiCommand::authorizationEntry :
      log_e("authorizationEntry");
      break;
    case (uint16_t)nukiCommand::authorizationDatInvite :
      log_e("authorizationDatInvite");
      break;
    case (uint16_t)nukiCommand::keyturnerStates :
      log_e("keyturnerStates");
      break;
    case (uint16_t)nukiCommand::lockAction :
      log_e("lockAction");
      break;
    case (uint16_t)nukiCommand::status :
      log_e("status");
      break;
    case (uint16_t)nukiCommand::mostRecentCommand :
      log_e("mostRecentCommand");
      break;
    case (uint16_t)nukiCommand::openingsClosingsSummary :
      log_e("openingsClosingsSummary");
      break;
    case (uint16_t)nukiCommand::batteryReport :
      log_e("batteryReport");
      break;
    case (uint16_t)nukiCommand::errorReport :
      log_e("errorReport");
      break;
    case (uint16_t)nukiCommand::setConfig :
      log_e("setConfig");
      break;
    case (uint16_t)nukiCommand::requestConfig :
      log_e("requestConfig");
      break;
    case (uint16_t)nukiCommand::config :
      log_e("config");
      break;
    case (uint16_t)nukiCommand::setSecurityPin :
      log_e("setSecurityPin");
      break;
    case (uint16_t)nukiCommand::requestCalibration :
      log_e("requestCalibration");
      break;
    case (uint16_t)nukiCommand::requestReboot :
      log_e("requestReboot");
      break;
    case (uint16_t)nukiCommand::authorizationIdConfirmation :
      log_e("authorizationIdConfirmation");
      break;
    case (uint16_t)nukiCommand::authorizationIdInvite :
      log_e("authorizationIdInvite");
      break;
    case (uint16_t)nukiCommand::verifySecurityPin :
      log_e("verifySecurityPin");
      break;
    case (uint16_t)nukiCommand::updateTime :
      log_e("updateTime");
      break;
    case (uint16_t)nukiCommand::updateUserAuthorization :
      log_e("updateUserAuthorization");
      break;
    case (uint16_t)nukiCommand::authorizationEntryCount :
      log_e("authorizationEntryCount");
      break;
    case (uint16_t)nukiCommand::requestLogEntries :
      log_e("requestLogEntries");
      break;
    case (uint16_t)nukiCommand::logEntry :
      log_e("logEntry");
      break;
    case (uint16_t)nukiCommand::logEntryCount :
      log_e("logEntryCount");
      break;
    case (uint16_t)nukiCommand::enableLogging :
      log_e("enableLogging");
      break;
    case (uint16_t)nukiCommand::setAdvancedConfig :
      log_e("setAdvancedConfig");
      break;
    case (uint16_t)nukiCommand::requestAdvancedConfig :
      log_e("requestAdvancedConfig");
      break;
    case (uint16_t)nukiCommand::advancedConfig :
      log_e("advancedConfig");
      break;
    case (uint16_t)nukiCommand::addTimeControlEntry :
      log_e("addTimeControlEntry");
      break;
    case (uint16_t)nukiCommand::timeControlEntryId :
      log_e("timeControlEntryId");
      break;
    case (uint16_t)nukiCommand::removeTimeControlEntry :
      log_e("removeTimeControlEntry");
      break;
    case (uint16_t)nukiCommand::requestTimeControlEntries :
      log_e("requestTimeControlEntries");
      break;
    case (uint16_t)nukiCommand::timeControlEntryCount :
      log_e("timeControlEntryCount");
      break;
    case (uint16_t)nukiCommand::timeControlEntry :
      log_e("timeControlEntry");
      break;
    case (uint16_t)nukiCommand::updateTimeControlEntry :
      log_e("updateTimeControlEntry");
      break;
    case (uint16_t)nukiCommand::addKeypadCode :
      log_e("addKeypadCode");
      break;
    case (uint16_t)nukiCommand::keypadCodeId :
      log_e("keypadCodeId");
      break;
    case (uint16_t)nukiCommand::requestKeypadCodes :
      log_e("requestKeypadCodes");
      break;
    case (uint16_t)nukiCommand::keypadCodeCount :
      log_e("keypadCodeCount");
      break;
    case (uint16_t)nukiCommand::keypadCode :
      log_e("keypadCode");
      break;
    case (uint16_t)nukiCommand::updateKeypadCode :
      log_e("updateKeypadCode");
      break;
    case (uint16_t)nukiCommand::removeKeypadCode :
      log_e("removeKeypadCode");
      break;
    case (uint16_t)nukiCommand::keypadAction :
      log_e("keypadAction");
      break;
    case (uint16_t)nukiCommand::simpleLockAction :
      log_e("simpleLockAction");
      break;
    default:
      log_e("UNKNOWN RETURN COMMAND");
    }
}


void NukiBle::pushNotificationToQueue(){
    bool notification = true;
    xQueueSendFromISR(nukiBleIsrFlagQueue,&notification, &xHigherPriorityTaskWoken);
}

void NukiBle::my_gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t* param) {
	ESP_LOGW(LOG_TAG, "custom gattc event handler, event: %d", (uint8_t)event);
    if(event == ESP_GATTC_DISCONNECT_EVT) {
      Serial.print("Disconnect reason: "); 
      Serial.println((int)param->disconnect.reason);
    }
}

void NukiBle::onConnect(BLEClient*){
  log_d("BLE connected");
};

void NukiBle::onDisconnect(BLEClient*){
    log_d("BLE disconnected");
};

void NukiBle::keyGen(uint8_t *key, uint8_t keyLen, uint8_t seedPin){
  randomSeed(analogRead(seedPin));
  for(int i=0; i < keyLen; i++){
    key[i] = random(1, 255);
  }
  printBuffer((byte*)key, keyLen, false, "Generated key");
}


// static void calculate_authenticator(uint8_t* output_buffer, uint8_t* message, uint16_t message_length) {
// 	crypto_auth_hmacsha256(output_buffer, message, message_length, pairing_ctx.shared_secret);
// }

// uint16_t create_authorization_authenticator_payload(uint8_t* output_buffer, uint8_t* received_data) 
// {
// 	uint8_t* nonce = &received_data[2];
// 	uint16_t command_length = 36;
// 	write_uint16LE(output_buffer, authorization_authenticator_cmd, 0);

// 	//Shared key calculation
// 	uint8_t dh_key[32]; //crypto_scalarmult_BYTES
// 	int ret = crypto_scalarmult_curve25519(dh_key, private_key_fob, public_key_nuki);
	
// 	if(ret == -1)
// 	{
// 		ESP_LOGE("NUKI Authorization", "Error in Crypto Scalarmult");
// 		return 0;
// 	}
	
// 	unsigned char _0[16];
// 	memset(_0, 0, 16);
// 	const unsigned char sigma[16] = "expand 32-byte k";
// 	crypto_core_hsalsa20(pairing_ctx.shared_secret, _0, dh_key, sigma);

// 	const uint16_t r_length = 32 + 32 + PAIRING_NONCEBYTES; 
// 	uint8_t r[32 + 32 + PAIRING_NONCEBYTES]; 
// 	memcpy(r, public_key_fob, 32);
// 	memcpy(&r[32], public_key_nuki, 32);
// 	memcpy(&r[32 + 32], nonce, PAIRING_NONCEBYTES);
// 	uint8_t authenticator[32];
// 	calculate_authenticator(authenticator, r, r_length);
// 	memcpy(&output_buffer[2], authenticator, 32);
// 	crc_payload(output_buffer, command_length);
	
// 	esp_log_buffer_hex("PUBLIC KEY FOB: ", public_key_fob, 32);
// 	esp_log_buffer_hex("PUBLIC KEY NUKI: ", public_key_nuki, 32);
// 	esp_log_buffer_hex("NONCE: ", nonce, 32);
// 	esp_log_buffer_hex("r: ", r, r_length);
// 	esp_log_buffer_hex("AUTHENTICATOR: ", authenticator, 32);
	
// 	return command_length;
// }