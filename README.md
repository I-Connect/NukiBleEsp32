# Nuki BLE for Esp32
This lib is made for communicating directly to a Nuki smart lock via BLE without the need of a Nuki Bridge.
Implementation is according to [Nuki Smart Lock BLE API](https://developer.nuki.io/page/nuki-smart-lock-api-2/2/) 
(kudo's to the Nuki developers for providing such an accurate and well made documentation!)

From v0.0.5 onwards Implementation is based on Espressif platform version 4.x.x

## How to use
This library is runnable as is in combination with the BLE scanner (https://github.com/I-Connect/BleScanner.git).
The BLE scanner should be added to the platformio.ini (lib_deps =) and registered as can be seen in the NukiSmartLockTest.h example.

When running main.cpp which includes the example NukiSmartLockTest.h (with #define DEBUG_NUKI_CONNECT) there will initially be some logging "No nuki in pairing mode found", if you then press and hold the button on the Nuki lock for 10 secs (untill the led ring lights up) the esp should automatically find the lock in pairing mode and pair with it.
Credentials will be saved and no pairing needs to be done the next time the esp starts.

There are some example methods in NukiSmartLockTest.h to get/write data and execute actions on the lock.

Be aware that if you have set a pincode on the lock you will have to store this in the esp using nukiBle.savePincode() otherwise the methods that need a pincode (most methods that write settings) will fail.
This only needs to be done once as the pincode will be stored in the preferences.

Logging can be enabled by setting the following defines (these are also available in platformio.ini):
- DEBUG_NUKI_CONNECT
- DDEBUG_NUKI_COMMUNICATION
- DEBUG_NUKI_HEX_DATA
- DEBUG_NUKI_READABLE_DATA

## BT processes
- The ESP establishes a new BT connection every time a command is sent, when no data is sent anymore the lock times out the connection.
- Scanning goes on continuously on the ESP with intervals chosen (in the BLE scanner) in such a way that it will never miss an advertisement sent from the lock.
- The lock always continuously sends advertisements (the interval is a setting in the config ( CmdResult setAdvertisingMode(AdvertisingMode mode); ), this interval determines the battery drain on the lock). When the lock state is changed a parameter is changed in the advertisement. This causes the SmartLockEventHandler to be triggered and then you could initiate a follow up like requesting the keyturner state.

## Tested Hardware
- ESP32 wroom
- Nuki smart lock v2
- Nuki smart lock v3

## Wip
- ..

## Todo
- Some data integrity could be checked
