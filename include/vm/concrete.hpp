#ifndef _CONCRETE_VM__
#define _CONCRETE_VM__

#include "hash/hashing.hpp"
#include "support/casts.hpp"
#include "support/operataions.hpp"
#include "support/pointer.hpp"
#include "vm/vm.hpp"
#include "vm/vmt.hpp"

#include <memory>

namespace MiniMC {
  namespace VM {
    namespace Concrete {
      MiniMC::VM::ValueLookup_ptr makeLookup(std::size_t);
      MiniMC::VM::PathControl_ptr makePathControl();
      MiniMC::VM::Memory_ptr makeMemory();
    } // namespace Concrete
  }   // namespace VM
  namespace VMT {
    namespace Concrete {

      class BoolValue {
      public:
        friend struct Caster;
        BoolValue(bool val = false) : val(val) {}
        BoolValue BoolNegate() { return BoolValue(!val); }
        MiniMC::Hash::hash_t hash() const {
          return val;
        }

	auto getValue () const {return val;}
	
	
      protected:
        bool val;
      };


      inline std::ostream& operator<< (std::ostream& os, const BoolValue& v) {return os << v.getValue ();}
      
      template<typename T>
      requires std::is_integral_v<T>
      struct TValue;
      
      class PointerValue {
      public:
        friend struct Caster;
        PointerValue(MiniMC::pointer_t val) : val(val) {}
        BoolValue PtrEq(const PointerValue& xx) { return BoolValue(val == xx.val); }
        PointerValue PtrAdd(const TValue<MiniMC::uint64_t>& xx);
	PointerValue PtrAdd(const TValue<MiniMC::uint32_t>& xx);
	PointerValue PtrAdd(const TValue<MiniMC::uint16_t>& xx);
	PointerValue PtrAdd(const TValue<MiniMC::uint8_t>& xx);
	
	auto getValue () const {return val;}
	
	
        MiniMC::Hash::hash_t hash() const {
          return std::bit_cast<MiniMC::Hash::hash_t>(val);
        }

      protected:
        MiniMC::pointer_t val;
      };

      inline std::ostream& operator<< (std::ostream& os, const PointerValue& v) {return os << v.getValue ();}
      
      
	template <size_t>
	struct RetTyp;
	
	template <>
	struct RetTyp<1> {
	  using backtype = MiniMC::uint8_t;
	  using type  = TValue<MiniMC::uint8_t>;
	};

	template <>
	struct RetTyp<2> {
	  using backtype = MiniMC::uint16_t;
	  using  type = TValue<MiniMC::uint16_t>;
	};

	template <>
	struct RetTyp<4> {
	  using backtype = MiniMC::uint32_t;
	  using  type = TValue<MiniMC::uint32_t>;
	};

	template <>
	struct RetTyp<8> {
	  using backtype = MiniMC::uint64_t;
	  using  type = TValue<MiniMC::uint64_t>;
	};
	
      
      template <typename T>
      requires std::is_integral_v<T>
      struct TValue {
        TValue(T val) : value(val) {}
        virtual TValue<T> Add(const TValue<T>& r) {
          return performOp<MiniMC::Support::TAC::Add>(r);
        }

        TValue<T> Sub(const TValue<T>& r) {
          return performOp<MiniMC::Support::TAC::Sub>(r);
        }

        TValue<T> Mul(const TValue<T>& r) {
          return performOp<MiniMC::Support::TAC::Mul>(r);
        }

        TValue<T> UDiv(const TValue<T>& r) {
          return performOp<MiniMC::Support::TAC::UDiv>(r);
        }

        TValue<T> SDiv(const TValue<T>& r) {
          return performOp<MiniMC::Support::TAC::SDiv>(r);
        }

        TValue<T> Shl(const TValue<T>& r) {
          return performOp<MiniMC::Support::TAC::Shl>(r);
        }

        TValue<T> LShr(const TValue<T>& r) {
          return performOp<MiniMC::Support::TAC::LShr>(r);
        }

        TValue<T> AShr(const TValue<T>& r) {
          return performOp<MiniMC::Support::TAC::AShr>(r);
        }

        TValue<T> And(const TValue<T>& r) {
          return performOp<MiniMC::Support::TAC::And>(r);
        }

        TValue<T> Or(const TValue<T>& r) {
          return performOp<MiniMC::Support::TAC::Or>(r);
        }

        TValue<T> Xor(const TValue<T>& r) {
          return performOp<MiniMC::Support::TAC::Xor>(r);
        }

        BoolValue SGt(const TValue<T>& r) {
	  std::cerr << "Compare" << value <<" "<< r.value << std::endl;
	  return performOp<MiniMC::Support::CMP::SGT>(r);
        }

        BoolValue SGe(const TValue<T>& r) {
          return performOp<MiniMC::Support::CMP::SGE>(r);
        }

        BoolValue UGt(const TValue<T>& r) {
          return performOp<MiniMC::Support::CMP::UGT>(r);
        }

        BoolValue UGe(const TValue<T>& r) {
          return performOp<MiniMC::Support::CMP::UGE>(r);
        }

        BoolValue SLt(const TValue<T>& r) {
          return performOp<MiniMC::Support::CMP::SLT>(r);
        }

        BoolValue SLe(const TValue<T>& r) {
          return performOp<MiniMC::Support::CMP::SLE>(r);
        }

        BoolValue ULt(const TValue<T>& r) {
          return performOp<MiniMC::Support::CMP::ULT>(r);
        }

        BoolValue ULe(const TValue<T>& r) {
          return performOp<MiniMC::Support::CMP::ULE>(r);
        }

        BoolValue Eq(const TValue<T>& r) {
          return performOp<MiniMC::Support::CMP::EQ>(r);
        }

        BoolValue NEq(const TValue<T>& r) {
          return performOp<MiniMC::Support::CMP::NEQ>(r);
        }

        template <MiniMC::Support::TAC op>
        TValue<T> performOp(const TValue<T>& r) {
          return TValue<T>(MiniMC::Support::Op<op>(value, r.value));
        }

        template <MiniMC::Support::CMP op>
        BoolValue performOp(const TValue<T>& r) {
          return BoolValue(MiniMC::Support::Op<op>(value, r.value));
        }

	template<size_t bw> 
        typename RetTyp<bw>::type Trunc () const  {
          if constexpr (sizeof(T) < bw) {
            throw MiniMC::Support::Exception("Improper Truncation");
          } else
	    if constexpr (bw == 1) {
	      return TValue<MiniMC::uint8_t>(MiniMC::Support::trunc<T, MiniMC::uint8_t>(value));
	    }
	    else if constexpr (bw == 2) {
	      return TValue<MiniMC::uint16_t>(MiniMC::Support::trunc<T, MiniMC::uint16_t>(value));
	    }
	    else if constexpr (bw == 4) {
	      return TValue<MiniMC::uint32_t>(MiniMC::Support::trunc<T, MiniMC::uint32_t>(value));
	    }
	    else if constexpr (bw == 8) {
	      return TValue<MiniMC::uint64_t>(MiniMC::Support::trunc<T, MiniMC::uint64_t>(value));
	    }
	    else {
	      []<bool t = false> (){static_assert(t);}();
	    }
	  
        }

	template<size_t bw>
        typename RetTyp<bw>::type ZExt () const {
          if constexpr (sizeof(T) > bw) {
            throw MiniMC::Support::Exception("Improper Extention");
          } else {
	    if constexpr (bw == 1)
			  return TValue<MiniMC::uint8_t>(MiniMC::Support::zext<T, MiniMC::uint8_t>(value));
	    else if constexpr (bw == 2)
			  return TValue<MiniMC::uint16_t>(MiniMC::Support::zext<T, MiniMC::uint16_t>(value));
	    else if constexpr (bw == 4)
			  return TValue<MiniMC::uint32_t>(MiniMC::Support::zext<T, MiniMC::uint32_t>(value));
	    else if constexpr (bw == 8)
			  return TValue<MiniMC::uint64_t>(MiniMC::Support::zext<T, MiniMC::uint64_t>(value));
	    
	    else {
	      []<bool t= false> (){static_assert(t);}();
	    }
	    
	  
	  }
	  
        }

	template<size_t bw>
        typename RetTyp<bw>::type SExt () const {
          if constexpr (sizeof(T) > bw) {
            throw MiniMC::Support::Exception("Improper Extention");
          } else {
	    if constexpr (bw == 1)
			   return TValue<MiniMC::uint8_t>(MiniMC::Support::sext<T, MiniMC::uint8_t>(value));
	    else if constexpr (bw == 2)
				return TValue<MiniMC::uint16_t>(MiniMC::Support::sext<T, MiniMC::uint16_t>(value));
	    else if constexpr (bw == 4)
				return TValue<MiniMC::uint32_t>(MiniMC::Support::sext<T, MiniMC::uint32_t>(value));
	    else if constexpr (bw == 8)
				return TValue<MiniMC::uint64_t>(MiniMC::Support::sext<T, MiniMC::uint64_t>(value));
	    else {
	      []<bool t= false> (){static_assert(t);} ();
	    }
	  }
        }
        

        virtual BoolValue IntToBool() {
          return BoolValue(value);
        }

        virtual PointerValue IntToPtr() {
          MiniMC::uint64_t n = MiniMC::Support::zext<T, MiniMC::uint64_t>(value);
          return std::bit_cast<pointer_t>(n);
        }

        MiniMC::Hash::hash_t hash() const {
          return value;
        }

	auto getValue () const {return value;}
	
      private:
        T value;
      };

      template<class T>
      inline std::ostream& operator<< (std::ostream& os, const TValue<T>& v) {return os << v.getValue ();}
      
      
      struct Caster {
	template<std::size_t bw>
	RetTyp<bw>::type BoolZExt (const BoolValue& val) {
	  return typename RetTyp<bw>::type ( val.getValue () ? 1 : 0);
	}

	template<std::size_t bw>
	RetTyp<bw>::type BoolSExt (const BoolValue& val) {
	  return typename RetTyp<bw>::type ( val.getValue () ? std::numeric_limits<typename RetTyp<bw>::backtype>::max() : 0);
	}
      };
      
      
      using ConcreteVMVal = MiniMC::VMT::GenericVal<TValue<MiniMC::uint8_t>,
                                                    TValue<MiniMC::uint16_t>,
                                                    TValue<MiniMC::uint32_t>,
                                                    TValue<MiniMC::uint64_t>,
                                                    PointerValue,
                                                    BoolValue>;
      using ConcreteEngine = MiniMC::VMT::Engine<ConcreteVMVal, Caster >;

      class Memory : public MiniMC::VMT::Memory<ConcreteVMVal> {
      public:
	Memory ();
	Memory (const Memory&);
	~Memory ();
        ConcreteVMVal loadValue(const typename ConcreteVMVal::Pointer&, const MiniMC::Model::Type_ptr&) const override;
        // First parameter is address to store at, second is the value to state
        void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I8&) override;
	void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I16&) override;
        void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I32&) override;
        void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I64&) override;
        void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::Pointer&) override;
        
	// PArameter is size to allocate
        ConcreteVMVal alloca(const ConcreteVMVal::I64&) override;

        void free(const ConcreteVMVal::Pointer&) override;
        void createHeapLayout(const MiniMC::Model::HeapLayout& layout) override;
        MiniMC::Hash::hash_t hash() const;
	
      private:
        struct internal;
        std::unique_ptr<internal> _internal;
      };

      class ValueLookup : public MiniMC::VMT::ValueLookup<ConcreteVMVal> {
      public:
	ValueLookup (std::size_t i) : values(i) {}
	ValueLookup (const ValueLookup&) = default;
        ConcreteVMVal lookupValue (const MiniMC::Model::Value_ptr& v) const override;
        void saveValue(const MiniMC::Model::Variable_ptr& v, ConcreteVMVal&& value) override {
	  std::cerr << "Save " << *v << " "  << value <<std::endl;
	  values[v] = std::move(value);
        }
        ConcreteVMVal unboundValue(const MiniMC::Model::Type_ptr&) const override;
        MiniMC::Hash::hash_t hash() const { return values.hash(0); }
	
      private:
        MiniMC::Model::VariableMap<ConcreteVMVal> values;
      };

      class PathControl : public MiniMC::VMT::PathControl<ConcreteVMVal> {
      public:
        TriBool addAssumption(const ConcreteVMVal::Bool& b) override{
	  return b.getValue () ? TriBool::True : TriBool::False;
	}
        TriBool addAssert(const ConcreteVMVal::Bool& b) override {
	    return b.getValue () ? TriBool::True : TriBool::False;
	}
      };

    } // namespace Concrete

  } // namespace VMT
} // namespace MiniMC

namespace std {
  template <>
  struct hash<MiniMC::VMT::Concrete::PointerValue> {
    auto operator()(const MiniMC::VMT::Concrete::PointerValue& t) { return t.hash(); }
  };

  template <typename T>
  struct hash<MiniMC::VMT::Concrete::TValue<T>> {
    auto operator()(const MiniMC::VMT::Concrete::TValue<T>& t) { return t.hash(); }
  };

  template <>
  struct hash<MiniMC::VMT::Concrete::BoolValue> {
    auto operator()(const MiniMC::VMT::Concrete::BoolValue& t) { return t.hash(); }
  };

  template <>
  struct hash<MiniMC::VMT::Concrete::ConcreteVMVal> {
    auto operator()(const MiniMC::VMT::Concrete::ConcreteVMVal& t) { return t.hash(); }
  };

  template <>
  struct hash<MiniMC::VMT::Concrete::ValueLookup> {
    auto operator()(const MiniMC::VMT::Concrete::ValueLookup& t) { return t.hash(); }
  };

  template <>
  struct hash<MiniMC::VMT::Concrete::Memory> {
    auto operator()(const MiniMC::VMT::Concrete::Memory& t) { return t.hash(); }
  };
  
} // namespace std

#endif
