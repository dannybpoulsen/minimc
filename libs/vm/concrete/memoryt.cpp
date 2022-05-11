#include "vm/concrete/concrete.hpp"

namespace MiniMC {
  namespace VMT {
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
            // Copy the existing memory
            content.set(arr, offset);
          } else {
            throw MiniMC::Support::BufferOverflow();
          }
        }

        void write(WriteBuffer&& buffer, MiniMC::uint64_t offset) {
          assert(state == EntryState::InUse);
          if (buffer.size + offset <= content.getSize()) {
            content.set_block(offset, buffer.size, buffer.buffer);
          } else {
            throw MiniMC::Support::BufferOverflow();
          }
        }

        void read(ReadBuffer&& buffer, MiniMC::uint64_t offset) const {
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

      ConcreteVMVal Memory::loadValue(const typename ConcreteVMVal::Pointer& p, const MiniMC::Model::Type_ptr& readType) const {

        // Find out what pointer we are going to read from
        auto pointer = p.getValue();
        auto base = MiniMC::Support::getBase(pointer);
        auto offset = MiniMC::Support::getOffset(pointer);
        if (base < _internal->entries.size()) {
          auto performRead = [&]<typename T>() {
            T readVal;
            _internal->entries.at(base).read({.buffer = reinterpret_cast<MiniMC::uint8_t*>(&readVal), .size = sizeof(T)}, offset);
            if constexpr (std::is_integral<T>::value)
              return ConcreteVMVal{TValue<T>(readVal)};
            else if constexpr (std::is_same<T, pointer_t>::value)
              return ConcreteVMVal{PointerValue(readVal)};
          };

          switch (readType->getTypeID()) {
            case MiniMC::Model::TypeID::Bool:
              MiniMC::uint8_t readVal;
              _internal->entries.at(base).read({.buffer = &readVal, .size = sizeof(MiniMC::uint8_t)}, offset);
              return ConcreteVMVal{BoolValue(readVal)};
	  case MiniMC::Model::TypeID::I8: 
	    return performRead.template operator()<MiniMC::uint8_t>();
	  case MiniMC::Model::TypeID::I16:
	    return performRead.template operator()<MiniMC::uint16_t>();
	  case MiniMC::Model::TypeID::I32:
	    return performRead.template operator()<MiniMC::uint32_t>();
	  case MiniMC::Model::TypeID::I64:
	    return performRead.template operator()<MiniMC::uint64_t>();
	
	  case MiniMC::Model::TypeID::Pointer:
	    return performRead.template operator()<MiniMC::pointer_t>();
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
      void Memory::storeValue(const ConcreteVMVal::Pointer& p, const ConcreteVMVal::I8& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Support::getBase(pointer);
        auto offset = MiniMC::Support::getOffset(pointer);
        if (base < _internal->entries.size()) {
          _internal->entries.at(base).write({.buffer = &value, .size = sizeof(value)}, offset);
        }
      }

      void Memory::storeValue(const ConcreteVMVal::Pointer& p, const ConcreteVMVal::I16& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Support::getBase(pointer);
        auto offset = MiniMC::Support::getOffset(pointer);
        if (base < _internal->entries.size()) {
          _internal->entries.at(base).write({.buffer = reinterpret_cast<MiniMC::uint8_t*>(&value), .size = sizeof(value)}, offset);
        }
      }

      void Memory::storeValue(const ConcreteVMVal::Pointer& p, const ConcreteVMVal::I32& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Support::getBase(pointer);
        auto offset = MiniMC::Support::getOffset(pointer);
        if (base < _internal->entries.size()) {
          _internal->entries.at(base).write({.buffer = reinterpret_cast<MiniMC::uint8_t*>(&value), .size = sizeof(value)}, offset);
        }
      }

      void Memory::storeValue(const ConcreteVMVal::Pointer& p, const ConcreteVMVal::I64& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Support::getBase(pointer);
        auto offset = MiniMC::Support::getOffset(pointer);
        if (base < _internal->entries.size()) {
          _internal->entries.at(base).write({.buffer = reinterpret_cast<MiniMC::uint8_t*>(&value), .size = sizeof(value)}, offset);
        }
      }

      void Memory::storeValue(const ConcreteVMVal::Pointer& p, const ConcreteVMVal::Pointer& v) {
	auto pointer = p.getValue();
        auto value = v.getValue();
        auto base = MiniMC::Support::getBase(pointer);
        auto offset = MiniMC::Support::getOffset(pointer);
        if (base < _internal->entries.size()) {
          _internal->entries.at(base).write({.buffer = reinterpret_cast<MiniMC::uint8_t*>(&value), .size = sizeof(value)}, offset);
        }
      }

      // PArameter is size to allocate
      ConcreteVMVal Memory::alloca(const ConcreteVMVal::I64& size) {
	auto size_ = size.getValue();
        auto pointer = MiniMC::Support::makeHeapPointer(_internal->entries.size(), 0);
        _internal->entries.emplace_back(size_);

        return ConcreteVMVal::Pointer(pointer);
        ;
      }

      void Memory::free(const ConcreteVMVal::Pointer&) {
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
