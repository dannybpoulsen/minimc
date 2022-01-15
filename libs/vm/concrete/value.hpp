#ifndef _VALUE_VM__
#define _VALUE_VM__

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
	
	
      private:
        bool value;
      };

      template <typename T>
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
        auto val() const { return value; }

        std::string output() override { return ""; }
        virtual MiniMC::Util::Array bytes() override {
          return MiniMC::Util::Array{};
        }

        virtual explicit operator MiniMC::offset_t() { return static_cast<MiniMC::offset_t>(value); }

	virtual std::size_t size () const override {return sizeof(T);}
	virtual const MiniMC::uint8_t* data () const override {return reinterpret_cast<const MiniMC::uint8_t*> (&value);}
	
	
      private:
        std::enable_if_t<std::is_integral_v<T>, T> value;
      };

      template <MiniMC::Support::TAC op, typename T>
      Value_ptr perfomOp(const Value& l, const Value& r) {

        auto& ll = static_cast<TValue<T>&>(l);
        auto& rr = static_cast<TValue<T>&>(r);
        return std::make_shared<TValue<T>>(MiniMC::Support::Op<op>(ll.val(), rr.val()));
      }

      template <MiniMC::Support::CMP op, typename T>
      Value_ptr perfomOp(const Value& l, const Value& r) {

        auto& ll = static_cast<TValue<T>&>(l);
        auto& rr = static_cast<TValue<T>&>(r);
        return std::make_shared<TValue<bool>>(MiniMC::Support::Op<op>(ll.val(), rr.val()));
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
  
	
      private:
        pointer_t val;
      };

    } // namespace Concrete
  }   // namespace VM
} // namespace MiniMC

#endif
