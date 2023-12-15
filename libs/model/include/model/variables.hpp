#ifndef _VARIABLE__
#define _VARIABLE__

#include "model/types.hpp"
#include "model/symbol.hpp"
#include "support/exceptions.hpp"
#include "host/types.hpp"
#include "util/valuemap.hpp"
#include "model/array.hpp"
#include <algorithm>
#include <limits>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace MiniMC {
    
  namespace Model {

    enum class PointerSegments {
    Stack = 'D',
    Heap = 'H',
    Location = 'L',
    Function = 'F'
  };
  using seg_t = BV8;//decltype(pointer_t::segment);
  using base_t = BV16;//decltype(pointer_t::base);
  using func_t = base_t;
  using proc_t = base_t;
  using offset_t = BV32; // decltype(pointer_t::offset);

  

  
  template <typename seg_t = BV16,
            typename base_t = BV16,
            typename offset_t = BV32,
            typename Ptrbv = BV64>
  struct __attribute__((packed)) pointer_struct {
    //Used for identifying if the pointer is a
    // data pointer
    // location pointer
    // function pointer
    seg_t segment{0};
    //base pointer
    //for function and location pointers base is the function_id
    base_t base{0};
    // offset into base_object
    //for function pointer offset must be zero
    //for location pointer offset is the location inside the function jumped to
    offset_t offset{0};
    using PtrBV = Ptrbv;  
    pointer_struct add (offset_t off) const  {
      return pointer_struct {.segment = segment,.base = base,.offset = offset+off};
    }

    pointer_struct sub (offset_t off) const  {
      return pointer_struct {.segment = segment,.base = base,.offset = offset-off};
    }
    
    bool is_null () const {return base == 0 && segment == 0 && offset == 0;}

    static auto makeFunctionPointer (func_t f) {
      pointer_struct<seg_t,base_t,offset_t,Ptrbv> ptr{};
      ptr.segment = static_cast<decltype(ptr.segment)> (PointerSegments::Function);
      ptr.base = f;
      ptr.offset = 0;
      return ptr;
    }

    static auto makeLocationPointer (func_t f,offset_t o) {
      pointer_struct<seg_t,base_t,offset_t,Ptrbv> ptr{};
      ptr.segment = static_cast<decltype(ptr.segment)> (PointerSegments::Location);
      ptr.base = f;
      ptr.offset = o;
      return ptr;
    }

    static auto makeHeapPointer (base_t f,offset_t o) {
      pointer_struct<seg_t,base_t,offset_t,Ptrbv> ptr{};
      ptr.segment = static_cast<decltype(ptr.segment)> (PointerSegments::Heap);
      ptr.base = f;
      ptr.offset = o;
      return ptr;
    }

    static auto makeStackPointer (base_t f,offset_t o) {
      pointer_struct<seg_t,base_t,offset_t,Ptrbv> ptr{};
      ptr.segment = static_cast<decltype(ptr.segment)> (PointerSegments::Stack);
      ptr.base = f;
      ptr.offset = o;
      return ptr;
    }

    static auto makeNullPointer () {
      pointer_struct<seg_t,base_t,offset_t,Ptrbv> ptr{};
      ptr.segment = 0;
      ptr.base = 0;
      ptr.offset = 0;
      return ptr;
    }
    
  };
  

  
  
  
  using pointer64_t = pointer_struct<BV16,BV16,BV32,BV64>;
  using pointer32_t = pointer_struct<BV8,BV8,BV16,BV32>;

  using pointer_t = pointer64_t;
  

  

  inline bool operator==(const pointer_t& l, const pointer_t& r) {
    return l.segment == r.segment &&
           l.base == r.base &&
           l.offset == r.offset;
  }

  
  template<typename T>
  struct is_pointer : public std::false_type {};

  template<>
  struct is_pointer<pointer32_t> : public std::true_type {};

    template<>
  struct is_pointer<pointer64_t> : public std::true_type {};

  
  template<typename T>
  constexpr bool is_pointer_v = is_pointer<T>::value;

  template<class P>
  bool is_null(const P& t) requires (is_pointer_v<P>) {
    return t.segment == 0 &&
           t.base == 0 &&
           t.offset == 0;
  }

  inline func_t getFunctionId(const pointer_t& p) {
    return p.base;
  }
  
  inline base_t getBase(const pointer_t& p) {
    return p.base;
  }
  
  inline offset_t getOffset(const pointer_t& p) {
    return p.offset;
  }
  
  template <class T, class P>
  T& operator<<(T& os, const P& p)  requires (is_pointer_v<P>) {
    if (is_null(p)) {
      return os << std::string("nullptr", 7);
    }
    return os << static_cast<BV8> (p.segment) << "(" << static_cast<int64_t>(p.base) << "+" << p.offset << ")";
  }
    
    
    
    
    
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
    protected:
      std::ostream& outputType (std::ostream& os) const {
	if (type)
	  return os << *type;
	else
	  return os;
      }
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

    using Constant_ptr = std::shared_ptr<Constant>;
    
    
    class Undef : public Constant {
    public:
      Undef();
      virtual bool isUndef() const override { return true; }
      virtual std::ostream& output(std::ostream& os) const override {
        os << "<Undef ";
	return outputType (os) <<  ">";
      }
    };

    using Constant_ptr = std::shared_ptr<Constant>;

    class ConstantFactory64;

    template <typename T, bool is_bool = false>
    class TConstant : public Constant {
    public:
      TConstant(T val);
      virtual ~TConstant () {}
      T getValue() const {
        return value;
      }

      std::size_t getSize() const override { return sizeof(T); }

      bool isBool() const override { return is_bool; }
      bool isInteger() const override { return std::is_integral_v<T>; }
      bool isPointer() const override {
        return MiniMC::Model::is_pointer_v<T>;
      }
      
      std::ostream& output(std::ostream& os) const override {
	std::ostream copy (os.rdbuf());
	if constexpr (std::is_integral_v<T>) {
	  MiniMC::BV64 val = value;
	  copy << std::showbase << std::hex ;
	  copy << "<" <<  val << " ";
	}
	else {
	  copy << "<" <<  value << " ";
	}
	return outputType (os) << ">";
      }

    private:
      T value;
    };

    using Bool = TConstant<MiniMC::BV8, true>;
    using I8Integer = TConstant<MiniMC::BV8>;
    using I16Integer = TConstant<MiniMC::BV16>;
    using I32Integer = TConstant<MiniMC::BV32>;
    using I64Integer = TConstant<MiniMC::BV64>;
    using Pointer = TConstant<MiniMC::Model::pointer64_t>;
    using Pointer32 = TConstant<MiniMC::Model::pointer32_t>;
    using SymbolicConstant = TConstant<MiniMC::Model::Symbol>;
    
    
    /**
     * Class for representing binary blobs which are useful when having to represent constant arrays/structs.
     *
     */
    class AggregateConstant : public Constant {
    public:
      //AggregateConstant(MiniMC::BV8* data, std::size_t s);
      AggregateConstant(MiniMC::Util::Array&& arr);
      /*template <class T>
      auto& getValue() const {
        assert(sizeof(T) == data.getSize ());
        return *reinterpret_cast<T*>(data.get_direct_access ());
	}*/
      
      auto& getData () const  {return data;}
      
      virtual bool isAggregate() const override { return true; }

      std::size_t getSize() const override { return data.getSize (); }

      virtual std::ostream& output(std::ostream& os) const override {
        MiniMC::Support::STDEncode encoder;
        os << "< $" << encoder.encode(reinterpret_cast<const char*>(data.get_direct_access ()), data.getSize ()) << "$ ";
	return outputType (os) << ">";
      
      }
      
    private:
      MiniMC::Util::Array data;
    };

    enum class RegType {
      CPU,
      Local,
      Meta
    };
    
    class RegisterInfo {
    public:
      explicit RegisterInfo(std::size_t id,RegType rtype = RegType::Local) :  id(id),type(rtype) {}
      std::size_t getId() const { return id; }
      auto getRegType () const {return type;}
    private:
      std::size_t id;
      RegType type;
    };

    class RegisterDescr;

    /**
     * Representation of Variable in MiniMC.
     * A variable is associated to an owning VariableStackDescr that sets its id and byte-placement in an activation record during execution
     */
    class Register : public Value {
    public:
      Register(const Symbol& name,RegisterInfo&& p);
      //const std::string getName() const { return name.to_string(); }
      virtual std::ostream& output(std::ostream& os) const {
        return os << "<" << name << " " << *getType ()  << ">";
      }
      
      bool isRegister() const override { return true; }
      auto getSymbol () const {return name;}
      auto getId  () const {return place.getId ();}
      auto getRegType () const {return place.getRegType ();}
    private:
      RegisterInfo place;
      Symbol name;
    };

    using Register_ptr = std::shared_ptr<Register>;
    
    /**
     * VariableStackDescr describes the structure of an activation record (in respect to variables in MiniMC - not
     * stack allocations).
     *
     */
    class RegisterDescr {
    public:
      RegisterDescr(RegType tt = RegType::Local) : _internal(std::make_shared<Data>(tt)) {}
      RegisterDescr(const RegisterDescr&) = default;
      RegisterDescr(RegisterDescr&&) = default;
      RegisterDescr& operator= (RegisterDescr&&) = default;
      Register_ptr addRegister(Symbol&& name, const Type_ptr& type);
      auto& getRegisters() const { return _internal->variables; }
      
      /**
       *
       * @return Total size in bytes of an activation record
       */
      auto getTotalRegisters() const { return _internal->variables.size(); }
    private:
      struct Data {
	Data (RegType tt) : types(tt) {}
	std::vector<Register_ptr> variables;
	RegType types;
      };
      std::shared_ptr<Data> _internal;
    };
    
    using RegisterDescr_uptr = std::unique_ptr<RegisterDescr>;
    
    
    class ConstantFactory {
    public:
      ConstantFactory(TypeFactory_ptr tfac) : typefact(tfac) {}
      virtual ~ConstantFactory() {}
      
      using aggr_input = std::vector<Constant_ptr>;
      virtual const Value_ptr makeAggregateConstant(const aggr_input& inp) = 0;
      virtual const Value_ptr makeIntegerConstant(MiniMC::BV64, TypeID) = 0;
      virtual const Value_ptr makeFunctionPointer(MiniMC::Model::func_t) = 0;
      virtual const Value_ptr makeHeapPointer(MiniMC::Model::base_t, MiniMC::Model::offset_t = 0) = 0;
      virtual const Value_ptr makeNullPointer() = 0;
      virtual const Value_ptr makeSymbolicConstant(const MiniMC::Model::Symbol&) = 0;
      
      virtual const Value_ptr makeLocationPointer(MiniMC::Model::func_t, MiniMC::Model::base_t) = 0;
      virtual const Value_ptr makeUndef(TypeID,std::size_t = 0) = 0;
    protected:
      TypeFactory_ptr typefact;
    };

    class ConstantFactory64 : public ConstantFactory {
    public:
      ConstantFactory64(TypeFactory_ptr tfac) : ConstantFactory(tfac) {}
      virtual ~ConstantFactory64() {}
      virtual const Value_ptr makeIntegerConstant(MiniMC::BV64, TypeID) override;
      const Value_ptr makeAggregateConstant(const aggr_input& inp) override ;
      const Value_ptr makeFunctionPointer(MiniMC::Model::func_t) override ;
      const Value_ptr makeLocationPointer(MiniMC::Model::func_t, MiniMC::Model::base_t) override;
      
      const Value_ptr makeHeapPointer(MiniMC::Model::base_t, MiniMC::Model::offset_t = 0) override;
      const Value_ptr makeNullPointer() override ;
      const Value_ptr makeSymbolicConstant(const MiniMC::Model::Symbol&) override ;
      
      const Value_ptr makeUndef(TypeID,std::size_t = 0) override;
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

    template <>
    constexpr type_id_t ValueInfo<SymbolicConstant>::type_t() { return ValueInfo<Pointer32>::type_t() + 1; }
    
    
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
