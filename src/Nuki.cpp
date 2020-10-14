/*
 * Nuki.cpp
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "Nuki.h"

Nuki::Nuki(){}

Nuki::~Nuki() {}

void Nuki::initialize() {
  log_d("Initializing Nuki");
}

bool Nuki::pairBle(std:string bleAddress) {
    log_d("Forming a connection to ");
        
    BLEClient*  pClient  = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());
 
    pClient->connect(myNukiAddr);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    log_d("Connected to server");
    delay(100);
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      log_d("Failed to find our service UUID: ");
      log_d(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    log_d(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      log_d("Failed to find our characteristic UUID: ");
      log_d(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    log_d(" - Found our characteristic");
    log_d("CanRead : ");
    log_d(pRemoteCharacteristic->canRead()); 
    log_d("CanWrite : ");
    log_d(pRemoteCharacteristic->canWrite()); 
    log_d("CanNotify : ");
    log_d(pRemoteCharacteristic->canNotify()); 
    log_d("CanIndicate : ");
    log_d(pRemoteCharacteristic->canIndicate()); 
   
    connected = true;
    log_d("connected : ");
    log_d(connected); 
    return true;
}

bool Nuki::executeLockAction(lockAction aLockAction) {
  log_d("Executing lock action: %d", aLockAction);
  return true;
}

