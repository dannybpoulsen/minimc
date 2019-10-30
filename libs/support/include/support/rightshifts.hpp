#ifndef __SHIFTS__
#define __SHIFTS__

#include "support/types.hpp"

namespace MiniMC  {
  namespace Support {
	MiniMC::uint8_t ashr (MiniMC::uint8_t l, MiniMC::uint8_t);
	MiniMC::uint16_t ashr (MiniMC::uint16_t l, MiniMC::uint16_t);
	MiniMC::uint32_t ashr (MiniMC::uint32_t l, MiniMC::uint32_t);
	MiniMC::uint64_t ashr (MiniMC::uint64_t l, MiniMC::uint64_t);
	
	MiniMC::uint8_t lshr (MiniMC::uint8_t l, MiniMC::uint8_t);
	MiniMC::uint16_t lshr (MiniMC::uint16_t l, MiniMC::uint16_t);
	MiniMC::uint32_t lshr (MiniMC::uint32_t l, MiniMC::uint32_t);
	MiniMC::uint64_t lshr (MiniMC::uint64_t l, MiniMC::uint64_t);
  }
}

#endif
