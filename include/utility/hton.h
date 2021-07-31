
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

template <typename T>
inline T htob(T value);

template <>
inline uint32_t htob(uint32_t value) {
  if constexpr (endian::native == endian::big) {
    return value;
  } else {
    return swap_bytes(value);
  }
}

template <>
inline uint16_t htob(uint16_t value) {
  if constexpr (endian::native == endian::big) {
    return value;
  } else {
    return swap_bytes(value);
  }
}

template <>
inline uint8_t htob(uint8_t value) {
  return value;
}

template <>
inline int32_t htob(int32_t value) {
  return (int32_t)htob((uint32_t)value);
}

template <>
inline int16_t htob(int16_t value) {
  return (int16_t)htob((uint16_t)value);
}

template <>
inline int8_t htob(int8_t value) {
  return value;
}

template <>
inline float htob(float value) {
  union {
    float fval;
    uint32_t ival;
  };
  fval = value;
  ival = htob(ival);
  return fval;
}

template <typename T>
inline T btoh(T value) {
  return htob(value);
}

// ltoh, htol -> Host to little endian

template <typename T>
inline T htol(T value);

template <>
inline uint32_t htol(uint32_t value) {
  if constexpr (endian::native == endian::little) {
    return value;
  } else {
    return swap_bytes(value);
  }
}

template <>
inline uint16_t htol(uint16_t value) {
  if constexpr (endian::native == endian::little) {
    return value;
  } else {
    return swap_bytes(value);
  }
}

template <>
inline uint8_t htol(uint8_t value) {
  return value;
}

template <>
inline int32_t htol(int32_t value) {
  return (int32_t)htol((uint32_t)value);
}

template <>
inline int16_t htol(int16_t value) {
  return (int16_t)htol((uint16_t)value);
}

template <>
inline int8_t htol(int8_t value) {
  return value;
}

template <>
inline float htol(float value) {
  union {
    float fval;
    uint32_t ival;
  };
  fval = value;
  ival = htol(ival);
  return fval;
}

template <typename T>
inline T ltoh(T value) {
  return htol(value);
}
