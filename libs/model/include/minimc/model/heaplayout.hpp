#ifndef _HEAP_LAYOUT__
#define _HEAP_LAYOUT__

#include "minimc/host/types.hpp"
#include "minimc/model/variables.hpp"

#include <vector>
#include <memory>
#include <ranges>

namespace MiniMC {
  namespace Model {
    struct HeapBlock {
      MiniMC::Model::Value_ptr heap_register = nullptr;
      MiniMC::Model::pointer_t baseobj;
      MiniMC::Model::offset_t size;
      MiniMC::Model::Value_ptr value = nullptr;
      MiniMC::Model::Symbol symbol; 
    };

    using HeapBlock_ptr = std::shared_ptr<HeapBlock>;
    
    class HeapLayout {
    public:
      
      auto addBlock (MiniMC::Model::Symbol symb,MiniMC::Model::pointer_t ptr, MiniMC::Model::offset_t size, MiniMC::Model::Value_ptr heap_register,MiniMC::Model::Value_ptr value = nullptr) {
	_blocks.push_back (std::make_shared<HeapBlock> (heap_register,ptr,size,value,symb));
	symb.setUserData (_blocks.back ());
	return _blocks.back()->baseobj;
      }

      auto blocks () const  {
	return _blocks | std::ranges::views::transform ([](auto& t) ->HeapBlock& {return *t;});
      }
      
      
      
    private:
      std::vector<HeapBlock_ptr> _blocks;
    };
    
  }
}

#endif
