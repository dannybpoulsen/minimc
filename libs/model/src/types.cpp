#include <unordered_map>

#include "minimc/model/types.hpp"
#include "minimc/model/variables.hpp"


#include "minimc/support/exceptions.hpp"


namespace MiniMC {
  namespace Model {

    template<TypeID id> requires (id!=TypeID::Aggregate)
    class TType : public Type{
    public:
      TType() {}
      std::size_t getSize() const override { return ByteWidth<id>; }
      std::ostream& output(std::ostream& os) const override  {
	return os << getTypeID();
      }

      TypeID getTypeID () const override {return id;}
      
      bool isInteger () const override {return true;}
    protected:
      bool innerEq(const Type&) const override {
        return true;
      }
    
    };

    using I8Type = TType<TypeID::I8>;
    using I16Type = TType<TypeID::I16>;
    using I32Type = TType<TypeID::I32>;
    using I64Type = TType<TypeID::I64>;
    using VoidType = TType<TypeID::Void>;
    using PointerType = TType<TypeID::Pointer>;
    using Pointer32Type = TType<TypeID::Pointer32>;
    using BoolType = TType<TypeID::Bool>;
    
    
    /*class IntegerType : public Type {
    public:
      IntegerType(size_t b,TypeID id) : Type(id),
                              bytes(b) {}
      std::size_t getSize() const override { return bytes; }
      std::ostream& output(std::ostream& os) const override  {
	std::ostream copy (os.rdbuf());
	copy << std::dec << std::noshowbase <<  "Int" << bytes * 8;
	return os;
      }
      bool isInteger () const override {return true;}
    protected:
      bool innerEq(const Type& t) const override {
        return bytes == static_cast<const IntegerType&>(t).bytes;
      }
      
    private:
      size_t bytes;
    };

    
#ifdef MINIMC32
    class PointerType : public Type {
    public:
      PointerType() : Type(TypeID::Pointer32) {}
      std::size_t getSize() const { return sizeof(MiniMC::Model::pointer32_t); }
      std::ostream& output(std::ostream& os) const { return os << "Pointer32"; }
      bool innerEq(const Type& ) const override  { return true; }
    };
#else
    class PointerType : public Type {
    public:
      PointerType() : Type(TypeID::Pointer) {}
      std::size_t getSize() const { return sizeof(MiniMC::Model::pointer_t); }
      std::ostream& output(std::ostream& os) const { return os << "Pointer"; }
      bool innerEq(const Type& ) const override  { return true; }
    };
#endif
    
class BoolType : public Type {
    public:
      BoolType() : Type(TypeID::Bool) {}
      std::size_t getSize() const { return 1; }
      std::ostream& output(std::ostream& os) const { return os << "Bool"; }
      bool innerEq(const Type&) const override { return true; }
    };

    class VoidType : public Type {
    public:
      VoidType() : Type(TypeID::Void) {}
      std::size_t getSize() const { return 0; }
      std::ostream& output(std::ostream& os) const { return os << "Void"; }
      bool innerEq(const Type&) const override { return true; }
    };
    */
    class AggregateType : public Type {
    public:
      AggregateType(size_t size) :  size(size) {}
      std::size_t getSize() const { return size; }
      std::ostream& output(std::ostream& os) const { 
	std::ostream copy (os.rdbuf());  
	copy << "Aggr" << std::dec << std::noshowbase << size;
	return os;
      }
      TypeID getTypeID () const override{ return TypeID::Aggregate;}
      
      bool innerEq(const Type& t) const override { return size == static_cast<const AggregateType&>(t).size; }
      bool isAggregate () const override {return true;}
      
    private:
      std::size_t size;
    };

    struct TypeFactory64::Inner {
      Inner() : vt(new VoidType()),
                bt(new BoolType()),
                pt(new PointerType()),
                i8(new I8Type()),
                i16(new I16Type()),
                i32(new I32Type()),
                i64(new I64Type()) {}
      Type_ptr vt;
      Type_ptr bt;
      Type_ptr pt;
      Type_ptr i8;
      Type_ptr i16;
      Type_ptr i32;
      Type_ptr i64;
      std::unordered_map<size_t, Type_ptr> arrays;
    };

    TypeFactory64::TypeFactory64() {
      impl = std::make_unique<Inner>();
    }
    TypeFactory64::~TypeFactory64() {}

    const Type_ptr TypeFactory64::makeIntegerType(size_t t) {
      if (t <= 8) {
        return impl->i8;
      } else if (t <= 16) {
        return impl->i16;
      } else if (t <= 32) {
        return impl->i32;
      } else if (t <= 64) {
        return impl->i64;
      }

      else
        return nullptr;
    }

    const Type_ptr TypeFactory64::makeBoolType() { return impl->bt; }
    const Type_ptr TypeFactory64::makePointerType() { return impl->pt; }
    const Type_ptr TypeFactory64::makeVoidType() { return impl->vt; }
    const Type_ptr TypeFactory64::makeAggregateType(size_t t) {
      if (!impl->arrays.count(t)) {
        impl->arrays.insert(std::make_pair(t, std::make_shared<AggregateType>(t)));
      }
      return impl->arrays.at(t);
    }


    std::ostream& operator<< (std::ostream& os, TypeID id){
      switch (id) {
      case TypeID::Void: return os << "Void";
      case TypeID::Bool: return os << "Bool";
      case TypeID::I8: return os << "I8";
      case TypeID::I16: return os << "I16";
      case TypeID::I32: return os << "I32";
      case TypeID::I64: return os << "I64";
      case TypeID::Pointer: return os << "Pointer";
      case TypeID::Pointer32: return os << "Pointer32";
      case TypeID::Aggregate: return os << "Aggregate";
      default:
	std::unreachable();
      }
    }
    
    
  } // namespace Model
} // namespace MiniMC
