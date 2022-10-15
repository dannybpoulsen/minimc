#include "host/operataions.hpp"
#include "host/types.hpp"
#include <cstdint>
#include <iostream>

namespace MiniMC {
  namespace Host {
#ifdef __x86_64__
    template <>
    BV8 udivimpl<BV8>(BV8 l, BV8 r) {
      BV8 res;
      ;
      asm("divb %%cl;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r));
      return res;
    }

    template <>
    BV16 udivimpl<BV16>(BV16 l, BV16 r) {
      BV16 res;
      asm("xor %%dx,%%dx;"
          "divw %%cx;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r)
          : "%dx");
      return res;
    }

    template <>
    uint32_t udivimpl<uint32_t>(uint32_t l, uint32_t r) {
      uint32_t res;
      asm("xor %%edx,%%edx;"
          "divl %%ecx;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r)
          : "%edx");
      return res;
    }

    template <>
    uint64_t udivimpl<uint64_t>(uint64_t l, uint64_t r) {
      uint64_t res;
      asm("xor %%rdx,%%rdx;"
          "divq %%rcx;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r)
          : "%rdx");
      return res;
    }
    template <>
    BV8 sdivimpl<BV8>(BV8 l, BV8 r) {
      BV8 res;
      asm("cbw;"
          "idivb %%cl;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r));
      return res;
    }

    template <>
    BV16 sdivimpl<BV16>(BV16 l, BV16 r) {
      BV16 res;
      asm("cwd;"
          "idivw %%cx;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r)
          : "%dx");
      return res;
    }

    template <>
    uint32_t sdivimpl<uint32_t>(uint32_t l, uint32_t r) {
      uint32_t res;
      asm("cdq;"
          "idivl %%ecx;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r)
          : "%edx");
      return res;
    }

    template <>
    uint64_t sdivimpl<uint64_t>(uint64_t l, uint64_t r) {
      uint64_t res;
      asm("cqo;"
          "idivq %%rcx;"
          : "=a"(res) /* output */
          : "a"(l),   /* input */
            "c"(r)
          : "%rdx");
      return res;
    }

    //

    BV8 rem(BV8 l, BV8 r) {
      BV8 res;
      ;
      asm("divb %%cl;"
          "mov %%ah, %[res];"
          : [res] "=r"(res) /* output */
          : "a"(l),         /* input */
            "c"(r));
      return res;
    }

    BV16 rem(BV16 l, BV16 r) {
      BV16 res;
      asm("xor %%dx,%%dx;"
          "divw %%cx;"
          : "=d"(res) /* output */
          : "a"(l),   /* input */
            "c"(r));
      return res;
    }

    uint32_t rem(uint32_t l, uint32_t r) {
      uint32_t res;
      asm("xor %%edx,%%edx;"
          "divl %%ecx;"
          : "=d"(res) /* output */
          : "a"(l),   /* input */
            "c"(r));
      return res;
    }

    uint64_t rem(uint64_t l, uint64_t r) {
      uint64_t res;
      asm("xor %%rdx,%%rdx;"
          "divq %%rcx;"
          : "=d"(res) /* output */
          : "a"(l),   /* input */
            "c"(r));
      return res;
    }

    BV8 irem(BV8 l, BV8 r) {
      BV8 res;
      asm("cbw;"
          "idivb %%cl;"
          "mov %%ah,%[res];"
          : [res] "=r"(res) /* output */
          : "a"(l),         /* input */
            "c"(r));

      return res;
    }

    BV16 irem(BV16 l, BV16 r) {
      BV16 res;
      asm("cwd;"
          "idivw %%cx;"
          : "=d"(res) /* output */
          : "a"(l),   /* input */
            "c"(r));
      return res;
    }

    uint32_t irem(uint32_t l, uint32_t r) {
      uint32_t res;
      asm("cdq;"
          "idivl %%ecx;"
          : "=d"(res) /* output */
          : "a"(l),   /* input */
            "c"(r));
      return res;
    }

    uint64_t irem(uint64_t l, uint64_t r) {
      uint64_t res;
      asm("cqo;"
          "idivq %%rcx;"
          : "=d"(res) /* output */
          : "a"(l),   /* input */
            "c"(r));
      return res;
    }

#endif
  } // namespace Support

} // namespace MiniMC
