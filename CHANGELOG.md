## V0.0.7 (2022-05-17)
- tweaked semaphore timeouts and delays
- added option to check if communication is done and then disconnect ble (saves battery and speeds up getting advertisements)
- improved semaphore logging

## V0.0.6 (2022-05-03)
- Removed local BLE scanner from library to be able to use the same scanner for multiple BLE devices in 1 project. https://github.com/I-Connect/BleScanner.git can be used.
- Added/updated header to all files
- Added documentation to all public methods
- Set NimBle version to be used
- Added changelog 

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