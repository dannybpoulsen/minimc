#ifndef _VM_VALUE__
#define _VM_VALUE__

#include <string>
#include <memory>
#include <variant>
#include "model/variables.hpp"
#include "util/array.hpp"
#include "support/exceptions.hpp"
#include "hash/hashing.hpp"


namespace MiniMC {
  namespace VMT {
    
    
    
    
    template<typename T,typename B>
    concept isVMIntBoolCompatible = requires (T x, const T& xx, B b, MiniMC::Model::Type_ptr& type)  {
      {(x.SGt (xx))} -> std::convertible_to<B>;
      {(x.SGe (xx))} -> std::convertible_to<B>;
      {(x.UGt (xx))} -> std::convertible_to<B>;
      {(x.UGe (xx))} -> std::convertible_to<B>;
      {(x.SLt (xx))} -> std::convertible_to<B>;
      {(x.SLe (xx))} -> std::convertible_to<B>;
      {(x.ULt (xx))} -> std::convertible_to<B>;
      {(x.ULe (xx))} -> std::convertible_to<B>;
      {(x.Eq (xx))} -> std::convertible_to<B>;
      {(x.NEq (xx))} -> std::convertible_to<B>;
    };
      
    
    template<typename T>
    concept isVMIntType = requires (T x, const T& xx,  const MiniMC::Model::Type_ptr& type)  {
      {x.Add (xx)} -> std::convertible_to<T>;
      {x.Sub (xx) } -> std::convertible_to<T>;
      {x.Mul (xx) } -> std::convertible_to<T>;
      {x.UDiv (xx) } -> std::convertible_to<T>;
      {x.SDiv (xx) } -> std::convertible_to<T>;
      {x.Shl (xx) } -> std::convertible_to<T>;
      {x.LShr (xx) } -> std::convertible_to<T>;
      {x.AShr (xx)  } -> std::convertible_to<T>;
      {x.And (xx) } -> std::convertible_to<T>;
      {x.Or (xx)  } -> std::convertible_to<T>;
      {x.Xor (xx) } -> std::convertible_to<T>;
      
    };
    
    template<typename B>
    concept isVMBoolType = requires (B x)  {
      {x.BoolNegate ()} -> std::convertible_to<B>;
    };

    template<typename P>
    concept isVMPtrType = requires (P x)  {
      {x.PtrEq ()} -> std::convertible_to<P>;
    };

    template<typename P,typename T,typename B>
    concept isVMPtrIntBoolCompatible = requires (P x,const P& xx, const T& y)  {
      {x.PtrEq (xx)} -> std::convertible_to<B>;
      {x.PtrAdd (y)} ->std::convertible_to<P>;
    };

    
    template<typename Int8,typename Int16,typename Int32,typename Int64, typename PointerT, typename BoolT> 
    struct GenericVal {
      using I8 = Int8;
      using I16 = Int16;
      using I32 = Int32;
      using I64 = Int64;
      using Pointer = PointerT;
      using Bool = BoolT;
      
      GenericVal () : content(BoolT{}) {} 
      GenericVal (I8 val) : content(val) {}
      GenericVal (I16 val) : content(val) {}
      GenericVal (I32 val) : content(val) {}
      GenericVal (I64 val) : content(val) {}
      
      GenericVal (Pointer val) : content(val) {}
      GenericVal (Bool val) : content(val) {}
      
      template<typename T>
      T& as () {	
	return std::get<T> (content);}

      template<typename T>
      bool is () {
	return std::holds_alternative<T> (content);
      }
      
      auto hash () const {return std::hash<decltype(content)>{} (content);}

      auto& output (std::ostream& os) const {return std::visit([&](const auto& x) ->std::ostream&  { return os << x; }, content);}

      
      
    private:
      
      std::variant<I8,I16,I32,I64,Pointer,Bool> content;
    };

    template<typename Int8,typename Int16,typename Int32,typename Int64, typename PointerT, typename BoolT> 
    inline std::ostream& operator<< (std::ostream&  os, const GenericVal<Int8,Int16,Int32,Int64,PointerT,BoolT>& val) {
      return val.output (os);
    }
    
			      }
  namespace VM {
    class Value;
    using Value_ptr = std::shared_ptr<Value>;
    
    enum class TriBool {
      True,
      False,
      Unk
    };

    
    class Value {
    public:
      virtual ~Value () {}
      virtual std::string output () = 0;
      virtual MiniMC::Util::Array bytes () = 0;
      virtual explicit operator MiniMC::offset_t () {throw MiniMC::Support::Exception ("Not implemented");}
      virtual TriBool triBool () const {throw MiniMC::Support::Exception ("Not implemented");}
      
      
      virtual Value_ptr Add (const Value_ptr&)  {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr Sub(const Value_ptr&) { throw MiniMC::Support::Exception("Not implemented"); }
      virtual Value_ptr Mul( const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr UDiv(const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr SDiv(const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr Shl(const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr LShr(const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr AShr(const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr And(const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr Or(const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr Xor(const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      
      virtual Value_ptr SGt (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr SGe (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr UGt (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr UGe (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr SLt (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr SLe (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr ULt (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr ULe (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr Eq  (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr NEq (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      
      /*virtual Value_ptr InsertValue (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
	virtual Value_ptr ExtractValue (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      */
      virtual Value_ptr PtrAdd (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr PtrEq (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      
      virtual Value_ptr BoolNegate () {throw MiniMC::Support::Exception ("Not implemented");}
      
      // Casts
      virtual Value_ptr Trunc (const MiniMC::Model::Type_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr ZExt (const MiniMC::Model::Type_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr SExt (const MiniMC::Model::Type_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr BoolSExt (const MiniMC::Model::Type_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr BoolZExt (const  MiniMC::Model::Type_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr IntToBool () {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr PtrToI64 () {throw MiniMC::Support::Exception ("Not implemented");}
      virtual Value_ptr IntToPtr () {throw MiniMC::Support::Exception ("Not implemented");}
      
      virtual MiniMC::Hash::hash_t hash () const = 0;
      
      
      
    };
    

    
    
  }

} // namespace MiniMC

namespace std {
  template<>
  struct hash<MiniMC::VM::Value> {
    auto operator() (const MiniMC::VM::Value& v) {return v.hash ();}
  };
}

#endif
