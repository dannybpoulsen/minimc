#ifndef _TYPES__
#define _TYPES__

#include <memory>
#include <iostream>
#include <string>
#include <sstream>

namespace MiniMC {
  namespace Model {

	enum class TypeID {
					   Void,
					   Bool,
					   Integer,
					   Float,
					   Double,
					   Pointer
	};


	
	
	class Type {
	public:
	  Type (const TypeID& ty) : id(ty) {} 
	  virtual ~Type () {}
	  virtual std::ostream& output (std::ostream& os) const = 0;

	  virtual std::size_t getSize () const = 0;

	  TypeID getTypeID () const  {return id;}

	  virtual bool isEqual (const Type& t) {
		return (&t == this) ||
				(getTypeID () == t.getTypeID () && innerEq (t));
	  }
	protected:
	  virtual bool innerEq (const Type& t) = 0;
	private:
	  TypeID id;
	};
	
	template<TypeID id>
	bool is (Type& t) {
	  return t.getTypeID () == id;
	}

	inline std::ostream& operator<< (std::ostream& os, const Type& t) {
	  return t.output (os);
	}
	
	class TypeFactory {
	public:
	  TypeFactory () {}
	  virtual ~TypeFactory () {}
	  virtual Type* makeIntegerType (size_t t) = 0;
	  virtual Type* makeFloatType () = 0;
	  virtual Type* makeBoolType () = 0;
	  virtual Type* makeDoubleType () = 0;
	  virtual Type* makePointerType () = 0;
	  virtual Type* makeVoidType () = 0;
	};

	class TypeFactory64 {
	public:
	  TypeFactory64 ();
	  ~TypeFactory64 ();
	  virtual Type* makeIntegerType (size_t t);
	  virtual Type* makeFloatType () ;
	  virtual Type* makeBoolType () ;
	  virtual Type* makeDoubleType ();
	  virtual Type* makePointerType ();
	  virtual Type* makeVoidType ();
	private:
	  struct Inner;
	  std::unique_ptr<Inner> impl;
	};
	
	
  }
}

#endif
