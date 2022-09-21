#pragma once
#include <cstdint>
#include <cinttypes>
#include <cfloat>
#include <climits>
#include <cstddef>

#define NOT_VALID  -1

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86))
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

//=============================================================================================================================================================================
// Byte swap
//=============================================================================================================================================================================
#if defined(_MSC_VER)
static inline uint16_t xSwapBytes16(uint16_t Value) { return _byteswap_ushort(Value); }
static inline  int16_t xSwapBytes16( int16_t Value) { return _byteswap_ushort(Value); }
static inline uint32_t xSwapBytes32(uint32_t Value) { return _byteswap_ulong (Value); }
static inline  int32_t xSwapBytes32( int32_t Value) { return _byteswap_ulong (Value); }
static inline uint64_t xSwapBytes64(uint64_t Value) { return _byteswap_uint64(Value); }
static inline  int64_t xSwapBytes64( int64_t Value) { return _byteswap_uint64(Value); }
#elif defined (__GNUC__)
static inline uint16_t xSwapBytes16(uint16_t Value) { return __builtin_bswap16(Value); }
static inline  int16_t xSwapBytes16( int16_t Value) { return __builtin_bswap16(Value); }
static inline uint32_t xSwapBytes32(uint32_t Value) { return __builtin_bswap32(Value); }
static inline  int32_t xSwapBytes32( int32_t Value) { return __builtin_bswap32(Value); }
static inline uint64_t xSwapBytes64(uint64_t Value) { return __builtin_bswap64(Value); }
static inline  int64_t xSwapBytes64( int64_t Value) { return __builtin_bswap64(Value); }
#else
#error Unrecognized compiler
#endif


static inline uint16_t From8To16(uint8_t hi, uint8_t lo) {
    uint16_t result = (uint16_t)hi;
    result <<= 8;
    return result | lo;
}

static inline uint16_t From8To24(uint8_t hi, uint8_t mid, uint8_t lo) {
    uint16_t result = (uint16_t)hi;
    result <<= 8;
    result |= mid;
    result <<= 8;
    return result | lo;
}

static inline uint32_t From8To32(uint8_t first, uint8_t second, uint8_t third = 0x0, uint8_t fourth = 0x0) {
    uint32_t result = (uint32_t)first;
    result <<= 8;
    result |= second;
    result <<= 8;
    result |= third;
    result <<= 8;
    return  result | fourth;
}

static inline uint32_t From8To64(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth,
                                uint8_t fifth, uint8_t seventh, uint8_t eigth, uint8_t ninth) {
    uint64_t result = (uint64_t)From8To32(first, second, third, fourth);
    result <<= 32;
    return  uint64_t(result | From8To32(fifth, seventh, eigth, ninth));
}

// 00000000 00000000 00000000 00000000