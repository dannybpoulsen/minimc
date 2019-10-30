#ifndef __DIVS__
#define __DIVS__

#include "support/types.hpp"

namespace MiniMC {
  namespace Support {
	MiniMC::uint8_t div (MiniMC::uint8_t l, MiniMC::uint8_t);
	MiniMC::uint16_t div (MiniMC::uint16_t l, MiniMC::uint16_t);
	MiniMC::uint32_t div (MiniMC::uint32_t l, MiniMC::uint32_t);
	MiniMC::uint64_t div (MiniMC::uint64_t l, MiniMC::uint64_t);
	
	MiniMC::uint8_t idiv (MiniMC::uint8_t l, MiniMC::uint8_t);
	MiniMC::uint16_t idiv (MiniMC::uint16_t l, MiniMC::uint16_t);
	MiniMC::uint32_t idiv (MiniMC::uint32_t l, MiniMC::uint32_t);
	MiniMC::uint64_t idiv (MiniMC::uint64_t l, MiniMC::uint64_t);
	
	MiniMC::uint8_t rem (MiniMC::uint8_t l, MiniMC::uint8_t);
	MiniMC::uint16_t rem (MiniMC::uint16_t l, MiniMC::uint16_t);
	MiniMC::uint32_t rem (MiniMC::uint32_t l, MiniMC::uint32_t);
	MiniMC::uint64_t rem (MiniMC::uint64_t l, MiniMC::uint64_t);
	
	MiniMC::uint8_t irem (MiniMC::uint8_t l, MiniMC::uint8_t);
	MiniMC::uint16_t irem (MiniMC::uint16_t l, MiniMC::uint16_t);
	MiniMC::uint32_t irem (MiniMC::uint32_t l, MiniMC::uint32_t);
	MiniMC::uint64_t irem (MiniMC::uint64_t l, MiniMC::uint64_t);
  }
}




#endif
