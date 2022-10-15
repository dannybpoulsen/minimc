#ifndef _VARIABLE__
#define _VARIABLE__

#include "model/types.hpp"
#include "model/symbol.hpp"
#include "support/binary_encode.hpp"
#include "support/exceptions.hpp"
#include "support/storehelp.hpp"
#include "host/types.hpp"
#include "util/valuemap.hpp"
#include <algorithm>
#include <limits>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace MiniMC {
  namespace Model {

    using type_id_t = MiniMC::BV8;
    const type_id_t untyped = std::numeric_limits<type_id_t>::max();

    template <class T>
    struct ValueInfo {
      static constexpr type_id_t type_t() { return untyped; }
    };

    /**
     *
     * Abstract representation of values in MiniMC.
     * A Value is associated to Type, and be either Variables or Constants.
     * Values can also be local or global to a given function.
     */
    class Value {
    public:
      Value(type_id_t v) : val_type(v) {}
      virtual ~Value() {}
      const Type_ptr& getType() const { return type; }
      void setType(const Type_ptr& t) { type = t; }
      virtual bool isRegister() const { return false; }
      virtual bool isConstant() const { return false; }

      virtual std::ostream& output(std::ostream& os) const = 0;
      const std::string string_repr() const {
        std::stringstream str;
        this->output(str);
        return str.str();
      }

      operator std::string() const {
        return this->string_repr();
      }

      type_id_t type_t() const { return val_type; }

    private:
      Type_ptr type;
      type_id_t val_type;
    };

    inline std::ostream& operator<<(std::ostream& os, const Value& v) {
      return v.output(os);
    }

    using Value_ptr = std::shared_ptr<Value>;

    class Constant : public Value {
    public:
      Constant(type_id_t val) : Value(val) {}
      virtual ~Constant() {}
      bool isConstant() const override { return true; }
      // virtual const MiniMC::uint8_t* getData() const  = 0;
      virtual std::size_t getSize() const { return 0; }
      virtual bool isInteger() const { return false; }
      virtual bool isPointer() const { return false; }
      virtual bool isAggregate() const { return false; }
      virtual bool isBool() const { return false; }
      virtual bool isUndef() const { return false; }
    };

    class Undef : public Constant {
    public:
      Undef();
      virtual bool isUndef() const override { return true; }
      virtual std::ostream& output(std::ostream& os) const override {
        return os << "<Undef " << *getType () << ">";
      }
    };

    using Constant_ptr = std::shared_ptr<Constant>;

    class ConstantFactory64;

    template <typename T, bool is_bool = false>
    class TConstant : public Constant {
    public:
      TConstant(T val);

      T getValue() const {
        // auto val = MiniMC::loadHelper<T>(reinterpret_cast<const MiniMC::uint8_t*>(&value), sizeof(value));
        return value;
      }

      std::size_t getSize() const override { return sizeof(T); }

      /*const MiniMC::uint8_t* getData() const override  {
        return reinterpret_cast<const MiniMC::uint8_t*>(&value);
        }*/

      bool isBool() const override { return is_bool; }
      bool isInteger() const override { return std::is_integral_v<T>; }
      bool isPointer() const override {
        return MiniMC::is_pointer_v<T>;
      }
      
      std::ostream& output(std::ostream& os) const override {
	std::ostream copy (os.rdbuf());
	if constexpr (std::is_integral_v<T>) {
	  MiniMC::BV64 val = value;
	  copy << std::showbase << std::hex ;
	  copy << "<" <<  val << " " << *getType () <<">";;
	}
	else {
	  copy << "<" <<  value << " " << *getType () <<">";;
	}
        return os;
      }

    private:
      T value;
    };

    using Bool = TConstant<MiniMC::BV8, true>;
    using I8Integer = TConstant<MiniMC::BV8>;
    using I16Integer = TConstant<MiniMC::BV16>;
    using I32Integer = TConstant<MiniMC::BV32>;
    using I64Integer = TConstant<MiniMC::BV64>;
    using Pointer = TConstant<MiniMC::pointer64_t>;
    using Pointer32 = TConstant<MiniMC::pointer32_t>;
    
    
    /**
     * Class for representing binary blobs which are useful when having to represent constant arrays/structs.
     *
     */
    class AggregateConstant : public Constant {
    public:
      AggregateConstant(MiniMC::BV8* data, std::size_t s);

      template <class T>
      auto& getValue() const {
        assert(sizeof(T) == size);
        return *reinterpret_cast<T*>(value.get());
      }

      /*virtual const MiniMC::uint8_t* getData() const override {
        return value.get();
        }*/

      auto begin() const {
        return value.get();
      }

      auto end() const {
        return value.get() + size;
      }

      virtual bool isAggregate() const override { return true; }

      std::size_t getSize() const override { return size; }

      virtual std::ostream& output(std::ostream& os) const override {
        MiniMC::Support::STDEncode encoder;
        return os << "< $" << encoder.encode(reinterpret_cast<const char*>(value.get()), size) << "$ " << *getType () << " >";
      }

    private:
      std::unique_ptr<MiniMC::BV8[]> value;
      std::size_t size;
    };

    template <class T>
    class Placed {
    public:
      Placed() : place(unused), id(unused) {}
      std::size_t getPlace() const { return place; }
      std::size_t getId() const { return id; }

      void setPlace(std::size_t p) { place = p; }
      void setId(std::size_t i) { id = i; }

    private:
      static constexpr std::size_t unused = std::numeric_limits<std::size_t>::max();
      std::size_t place;
      std::size_t id;
    };

    class RegisterDescr;

    /**
     * Representation of Variable in MiniMC.
     * A variable is associated to an owning VariableStackDescr that sets its id and byte-placement in an activation record during execution
     */
    class Register : public Value,
                     public Placed<Register>,
                     public std::enable_shared_from_this<Register> {
    public:
      Register(const Symbol& name, RegisterDescr* owner);
      const std::string getName() const { return name.to_string(); }
      virtual std::ostream& output(std::ostream& os) const {
        return os << "<" << name << " " << *getType ()  << ">";
      }
      
      bool isRegister() const override { return true; }
      auto& getOwner() const { return owner; }
      auto getSymbol () const {return name;}
      
    private:
      Symbol name;
      const RegisterDescr* owner{nullptr};
    };

    using Register_ptr = std::shared_ptr<Register>;
    
    /**
     * VariableStackDescr describes the structure of an activation record (in respect to variables in MiniMC - not
     * stack allocations).
     *
     */
    class RegisterDescr {
    public:
      RegisterDescr(Symbol pref = Symbol{}) : pref(std::move(pref)) {}
      RegisterDescr(const RegisterDescr&) = delete;
      RegisterDescr(RegisterDescr&&) = default;
      Register_ptr addRegister(Symbol&& name, const Type_ptr& type);
      auto& getRegisters() const { return variables; }
      
      /**
       *
       * @return Total size in bytes of an activation record
       */
      auto getTotalSize() const { return totalSize; }
      auto getTotalRegisters() const { return variables.size(); }
      auto getPref() const { return pref; }
      
    private:
      std::vector<Register_ptr> variables;
      std::size_t totalSize = 0;
      const Symbol pref;
    };

    using RegisterDescr_uptr = std::unique_ptr<RegisterDescr>;
    
    
    class ConstantFactory {
    public:
      ConstantFactory(TypeFactory_ptr tfac) : typefact(tfac) {}
      virtual ~ConstantFactory() {}

      using aggr_input = std::vector<Constant_ptr>;
      virtual const Value_ptr makeAggregateConstant(const aggr_input& inp, bool) = 0;
      virtual const Value_ptr makeIntegerConstant(MiniMC::BV64, TypeID) = 0;
      virtual const Value_ptr makeFunctionPointer(MiniMC::func_t) = 0;
      virtual const Value_ptr makeHeapPointer(MiniMC::base_t) = 0;
      
      virtual const Value_ptr makeLocationPointer(MiniMC::func_t, MiniMC::base_t) = 0;
      virtual const Value_ptr makeUndef(TypeID,std::size_t = 0) = 0;
    protected:
      TypeFactory_ptr typefact;
    };

    class ConstantFactory64 : public ConstantFactory {
    public:
      ConstantFactory64(TypeFactory_ptr tfac) : ConstantFactory(tfac) {}
      virtual ~ConstantFactory64() {}
      virtual const Value_ptr makeIntegerConstant(MiniMC::BV64, TypeID);
      virtual const Value_ptr makeAggregateConstant(const aggr_input& inp, bool);
      virtual const Value_ptr makeFunctionPointer(MiniMC::func_t);
      virtual const Value_ptr makeLocationPointer(MiniMC::func_t, MiniMC::base_t);
      
      virtual const Value_ptr makeHeapPointer(MiniMC::base_t);
      virtual const Value_ptr makeUndef(TypeID,std::size_t = 0);
    };


    using ConstantFactory_ptr = std::shared_ptr<ConstantFactory>;

    struct VariablePtrIndexer {

      std::size_t operator()(const Register& t) { return t.getId(); }
    };

    template <class T>
    using VariableMap = MiniMC::Util::FixedVector<Register, T, VariablePtrIndexer>;

    template <>
    constexpr type_id_t ValueInfo<I8Integer>::type_t() { return 1; }

    template <>
    constexpr type_id_t ValueInfo<I16Integer>::type_t() { return ValueInfo<I8Integer>::type_t() + 1; }

    template <>
    constexpr type_id_t ValueInfo<I32Integer>::type_t() { return ValueInfo<I16Integer>::type_t() + 1; }

    template <>
    constexpr type_id_t ValueInfo<I64Integer>::type_t() { return ValueInfo<I32Integer>::type_t() + 1; }

    template <>
    constexpr type_id_t ValueInfo<Bool>::type_t() { return ValueInfo<I64Integer>::type_t() + 1; }

    template <>
    constexpr type_id_t ValueInfo<Pointer>::type_t() { return ValueInfo<Bool>::type_t() + 1; }

    
    
    template <>
    constexpr type_id_t ValueInfo<AggregateConstant>::type_t() { return ValueInfo<Pointer>::type_t() + 1; }
    
    template <>
    constexpr type_id_t ValueInfo<Register>::type_t() { return ValueInfo<AggregateConstant>::type_t() + 1; }

    template <>
    constexpr type_id_t ValueInfo<Undef>::type_t() { return ValueInfo<Register>::type_t() + 1; }

    template <>
    constexpr type_id_t ValueInfo<Pointer32>::type_t() { return ValueInfo<Undef>::type_t() + 1; }
    
    
    template <class T, bool is_bool>
    inline TConstant<T, is_bool>::TConstant(T val) : Constant(ValueInfo<TConstant<T, is_bool>>::type_t()),
                                                     value(val) {
    }

  } // namespace Model
} // namespace MiniMC

namespace MiniMC {
  namespace Util {
    template <>
    struct GetIndex<MiniMC::Model::Register> {
      auto operator()(const MiniMC::Model::Register& r) { return r.getId(); }
    };
  } // namespace Util
} // namespace MiniMC

#endif
