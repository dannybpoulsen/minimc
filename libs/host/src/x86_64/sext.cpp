#include "host/casts.hpp"
#include "host/types.hpp"
#include <iostream>

namespace MiniMC {
  namespace Host {
#ifdef __x86_64__
    template <>
    BV16 sext<BV8, BV16>(const BV8 l) {
      BV16 res;
      ;
      asm("cbw"
          : "=a"(res) /* output */
          : "a"(l));
      return res;
    }

    template <>
    uint32_t sext<BV16, uint32_t>(const BV16 l) {
      uint32_t res;
      ;
      asm("cwde"
          : "=a"(res) /* output */
          : "a"(l));
      return res;
    }

    template <>
    uint64_t sext<uint32_t, uint64_t>(const uint32_t l) {
      uint64_t res;
      ;
      asm("cdqe"
          : "=a"(res) /* output */
          : "a"(l));
      return res;
    }

    template <>
    uint32_t sext<BV8, uint32_t>(const BV8 l) {
      BV16 temp = sext<BV8, BV16>(l);
      return sext<BV16, uint32_t>(temp);
    }

    template <>
    uint64_t sext<BV8, uint64_t>(const BV8 l) {
      uint32_t temp = sext<BV8, uint32_t>(l);
      return sext<uint32_t, uint64_t>(temp);
    }

    template <>
    uint64_t sext<BV16, uint64_t>(const BV16 l) {
      uint32_t temp = sext<BV16, uint32_t>(l);
      return sext<uint32_t, uint64_t>(temp);
    }

    template <>
    BV8 sext<BV8, BV8>(const BV8 l) {
      return l;
    }

    template <>
    BV16 sext<BV16, BV16>(const BV16 l) {
      return l;
    }

    template <>
    uint32_t sext<uint32_t, uint32_t>(const uint32_t l) {
      return l;
    }

    template <>
    uint64_t sext<uint64_t, uint64_t>(const uint64_t l) {
      return l;
    }

    template <>
    int16_t sext<int8_t, int16_t>(const int8_t l) {
      int16_t res;
      ;
      asm("cbw"
          : "=a"(res) /* output */
          : "a"(l));
      return res;
    }

    template <>
    int32_t sext<int16_t, int32_t>(const int16_t l) {
      int32_t res;
      ;
      asm("cwde"
          : "=a"(res) /* output */
          : "a"(l));
      return res;
    }

    template <>
    int64_t sext<int32_t, int64_t>(const int32_t l) {
      int64_t res;
      ;
      asm("cdqe"
          : "=a"(res) /* output */
          : "a"(l));
      return res;
    }

    template <>
    int32_t sext<int8_t, int32_t>(const int8_t l) {
      int16_t temp = sext<int8_t, int16_t>(l);
      return sext<int16_t, int32_t>(temp);
    }

    template <>
    int64_t sext<int8_t, int64_t>(const int8_t l) {
      int32_t temp = sext<int8_t, int32_t>(l);
      return sext<int32_t, int64_t>(temp);
    }

    template <>
    int64_t sext<int16_t, int64_t>(const int16_t l) {
      int32_t temp = sext<int16_t, int32_t>(l);
      return sext<int32_t, int64_t>(temp);
    }

    template <>
    int8_t sext<int8_t, int8_t>(const int8_t l) {
      return l;
    }

    template <>
    int16_t sext<int16_t, int16_t>(const int16_t l) {
      return l;
    }

    template <>
    int32_t sext<int32_t, int32_t>(const int32_t l) {
      return l;
    }

    template <>
    int64_t sext<int64_t, int64_t>(const int64_t l) {
      return l;
    }
  }
}
#endif
