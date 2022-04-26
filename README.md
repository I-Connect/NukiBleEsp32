# Nuki BLE for Esp32
This lib is made for communicating directly to a Nuki smart lock via BLE without the need of a Nuki Bridge.
Implementation is according to [Nuki Smart Lock BLE API](https://developer.nuki.io/page/nuki-smart-lock-api-2/2/) 
(kudo's to the Nuki developers for providing such an accurate and well made documentation!)

Implementation is based on Espressif platform version 4.x.x

## How to use
This library is runnable as is.

When running main.cpp (with #define DEBUG_NUKI_CONNECT) there will initially be some logging "No nuki in pairing mode found", if you then press and hold the button on the Nuki lock for 10 secs (untill the led ring lights up) the esp should automatically find the lock in pairing mode and pair with it.
Credentials will be saved and no pairing needs to be done the next time the esp starts.

There are some example methods in main.cpp to get/write data and execute actions on the lock.

Be aware that if you have set a pincode on the lock you will have to store this in the esp using nukiBle.savePincode() otherwise the methods that need a pincode (most methods that write settings) will fail.
This only needs to be done once as the pincode will be stored in the preferences.

Logging can be enabled by setting the following defines (these are also available in platformio.ini):
- DEBUG_NUKI_CONNECT
- DDEBUG_NUKI_COMMUNICATION
- DEBUG_NUKI_HEX_DATA
- DEBUG_NUKI_READABLE_DATA

(More documentation will be added later)

## BT processes
- The ESP establishes a new BT connection every time a command is sent, when no data is sent anymore the lock times out the connection.
- Scanning goes on continuously on the ESP with intervals chosen in such a way that it will never miss an advertisement sent from the lock.
- The lock always continuously sends advertisements (the interval is a setting in the config ( CmdResult setAdvertisingMode(AdvertisingMode mode); ), this interval determines the battery drain on the lock). When the lock state is changed a parameter is changed in the advertisement. This causes the SmartLockEventHandler to be triggered and then you could initiate a follow up like requesting the keyturner state.

## Tested Hardware
- ESP32 wroom
- Nuki smart lock v2
- Nuki smart lock v3

## V0.0.5
- Updated to Espressif platform 4.x.x
- Fixed battery indications
- Some general refactoring

## V0.0.4
- Added semaphores to make it (more) threadsafe
- Fixed handling payload len in lock action in case in case a namesuffix is used
- Some general refactoring

## V0.0.3
- Cleanup and refactor
- Fixed loosing pincode on re-pairing
- Updated scanning intervals according to recommendations Nuki dev
- Made BLE scanner injectable

## V0.0.2
- Added eventhandler

## V0.0.1
lib is ready for beta testing, most if not all Nuki lock v2 functionality is implemented.
Most of the basic methods have been tested, some of the more advanced (mostly settings related) methods still need to be tested
There can still be braking changes....!
Implementation is according to Nuki Smart Lock API V2.2.1 (22.06.2021)

## Wip
- Add documentation to the readme and classes

## Todo
- Some data integrity could be checked
