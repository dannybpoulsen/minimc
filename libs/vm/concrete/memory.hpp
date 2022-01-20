#ifndef _memory__
#define _memory__

#include <memory>
#include <type_traits>

#include "model/types.hpp"
#include "hash/hashing.hpp"
#include "support/pointer.hpp"
#include "support/types.hpp"
#include "support/exceptions.hpp"
#include "util/array.hpp"
#include "vm/vm.hpp"

#include "value.hpp"

namespace MiniMC {
  namespace VM {
    namespace Concrete {
      struct ReadBuffer {
	MiniMC::uint8_t* buffer;
	std::size_t size;
      };

      struct WriteBuffer {
	const MiniMC::uint8_t* buffer;
	std::size_t size;
      };
      
      enum class EntryState {
        InUse = 2,
        Freed = 4
      };
      struct HeapEntry {
        HeapEntry(MiniMC::uint64_t size) : state(EntryState::InUse),
                                           content(size) {
          
        }

        void write(const MiniMC::Util::Array& arr, MiniMC::uint64_t offset) {
          assert(state == EntryState::InUse);
          if (arr.getSize() + offset <= content.getSize()) {
            //Copy the existing memory
	    content.set (arr,offset);
	  } else {
            throw MiniMC::Support::BufferOverflow();
          }
        }

	void write(WriteBuffer&& buffer, MiniMC::uint64_t offset) {
          assert(state == EntryState::InUse);
          if (buffer.size + offset <= content.getSize()) {
            content.set_block (offset,buffer.size,buffer.buffer);
	  } else {
            throw MiniMC::Support::BufferOverflow();
          }
        }
	
        
        
        void read(ReadBuffer&& buffer, MiniMC::uint64_t offset)  const {
          if (buffer.size + offset <= content.getSize ()) {
	    content.get_block (offset,buffer.size,buffer.buffer);
	  } else
            throw MiniMC::Support::BufferOverread();
        }
	
	
        auto hash() const {
	  MiniMC::Hash::seed_t seed = static_cast<MiniMC::Hash::seed_t>(state);
	  MiniMC::Hash::hash_combine (seed, content);
	  return seed;
	}

      auto size () const {return content.getSize ();}
      
        void setState(EntryState state) {
          this->state = state;
        }

        EntryState state;
	MiniMC::Util::Array content;
      };

    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC

namespace std {
  template <>
  struct hash<MiniMC::VM::Concrete::HeapEntry> {
    auto operator()(const MiniMC::VM::Concrete::HeapEntry& entry) {
      return entry.hash();
    }
  };
} // namespace std

namespace MiniMC {
  namespace VM {
    namespace Concrete {
      class Heap : public Memory {
      public:
        Heap() {}
	
	Value_ptr alloca(const MiniMC::VM::Value_ptr size) override  {
	  
	  auto size_ = static_cast<MiniMC::offset_t> (*size);
	  auto pointer = MiniMC::Support::makeHeapPointer(entries.size(), 0);
          entries.emplace_back(size_);
	  
	  
	  return std::make_shared<PointerValue> (pointer);;
        }

        
        void free(const MiniMC::VM::Value_ptr& val) override {
          auto pointer = std::static_pointer_cast<PointerValue> (val)->getPtr ();
	  auto base = MiniMC::Support::getBase(pointer);
          auto offset = MiniMC::Support::getOffset(pointer);
          if (base < entries.size() &&
              offset == 0) {
            entries.at(base).setState(EntryState::Freed);
          } else {
            throw MiniMC::Support::InvalidFree();
          }
        }
	
	virtual Value_ptr loadValue (const Value_ptr& ptr, const MiniMC::Model::Type_ptr& readType) const  override {
	  //Find out what pointer we are going to read from 
	  auto pointer = std::static_pointer_cast<PointerValue> (ptr)->getPtr ();
	  auto base = MiniMC::Support::getBase(pointer);
          auto offset = MiniMC::Support::getOffset(pointer);
          auto size_to_read = readType->getSize ();
	  if (base < entries.size()) {
	    auto performRead = [&]<typename T> () {
	      T readVal;
	      entries.at(base).read ({.buffer =reinterpret_cast<MiniMC::uint8_t*> (&readVal), .size = sizeof(T)},offset);
	      if constexpr (std::is_integral<T>::value)
	      return std::make_shared<TValue<T> >  (readVal);
	      else if  constexpr (std::is_same<T,pointer_t>::value)
	      return std::make_shared<PointerValue >  (readVal);
	      
	    };
	    
	    switch (readType->getTypeID ()) {
	    case MiniMC::Model::TypeID::Bool:
	      MiniMC::uint8_t readVal;
	      entries.at(base).read ({.buffer = &readVal, .size = sizeof(MiniMC::uint8_t)},offset);
	      return std::make_shared<BoolValue> (readVal);
	    case MiniMC::Model::TypeID::Integer: {
	      
	      switch (size_to_read) {
	      case 1:
		return performRead.template operator()<MiniMC::uint8_t> ();
	      case 2:
		return performRead.template operator()<MiniMC::uint16_t> ();
	      case 4:
		return performRead.template operator()<MiniMC::uint32_t> ();
	      case 8:
		return performRead.template operator()<MiniMC::uint64_t> ();
		
	      }
	      throw  MiniMC::Support::Exception ("Error");
	    }
	    case MiniMC::Model::TypeID::Pointer:
	      return performRead.template operator()<MiniMC::pointer_t> ();
	    case MiniMC::Model::TypeID::Struct:
	    case MiniMC::Model::TypeID::Array:
	      {
		MiniMC::Util::Array arr{size_to_read};
		entries.at(base).read ({.buffer = arr.get_direct_access (), .size = arr.getSize ()},offset);
		return std::make_shared<AggregateValue> (arr);
		
	      }
	    default:
	      throw  MiniMC::Support::Exception ("Error");
	      
	      break;
	    }

	    
	    
	  }

	  throw  MiniMC::Support::BufferOverread ();
	}

	virtual void storeValue (const Value_ptr& ptr, const Value_ptr& storee) override {
	  auto pointer = std::static_pointer_cast<PointerValue> (ptr)->getPtr ();
	  auto base = MiniMC::Support::getBase(pointer);
          auto offset = MiniMC::Support::getOffset(pointer);
	  auto to_store = std::static_pointer_cast<CValue> (storee);
          if (base < entries.size()) {
	    entries.at(base).write ({.buffer = to_store->data (), .size = to_store->size  ()},offset);
	  }
	}

	virtual  Memory_ptr copy () const override{
	  return std::make_shared<Heap> (*this);
	}
	
	MiniMC::Hash::hash_t hash() const override{
          MiniMC::Hash::seed_t seed = 0;
          for (auto& entryt : entries) {
            MiniMC::Hash::hash_combine(seed, entryt);
          }
          return seed;
        }

	virtual void createHeapLayout (const MiniMC::Model::HeapLayout& layout) {
	  for (auto block : layout) {

	    entries.emplace_back (block.size);
	  }
	  
	}
	
	
      private:
        std::vector<HeapEntry> entries;
      };

      MiniMC::VM::Memory_ptr makeMemory ( )  {return std::make_shared<Heap> ();}
      
      
    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC

namespace std {

  template <>
  struct hash<MiniMC::VM::Concrete::Heap> {
    auto operator()(const MiniMC::VM::Concrete::Heap& heap) {
      return heap.hash();
    }
  };
} // namespace std

#endif
  
