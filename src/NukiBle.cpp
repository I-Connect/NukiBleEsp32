/*
 * NukiBle.cpp
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "NukiBle.h"
#include "crc16.h"
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

  //send public key command
  // char payload[100];
  // sprintf(payload, "%04x", (uint16_t)nukiCommand::publicKey);


  // uint16_t swappedPayload = ((uint16_t)nukiCommand::publicKey>>8) | ((uint16_t)nukiCommand::publicKey<<8);
  uint16_t payload = (uint16_t)nukiCommand::publicKey;
  sendPlainMessage(nukiCommand::requestData, (char*)&payload, sizeof(payload));
  log_d("Sent message should be 0100030027A7");

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
  CRC16 crcObj;
  uint16_t dataCrc;
  // uint16_t tempPayload = 3;  //public key command. This needs to change to be able to receive n characters as payload

  //message sent needs to be little endian
  // uint16_t swappedCommandIdentifier = ((uint16_t)commandIdentifier>>8) | ((uint16_t)commandIdentifier<<8);
 

  //get crc over data
  char dataToSend[200];
  // sprintf(dataToSend, "%04x%04x", swappedCommandIdentifier, swappedPayload);
  // log_d("Data to send: %s", dataToSend);
  
  memcpy(&dataToSend, &commandIdentifier, sizeof(commandIdentifier));
  memcpy(&dataToSend[2], payload, payloadLen);
 
  crcObj.processBuffer(dataToSend, payloadLen + 2);
  dataCrc = crcObj.getCrc();
  // dataCrc = calc_crc(dataToSend, strlen(dataToSend), 0xFFFF);
  // uint16_t swappedCrc = (dataCrc>>8) | (dataCrc<<8);
  log_d("CRC: %04x", dataCrc);

  // char msgToSend[200];
  // sprintf(msgToSend, "%s%x", dataToSend, swappedCrc);
  // log_d("msgtosend: %s", msgToSend);
  memcpy(&dataToSend[2+payloadLen], &dataCrc, sizeof(dataCrc));

  //using temp message for testing
    // uint8_t arrayFV[] = {0x01,0x00,0x03,0x00,0x27,0xA7}; //"0x0100030027A7";//first value to send as an array of byte to initiate Nuki Pairing

    // log_d("Sending plain message (%d). Command identifier: %02x, payload: %s, CRC: %x", arrayFV, (uint32_t)commandIdentifier, payload, swappedCrc);
    // pGdioCharacteristic->writeValue(arrayFV, sizeof(arrayFV), true);
  //end test

  log_d("Sending plain message %02x%02x%02x%02x%02x%02x", dataToSend[0], dataToSend[1], dataToSend[2], dataToSend[3], dataToSend[4] , dataToSend[5]);
  log_d("Command identifier: %02x, CRC: %x", (uint32_t)commandIdentifier, dataCrc);
  pGdioCharacteristic->writeValue((uint8_t*)dataToSend, payloadLen + 4, true);
  delay(1000);
  log_d("received data: %x", pGdioCharacteristic->readValue());

}

bool NukiBle::executeLockAction(lockAction aLockAction) {
  log_d("Executing lock action: %d", aLockAction);
  return true;
}

void NukiBle::notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  log_d(" Notify callback for characteristic: %s of length: %d", pBLERemoteCharacteristic->getUUID().toString().c_str(), length);
  printBuffer((byte*)pData, length, false, "Received data");
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

uint16_t NukiBle::calc_crc(char *msg,int n,uint16_t init){
  uint16_t x = init;

  while(n--)
  {
    x = crc_xmodem_update(x, (uint8_t)*msg++);
  }

  return(x);
}

uint16_t NukiBle::crc_xmodem_update (uint16_t crc, uint8_t data){
  int i;

  crc = crc ^ ((uint16_t)data << 8);
  for (i=0; i<8; i++)
  {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021; //(polynomial = 0x1021)
    else
      crc <<= 1;
  }
  return crc;
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