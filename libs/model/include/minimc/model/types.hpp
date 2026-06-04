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

#include "minimc/io/ostream.hpp"

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
      Aggregate,
      Memory
    };

    std::ostream& operator<< (std::ostream& os, TypeID id);
    MiniMC::IO::ostream& operator<< (MiniMC::IO::ostream& os, TypeID id);
    
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

    template<>
    struct Type_trait<TypeID::Memory> {
      constexpr static std::size_t bitwidth () {return std::numeric_limits<std::size_t>::max ();}
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
      virtual MiniMC::IO::ostream& output(MiniMC::IO::ostream& os) const = 0;
      
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
    
    template<TypeID id> requires (id!=TypeID::Aggregate)
    class TType : public Type{
    public:
      static Type_ptr get  ()  {
	static Type_ptr singleton {new TType()};
	return singleton;
      }
      
      std::size_t getSize() const override { return ByteWidth<id>; }
      std::ostream& output(std::ostream& os) const override  {
	return os << getTypeID();
      }

      MiniMC::IO::ostream& output(MiniMC::IO::ostream& os) const override  {
	return os << getTypeID();
      }
      
      TypeID getTypeID () const override {return id;}
      
      bool isInteger () const override {return id != TypeID::Memory;}
    protected:
      bool innerEq(const Type&) const override {
        return true;
      }
    private:
      TType() {}
      
    };

    using I8Type = TType<TypeID::I8>;
    using I16Type = TType<TypeID::I16>;
    using I32Type = TType<TypeID::I32>;
    using I64Type = TType<TypeID::I64>;
    using VoidType = TType<TypeID::Void>;
    using PointerType = TType<TypeID::Pointer>;
    using Pointer32Type = TType<TypeID::Pointer32>;
    using BoolType = TType<TypeID::Bool>;
    using MemoryType = TType<TypeID::Memory>;
    
    

    class AggregateType : public Type {
    public:
      static Type_ptr get(std::size_t i) {return Type_ptr( new AggregateType (i));}
      std::size_t getSize() const { return size; }
      std::ostream& output(std::ostream& os) const { 
	std::ostream copy (os.rdbuf());  
	copy << "Aggr" << std::dec << std::noshowbase << size;
	return os;
      }

      MiniMC::IO::ostream& output(MiniMC::IO::ostream& os) const { 
	//std::ostream copy (os.rdbuf());  
	//copy << "Aggr" << std::dec << std::noshowbase << size;
	os <<  "Aggr" <<  size;
	return os;
      }
      
      TypeID getTypeID () const override{ return TypeID::Aggregate;}
      
      bool innerEq(const Type& t) const override { return size == static_cast<const AggregateType&>(t).size; }
      bool isAggregate () const override {return true;}
      
    private:
      std::size_t size;
      AggregateType(size_t size) :  size(size) {}
      
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
