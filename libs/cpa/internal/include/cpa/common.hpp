#ifndef _CPA_COMMON__
#define _CPA_COMMON__

#include "minimc/vm/vmt.hpp"

namespace MiniMC {
  namespace CPA {
  namespace Common {
    template<class Value>
    struct ActivationRecord {
      ActivationRecord(MiniMC::Model::VariableMap<Value>&& values, const MiniMC::Model::Value_ptr& ret) : values(std::move(values)), ret(ret) {}
      ActivationRecord(const ActivationRecord&) = default;
      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	hash << values << ret.get();
	return hash;
      }
      
      MiniMC::Model::VariableMap<Value> values;
      MiniMC::Model::Value_ptr ret{nullptr};
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

      
      
      MiniMC::Model::VariableMap<Value> cpuregs;
      std::vector<ActRecord> frames;
    };

    template<class Value>
    class StackControl {
    public:
      StackControl (ActivationStack<Value>& stack) : stack(stack) {}
      void  push (MiniMC::Model::Location_ptr, std::size_t registers, const MiniMC::Model::Value_ptr& ret)  {
	ActivationRecord<Value> sf {{registers},ret};
	stack.push (std::move(sf));
      }
      
      void pop (Value&& val) {
	auto ret = stack.back ().ret;
	stack.pop ();
	if (ret)
	  stack.back().values.set (*std::static_pointer_cast<MiniMC::Model::Register> (ret),std::move(val));
      }
      
      void popNoReturn ()  {
	stack.pop ();
      }
      
      
    private:
      ActivationStack<Value>& stack;
    };

    template<class T>
    struct BaseValueLookup  {
    public:
      BaseValueLookup (ActivationStack<T>& values,MiniMC::Model::VariableMap<T>& metas) : values(values),metas(metas) {}
      BaseValueLookup (const BaseValueLookup&) = delete;
      virtual  ~BaseValueLookup () {}
      
      using Value = T;
    protected:
      T lookupRegister (const MiniMC::Model::Register& reg) const  {
	switch (reg.getRegType ()) {
	case MiniMC::Model::RegType::Local: return values.back().values[reg];
	case MiniMC::Model::RegType::CPU: return values.cpus()[reg];
	case MiniMC::Model::RegType::Meta: return metas[reg];
	default:
	  throw MiniMC::Support::Exception ("Temporaries not fulle implemented yet");
	  
	}
      }
      
      void saveRegister(const MiniMC::Model::Register& v, T&& value)  {
	switch (v.getRegType ()) {
	case MiniMC::Model::RegType::Local: values.back().values.set (v,std::move(value));break;
	case MiniMC::Model::RegType::CPU:   values.cpus ().set (v,std::move(value));break;
	case MiniMC::Model::RegType::Meta: return metas.set(v,std::move(value));
	
	default:
	  throw MiniMC::Support::Exception ("Temporaries not fulle implemented yet");
	}
	
      }
      
      
    private:
      ActivationStack<T>& values;
      MiniMC::Model::VariableMap<T>& metas;
    };
    
    template<class T,MiniMC::VMT::Evaluator<T> Eval, MiniMC::VMT::Memory<T> Mem,MiniMC::VMT::PathControl<T> PathC,MiniMC::VMT::StackControl<T> stackC>  
    struct VMState {
      using Domain = T;
      
      
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
      using Domain = T;
      
      
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
