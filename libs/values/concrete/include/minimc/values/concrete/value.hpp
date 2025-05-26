#ifndef _VM_CONRETE_VALUE__
#define _VM_CONRETE_VALUE__

#include <iosfwd>

namespace MiniMC {
  namespace VMT {
    namespace Concrete {
      
      class BoolValue {
      public:
        BoolValue(MiniMC::BV8 val = 0) : val(val) {}
        BoolValue BoolNegate() const { return BoolValue(!val); }
        MiniMC::Hash::hash_t hash() const {
          return val;
        }

	MiniMC::VMT::TriBool boolState () const {
	  return val ? MiniMC::VMT::TriBool::True : MiniMC::VMT::TriBool::False; 
	    
	}
	
        auto getValue() const { return val; }
	using underlying_type = MiniMC::BV8;
        
      protected:
	MiniMC::BV8 val;
      };

      inline std::ostream& operator<<(std::ostream& os, const BoolValue& v) { return os << v.getValue(); }
      
      template <typename T>
      requires std::is_integral_v<T> || MiniMC::Model::is_pointer_v<T>
      struct TValue {
        using underlying_type = T;
        TValue(T val) : value(val) {}

        MiniMC::Hash::hash_t hash() const {
	  if constexpr (std::is_integral_v<T>) 
	    return value;
	  else if constexpr (MiniMC::Model::is_pointer_v<T>)  {
	    return std::bit_cast<typename T::PtrBV>(value);  
	  }
        }

        static constexpr std::size_t intbitsize() { return sizeof(T) * 8; }
	
        auto getValue() const { return value; }

      private:
        T value;
      };

      using PointerValue = TValue<MiniMC::Model::pointer64_t>;
      
      struct AggregateValue {
        AggregateValue(const MiniMC::Util::Array& array) : val(array) {}
        AggregateValue(const MiniMC::Util::Array&& array) : val(std::move(array)) {}

        MiniMC::Hash::hash_t hash() const { return val.hash(); }
        auto getValue() const { return val; }
	
      private:
        MiniMC::Util::Array val;
      };

      inline std::ostream& operator<<(std::ostream& os, const AggregateValue& aggr) { return os << aggr.getValue (); }

      template <class T>
      inline std::ostream& operator<<(std::ostream& os, const TValue<T>& v) {
	if constexpr (!std::is_same_v<MiniMC::BV8,T>) 
	  return os << v.getValue();
	else {
	  return os << (0xFF & static_cast<MiniMC::BV32> (v.getValue()));
	}
      }


      enum class EntryState {
        InUse = 2,
        Freed = 4
      };
      struct HeapEntry {
        HeapEntry
	(std::size_t size) : state(EntryState::InUse),
				      content(size) {
        }
	
        
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
	    return content.get_direct_access().subspan (offset,size);
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
      
      class MemoryValue {
      private:
	
	struct internal;
	std::shared_ptr<internal> _internal;
	
      public:
	MemoryValue ();
	MemoryValue (std::shared_ptr<internal>&& );
	MemoryValue (const MemoryValue&) = default;
	MemoryValue (MemoryValue&&) = default;
	
	MemoryValue& operator= (const MemoryValue&) = default;
	MemoryValue& operator= (MemoryValue&&) = default;
	auto& getInternal () const {return *_internal;}
	MemoryValue deep_copy () const;
	MiniMC::Hash::hash_t hash() const;
	
      };

      
      inline std::ostream& operator<< (std::ostream& os, const MemoryValue&) {
	return os << "Mem";
      }
      
      using Value = MiniMC::VMT::GenericVal<TValue<MiniMC::BV8>,
					    TValue<MiniMC::BV16>,
					    TValue<MiniMC::BV32>,
					    TValue<MiniMC::BV64>,
					    PointerValue,
					    TValue<MiniMC::Model::pointer32_t>,
					    BoolValue,
					    AggregateValue,
					    MemoryValue>;
      
    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC

namespace std {
  template <>
  struct hash<MiniMC::VMT::Concrete::MemoryValue> {
    auto operator()(const MiniMC::VMT::Concrete::MemoryValue& t) { return t.hash(); }
  };
} // namespace std

  

#endif
