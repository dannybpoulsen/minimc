#ifndef _VALUE_VM__
#define _VALUE_VM__

#include <bit>
#include "vm/value.hpp"
#include "support/casts.hpp"

namespace MiniMC {
  namespace VM {
    namespace Concrete {

      class CValue : public MiniMC::VM::Value {
      public:
	virtual std::size_t size () const = 0;
	virtual const MiniMC::uint8_t* data () const = 0;
  
      };
      
      template <MiniMC::Support::TAC op, typename T>
      Value_ptr performOp(const Value& l, const Value& r);

      template <MiniMC::Support::CMP op, typename T>
      Value_ptr performOp(const Value& l, const Value& r);
      
      struct AggregateValue : public CValue {
        AggregateValue(const MiniMC::Util::Array& array) : val(array) {}

        std::string output() override { return ""; }
        virtual MiniMC::Util::Array bytes() override {
          return val;
        }

	virtual std::size_t size () const override {return val.getSize();}
	virtual const MiniMC::uint8_t* data () const override {return val.get_direct_access ();}
	MiniMC::Hash::hash_t hash () const override {return val.hash (0);}
	
      private:
        MiniMC::Util::Array val;
      };

      struct BoolValue : public CValue {
        BoolValue(bool v) : value(v) {}

        std::string output() override { return ""; }
        virtual MiniMC::Util::Array bytes() override {
          return MiniMC::Util::Array{};
        }

	virtual std::size_t size () const override {return sizeof(bool);}
	virtual const MiniMC::uint8_t* data () const override {return reinterpret_cast<const MiniMC::uint8_t*>(&value);}
	
	virtual MiniMC::VM::TriBool triBool () const {
	  return (value) ? MiniMC::VM::TriBool::True : MiniMC::VM::TriBool::False;
	}

	virtual Value_ptr BoolNegate () override {
	  return std::make_shared<BoolValue> (!value);
	}

	Value_ptr BoolSExt (const MiniMC::Model::Type_ptr& t) override;
	Value_ptr BoolZExt (const MiniMC::Model::Type_ptr& t) override ;
	MiniMC::Hash::hash_t hash ()  const override {return value;}
      private:
        bool value;
      };

      template <typename T>
      requires std::is_integral_v<T> 
      struct TValue : public CValue {
        TValue(T val) : value(val) {}
        virtual Value_ptr Add(const Value_ptr& r) override {
          return performOp<MiniMC::Support::TAC::Add, T>(*this, *r);
        }

        Value_ptr Sub(const Value_ptr& r) override {
          return performOp<MiniMC::Support::TAC::Sub, T>(*this, *r);
        }

        Value_ptr Mul(const Value_ptr& r) override {
          return performOp<MiniMC::Support::TAC::Mul, T>(*this, *r);
        }

        Value_ptr UDiv(const Value_ptr& r) override {
          return performOp<MiniMC::Support::TAC::UDiv, T>(*this, *r);
        }

        Value_ptr SDiv(const Value_ptr& r) override {
          return performOp<MiniMC::Support::TAC::SDiv, T>(*this, *r);
        }

        Value_ptr Shl(const Value_ptr& r) override {
          return performOp<MiniMC::Support::TAC::Shl, T>(*this, *r);
        }

        Value_ptr LShr(const Value_ptr& r) override {
          return performOp<MiniMC::Support::TAC::LShr, T>(*this, *r);
        }

        Value_ptr AShr(const Value_ptr& r) override {
          return performOp<MiniMC::Support::TAC::AShr, T>(*this, *r);
        }

        Value_ptr And(const Value_ptr& r) override {
          return performOp<MiniMC::Support::TAC::And, T>(*this, *r);
        }

        Value_ptr Or(const Value_ptr& r) override {
          return performOp<MiniMC::Support::TAC::Or, T>(*this, *r);
        }

        Value_ptr Xor(const Value_ptr& r) override {
          return performOp<MiniMC::Support::TAC::Xor, T>(*this, *r);
        }

        Value_ptr SGt(const Value_ptr& r) override {
          return performOp<MiniMC::Support::CMP::SGT, T>(*this, *r);
        }

        Value_ptr SGe(const Value_ptr& r) override {
          return performOp<MiniMC::Support::CMP::SGE, T>(*this, *r);
        }

        Value_ptr UGt(const Value_ptr& r) override {
          return performOp<MiniMC::Support::CMP::UGT, T>(*this, *r);
        }

        Value_ptr UGe(const Value_ptr& r) override {
          return performOp<MiniMC::Support::CMP::UGE, T>(*this, *r);
        }

        Value_ptr SLt(const Value_ptr& r) override {
          return performOp<MiniMC::Support::CMP::SLT, T>(*this, *r);
        }

        Value_ptr SLe(const Value_ptr& r) override {
          return performOp<MiniMC::Support::CMP::SLE, T>(*this, *r);
        }

        Value_ptr ULt(const Value_ptr& r) override {
          return performOp<MiniMC::Support::CMP::ULT, T>(*this, *r);
        }

        Value_ptr ULe(const Value_ptr& r) override {
          return performOp<MiniMC::Support::CMP::ULE, T>(*this, *r);
        }

        Value_ptr Eq(const Value_ptr& r) override {
          return performOp<MiniMC::Support::CMP::EQ, T>(*this, *r);
        }

        Value_ptr NEq(const Value_ptr& r) override {
          return performOp<MiniMC::Support::CMP::NEQ, T>(*this, *r);
        }

	
	virtual Value_ptr Trunc (const MiniMC::Model::Type_ptr& t) {
	  
	  auto performTrunc = []<typename To>(auto& val){
	    if constexpr(sizeof(To) > sizeof(T)) {
		throw MiniMC::Support::Exception ("Improper truncation");
		return nullptr;
	      }
	    else {
	      return std::make_shared<TValue<To>> (MiniMC::Support::trunc<T,To> (val));}
	  };
	  switch (t->getSize()) {
	  case 1:
	    return performTrunc.template operator()<MiniMC::uint8_t> (value);
	  case 2:
	    return performTrunc.template operator()<MiniMC::uint16_t> (value);
	  case 4:
	    return performTrunc.template operator()<MiniMC::uint32_t> (value);
	  case 8:
	    return performTrunc.template operator()<MiniMC::uint64_t> (value);
	  }
	  throw MiniMC::Support::Exception ("Impropert Truncation");
	}
	
	virtual Value_ptr ZExt (const MiniMC::Model::Type_ptr& t) {
	  auto performZExt = []<typename To>(auto& val){
	    if constexpr(sizeof(To) < sizeof(T)) {
		throw MiniMC::Support::Exception ("Improper Extenstion");
		return nullptr;
	      }
	    else {
	      return std::make_shared<TValue<To>> (MiniMC::Support::zext<T,To> (val));}
	  };
	  switch (t->getSize()) {
	  case 1:
	    return performZExt.template operator()<MiniMC::uint8_t> (value);
	  case 2:
	    return performZExt.template operator()<MiniMC::uint16_t> (value);
	  case 4:
	    return performZExt.template operator()<MiniMC::uint32_t> (value);
	  case 8:
	    return performZExt.template operator()<MiniMC::uint64_t> (value);
	  }
	  throw MiniMC::Support::Exception ("Impropert Extension");
	}
	
	virtual Value_ptr SExt (const MiniMC::Model::Type_ptr& t) {
	  auto performSExt = []<typename To>(auto& val){
	    if constexpr(sizeof(To) < sizeof(T)) {
		throw MiniMC::Support::Exception ("Improper Extenstion");
		return nullptr;
	      }
	    else {
	      return std::make_shared<TValue<To>> (MiniMC::Support::sext<T,To> (val));}
	  };
	  switch (t->getSize()) {
	  case 1:
	    return performSExt.template operator()<MiniMC::uint8_t> (value);
	  case 2:
	    return performSExt.template operator()<MiniMC::uint16_t> (value);
	  case 4:
	    return performSExt.template operator()<MiniMC::uint32_t> (value);
	  case 8:
	    return performSExt.template operator()<MiniMC::uint64_t> (value);
	  }
	  throw MiniMC::Support::Exception ("Impropert Extension");
	}
	
	virtual Value_ptr IntToBool () {
	  return std::make_shared<BoolValue> (value);
	}
	
	virtual Value_ptr IntToPtr () override;
	
	
        auto val() const { return value; }

        std::string output() override { return ""; }
        virtual MiniMC::Util::Array bytes() override {
          return MiniMC::Util::Array{};
        }

        virtual explicit operator MiniMC::offset_t() { return static_cast<MiniMC::offset_t>(value); }
	
	virtual std::size_t size () const override {return sizeof(T);}
	virtual const MiniMC::uint8_t* data () const override {return reinterpret_cast<const MiniMC::uint8_t*> (&value);}
	
	MiniMC::Hash::hash_t hash () const override {return value;}
      private:
        std::enable_if_t<std::is_integral_v<T>, T> value;
      };
      
      template <MiniMC::Support::TAC op, typename T>
      inline Value_ptr performOp(const Value& l, const Value& r) {

        auto& ll = static_cast<const TValue<T>&>(l);
        auto& rr = static_cast<const TValue<T>&>(r);
        return std::make_shared<TValue<T>>(MiniMC::Support::Op<op>(ll.val(), rr.val()));
      }

      template <MiniMC::Support::CMP op, typename T>
      inline Value_ptr performOp(const Value& l, const Value& r) {

        auto& ll = static_cast<const TValue<T>&>(l);
        auto& rr = static_cast<const TValue<T>&>(r);
        return std::make_shared<BoolValue>(MiniMC::Support::Op<op>(ll.val(), rr.val()));
      }

      struct PointerValue : public CValue {
        PointerValue(MiniMC::pointer_t val) : val(val) {}

        virtual Value_ptr PtrAdd(const Value_ptr& addend) override {

          MiniMC::offset_t offset = static_cast<MiniMC::offset_t>(*addend);

          return std::make_shared<PointerValue>(MiniMC::Support::ptradd(val, offset));
        }

        virtual Value_ptr PtrEq(const Value_ptr& oth) override {
          auto& other = *std::static_pointer_cast<PointerValue>(oth);
          return std::make_shared<BoolValue>(val == other.val);
        }

        std::string output() override { return ""; }
        virtual MiniMC::Util::Array bytes() override {
          return MiniMC::Util::Array{};
        }

        auto getPtr() { return val; }

	virtual std::size_t size () const override {return sizeof(pointer_t);}
	virtual const MiniMC::uint8_t* data () const override {return reinterpret_cast<const MiniMC::uint8_t*> (&val);}

	virtual Value_ptr PtrToI64 () override {
	  return std::make_shared<TValue<MiniMC::uint64_t> > (std::bit_cast<MiniMC::uint64_t> (val));
	}
	
	MiniMC::Hash::hash_t hash () const override {return std::bit_cast<MiniMC::uint64_t> (val);}
      private:
        pointer_t val;
      };

      template<typename T>
      Value_ptr TValue<T>::IntToPtr ()  {
	MiniMC::uint64_t n = MiniMC::Support::zext<T,MiniMC::uint64_t> (value);
	return std::make_shared<PointerValue> (std::bit_cast<pointer_t> (n));
      }
      
      
    } // namespace Concrete
  }   // namespace VM
} // namespace MiniMC

#endif
