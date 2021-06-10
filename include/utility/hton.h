
#pragma once

#include <cstdint>

enum class endian {
#ifdef _WIN32
  little = 0,
  big = 1,
  native = little
#else
  little = __ORDER_LITTLE_ENDIAN__,
  big = __ORDER_BIG_ENDIAN__,
  native = __BYTE_ORDER__
#endif
};

inline uint32_t htonl(uint32_t value) {
  if constexpr (endian::native == endian::big) {
    return value;
  } else {
    union {
      uint8_t bytes[4];
      uint32_t host_value;
    };
    host_value = value;
    uint8_t swap = bytes[0];
    bytes[0] = bytes[3];
    bytes[3] = swap;
    swap = bytes[1];
    bytes[1] = bytes[2];
    bytes[2] = swap;
    return host_value;
  }
}

inline uint16_t htons(uint16_t value) {
  if constexpr (endian::native == endian::big) {
    return value;
  } else {
    union {
      uint8_t bytes[2];
      uint16_t host_value;
    };
    host_value = value;
    uint8_t swap = bytes[0];
    bytes[0] = bytes[1];
    bytes[1] = swap;
    return host_value;
  }
}

inline uint32_t ntohl(uint32_t value) { return htonl(value); }

inline uint16_t ntohs(uint16_t value) { return htons(value); }
