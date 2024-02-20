#ifndef _HEAP_LAYOUT__
#define _HEAP_LAYOUT__

#include "minimc/host/types.hpp"
#include "minimc/model/variables.hpp"

#include <vector>

namespace MiniMC {
  namespace Model {
    struct HeapBlock {
      MiniMC::Model::pointer_t baseobj;
      MiniMC::Model::offset_t size;
      MiniMC::Model::Value_ptr value = nullptr;
    };

    class HeapLayout {
    public:
      auto addBlock (MiniMC::Model::pointer_t ptr, MiniMC::Model::offset_t size, MiniMC::Model::Value_ptr value = nullptr) {
	blocks.push_back ({ptr,size,value});
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
