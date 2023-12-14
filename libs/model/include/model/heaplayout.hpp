#ifndef _HEAP_LAYOUT__
#define _HEAP_LAYOUT__

#include "host/types.hpp"

#include <vector>

namespace MiniMC {
  namespace Model {
    struct HeapBlock {
      MiniMC::Model::pointer_t baseobj;
      MiniMC::Model::offset_t size;
    };

    class HeapLayout {
    public:
      auto addBlock (MiniMC::Model::pointer_t ptr, MiniMC::Model::offset_t size) {
	blocks.push_back ({ptr,size});
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
