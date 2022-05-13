#include "host/rightshifts.hpp"
#include <cstdint>

namespace MiniMC {
  namespace Host {
#ifdef __x86_64__
    template <>
    uint8_t ashr<uint8_t>(uint8_t l, uint8_t r) {
      uint8_t res{0};
      asm("sarb %%cl,%%al;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r));
      return res;
      
    }

    template <>
    uint16_t ashr<uint16_t>(uint16_t l, uint16_t r) {
      uint16_t res;
      asm("sarw %%cl,%%ax;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r));
      return res;
    }

    template <>
    uint32_t ashr<uint32_t>(uint32_t l, uint32_t r) {
      uint32_t res;
      asm(
          "sarl %%cl,%%eax;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r)

      );
      return res;
    }

    template <>
    uint64_t ashr<uint64_t>(uint64_t l, uint64_t r) {
      uint64_t res;
      asm("sarq %%cl,%%rax;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r));
      return res;
    }

    template <>
    uint8_t lshr<uint8_t>(uint8_t l, uint8_t r) {
      uint8_t res;
      ;
      asm(
          "shrb %%cl,%%al;"
          : [x] "=a"(res) /* output */
          : [l] "a"(l),   /* input */
            [r] "c"(r));
      return res;
    }

    template <>
    uint16_t lshr<uint16_t>(uint16_t l, uint16_t r) {
      uint16_t res;
      asm("shrw %%cl,%%ax;"
          : [res] "=a"(res) /* output */
          : [l] "a"(l),     /* input */
            [r] "c"(r));
      return res;
    }

    template <>
    uint32_t lshr<uint32_t>(uint32_t l, uint32_t r) {
      uint32_t res;
      asm("shrl %%cl,%%eax;"
          : [res] "=a"(res) /* output */
          : [l] "a"(l),     /* input */
            [r] "c"(r));
      return res;
    }

    template <>
    uint64_t lshr<uint64_t>(uint64_t l, uint64_t r) {
      uint64_t res;
      asm("shrq %%cl,%%rax;"
          : [res] "=a"(res) /* output */
          : [l] "a"(l),     /* input */
            [r] "c"(r));
      return res;
    }
#endif

  } // namespace Support
} // namespace MiniMC
