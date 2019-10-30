#include <cstdint>
#include <iostream>
#include "support/div.hpp"

namespace MiniMC {
  namespace Support {
#ifdef __x86_64__
  uint8_t div (uint8_t l, uint8_t r) {
	uint8_t res;;
	asm ("divb %%cl;"
		 :"=a"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 );
	return res;
  }
	  
  uint16_t div (uint16_t l, uint16_t r) {
	uint16_t res;
	asm ("xor %%dx,%%dx;"
         "divw %%cx;"
		 :"=a"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 :"%dx"
		 );
	return res;
  }
  
  uint32_t div (uint32_t l, uint32_t r) {
	uint32_t res;
	asm ("xor %%edx,%%edx;"
         "divl %%ecx;"
		 :"=a"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 :"%edx"
		 );
	return res;
  }

  uint64_t div (uint64_t l, uint64_t r) {	
	uint64_t res;
	asm ("xor %%rdx,%%rdx;"
         "divq %%rcx;"
		 :"=a"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 :"%rdx"
		 );
	return res;
  }

    uint8_t idiv (uint8_t l, uint8_t r) {
	uint8_t res;
	asm ("cbw;"
		 "idivb %%cl;"
		 :"=a"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 );
	return res;
  }
	  
  uint16_t idiv (uint16_t l, uint16_t r) {
	uint16_t res;
	asm ("cwd;"
         "idivw %%cx;"
		 :"=a"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 :"%dx"
		 );
	return res;
  }
  
  uint32_t idiv (uint32_t l, uint32_t r) {
	uint32_t res;
	asm ("cdq;"
         "idivl %%ecx;"
		 :"=a"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 :"%edx"
		 );
	return res;
  }

  uint64_t idiv (uint64_t l, uint64_t r) {	
	uint64_t res;
	asm ("cqo;"
         "idivq %%rcx;"
		 :"=a"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 :"%rdx"
		 );
	return res;
  }

  //
  

   uint8_t rem (uint8_t l, uint8_t r) {
	uint8_t res;;
	asm ("divb %%cl;"
		 "mov %%ah, %[res];"
		 :[res]"=r"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 );
	return res;
  }
	  
  uint16_t rem (uint16_t l, uint16_t r) {
	uint16_t res;
	asm ("xor %%dx,%%dx;"
         "divw %%cx;"
		 :"=d"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 );
	return res;
  }
  
  uint32_t rem (uint32_t l, uint32_t r) {
	uint32_t res;
	asm ("xor %%edx,%%edx;"
         "divl %%ecx;"
		 :"=d"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 );
	return res;
  }

  uint64_t rem (uint64_t l, uint64_t r) {	
	uint64_t res;
	asm ("xor %%rdx,%%rdx;"
         "divq %%rcx;"
		 :"=d"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 );
	return res;
  }

  
  
  uint8_t irem (uint8_t l, uint8_t r) {
	uint8_t res;
	asm ( "cbw;"
		 "idivb %%cl;"
		  "mov %%ah,%[res];"
		 :[res] "=r"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		  );
	
	return res;
  }
	  
  uint16_t irem (uint16_t l, uint16_t r) {
	uint16_t res;
	asm ("cwd;"
         "idivw %%cx;"
		 :"=d"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 );
	return res;
  }
  
  uint32_t irem (uint32_t l, uint32_t r) {
	uint32_t res;
	asm ("cdq;"
         "idivl %%ecx;"
		 :"=d"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 );
	return res;
	
  }

  uint64_t irem (uint64_t l, uint64_t r) {	
	uint64_t res;
	asm ("cqo;"
         "idivq %%rcx;"
		 :"=d"(res) /* output */
		 :"a"(l),/* input */
		  "c"(r)
		 );
	return res;
  }

  
  
#endif
}

}
