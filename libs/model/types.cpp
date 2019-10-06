#include "model/types.hpp"

namespace MiniMC {
  namespace Model {
	class IntegerType : public Type {
	public:
	  IntegerType (size_t b) : Type(TypeID::Integer) ,
							   bits(b) {}
	  virtual std::size_t getSize () const {return bits;}
	  virtual std::ostream& output (std::ostream& os) const {return os << "Int"<< bits;}
	  
	protected:
	  virtual bool innerEq (const Type& t) {return bits== static_cast<const IntegerType&> (t).bits;
	  }
	private:
	  size_t bits;
	};

	class FloatType : public Type {
	public:
	  FloatType () : Type(TypeID::Float) {}
	  virtual std::size_t getSize () const {return 32;}
	  virtual std::ostream& output (std::ostream& os) const {return os << "Float";}
	  virtual bool innerEq (const Type& t) {return true;}
	};

	class DoubleType : public Type {
	public:
	  DoubleType () : Type(TypeID::Double) {}
	  virtual std::size_t getSize () const {return 64;}
	  virtual std::ostream& output (std::ostream& os) const {return os << "Double";}
	  virtual bool innerEq (const Type& t) {return false;}
	};

	class PointerType : public Type {
	public:
	  PointerType () : Type(TypeID::Pointer) {}
	  virtual std::size_t getSize () const {return 64;}
	  virtual std::ostream& output (std::ostream& os) const {return os << "Float";}
	  virtual bool innerEq (const Type& t) {return true;}
	};

	class BoolType : public Type {
	public:
	  BoolType () : Type(TypeID::Pointer) {}
	  virtual std::size_t getSize () const {return 64;}
	  virtual std::ostream& output (std::ostream& os) const {return os << "Bool";}
	  virtual bool innerEq (const Type& t) {return true;}
	};

	class VoidType : public Type {
	public:
	  VoidType () : Type(TypeID::Pointer) {}
	  virtual std::size_t getSize () const {return 64;}
	  virtual std::ostream& output (std::ostream& os) const {return os << "Void";}
	  virtual bool innerEq (const Type& t) {return true;}
	};

	struct TypeFactory64::Inner {
	  Inner () : i8(8),
				 i16(16),
				 i32(32),
				 i64(64) {}
	  VoidType vt;
	  DoubleType dt;
	  FloatType ft;
	  BoolType bt;
	  PointerType pt;
	  IntegerType i8 ;
	  IntegerType i16;
	  IntegerType i32;
	  IntegerType i64;
	};

	
	TypeFactory64::TypeFactory64 () {
	  impl = std::make_unique<Inner> ();
	}
	TypeFactory64::~TypeFactory64 () {}


	Type* TypeFactory64::makeIntegerType (size_t t) {
	  if (t <= 8) {
		return &impl->i8;
	  }
	  else if (t <= 16) {
		return &impl->i16;
	  }
	  else if (t <= 32) {
		return &impl->i32;
	  }
	  else if (t <= 64) {
		return &impl->i64;
	  }

	  else
		return nullptr;
	}
	
	Type* TypeFactory64::makeFloatType () {return &impl->ft;}
	Type* TypeFactory64::makeBoolType () {return &impl->bt;}
	Type* TypeFactory64::makeDoubleType () {return &impl->dt;}
	Type* TypeFactory64::makePointerType () {return &impl->pt;}
	Type* TypeFactory64::makeVoidType () {return &impl->vt;}		
	
  }
}
