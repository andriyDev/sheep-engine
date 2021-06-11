
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

inline uint32_t swap_bytes(uint32_t value) {
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

inline uint16_t swap_bytes(uint16_t value) {
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

// htob, btoh -> Host to big endian

inline uint32_t htobl(uint32_t value) {
  if constexpr (endian::native == endian::big) {
    return value;
  } else {
    return swap_bytes(value);
  }
}

inline uint16_t htobs(uint16_t value) {
  if constexpr (endian::native == endian::big) {
    return value;
  } else {
    return swap_bytes(value);
  }
}

inline uint32_t btohl(uint32_t value) { return htobl(value); }

inline uint16_t btohs(uint16_t value) { return htobs(value); }

// ltoh, htol -> Host to little endian

inline uint32_t htoll(uint32_t value) {
  if constexpr (endian::native == endian::little) {
    return value;
  } else {
    return swap_bytes(value);
  }
}

inline uint16_t htols(uint16_t value) {
  if constexpr (endian::native == endian::little) {
    return value;
  } else {
    return swap_bytes(value);
  }
}

inline uint32_t ltohl(uint32_t value) { return htoll(value); }

inline uint16_t ltohs(uint16_t value) { return htols(value); }
