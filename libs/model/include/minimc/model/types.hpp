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

#include <iosfwd>
#include <memory>
#include <sstream>
#include <string>

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
      Pointer,
      Pointer32,
      Aggregate
    };

    std::ostream& operator<< (std::ostream& os, TypeID id);
    
    template<TypeID id>
    struct Type_trait {
    };

    template<>
    struct Type_trait<TypeID::I8> {
      constexpr static std::size_t bitwidth () {return 8;}
    };

    template<>
    struct Type_trait<TypeID::I16> {
      constexpr static std::size_t bitwidth () {return 16;}
    };

    template<>
    struct Type_trait<TypeID::I32> {
      constexpr static std::size_t bitwidth () {return 32;}
    };

    template<>
    struct Type_trait<TypeID::I64> {
      constexpr static std::size_t bitwidth () {return 64;}
    };

    template<>
    struct Type_trait<TypeID::Bool> {
      constexpr static std::size_t bitwidth () {return 8;}
    };

    template<>
    struct Type_trait<TypeID::Pointer> {
      constexpr static std::size_t bitwidth () {return 64;}
    };

    template<>
    struct Type_trait<TypeID::Pointer32> {
      constexpr static std::size_t bitwidth () {return 32;}
    };

    template<>
    struct Type_trait<TypeID::Void> {
      constexpr static std::size_t bitwidth () {return 0;}
    };
    
    template<TypeID id>
    constexpr std::size_t BitWidth = Type_trait<id>::bitwidth();

    template<TypeID id>
    constexpr std::size_t ByteWidth = Type_trait<id>::bitwidth() / 8;
    
    
    /** 
	 * Representation of a type in MiniMC. 
	 * All types have a TypeID and a size.
	 */
    class Type : public std::enable_shared_from_this<Type> {
    public:
      Type() {}
      virtual ~Type() {}
      virtual std::ostream& output(std::ostream& os) const = 0;
      
      /** 
       * Calculate this types size in bytes
       *
       *
       * @return Size in bytes this type occupies.
       */
      virtual std::size_t getSize() const = 0;
      
      virtual TypeID getTypeID() const = 0;
      
      virtual bool isEqual(const Type& t) const {
        return (&t == this) ||
               (getTypeID() == t.getTypeID() && innerEq(t));
      }

      bool operator== (const Type& t) const {
	return this->isEqual (t);
      }

      bool operator!= (const Type& t) const {
	return !(*this == t); 
      }
      
      virtual bool isInteger () const {return false;}
      virtual bool isAggregate () const {return false;}
      
    protected:
      virtual bool innerEq(const Type& t) const = 0;
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
    class TypeFactory : public std::enable_shared_from_this<TypeFactory> {
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
      virtual const Type_ptr makeBoolType() = 0;
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
      virtual const Type_ptr makeBoolType();
      virtual const Type_ptr makePointerType();
      virtual const Type_ptr makeVoidType();
      virtual const Type_ptr makeAggregateType(size_t);
      
    private:
      struct Inner;
      std::unique_ptr<Inner> impl;
    };
    
    inline bool isSameType(std::initializer_list<Type_ptr> inp) {
      auto it = inp.begin();
      auto end = inp.end();
      Type& type = *(*it);
      ++it;
      for (; it != end; ++it) {
        if (*(*it) != type)
          return false;
      }

      return true;
      }
    
  } // namespace Model
} // namespace MiniMC

#endif
