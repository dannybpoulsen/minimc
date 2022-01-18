#ifndef _VM_VALUE__
#define _VM_VALUE__

#include <string>
#include <memory>
#include "model/variables.hpp"
#include "util/array.hpp"
#include "support/exceptions.hpp"
#include "hash/hashing.hpp"


namespace MiniMC {
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
