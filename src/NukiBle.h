#pragma once
/*
 * nukiBle.h
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "BLEDevice.h"
#include "nukiConstants.h"
#include "Arduino.h"

class NukiBle : public BLEClientCallbacks{
  public:
    NukiBle(std::string& bleAddress);
    virtual ~NukiBle();

    void onConnect(BLEClient*) override {};
    void onDisconnect(BLEClient*) override {
        // pairedStatus(false);  
        log_d("onDisconnect");
    };

    virtual void initialize();
    bool pairBle();
    bool isPaired = false;
    void pairedStatus(bool status){
      isPaired = status;
    }

    bool executeLockAction(lockAction action);

//   protected:

  private:
    
    bool registerOnGdioChar();
    void sendPlainMessage(nukiCommand commandIdentifier, char* payload);
    static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
    
    std::string bleAddress = "";
    BLEClient* pClient;
    BLERemoteService* pKeyturnerService;
    BLERemoteService* pKeyturnerPairingService;
    BLERemoteService* pkeyturnerInitService;
    BLERemoteCharacteristic* pGdioCharacteristic;
};

// class MyClientCallback: public BLEClientCallbacks {
//     public:
//     // friend NukiBle;
//     MyClientCallback();
//     virtual ~MyClientCallback() {};
//     void onConnect(BLEClient*) {};
//     void onDisconnect(BLEClient*) {
//         // pairedStatus(false);  
//         log_d("onDisconnect");
//     };
// };