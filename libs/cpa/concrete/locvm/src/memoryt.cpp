#include "concvm/concrete.hpp"
#include <unordered_map>

namespace MiniMC {
  namespace VMT {
    namespace Concrete {
      struct ReadBuffer {
        MiniMC::BV8* buffer;
        std::size_t size;
      };

      struct WriteBuffer {
        const MiniMC::BV8* buffer;
        std::size_t size;
      };

      enum class EntryState {
        InUse = 2,
        Freed = 4
      };
      struct HeapEntry {
        HeapEntry(std::size_t size) : state(EntryState::InUse),
				      content(size) {
        }
	
        void write(const MiniMC::Util::Array& arr, std::size_t offset) {
          assert(state == EntryState::InUse);
          if (arr.getSize() + offset <= content.getSize()) {
            // Copy the existing memory
            content.set(arr, offset);
          } else {
            throw MiniMC::Support::BufferOverflow();
          }
        }

        void write(WriteBuffer&& buffer, std::size_t offset) {
          assert(state == EntryState::InUse);
	 
	  if (buffer.size + offset <= content.getSize()) {
            content.set_block(offset, buffer.size, buffer.buffer);
          } else {
            throw MiniMC::Support::BufferOverflow();
          }
        }

        void read(ReadBuffer&& buffer, std::size_t offset) const {
          if (buffer.size + offset <= content.getSize()) {
            content.get_block(offset, buffer.size, buffer.buffer);
          } else
            throw MiniMC::Support::BufferOverread();
        }

        auto hash() const {
	  MiniMC::Hash::Hasher hash;
	  hash << static_cast<MiniMC::Hash::seed_t>(state) << content;
	  return hash;
        }

        auto size() const { return content.getSize(); }

        void setState(EntryState state) {
          this->state = state;
        }

        EntryState state;
        MiniMC::Util::Array content;
      };
    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC

namespace std {
  template <>
  struct hash<MiniMC::VMT::Concrete::HeapEntry> {
    auto operator()(const MiniMC::VMT::Concrete::HeapEntry& e) { return e.hash(); }
  };

  template<>
  struct hash<MiniMC::Model::pointer_t> {
    auto operator()(const MiniMC::Model::pointer_t& op)  const {return std::bit_cast<MiniMC::Model::pointer_t::PtrBV> (op);
    }
  };

  template<>
  struct hash<MiniMC::Model::pointer32_t> {
    auto operator()(const MiniMC::Model::pointer32_t& op) const {return std::bit_cast<MiniMC::Model::pointer32_t::PtrBV> (op);}
  };

} // namespace std

namespace MiniMC {
  namespace VMT {
    namespace Concrete {
      struct Memory::internal {
	MiniMC::Model::pointer_t allocate (const Value::I64& size) {
	  auto pointer = MiniMC::Model::pointer_t::makeHeapPointer(next, 0);
	  return allocate (size,pointer);
	}

	
	MiniMC::Model::pointer_t allocate (const Value::I64& size, MiniMC::Model::pointer_t pointer) {
	  if (!entries.count(pointer) && MiniMC::Model::getOffset (pointer)==0) { 
	    auto size_ = size.getValue();
	    auto base = MiniMC::Model::getBase (pointer);
	    next = (base > next) ? base +1 : next;
	    allocated_ptrs.push_back (pointer);
	    entries.emplace(pointer,size_);
	    
	    return pointer;
	  }
	  throw MiniMC::Support::Exception ("Error allocating memory");
	}
	
        std::unordered_map<MiniMC::Model::pointer_t, HeapEntry> entries;
	std::vector<MiniMC::Model::pointer_t> allocated_ptrs;
	MiniMC::Model::base_t next{0};
      };
      Memory::Memory() : _internal(std::make_unique<internal>()) {}
      Memory::Memory(const Memory& m) : _internal(std::make_unique<internal>(*m._internal)) {
      }
      Memory::~Memory() {}

      Value Memory::load(const typename Value::Pointer& p, const MiniMC::Model::Type_ptr& readType) const {

        // Find out what pointer we are going to read from
        auto pointer = p.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = MiniMC::Model::pointer_t::makeHeapPointer (base,0);
	if (_internal->entries.count(base_pointer)) {
          auto performRead = [this,base,offset,&base_pointer]<typename T>() {
            typename T::underlying_type readVal{0};
	    if constexpr (sizeof(readVal) == 1) {
	      readVal = _internal->entries.at(base_pointer).content.get_direct_access()[offset]; 
	    }
	    else {
	      _internal->entries.at(base_pointer).read({.buffer = reinterpret_cast<MiniMC::BV8*>(&readVal), .size = sizeof(readVal)}, offset);
	    }
            return T{readVal};
          };

          switch (readType->getTypeID()) {
	  case MiniMC::Model::TypeID::Bool:
	    return performRead.template operator()<Value::Bool>();
	  case MiniMC::Model::TypeID::I8: 
	    return performRead.template operator()<Value::I8>();
	  case MiniMC::Model::TypeID::I16:
	    return performRead.template operator()<Value::I16>();
	  case MiniMC::Model::TypeID::I32:
	    return performRead.template operator()<Value::I32>();
	  case MiniMC::Model::TypeID::I64:
	    return performRead.template operator()<Value::I64>();
	  case MiniMC::Model::TypeID::Pointer32:
	    return performRead.template operator()<Value::Pointer32>();
	  case MiniMC::Model::TypeID::Pointer:
	    return performRead.template operator()<Value::Pointer>();
	  case MiniMC::Model::TypeID::Aggregate:
	  default:
	    throw MiniMC::Support::Exception("Error");
	    
	    break;
          }
        }
	
        throw MiniMC::Support::BufferOverread();
      }
      // First parameter is address to store at, second is the value to state
      void Memory::store(const Value::Pointer& p, const Value::I8& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
        auto offset = MiniMC::Model::getOffset(pointer);
        if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write({.buffer = &value, .size = sizeof(value)}, offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::I16& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write({.buffer = reinterpret_cast<MiniMC::BV8*>(&value), .size = sizeof(value)}, offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::I32& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write({.buffer = reinterpret_cast<MiniMC::BV8*>(&value), .size = sizeof(value)}, offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::I64& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write({.buffer = reinterpret_cast<MiniMC::BV8*>(&value), .size = sizeof(value)}, offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::Aggregate& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write({.buffer = value.get_direct_access(), .size = value.getSize ()}, offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::Pointer& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write({.buffer = reinterpret_cast<MiniMC::BV8*>(&value), .size = sizeof(value)}, offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::Pointer32& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write({.buffer = reinterpret_cast<MiniMC::BV8*>(&value), .size = sizeof(value)}, offset);
        }
      }
      
      // PArameter is size to allocate
      Value::Pointer Memory::alloca(const Value::I64& size) {
        return Value::Pointer(_internal->allocate (size));
      }

      void Memory::free(const Value::Pointer&) {
      }
      void Memory::createHeapLayout(const MiniMC::Model::HeapLayout& layout) {
	for (auto block : layout) {
	  _internal->allocate (block.size, block.baseobj);
	}
      }
      MiniMC::Hash::hash_t Memory::hash() const {
	MiniMC::Hash::Hasher hash;
	for (auto& ptr : _internal->allocated_ptrs) {
	  hash << _internal->entries.at (ptr);
	}
	return hash;
      }
      
    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC
