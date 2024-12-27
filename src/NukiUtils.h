#pragma once

/**
 * @file NukiUtills.h
 * Implementation of generic/helper functions
 *
 * Created on: 2022
 * License: GNU GENERAL PUBLIC LICENSE (see LICENSE)
 *
 * This library implements the communication from an ESP32 via BLE to a Nuki smart lock.
 * Based on the Nuki Smart Lock API V2.2.1
 * https://developer.nuki.io/page/nuki-smart-lock-api-2/2/
 *
 */

#include "Arduino.h"
#include "NukiDataTypes.h"
#include "NukiConstants.h"
#include <bitset>

namespace Nuki {

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00)>>8) + (((x)&0xFF)<<8))

void printBuffer(const byte* buff, const uint8_t size, const boolean asChars, const char* header, bool debug = false, Print* Log = nullptr);
bool isCharArrayNotEmpty(unsigned char* array, uint16_t len);
bool isCharArrayEmpty(unsigned char* array, uint16_t len);
bool compareCharArray(unsigned char* a, unsigned char* b, uint8_t len);
int encode(unsigned char* output, unsigned char* input, unsigned long long len, unsigned char* nonce, unsigned char* keyS, Print* Log = nullptr);
int decode(unsigned char* output, unsigned char* input, unsigned long long len, unsigned char* nonce, unsigned char* keyS, Print* Log = nullptr);
void generateNonce(unsigned char* hexArray, uint8_t nrOfBytes, bool debug = false, Print* Log = nullptr);

unsigned int calculateCrc(uint8_t data[], uint8_t start, uint16_t length);
bool crcValid(uint8_t* pData, uint16_t length, bool debug = false, Print* Log = nullptr);

/**
 * @brief Translate a bitset<N> into Nuki weekdays int
 *
 * @tparam N
 * @param bitset with bitset[0] = Monday ... bitset[7] = Sunday
 * @return uint8_t with bit6 = Monday ... bit0 = Sunday
 */
template<std::size_t N>
uint8_t getWeekdaysIntFromBitset(const std::bitset<N> bits);
}