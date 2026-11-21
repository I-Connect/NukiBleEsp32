#include "NukiUtils.h"

#include "sodium/crypto_secretbox.h"
#include "psa/crypto.h"
#include "Crc16.h"


namespace Nuki {

void printBuffer(const byte* buff, const uint8_t size, const boolean asChars, const char* header, bool debug, Print* Log) {
  if (Log == nullptr) {
    Log = &Serial;
  }

  if (debug) {
    delay(10); //delay otherwise first part of print will not be shown
    char tmp[16];

    if (strlen(header) > 0) {
      Log->print(header);
      Log->print(": ");
    }
    for (int i = 0; i < size; i++) {
      if (asChars) {
        Log->print((char)buff[i]);
      } else {
        sprintf(tmp, "%02x", buff[i]);
        Log->print(tmp);
        Log->print(" ");
      }
    }
    Log->println();
  }
}

bool isCharArrayNotEmpty(unsigned char* array, uint16_t len) {
  for (size_t i = 0; i < len; i++) {
    if (array[i] != 0) {
      return true;
    }
  }
  return false;
}

bool isCharArrayEmpty(unsigned char* array, uint16_t len) {
  for (size_t i = 0; i < len; i++) {
    if (array[i] != 0) {
      return false;
    }
  }
  return true;
}

bool compareCharArray(unsigned char* a, unsigned char* b, uint8_t len) {
  for (int i = 0; i < len; i++) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

bool generateKeyPair(unsigned char* publicKey, unsigned char* privateKey) {
  psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
  psa_key_id_t key = 0;
  size_t length = 0;

  psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_MONTGOMERY));
  psa_set_key_bits(&attributes, 255);
  psa_set_key_algorithm(&attributes, PSA_ALG_ECDH);
  psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE | PSA_KEY_USAGE_EXPORT);

  if (psa_crypto_init() != PSA_SUCCESS || psa_generate_key(&attributes, &key) != PSA_SUCCESS) {
    return false;
  }

  bool result = psa_export_key(key, privateKey, 32, &length) == PSA_SUCCESS && length == 32
                && psa_export_public_key(key, publicKey, 32, &length) == PSA_SUCCESS && length == 32;
  psa_destroy_key(key);
  return result;
}

bool keyAgreement(unsigned char* sharedKey, const unsigned char* privateKey, const unsigned char* remotePublicKey) {
  psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
  psa_key_id_t key = 0;
  size_t length = 0;

  psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_MONTGOMERY));
  psa_set_key_bits(&attributes, 255);
  psa_set_key_algorithm(&attributes, PSA_ALG_ECDH);
  psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE);

  if (psa_crypto_init() != PSA_SUCCESS || psa_import_key(&attributes, privateKey, 32, &key) != PSA_SUCCESS) {
    return false;
  }

  bool result = psa_raw_key_agreement(PSA_ALG_ECDH, key, remotePublicKey, 32, sharedKey, 32, &length) == PSA_SUCCESS && length == 32;
  psa_destroy_key(key);
  return result;
}

int encode(unsigned char* output, unsigned char* input, unsigned long long len, unsigned char* nonce, unsigned char* keyS, Print* Log) {
  int result = crypto_secretbox_easy(output, input, len, nonce, keyS);

  if (result) {
    if (Log == nullptr) {
      log_d("Encryption failed (length %i, given result %i)\n", len, result);
    }
    else
    {
      Log->printf("Encryption failed (length %i, given result %i)\r\n", len, result);
    }
    return -1;
  }
  return len;
}

int decode(unsigned char* output, unsigned char* input, unsigned long long len, unsigned char* nonce, unsigned char* keyS, Print* Log) {

  int result = crypto_secretbox_open_easy(output, input, len, nonce, keyS);

  if (result) {
    if (Log == nullptr) {
      log_w("Decryption failed (length %i, given result %i)\n", len, result);
    }
    else
    {
      Log->printf("Decryption failed (length %i, given result %i)\r\n", len, result);
    }
    return -1;
  }
  return len;
}

void generateNonce(unsigned char* hexArray, uint8_t nrOfBytes, bool debug, Print* Log) {
  randomSeed(millis());
  for (int i = 0 ; i < nrOfBytes ; i++) {
    hexArray[i] = random(0, 255);
  }
  printBuffer((byte*)hexArray, nrOfBytes, false, "Nonce", debug, Log);
}

unsigned int calculateCrc(uint8_t* data, uint8_t start, uint16_t length) {
  Crc16 crcObj;
  crcObj.clearCrc();
  // CCITT-False:	width=16 poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1
  return crcObj.fastCrc(data, start, length, false, false, 0x1021, 0xffff, 0x0000, 0x8000, 0xffff);
}

bool crcValid(uint8_t* pData, uint16_t length, bool debug, Print* Log) {
  uint16_t receivedCrc = ((uint16_t)pData[length - 1] << 8) | pData[length - 2];
  uint16_t dataCrc = calculateCrc(pData, 0, length - 2);

  if (!(receivedCrc == dataCrc)) {
    if (Log == nullptr) {
      log_e("CRC CHECK FAILED!");
    }
    else
    {
      Log->println("CRC CHECK FAILED!");
    }
    return false;
  }
  if (debug) {
    if (Log == nullptr) {
      log_d("CRC CHECK OK");
    }
    else
    {
      Log->println("CRC CHECK OK");
    }
  }
  return true;
}

template<std::size_t N>
uint8_t getWeekdaysIntFromBitset(const std::bitset<N> bits) {
  uint8_t result = 0;
  for (auto idx = 0; idx < 7 && idx < N ; idx++) {
    result |= bits[idx] << (7 - idx);
  }
  return result;
}
}