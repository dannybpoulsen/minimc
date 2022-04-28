#ifndef _VARIABLE__
#define _VARIABLE__

#include "model/types.hpp"
#include "support/binary_encode.hpp"
#include "support/exceptions.hpp"
#include "support/storehelp.hpp"
#include "support/types.hpp"
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

    /**
	 * 
	 * Abstract representation of values in MiniMC. 
	 * A Value is associated to Type, and be either Variables or Constants.
	 * Values can also be local or global to a given function. 
	 */
    class Value {
    public:
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

    private:
      Type_ptr type;
    };

    inline std::ostream& operator<<(std::ostream& os, const Value& v) {
      return v.output(os);
    }

    using Value_ptr = std::shared_ptr<Value>;

    class Constant : public Value {
    public:
      virtual ~Constant() {}
      bool isConstant() const override { return true; }
      //virtual const MiniMC::uint8_t* getData() const  = 0;
      virtual std::size_t getSize() { return 0; }
      virtual bool isInteger() const { return false; }
      virtual bool isPointer() const { return false; }
      virtual bool isAggregate() const  { return false; }
      virtual bool isBool() const  { return false; }
      virtual bool isUndef() const { return false; }
    };

    class Undef : public Constant {
    public:
      virtual bool isUndef() const override { return true; }
      virtual std::ostream& output(std::ostream& os) const override {
        return os << "Undef";
      }

    };
    
    using Constant_ptr = std::shared_ptr<Constant>;

    class ConstantFactory64;

      
    template <typename T,bool is_bool = false>
    class TConstant : public Constant {
    public:
      TConstant(T val) : value(val) {
      }


      T getValue() const  {
	// auto val = MiniMC::loadHelper<T>(reinterpret_cast<const MiniMC::uint8_t*>(&value), sizeof(value));
	return value;
      }

      std::size_t getSize() override { return sizeof(T); }

      /*const MiniMC::uint8_t* getData() const override  {
        return reinterpret_cast<const MiniMC::uint8_t*>(&value);
	}*/

      bool isBool() const override { return is_bool; }
      bool isInteger() const override { return std::is_integral_v<T>; }
      bool isPointer() const override { return std::is_same_v<T,MiniMC::pointer_t>;; }

      std::ostream& output(std::ostream& os) const override {
        os << value << std::endl;
        if (getType())
          os << *getType();
        else
          os << "??";
        return os << " >";
      }

    private:
      T value;
    };

    using Bool = TConstant<MiniMC::uint8_t,true>;
    using I8Integer = TConstant<MiniMC::uint16_t>;
    using I16Integer = TConstant<MiniMC::uint16_t>;
    using I32Integer = TConstant<MiniMC::uint32_t>;
    using I64Integer = TConstant<MiniMC::uint64_t>;
    using Pointer = TConstant<MiniMC::pointer_t>;

    
    
    
    /**
	 * Class for representing binary blobs which are useful when having to represent constant arrays/structs.
	 *
	 */
    class AggregateConstant : public Constant {
    public:
      AggregateConstant(MiniMC::uint8_t* data, std::size_t s) : value(new MiniMC::uint8_t[s]), size(s) {
        std::copy(data, data + s, value.get());
      }

      template <class T>
      auto& getValue() const {
        assert(sizeof(T) == size);
        return *reinterpret_cast<T*>(value.get());

      }

      /*virtual const MiniMC::uint8_t* getData() const override {
        return value.get();
	}*/

      auto begin () const {
	return value.get ();
      }

      auto end () const {
	return value.get ()+size;
      }

      virtual bool isAggregate() const override { return true; }

      std::size_t getSize() override { return size; }

      virtual std::ostream& output(std::ostream& os) const override  {
        MiniMC::Support::Base64Encode encoder;
        os << encoder.encode(reinterpret_cast<const char*>(value.get()), size);
        if (getType())
          os << *getType();
        else
          os << "??";
        return os << " >";
      }

    private:
      std::unique_ptr<MiniMC::uint8_t[]> value;
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
      Register(const std::string& name, RegisterDescr* owner) : name(name),owner(owner) {}
      const std::string& getName() const { return name; }
      virtual std::ostream& output(std::ostream& os) const {
        os << " < " << getName() << " ";
        if (getType())
          os << *getType();
        else {
          os << " ___ ";
        }
        return os << " >";
      }

      bool isRegister () const override { return true; }
      auto& getOwner() const { return owner; }

    private:
      std::string name;
      const RegisterDescr* owner{nullptr};
    };

    using Register_ptr = std::shared_ptr<Register>;

    /**
	 * VariableStackDescr describes the structure of an activation record (in respect to variables in MiniMC - not 
	 * stack allocations). 
	 *
	 */
    class RegisterDescr  {
    public:
      RegisterDescr(const std::string& pref) : pref(pref) {}
      RegisterDescr (const RegisterDescr&) = delete;
      RegisterDescr (RegisterDescr&& ) = default;
      Register_ptr addRegister(const std::string& name, const Type_ptr& type);
      auto& getRegisters() const { return variables; }

      /** 
       *
       * @return Total size in bytes of an activation record
       */
      auto getTotalSize() const { return totalSize; }
      auto getTotalRegisters() const { return variables.size(); }
      auto getPref () const {return pref;}
    private:
      std::vector<Register_ptr> variables;
      std::size_t totalSize = 0;
      const std::string pref;
    };

    class ConstantFactory {
    public:
      ConstantFactory() {}
      virtual ~ConstantFactory() {}

      using aggr_input = std::vector<Constant_ptr>;
      virtual const Value_ptr makeAggregateConstant(const aggr_input& inp, bool) = 0;
      virtual const Value_ptr makeIntegerConstant(MiniMC::uint64_t, const Type_ptr&) = 0;
      virtual const Value_ptr makeLocationPointer(MiniMC::func_t, MiniMC::offset_t) = 0;
      virtual const Value_ptr makeFunctionPointer(MiniMC::func_t) = 0;
      virtual const Value_ptr makePointer(MiniMC::pointer_t) = 0;
      virtual const Value_ptr makeUndef(const Type_ptr&) = 0;
    };

    class ConstantFactory64 : public ConstantFactory {
    public:
      ConstantFactory64() {}
      virtual ~ConstantFactory64() {}
      virtual const Value_ptr makeIntegerConstant(MiniMC::uint64_t, const Type_ptr&);
      virtual const Value_ptr makeAggregateConstant(const aggr_input& inp, bool);
      virtual const Value_ptr makeLocationPointer(MiniMC::func_t, MiniMC::offset_t);
      virtual const Value_ptr makeFunctionPointer(MiniMC::func_t);
      virtual const Value_ptr makePointer(MiniMC::pointer_t);
      virtual const Value_ptr makeUndef(const Type_ptr&);
    };
    using ConstantFactory_ptr = std::shared_ptr<ConstantFactory>;

    struct VariablePtrIndexer {
      std::size_t operator()(const Register_ptr& t) { return t->getId(); }
    };

    template <class T>
    using VariableMap = MiniMC::Util::FixedVector<Register_ptr, T, VariablePtrIndexer>;
    
  } // namespace Model
} // namespace MiniMC

namespace MiniMC {
  namespace Util {
    template<>
    struct GetIndex<MiniMC::Model::Register> {
      auto operator () (const MiniMC::Model::Register& r) {return r.getId ();}
    };
  }
}
  

#endif
