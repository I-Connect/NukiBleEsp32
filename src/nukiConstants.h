
//Keyturner initialization service
#define keyturnerInitServiceUUID a92ee000-5501-11e4-916c-0800200c9a66
//Keyturner Pairing Service 
#define keyturnerPairingServiceUUID a92ee100-5501-11e4-916c-0800200c9a66
//Keyturner pairing Data Input Output characteristic
#define keyturnerPairingDataUUID a92ee101-5501-11e4-916c-0800200c9a66
//Keyturner Service
#define keyturnerServiceUUID a92ee200-5501-11e4-916c-0800200c9a66
//Keyturner Data Input Output characteristic
#define keyturnerDataUUID a92ee201-5501-11e4-916c-0800200c9a66
//User-Specific Data Input Output characteristic
#define userDataUUID a92ee202-5501-11e4-916c-0800200c9a66

enum class commands{
    lockActionCommand = 0x000D
};

enum class lockAction {
    unlock = 0x01,
    lock = 0x02,
    unlatch = 0x03,
    lockNgo = 0x04,
    lockNgoUnlatch = 0x05,
    fullLock = 0x06,
    fobAction1 = 0x81,
    fobAction2 = 0x82,
    fobAction3 = 0x83
};