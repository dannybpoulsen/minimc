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
					   Pointer,
					   Struct,
					   Array
	};


	
	
    class Type : public std::enable_shared_from_this<Type> {
	public:
	  Type (const TypeID& ty) : id(ty) {} 
	  virtual ~Type () {}
	  virtual std::ostream& output (std::ostream& os) const = 0;

      //Size in bytes
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

    using Type_ptr = std::shared_ptr<Type>;
    
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
	  virtual const Type_ptr makeIntegerType (size_t t) = 0;
	  virtual const Type_ptr makeFloatType () = 0;
	  virtual const Type_ptr makeBoolType () = 0;
	  virtual const Type_ptr makeDoubleType () = 0;
	  virtual const Type_ptr makePointerType () = 0;
	  virtual const Type_ptr makeVoidType () = 0;
	  virtual const Type_ptr makeArrayType (size_t) = 0;
	  virtual const Type_ptr makeStructType (size_t) = 0;
	};

	using TypeFactory_ptr = std::shared_ptr<TypeFactory>;
	
	class TypeFactory64 : public TypeFactory {
	public:
	  TypeFactory64 ();
	  ~TypeFactory64 ();
	  virtual const Type_ptr makeIntegerType (size_t t);
	  virtual const Type_ptr makeFloatType () ;
	  virtual const Type_ptr makeBoolType () ;
	  virtual const Type_ptr makeDoubleType ();
	  virtual const Type_ptr makePointerType ();
	  virtual const Type_ptr makeVoidType ();
	  virtual const Type_ptr makeArrayType (size_t);
	  virtual const Type_ptr makeStructType (size_t);
	private:
	  struct Inner;
	  std::unique_ptr<Inner> impl;
	};
	
	
  }
}

#endif
