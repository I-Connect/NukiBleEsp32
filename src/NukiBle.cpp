/*
 * NukiBle.cpp
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "NukiBle.h"
#include "crc16.h"
#include "string.h"

NukiBle::NukiBle(std::string &bleAddress): bleAddress(bleAddress){}

NukiBle::~NukiBle() {}

void NukiBle::initialize() {
  log_d("Initializing Nuki");
}

bool NukiBle::pairBle() {
  log_d("Pairing to: %s ", bleAddress.c_str());
      
  pClient  = BLEDevice::createClient();
  pClient->setClientCallbacks(this);

  pClient->connect(bleAddress);

  registerOnGdioChar();

  return false;
}

bool NukiBle::registerOnGdioChar(){
  // check if keyturnerInitServiceUUID is visible
  log_d("keyturnerInitServiceUUID UUID: %s", STRING(keyturnerInitServiceUUID));
  pkeyturnerInitService = pClient->getService(STRING(keyturnerInitServiceUUID));
  if (pkeyturnerInitService == nullptr) {
    log_d("Nuki not in pairing mode, press front button for 5 seconds");
    return false;
  }
  else{
    log_d("Nuki in pairing mode");
  }

  // Obtain a reference to the KeyTurner service
  log_d("keyturner UUID: %s", STRING(keyturnerServiceUUID));
  pKeyturnerService = pClient->getService(STRING(keyturnerServiceUUID));
  if (pKeyturnerService == nullptr) {
    log_d("Failed to find keyturner Service UUID: %s", STRING(keyturnerServiceUUID));
    pClient->disconnect();
    return false;
  }
  else{
    log_d("Found keyturner Service UUID: %s", STRING(keyturnerServiceUUID));
    //Obtain reference to GDIO char
    pGdioCharacteristic = pKeyturnerService->getCharacteristic(STRING(keyturnerPairingDataUUID));
    pGdioCharacteristic->registerForNotify(notifyCallback);
    if (pGdioCharacteristic == nullptr) {
      log_w("Failed to find GDIO characteristic UUID: %s", STRING(keyturnerServiceUUID));
      pClient->disconnect();
      return false;
    }
    else{
      log_d("Found GDIO characteristic UUID: ");
      if(pGdioCharacteristic->canIndicate()){
        //register for indication on GDIO char
        log_d("GDIO characteristic canIndicate true");
        const uint8_t indicationOn[] = {0x2, 0x0};
        pGdioCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)indicationOn, 2, true);
        char payload[100];
        sprintf(payload, "%d", (uint16_t)nukiCommand::publicKey);
        sendPlainMessage(nukiCommand::requestData, payload);
        log_d("Sent message should be 0100030027A7");

      }
      else{
        log_d("GDIO characteristic canIndicate false, stop connecting");
        return false;
      }
    }  
  }

  return false;
}

void NukiBle::sendPlainMessage(nukiCommand commandIdentifier, char* payload) {
  CRC16 crcObj;
  uint16_t payloadCrc;
  char msgPayload[100];
  
  memcpy(msgPayload, payload, strlen(msgPayload));

  crcObj.processBuffer(msgPayload, strlen(msgPayload));
  payloadCrc = crcObj.getCrc();

  char msgToSend[200];
  sprintf(msgToSend, "%d%s%d", (uint16_t)commandIdentifier, payload, payloadCrc );

  log_d("Sending plain message: %s", msgToSend);
  pGdioCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)msgToSend, strlen(msgToSend), true);
}

bool NukiBle::executeLockAction(lockAction aLockAction) {
  log_d("Executing lock action: %d", aLockAction);
  return true;
}

void NukiBle::notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    log_d(" Notify callback for characteristic: %s of length: %d", pBLERemoteCharacteristic->getUUID().toString().c_str(), length);
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