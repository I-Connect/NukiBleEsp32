//Keyturner initialization service
#define keyturnerInitServiceUUID           a92ee000-5501-11e4-916c-0800200c9a66
//Keyturner Pairing Service 
#define keyturnerPairingServiceUUID        a92ee100-5501-11e4-916c-0800200c9a66
//Keyturner pairing Data Input Output characteristic
#define keyturnerGdioUUID                  a92ee101-5501-11e4-916c-0800200c9a66
//Keyturner Service
#define keyturnerServiceUUID               a92ee200-5501-11e4-916c-0800200c9a66
//Keyturner Data Input Output characteristic
#define keyturnerDataUUID                  a92ee201-5501-11e4-916c-0800200c9a66
//User-Specific Data Input Output characteristic
#define userDataUUID                       a92ee202-5501-11e4-916c-0800200c9a66


// static uint8_t private_key[32] = {0x8C, 0xAA, 0x54, 0x67, 0x23, 0x07, 0xBF, 0xFD, 0xF5, 0xEA, 0x18, 0x3F, 0xC6, 0x07, 0x15, 0x8D, 0x20, 0x11, 0xD0, 0x08, 0xEC, 0xA6, 0xA1, 0x08, 0x86, 0x14, 0xFF, 0x08, 0x53, 0xA5, 0xAA, 0x07};
// static uint8_t public_key[32] = {0xF8, 0x81, 0x27, 0xCC, 0xF4, 0x80, 0x23, 0xB5, 0xCB, 0xE9, 0x10, 0x1D, 0x24, 0xBA, 0xA8, 0xA3, 0x68, 0xDA, 0x94, 0xE8, 0xC2, 0xE3, 0xCD, 0xE2, 0xDE, 0xD2, 0x9C, 0xE9, 0x6A, 0xB5, 0x0C, 0x15};

#define STRINGIFY_(x) #x
#define STRING(VAR) STRINGIFY_(VAR)

enum class nukiCommand {
    requestData	                = 0x0001,
    publicKey	                = 0x0003,
    challenge	                = 0x0004,
    authorizationAuthenticator	= 0x0005,
    authorizationData	        = 0x0006,
    authorizationId	            = 0x0007,
    removeUserAuthorization	    = 0x0008,
    requestAuthorizationEntries	= 0x0009,
    authorizationEntry	        = 0x000A,
    authorizationDatInvite	    = 0x000B,
    keyturnerStates	            = 0x000C,
    lockAction	                = 0x000D,
    status	                    = 0x000E,
    mostRecentCommand	        = 0x000F,
    openingsClosingsSummary	    = 0x0010,
    batteryReport	            = 0x0011,
    errorReport	                = 0x0012,
    setConfig	                = 0x0013,
    requestConfig	            = 0x0014,
    config	                    = 0x0015,
    setSecurityPin	            = 0x0019,
    requestCalibration	        = 0x001A,
    requestReboot	            = 0x001D,
    authorizationIdConfirmation	= 0x001E,
    authorizationIdInvite	    = 0x001F,
    verifySecurityPin	        = 0x0020,
    updateTime	                = 0x0021,
    updateUserAuthorization	    = 0x0025,
    authorizationEntryCount	    = 0x0027,
    requestLogEntries	        = 0x0031,
    logEntry	                = 0x0032,
    logEntryCount	            = 0x0033,
    enableLogging	            = 0x0034,
    setAdvancedConfig	        = 0x0035,
    requestAdvancedConfig	    = 0x0036,
    advancedConfig	            = 0x0037,
    addTimeControlEntry	        = 0x0039,
    timeControlEntryId	        = 0x003A,
    removeTimeControlEntry	    = 0x003B,
    requestTimeControlEntries	= 0x003C,
    timeControlEntryCount	    = 0x003D,
    timeControlEntry	        = 0x003E,
    updateTimeControlEntry	    = 0x003F,
    addKeypadCode	            = 0x0041,
    keypadCodeId	            = 0x0042,
    requestKeypadCodes	        = 0x0043,
    keypadCodeCount	            = 0x0044,
    keypadCode	                = 0x0045,
    updateKeypadCode	        = 0x0046,
    removeKeypadCode	        = 0x0047,
    keypadAction	            = 0x0048,
    simpleLockAction	        = 0x0100	
};

enum class nukiErrorCode {
    ERROR_BAD_CRC	                = 0xFD,
    ERROR_BAD_LENGTH	            = 0xFE,
    ERROR_UNKNOWN	                = 0xFF,
    P_ERROR_NOT_PAIRING	            = 0x10,
    P_ERROR_BAD_AUTHENTICATOR	    = 0x11,
    P_ERROR_BAD_PARAMETER	        = 0x12,
    P_ERROR_MAX_USER	            = 0x13,
    K_ERROR_NOT_AUTHORIZED	        = 0x20,
    K_ERROR_BAD_PIN	                = 0x21,
    K_ERROR_BAD_NONCE	            = 0x22,
    K_ERROR_BAD_PARAMETER	        = 0x23,
    K_ERROR_INVALID_AUTH_ID	        = 0x24,
    K_ERROR_DISABLED	            = 0x25,
    K_ERROR_REMOTE_NOT_ALLOWED	    = 0x26,
    K_ERROR_TIME_NOT_ALLOWED	    = 0x27,
    K_ERROR_TOO_MANY_PIN_ATTEMPTS	= 0x28,
    K_ERROR_TOO_MANY_ENTRIES	    = 0x29,
    K_ERROR_CODE_ALREADY_EXISTS	    = 0x2A,
    K_ERROR_CODE_INVALID	        = 0x2B,
    K_ERROR_CODE_INVALID_TIMEOUT_1  = 0x2C,
    K_ERROR_CODE_INVALID_TIMEOUT_2	= 0x2D,
    K_ERROR_CODE_INVALID_TIMEOUT_3	= 0x2E,
    K_ERROR_AUTO_UNLOCK_TOO_RECENT	= 0x40,
    K_ERROR_POSITION_UNKNOWN	    = 0x41,
    K_ERROR_MOTOR_BLOCKED	        = 0x42,
    K_ERROR_CLUTCH_FAILURE	        = 0x43,
    K_ERROR_MOTOR_TIMEOUT	        = 0x44,
    K_ERROR_BUSY	                = 0x45,
    K_ERROR_CANCELED	            = 0x46,
    K_ERROR_NOT_CALIBRATED	        = 0x47,
    K_ERROR_MOTOR_POSITION_LIMIT	= 0x48,
    K_ERROR_MOTOR_LOW_VOLTAGE	    = 0x49,
    K_ERROR_MOTOR_POWER_FAILURE	    = 0x4A,
    K_ERROR_CLUTCH_POWER_FAILURE	= 0x4B,
    K_ERROR_VOLTAGE_TOO_LOW	        = 0x4C,
    K_ERROR_FIRMWARE_UPDATE_NEEDED	= 0x4D
};

enum class lockAction {
    unlock          = 0x01,
    lock            = 0x02,
    unlatch         = 0x03,
    lockNgo         = 0x04,
    lockNgoUnlatch  = 0x05,
    fullLock        = 0x06,
    fobAction1      = 0x81,
    fobAction2      = 0x82,
    fobAction3      = 0x83
};