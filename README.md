Note that generated documentation can be found here: https://i-connect.github.io/NukiBleEsp32/

# Nuki BLE for Esp32
This lib is made for communicating directly to a Nuki smart lock or a Nuki opener via BLE without the need of a Nuki Bridge.
Implementation is according to [Nuki Smart Lock BLE API](https://developer.nuki.io/page/nuki-smart-lock-api-2/2/) 
(kudo's to the Nuki developers for providing such an accurate and well made documentation!)

From v0.0.5 onwards Implementation is based on Espressif platform version 4.x.x

## How to use
This library makes use of BLE scanner to receive the Ble advertisements sent by Nuki devices and other Ble devices (https://github.com/I-Connect/BleScanner.git).

When running `main.cpp` which includes the example NukiSmartLockTest.h (with #define DEBUG_NUKI_CONNECT) there will initially be some logging "No nuki in pairing mode found", if you then press and hold the button on the Nuki lock for 10 secs (until the led ring lights up) the ESP should automatically find the lock in pairing mode and pair with it.
Credentials will be saved in a persistent `Preference` store with store name equal to the devicename specified on construction of the NukiBle object. No pairing needs to be done the next time the ESP starts.

There are some example methods in `NukiSmartLockTest.h` to get/write data and execute actions on the lock.

Be aware that if you have set a pincode on the lock you will have to store this in the esp using `nukiLock.savePincode()` otherwise the methods that need a pincode (most methods that write settings) will fail.
This only needs to be done once as the pincode will be stored in the preferences.

Logging can be enabled by setting the following defines (these are also available in platformio.ini):
- DEBUG_NUKI_CONNECT
- DEBUG_NUKI_COMMUNICATION
- DEBUG_NUKI_HEX_DATA
- DEBUG_NUKI_READABLE_DATA

## Setup
1. Define a `Handler` class derived from `Nuki::SmartlockEventHandler` which will implement the `notify(Nuki::EventType eventType)` method. This method will be called by the `BleScanner` when an advertisement has been received
1. Create instances of `BleScanner::Scanner` and the `Handler`
1. Create an instance of `Nuki::NukiBle` with a devicename and the id of the Nuki App, Nuki Bridge or Nuki Fob to be authorized.
1. Register the NukiBle with the BleScanner
1. Initialize both the scanner and the nukiLock
1. Register an instance of the `Handler` with the `nukiLock`
1. DO NOT execute any BLE actions within the `notify(Nuki::EventType eventType)` method as this runs in the BleScanner context and BLE is not able to handle this simultaniously

        Nuki::NukiLock nukiLock{deviceName, deviceId};
        BleScanner::Scanner scanner;
        Handler handler;

        void setup() {
          scanner.initialize();
          nukiLock.registerBleScanner(&scanner);
          nukiLock.initialize();
          nukiLock.setEventHandler(&handler);
        }

        void loop() {
          scanner.update();
          delay(10);
        }

## Nuki opener

The setup for the opener is very much the same as for the lock, except you create a NukiOpener object instead of a NukiLock object.
Most functionality is shared between lock and opener, except for device-specific functionality.

For example:
- The lock action for the opener is different (e. g. ElectricStrikeActuation or activateRTO instead of unlock)
- The reported state is different (e. g. unlocked vs RTOactive)
- Config entries are different (e.g. The opener supports sounds, the lock doesn't)

## BT processes
- The ESP establishes a new BT connection every time a command is sent, when no data is sent anymore the lock times out the connection.
- Scanning goes on continuously on the ESP with intervals chosen (in the BLE scanner) in such a way that it will never miss an advertisement sent from the lock.
- The lock always continuously sends advertisements (the interval is a setting in the config ( `CmdResult setAdvertisingMode(AdvertisingMode mode);` ), this interval determines the battery drain on the lock). When the lock state is changed a parameter is changed in the advertisement. This causes `SmartLockEventHandler::notify(...)` to be called and then you could initiate a follow up like requesting the keyturner state.

## Tested Hardware
- ESP32 wroom
- Nuki smart lock v2
- Nuki smart lock v3

## Wip
- ..

## Todo
- Some data integrity could be checked
- Add `const` qualifiers where needed
