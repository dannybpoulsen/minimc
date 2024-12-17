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

        /*void write(WriteBuffer&& buffer, std::size_t offset) {
          assert(state == EntryState::InUse);
	  
	  if (buffer.size + offset <= content.getSize()) {
            content.set_block(offset, buffer.size, buffer.buffer);
          } else {
            throw MiniMC::Support::BufferOverflow();
          }
	  }*/
	
	void write(const std::span<const MiniMC::BV8> buffer, std::size_t offset) {
          assert(state == EntryState::InUse);
	  
	  if (buffer.size () + offset <= content.getSize()) {
            content.set_block(offset, buffer);
          } else {
            throw MiniMC::Support::BufferOverflow();
          }
	}
	
        
	std::span<const MiniMC::BV8> read (std::size_t offset, std::size_t size) const {
	  if (offset+size <= content.getSize()) {
	    return content.get_direct_access().subspan (offset,offset+size);
	  }
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

      Value Memory::load(const typename Value::Pointer& p, const MiniMC::Model::Type& readType) const {

        // Find out what pointer we are going to read from
        auto pointer = p.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = MiniMC::Model::pointer_t::makeHeapPointer (base,0);
	if (_internal->entries.count(base_pointer)) {
          auto read = _internal->entries.at(base_pointer).read (offset,readType.getSize ());
	  
	  
          switch (readType.getTypeID()) {
	  case MiniMC::Model::TypeID::Bool:
	    return Value::Bool{*reinterpret_cast<const Value::Bool::underlying_type*> (read.data())};
	  case MiniMC::Model::TypeID::I8: 
	    return Value::I8{*reinterpret_cast<const Value::I8::underlying_type*> (read.data())};
	  case MiniMC::Model::TypeID::I16:
	    return Value::I16{*reinterpret_cast<const Value::I16::underlying_type*> (read.data())};
	    
	  case MiniMC::Model::TypeID::I32:
	    return Value::I32{*reinterpret_cast<const Value::I32::underlying_type*> (read.data())};
	    
	  case MiniMC::Model::TypeID::I64:
	    return Value::I64{*reinterpret_cast<const Value::I64::underlying_type*> (read.data())};
	    
	  case MiniMC::Model::TypeID::Pointer32:
	    return Value::Pointer32{*reinterpret_cast<const Value::Pointer32::underlying_type*> (read.data())};
	    
	  case MiniMC::Model::TypeID::Pointer:
	    return Value::Pointer{*reinterpret_cast<const Value::Pointer::underlying_type*> (read.data())};
	  case MiniMC::Model::TypeID::Aggregate:
	    return Value::Aggregate {read};
	  default:
	    throw MiniMC::Support::Exception("Error");
	    
	    break;
          }
        }
	
        throw MiniMC::Support::BufferOverread();
      }

      template<class T>
      std::span<const MiniMC::BV8> make_span (const T& v) {
	return {reinterpret_cast<const MiniMC::BV8*>(&v),reinterpret_cast<const MiniMC::BV8*>(&v)+sizeof(T)};
      }
      
      // First parameter is address to store at, second is the value to state
      void Memory::store(const Value::Pointer& p, const Value::I8& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
        auto offset = MiniMC::Model::getOffset(pointer);
        if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write(make_span(value), offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::I16& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write(make_span(value), offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::I32& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
	auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write(make_span(value), offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::I64& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write(make_span(value), offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::Aggregate& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write(value.get_direct_access(), offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::Pointer& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write(make_span(value), offset);
        }
      }

      void Memory::store(const Value::Pointer& p, const Value::Pointer32& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (_internal->entries.count(base_pointer)) {
          _internal->entries.at(base_pointer).write(make_span(value), offset);
        }
      }
      
      // PArameter is size to allocate
      Value::Pointer Memory::alloca(const Value::I64& size) {
        return Value::Pointer(_internal->allocate (size));
      }

      void Memory::free(const Value::Pointer&) {
      }
      void Memory::createHeapLayout(const MiniMC::Model::HeapLayout& layout, MiniMC::CPA::Common::StaticContext<Value>& val) {
	for (auto block : layout.blocks()) {
	  auto ptr = _internal->allocate (block.size, block.baseobj);
	  val.addSymbol (block.symbol,Value::Pointer(ptr));
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
