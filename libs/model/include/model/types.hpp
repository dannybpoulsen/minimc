/**
 * @file   types.hpp
 * @date   Sun Apr 19 11:31:42 2020
 * 
 * @brief  File containing the overall definitions of types in MiniMC
  * 
 * 
 */
#ifndef _TYPES___
#define _TYPES___

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "support/exceptions.hpp"

namespace MiniMC {
  namespace Model {
    /**
	 * \brief Strongly typed enum containing all possible type ids in MiniMC 
	 *
	 */
    enum class TypeID {
      Void,
      Bool,
      I8,
      I16,
      I32,
      I64,
      Float,
      Double,
      Pointer,
      Pointer32,
      Aggregate
    };

    inline std::ostream& operator<< (std::ostream& os, TypeID id) {
      switch (id) {
      case TypeID::Void: return os << "Void";
      case TypeID::Bool: return os << "Bool";
      case TypeID::I8: return os << "I8";
      case TypeID::I16: return os << "I16";
      case TypeID::I32: return os << "I32";
      case TypeID::I64: return os << "I64";
      case TypeID::Float: return os << "Float";
      case TypeID::Double: return os << "Double";
      case TypeID::Pointer: return os << "Pointer";
      case TypeID::Pointer32: return os << "Pointer32";
      case TypeID::Aggregate: return os << "Aggregate";
      default:
	throw MiniMC::Support::Exception ("Not a Typeid");
      }
    }
    
    /** 
	 * Representation of a type in MiniMC. 
	 * All types have a TypeID and a size.
	 */
    class Type : public std::enable_shared_from_this<Type> {
    public:
      Type(const TypeID& ty) : id(ty) {}
      virtual ~Type() {}
      virtual std::ostream& output(std::ostream& os) const = 0;
      
      /** 
       * Calculate this types size in bytes
       *
       *
       * @return Size in bytes this type occupies.
       */
      virtual std::size_t getSize() const = 0;
      
      TypeID getTypeID() const { return id; }
      
      virtual bool isEqual(const Type& t) {
        return (&t == this) ||
               (getTypeID() == t.getTypeID() && innerEq(t));
      }
      
      virtual bool isInteger () const {return false;}
      virtual bool isAggregate () const {return false;}
      
    protected:
      virtual bool innerEq(const Type& t) = 0;

    private:
      TypeID id;
    };

    using Type_ptr = std::shared_ptr<Type>;

    template <TypeID id>
    bool is(Type& t) {
      return t.getTypeID() == id;
    }

    inline std::ostream& operator<<(std::ostream& os, const Type& t) {
      return t.output(os);
    }

    /** 
	 * Factory creating types. 
	 */
    class TypeFactory {
    public:
      TypeFactory() {}
      virtual ~TypeFactory() {}

      /** 
	   * Create an integer type of width at least \p t bits long. It will choose the smallest supported integer size larger than \p t  
	   *
	   * @param t  The minimum size in bits
	   * 
	   * @return  The created integer type
	   */

      virtual const Type_ptr makeIntegerType(size_t t) = 0;
      virtual const Type_ptr makeFloatType() = 0;
      virtual const Type_ptr makeBoolType() = 0;
      virtual const Type_ptr makeDoubleType() = 0;
      virtual const Type_ptr makePointerType() = 0;
      virtual const Type_ptr makeVoidType() = 0;

      virtual const Type_ptr makeAggregateType(size_t t) = 0;
    };
    
    using TypeFactory_ptr = std::shared_ptr<TypeFactory>;

    /** 
	 * Factory creating types. 
	 */
    class TypeFactory64 : public TypeFactory {
    public:
      TypeFactory64();
      ~TypeFactory64();
      virtual const Type_ptr makeIntegerType(size_t t);
      virtual const Type_ptr makeFloatType();
      virtual const Type_ptr makeBoolType();
      virtual const Type_ptr makeDoubleType();
      virtual const Type_ptr makePointerType();
      virtual const Type_ptr makeVoidType();
      virtual const Type_ptr makeAggregateType(size_t);
      
    private:
      struct Inner;
      std::unique_ptr<Inner> impl;
    };
    
    inline bool hasSameTypeID(std::initializer_list<Type_ptr> inp) {
      auto it = inp.begin();
      auto end = inp.end();
      TypeID type = (*it)->getTypeID();
      ++it;
      for (; it != end; ++it) {
        if ((*it)->getTypeID() != type)
          return false;
      }

      return true;
      }

  } // namespace Model
} // namespace MiniMC

#endif
