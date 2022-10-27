#include "vm/concrete/concrete.hpp"

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
} // namespace std

namespace MiniMC {
  namespace VMT {
    namespace Concrete {
      struct Memory::internal {
        std::vector<HeapEntry> entries;
      };
      Memory::Memory() : _internal(std::make_unique<internal>()) {}
      Memory::Memory(const Memory& m) : _internal(std::make_unique<internal>(*m._internal)) {
      }
      Memory::~Memory() {}

      Memory::Value Memory::loadValue(const typename Memory::Value::Pointer& p, const MiniMC::Model::Type_ptr& readType) const {

        // Find out what pointer we are going to read from
        auto pointer = p.getValue();
        auto base = MiniMC::getBase(pointer);
        auto offset = MiniMC::getOffset(pointer);
        if (base < _internal->entries.size()) {
          auto performRead = [&]<typename T>() {
            typename T::underlying_type readVal;
            _internal->entries.at(base).read({.buffer = reinterpret_cast<MiniMC::BV8*>(&readVal), .size = sizeof(readVal)}, offset);
            return T{readVal};
          };

          switch (readType->getTypeID()) {
            case MiniMC::Model::TypeID::Bool:
              MiniMC::BV8 readVal;
              _internal->entries.at(base).read({.buffer = &readVal, .size = sizeof(MiniMC::BV8)}, offset);
              return Memory::Value{BoolValue(readVal)};
	  case MiniMC::Model::TypeID::I8: 
	    return performRead.template operator()<Value::I8>();
	  case MiniMC::Model::TypeID::I16:
	    return performRead.template operator()<Value::I16>();
	  case MiniMC::Model::TypeID::I32:
	    return performRead.template operator()<Value::I32>();
	  case MiniMC::Model::TypeID::I64:
	    return performRead.template operator()<Value::I64>();
	
	  case MiniMC::Model::TypeID::Pointer:
	    return performRead.template operator()<Value::Pointer>();
	  case MiniMC::Model::TypeID::Struct:
	  case MiniMC::Model::TypeID::Array:
	  default:
	    throw MiniMC::Support::Exception("Error");
	    
	    break;
          }
        }
	
        throw MiniMC::Support::BufferOverread();
      }
      // First parameter is address to store at, second is the value to state
      void Memory::storeValue(const Memory::Value::Pointer& p, const Memory::Value::I8& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::getBase(pointer);
        auto offset = MiniMC::getOffset(pointer);
        if (base < _internal->entries.size()) {
          _internal->entries.at(base).write({.buffer = &value, .size = sizeof(value)}, offset);
        }
      }

      void Memory::storeValue(const Memory::Value::Pointer& p, const Memory::Value::I16& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::getBase(pointer);
        auto offset = MiniMC::getOffset(pointer);
        if (base < _internal->entries.size()) {
          _internal->entries.at(base).write({.buffer = reinterpret_cast<MiniMC::BV8*>(&value), .size = sizeof(value)}, offset);
        }
      }

      void Memory::storeValue(const Memory::Value::Pointer& p, const Memory::Value::I32& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::getBase(pointer);
        auto offset = MiniMC::getOffset(pointer);
        if (base < _internal->entries.size()) {
          _internal->entries.at(base).write({.buffer = reinterpret_cast<MiniMC::BV8*>(&value), .size = sizeof(value)}, offset);
        }
      }

      void Memory::storeValue(const Memory::Value::Pointer& p, const Memory::Value::I64& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::getBase(pointer);
        auto offset = MiniMC::getOffset(pointer);
        if (base < _internal->entries.size()) {
          _internal->entries.at(base).write({.buffer = reinterpret_cast<MiniMC::BV8*>(&value), .size = sizeof(value)}, offset);
        }
      }

      void Memory::storeValue(const Memory::Value::Pointer& p, const Memory::Value::Pointer& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::getBase(pointer);
        auto offset = MiniMC::getOffset(pointer);
        if (base < _internal->entries.size()) {
          _internal->entries.at(base).write({.buffer = reinterpret_cast<MiniMC::BV8*>(&value), .size = sizeof(value)}, offset);
        }
      }

      void Memory::storeValue(const Memory::Value::Pointer& p, const Memory::Value::Pointer32& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::getBase(pointer);
        auto offset = MiniMC::getOffset(pointer);
        if (base < _internal->entries.size()) {
          _internal->entries.at(base).write({.buffer = reinterpret_cast<MiniMC::BV8*>(&value), .size = sizeof(value)}, offset);
        }
      }
      
      // PArameter is size to allocate
      Memory::Value Memory::alloca(const Memory::Value::I64& size) {
	auto size_ = size.getValue();
        auto pointer = MiniMC::pointer_t::makeHeapPointer(_internal->entries.size(), 0);
        _internal->entries.emplace_back(size_);

        return Memory::Value::Pointer(pointer);
        ;
      }

      void Memory::free(const Memory::Value::Pointer&) {
      }
      void Memory::createHeapLayout(const MiniMC::Model::HeapLayout& layout) {
	for (auto block : layout) {

          _internal->entries.emplace_back(block.size);
        }
      }
      MiniMC::Hash::hash_t Memory::hash() const {
	MiniMC::Hash::Hasher hash;
	for (auto& entryt : _internal->entries) {
	  hash << entryt;
	}
	return hash;
      }

    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC
