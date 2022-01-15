#ifndef _VM_VALUE__
#define _VM_VALUE__

#include <string>
#include <memory>
#include "model/variables.hpp"
#include "util/array.hpp"
#include "support/exceptions.hpp"


namespace MiniMC {
  namespace VM {
    class Value;
    using Value_ptr = std::shared_ptr<Value>;
    
    class Value {
    public:
      virtual ~Value () {}
      virtual std::string output () = 0;
      virtual MiniMC::Util::Array bytes () = 0;
      virtual explicit operator MiniMC::offset_t () {throw MiniMC::Support::Exception ("Not implemented");}
      
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
      
      
      
      
    };
    
  
    
    struct ValueLookup {
    public:
      virtual ~ValueLookup ()  {}
      virtual Value_ptr lookupValue (const MiniMC::Model::Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");};
      virtual void saveValue (const MiniMC::Model::Variable_ptr&, Value_ptr&&) {throw MiniMC::Support::Exception ("Not implemented");};
      virtual void unboundValue (const MiniMC::Model::Type_ptr&) {throw MiniMC::Support::Exception ("Not implemented");};
    };
    

    
    using ValueLookup_ptr = std::shared_ptr<ValueLookup>;
    
    struct Memory {
    public:
      virtual ~Memory ()  {}
      virtual const Value_ptr loadValue (const Value_ptr&, const MiniMC::Model::Type_ptr& ) {throw MiniMC::Support::Exception ("Not implemented");};
      //First parameter is address to store at, second is the value to state
      virtual void storeValue (const Value_ptr&, const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");};
      //PArameter is size to allocate
      virtual Value_ptr alloca (const Value_ptr) {throw MiniMC::Support::Exception ("Not implemented");};
      
      virtual void free (const MiniMC::Model::Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");};
    };
    
    using Memory_ptr = std::shared_ptr<Memory>;

    enum class DomainType {
      Concrete
    };

    
    struct VMState {
      VMState (ValueLookup_ptr p, Memory_ptr m) : lookup(p),memory(m) {}
      ValueLookup_ptr lookup;
      Memory_ptr memory;
    };
    
  }
  
}

#endif
