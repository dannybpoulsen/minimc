#ifndef _VM_VM__
#define _VM_VM__

#include "vm/value.hpp"
#include "model/instructions.hpp"

namespace MiniMC {
  namespace VM {

    template<MiniMC::Model::InstructionCode c>
    class NotImplemented : public MiniMC::Support::Exception {
    public:
      NotImplemented () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Instruction '%1%' not supported."}.format (c)) {}
    };

    class Value;
    using Value_ptr = std::shared_ptr<Value>;
    
    struct ValueLookup {
    public:
      virtual ~ValueLookup ()  {}
      virtual Value_ptr lookupValue (const MiniMC::Model::Value_ptr&) const {throw MiniMC::Support::Exception ("Not implemented");};
      virtual void saveValue (const MiniMC::Model::Variable_ptr&, Value_ptr&&) {throw MiniMC::Support::Exception ("Not implemented");};
      virtual void unboundValue (const MiniMC::Model::Type_ptr&) const {throw MiniMC::Support::Exception ("Not implemented");};
      virtual std::shared_ptr<ValueLookup> copy () = 0;
    };
    

    
    using ValueLookup_ptr = std::shared_ptr<ValueLookup>;
    
    struct Memory {
    public:
      virtual ~Memory ()  {}
      virtual Value_ptr loadValue (const Value_ptr&, const MiniMC::Model::Type_ptr& ) const  {throw MiniMC::Support::Exception ("Not implemented");};
      //First parameter is address to store at, second is the value to state
      virtual void storeValue (const Value_ptr&, const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");};
      //PArameter is size to allocate
      virtual Value_ptr alloca (const Value_ptr) {throw MiniMC::Support::Exception ("Not implemented");};
      
      virtual void free (const Value_ptr&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual std::shared_ptr<Memory> copy () = 0;
    };

    using Memory_ptr = std::shared_ptr<Memory>;
    
    struct PathControl {
      virtual ~PathControl ()  {}
      virtual void addAssumption (const Value_ptr) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual void addAssert (const Value_ptr) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual std::shared_ptr<PathControl> copy () = 0;
      
    };
    
    using PathControl_ptr = std::shared_ptr<PathControl>;
    
    
    
    struct VMState {
      VMState (ValueLookup_ptr p, Memory_ptr m, PathControl_ptr& path) : lookup(p),memory(m),control(path) {}
      auto& getValueLookup () {return *lookup;}
      auto& getMemory () {return *memory;}
      auto& getValueLookup () const {return *lookup;}
      auto& getMemory () const {return *memory;}
      auto& getPathControl () const {return *control;}
    private:
      ValueLookup_ptr lookup;
      Memory_ptr memory;
      PathControl_ptr control;
    };

    enum class  Status{
      Ok,
      AssumeViolated,
      AssertViolated,
      UnsupportedOperation
    };
    
    class Engine {
    public:
      virtual Status execute (const MiniMC::Model::InstructionStream&, VMState&, const VMState&  ) ;
    };
    
  }

		
}

#endif
