#include <unordered_map>

#include "model/types.hpp"
#include "host/types.hpp"

namespace MiniMC {
  namespace Model {
    class IntegerType : public Type {
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
      virtual bool innerEq(const Type& t) override {
        return bytes == static_cast<const IntegerType&>(t).bytes;
      }

    private:
      size_t bytes;
    };

    class FloatType : public Type {
    public:
      FloatType() : Type(TypeID::Float) {}
      std::size_t getSize() const { return 4; }
       std::ostream& output(std::ostream& os) const { return os << "Float"; }
      bool innerEq(const Type& ) { return true; }
    };

    class DoubleType : public Type {
    public:
      DoubleType() : Type(TypeID::Double) {}
      std::size_t getSize() const { return 8; }
      std::ostream& output(std::ostream& os) const { return os << "Double"; }
      bool innerEq(const Type& ) { return false; }
    };

#ifdef MINIMC32
    class PointerType : public Type {
    public:
      PointerType() : Type(TypeID::Pointer32) {}
      std::size_t getSize() const { return sizeof(MiniMC::pointer32_t); }
      std::ostream& output(std::ostream& os) const { return os << "Pointer32"; }
      bool innerEq(const Type& ) { return true; }
    };
#else
    class PointerType : public Type {
    public:
      PointerType() : Type(TypeID::Pointer) {}
      std::size_t getSize() const { return sizeof(MiniMC::pointer_t); }
      std::ostream& output(std::ostream& os) const { return os << "Pointer"; }
      bool innerEq(const Type& ) { return true; }
    };
#endif
    
class BoolType : public Type {
    public:
      BoolType() : Type(TypeID::Bool) {}
      std::size_t getSize() const { return 1; }
      std::ostream& output(std::ostream& os) const { return os << "Bool"; }
      bool innerEq(const Type&) { return true; }
    };

    class VoidType : public Type {
    public:
      VoidType() : Type(TypeID::Void) {}
      std::size_t getSize() const { return 0; }
      std::ostream& output(std::ostream& os) const { return os << "Void"; }
      bool innerEq(const Type&) { return true; }
    };

    class AggregateType : public Type {
    public:
      AggregateType(TypeID h, size_t size) : Type(h), size(size) {}
      std::size_t getSize() const { return size; }
      std::ostream& output(std::ostream& os) const { 
	std::ostream copy (os.rdbuf());  
	copy << "Aggr" << std::dec << std::noshowbase << size;
	return os;
      }
      bool innerEq(const Type& t) { return size == static_cast<const AggregateType&>(t).size; }
      bool isAggregate () const override {return true;}
      
    private:
      std::size_t size;
    };

    class StructType : public AggregateType {
    public:
      StructType(std::size_t size) : AggregateType(TypeID::Struct, size) {}
    };

    class ArrayType : public AggregateType {
    public:
      ArrayType(std::size_t size) : AggregateType(TypeID::Array, size) {}
    };

    struct TypeFactory64::Inner {
      Inner() : vt(new VoidType()),
                dt(new DoubleType()),
                ft(new FloatType()),
                bt(new BoolType()),
                pt(new PointerType()),
                i8(new IntegerType(1,TypeID::I8)),
                i16(new IntegerType(2,TypeID::I16)),
                i32(new IntegerType(4,TypeID::I32)),
                i64(new IntegerType(8,TypeID::I64)) {}
      Type_ptr vt;
      Type_ptr dt;
      Type_ptr ft;
      Type_ptr bt;
      Type_ptr pt;
      Type_ptr i8;
      Type_ptr i16;
      Type_ptr i32;
      Type_ptr i64;

      std::unordered_map<size_t, Type_ptr> structs;
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

    const Type_ptr TypeFactory64::makeFloatType() { return impl->ft; }
    const Type_ptr TypeFactory64::makeBoolType() { return impl->bt; }
    const Type_ptr TypeFactory64::makeDoubleType() { return impl->dt; }
    const Type_ptr TypeFactory64::makePointerType() { return impl->pt; }
    const Type_ptr TypeFactory64::makeVoidType() { return impl->vt; }
    const Type_ptr TypeFactory64::makeArrayType(size_t t) {
      if (!impl->arrays.count(t)) {
        impl->arrays.insert(std::make_pair(t, std::make_shared<ArrayType>(t)));
      }
      return impl->arrays.at(t);
    }

    const Type_ptr TypeFactory64::makeStructType(size_t t) {
      if (!impl->structs.count(t)) {
        impl->structs.insert(std::make_pair(t, std::make_shared<ArrayType>(t)));
      }
      return impl->structs.at(t);
    }

  } // namespace Model
} // namespace MiniMC
