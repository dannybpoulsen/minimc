#include "minimc/values/concrete/concrete.hpp"
#include <unordered_map>

namespace MiniMC {
  namespace VMT {
    namespace Concrete {
      

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
      MemoryValue::MemoryValue () : _internal(std::make_shared<internal> ()) {}
      MemoryValue::MemoryValue (std::shared_ptr<internal>&& v) : _internal(v) {}
      
      MemoryValue MemoryValue::deep_copy () const {
	return {std::make_shared<internal> (*_internal)};
      }
      
      struct MemoryValue::internal {
	MiniMC::Model::pointer_t allocate (const Value::I64& size) {
	  auto pointer = MiniMC::Model::pointer_t::makeHeapPointer(next, 0);
	  return allocate (size,pointer);
	}

	MiniMC::Model::pointer_t find_space (const Value::I64& ) {
	  return  MiniMC::Model::pointer_t::makeHeapPointer(next, 0);
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
      
      Memory::Memory()  {}
      
      
      
    

      
      Value Memory::load(const MemoryValue& mvalue, const typename Value::Pointer& p, const MiniMC::Model::Type& readType) const {

        // Find out what pointer we are going to read from
        auto pointer = p.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = MiniMC::Model::pointer_t::makeHeapPointer (base,0);
	if (mvalue.getInternal().entries.count(base_pointer)) {
          auto read = mvalue.getInternal().entries.at(base_pointer).read (offset,readType.getSize ());
	  
	  
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
      MemoryValue Memory::store(const MemoryValue& mvalue,const Value::Pointer& p, const Value::I8& v) const {
	MemoryValue m = mvalue.deep_copy();
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
        auto offset = MiniMC::Model::getOffset(pointer);
        if (m.getInternal().entries.count(base_pointer)) {
          m.getInternal().entries.at(base_pointer).write(make_span(value), offset);
        }
	return m;
      }

      MemoryValue Memory::store(const MemoryValue& mvalue,const Value::Pointer& p, const Value::I16& v) const {
	auto m = mvalue.deep_copy();
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (m.getInternal().entries.count(base_pointer)) {
          m.getInternal().entries.at(base_pointer).write(make_span(value), offset);
        }
	return m;
      }

      MemoryValue Memory::store(const MemoryValue& mvalue,const Value::Pointer& p, const Value::I32& v) const{
	auto m = mvalue.deep_copy();
	auto pointer = p.getValue();
        auto value = v.getValue();
	auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (m.getInternal().entries.count(base_pointer)) {
          m.getInternal().entries.at(base_pointer).write(make_span(value), offset);
        }
	return m;
      }

      MemoryValue Memory::store(const MemoryValue& mvalue,const Value::Pointer& p, const Value::I64& v) const{
	auto m = mvalue.deep_copy();
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (m.getInternal().entries.count(base_pointer)) {
          m.getInternal().entries.at(base_pointer).write(make_span(value), offset);
        }
	return m;
      }

      MemoryValue Memory::store(const MemoryValue& mvalue,const Value::Pointer& p, const Value::Aggregate& v) const {
	auto m = mvalue.deep_copy();
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (m.getInternal().entries.count(base_pointer)) {
          m.getInternal().entries.at(base_pointer).write(value.get_direct_access(), offset);
        }
	return m;
      }

      MemoryValue Memory::store(const MemoryValue& mvalue,const Value::Pointer& p, const Value::Pointer& v) const {
	auto m = mvalue.deep_copy();
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (m.getInternal().entries.count(base_pointer)) {
          m.getInternal().entries.at(base_pointer).write(make_span(value), offset);
        }
	return m;
      }

      MemoryValue Memory::store(const MemoryValue& mvalue,const Value::Pointer& p, const Value::Pointer32& v) const{
	auto m = mvalue.deep_copy();
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Model::getBase(pointer);
        auto offset = MiniMC::Model::getOffset(pointer);
	auto base_pointer = decltype(pointer)::makeHeapPointer (base,0); 
	if (m.getInternal().entries.count(base_pointer)) {
	  m.getInternal().entries.at(base_pointer).write(make_span(value), offset);
        }
	return m;
      }
      
      // PArameter is size to allocate
      MemoryValue Memory::allocate(const MemoryValue& mvalue,const Value::Pointer& pointer, const Value::I64& size) {
	MemoryValue m = mvalue.deep_copy();
	m.getInternal().allocate (size,pointer.getValue());
	return m;
      }
      
      Value::Pointer Memory::find_space(const MemoryValue& mvalue,const Value::I64& size) {
        return Value::Pointer(mvalue.getInternal().find_space(size));
      }
      
      MemoryValue Memory::free(const MemoryValue& mvalue,const Value::Pointer&) {
	return mvalue;
      }

      
      MiniMC::Hash::hash_t MemoryValue::hash() const {
	MiniMC::Hash::Hasher hash;
	for (auto& ptr : getInternal().allocated_ptrs) {
	  hash << getInternal().entries.at (ptr);
	}
	return hash;
      }
      
    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC
