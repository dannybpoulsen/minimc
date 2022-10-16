#ifndef _HEAP_LAYOUT__
#define _HEAP_LAYOUT__

#include "host/types.hpp"

#include <vector>

namespace MiniMC {
  namespace Model {
    struct HeapBlock {
      MiniMC::base_t baseobj;
      MiniMC::offset_t size;
    };

    class HeapLayout {
    public:
      auto addBlock (MiniMC::offset_t size) {
	blocks.push_back ({static_cast<MiniMC::base_t>(blocks.size ()),size});
	return blocks.back().baseobj;
      }

      auto begin () const {
	return blocks.begin ();
      }

      auto end () const {
	return blocks.end ();
      }
      
    private:
      std::vector<HeapBlock> blocks;
    };
    
  }
}

#endif
