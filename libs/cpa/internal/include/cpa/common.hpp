#ifndef _CPA_COMMON__
#define _CPA_COMMON__

#include "minimc/vm/vmt.hpp"

namespace MiniMC {
  namespace CPA {
  namespace Common {
    template<class Value>
    struct ActivationRecord {
      ActivationRecord(MiniMC::Model::VariableMap<Value>&& values, const MiniMC::Model::Value_ptr& ret, MiniMC::Model::Location_ptr l) : values(std::move(values)), ret(ret),loc(l) {}
      ActivationRecord(const ActivationRecord&) = default;
      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	hash << values << ret.get() << loc.get();
	return hash;
      }

      auto& getLocation () const {return loc;}
      void setLocation (MiniMC::Model::Location_ptr l)  {loc = l;}
      
      
      MiniMC::Model::VariableMap<Value> values;
      MiniMC::Model::Value_ptr ret{nullptr};
      MiniMC::Model::Location_ptr loc;
    };
    
    template <class Value>
    struct ActivationStack {
      using ActRecord = ActivationRecord<Value>;
      ActivationStack(MiniMC::Model::VariableMap<Value>&& cpuregs) : cpuregs(std::move(cpuregs)) {
      }
      ActivationStack(MiniMC::Model::VariableMap<Value>&& cpuregs, ActRecord&& sf) : cpuregs(std::move(cpuregs)) {
        frames.push_back(std::move(sf));
      }
      ActivationStack(const ActivationStack&) = default;

      auto pop() {
        auto val = frames.back();
        frames.pop_back();
        return val;
      }

      void push(ActRecord&& frame) {
        frames.push_back (std::move(frame));
      }

      auto& back () {return frames.back ();}
      auto& back () const {return frames.back ();}
      auto& cpus () {return cpuregs;}
      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	hash << cpuregs;
	for (auto& vl : frames) {
	  hash << vl;
	}
	return hash;
      }

      auto getDepth () const {return frames.size();} 
      
      MiniMC::Model::VariableMap<Value> cpuregs;
      std::vector<ActRecord> frames;
    };

    template<class Value>
    class StackControl {
    public:
      StackControl (ActivationStack<Value>& stack) : stack(stack) {}
      void  push (MiniMC::Model::Location_ptr loc, std::size_t registers, const MiniMC::Model::Value_ptr& ret)  {
	ActivationRecord<Value> sf {{registers},ret,loc};
	stack.push (std::move(sf));
      }
      
      MiniMC::Model::Value_ptr pop () {
	auto ret = stack.back ().ret;
	stack.pop ();
	return ret;
      }
      
    private:
      ActivationStack<Value>& stack;
    };

    template<class Value>
    class RegisterStore {
    public:
      RegisterStore (ActivationStack<Value>& values,MiniMC::Model::VariableMap<Value>& metas) : values(values),
												metas(metas) {}
    public:
      Value lookupRegister (const MiniMC::Model::Register& reg) const  {
	switch (reg.getRegType ()) {
	case MiniMC::Model::RegType::Local: return values.back().values[reg];
	case MiniMC::Model::RegType::CPU: return values.cpus()[reg];
	case MiniMC::Model::RegType::Meta: return metas[reg];
	default:
	  throw MiniMC::Support::Exception ("Temporaries not fulle implemented yet");
	  
	}
      }
      
      void saveRegister(const MiniMC::Model::Register& v, Value&& value)  {
	switch (v.getRegType ()) {
	case MiniMC::Model::RegType::Local: values.back().values.set (v,std::move(value));break;
	case MiniMC::Model::RegType::CPU:   values.cpus ().set (v,std::move(value));break;
	case MiniMC::Model::RegType::Meta: return metas.set(v,std::move(value));
	
	default:
	  throw MiniMC::Support::Exception ("Temporaries not fulle implemented yet");
	}
	
      }
      
      
    private:
      ActivationStack<Value>& values; 
      MiniMC::Model::VariableMap<Value>& metas;
    };

    template<class Value>
    class DummyRegisterStore {
    public:  
      DummyRegisterStore () {}
      Value lookupRegister (const MiniMC::Model::Register& ) const  {
	throw MiniMC::Support::Exception ("No registers to load fraom");
        
      }
      
      void saveRegister(const MiniMC::Model::Register&, Value&&)  {
	throw MiniMC::Support::Exception ("Cannot save register");
      }
      
      
    };
    
    template<class Value,class Creator,class RegStore = DummyRegisterStore<Value>>
    struct ValueLookup  {
    public:
      ValueLookup (Creator&& creator, RegStore&& store = RegStore{}) : store(std::move(store)),creator(std::move(creator)) {}
      ValueLookup (const ValueLookup&) = delete;
      virtual  ~ValueLookup () {}
      
      Value lookupValue (const MiniMC::Model::Value& v) const {
	return MiniMC::Model::visitValue(
				  MiniMC::Model::Overload{
				    [this](const MiniMC::Model::Register& val) -> Value {
				      return store.lookupRegister (val);
				    },
				      [this](const auto& v) -> Value {
					return creator.create(v);
				      }
				      },
				  v);

      }

      void saveValue(const MiniMC::Model::Register& r, Value&& v)  {
	store.saveRegister (r,std::move(v));
      } 
	
      
      Value unboundValue(const MiniMC::Model::Type& t) const {return creator.unboundValue (t);}
      Value defaultValue(const MiniMC::Model::Type& t) const {return creator.defaultValue (t);}
            
    private:
      RegStore store;
      Creator  creator;
    };
    
    template<class T,MiniMC::VMT::Evaluator<T> Eval, MiniMC::VMT::Memory<T> Mem,MiniMC::VMT::PathControl<T> PathC,MiniMC::VMT::StackControl stackC>  
    struct VMState {
      
      
      VMState (Mem& m, PathC& path, stackC& stack,Eval& vlook) : memory(m),control(path),scontrol(stack),lookup(vlook) {}
      auto& getValueLookup () {return lookup;}
      auto& getMemory () {return memory;}
      auto& getPathControl ()  {return control;}
      auto& getStackControl ()  {return scontrol;}
    private:
      Mem& memory;
      PathC& control;
      stackC& scontrol;
      Eval& lookup;
    };
    
    
    template<class T,MiniMC::VMT::Evaluator<T> Eval,MiniMC::VMT::Memory<T> Mem,MiniMC::VMT::PathControl<T> PathC>  
    struct VMInitState {
      
      VMInitState (Mem& m, PathC& path, Eval& vlook) : memory(m),control(path),lookup(vlook) {}
      auto& getValueLookup () {return lookup;}
      auto& getPathControl () const {return control;}
    private:
      Mem& memory;
      PathC& control;
      Eval& lookup;
    };

    
  }
  }
}

#endif
