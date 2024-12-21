#ifndef _CPA_COMMON__
#define _CPA_COMMON__

#include "minimc/vm/vmt.hpp"
#include "minimc/cpa/state.hpp"
#include "minimc/cpa/interface.hpp"
#include <ranges>

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
      bool isCPU() const {return loc == nullptr;}

      auto returnRegister () const {return ret;}
      auto getValueOfRegister (const MiniMC::Model::Register& r) const {
	return values[r];
      }

      auto isActiveRegister (const MiniMC::Model::Symbol& r) const {
	if (!loc)
	  return false;
	
	else return loc->getInfo().getRegisters().hasSymbol (r);
	
      }

      auto getValue (const MiniMC::Model::Symbol& r) const {
	return getValueOfRegister(*loc->getInfo().getRegisters().getRegister (r));
      }
      
      void setValueOfRegister (const MiniMC::Model::Register& r,Value&& v) {
	values.set(r,std::move(v));
      }
      
      
    private:
      MiniMC::Model::VariableMap<Value> values;
      MiniMC::Model::Value_ptr ret{nullptr};
      MiniMC::Model::Location_ptr loc;
    };
    
    template <class Value>
    struct ActivationStack {
      ActivationStack(const MiniMC::Model::RegisterDescr& cpuregs,const MiniMC::Model::RegisterDescr& metaregs)  {
	
	metas = std::make_shared<MiniMC::Model::VariableMap<Value>> (metaregs.getTotalRegisters()); 
	frames.push_back(ActivationRecord<Value>{cpuregs.getTotalRegisters(),nullptr,nullptr});
      } 
      
      ActivationStack(const ActivationStack&) = default;
      
      auto pop()  {
	if(frames.size () > 1) {
	  auto retval = frames.back().returnRegister();
	  frames.pop_back();
	  return retval;
	}
	else {
	  throw MiniMC::Support::Exception ("Cannot pop CPU_frame of stack");
	}
      }
      
      void push(MiniMC::Model::Location_ptr loc, const MiniMC::Model::Value_ptr& ret) {
        frames.push_back (ActivationRecord<Value>{{loc->getInfo().getRegisters().getTotalRegisters()},ret,loc});
      }

      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	for (auto& vl : frames) {
	  hash << vl;
	}
	return hash;
      }

      auto getDepth () const {return frames.size();}

      auto& activeRecord () const {return frames.back();}
      auto& activeRecord () {return frames.back();}
      
      Value lookupRegister (const MiniMC::Model::Register& reg) const  {
	switch (reg.getRegType ()) {
	case MiniMC::Model::RegType::Local: return frames.back().getValueOfRegister(reg);
	case MiniMC::Model::RegType::CPU: return frames.front().getValueOfRegister(reg);
	case MiniMC::Model::RegType::Meta: return (*metas)[reg];
	case MiniMC::Model::RegType::Persistent: throw MiniMC::Support::Exception ("Persistent registers not implemented yet");
		  
	default:
	  std::unreachable();
	  
	}
      }
      
      void saveValue(const MiniMC::Model::Register& v, Value&& value)  {
	switch (v.getRegType ()) {
	case MiniMC::Model::RegType::Local: frames.back().setValueOfRegister (v,std::move(value));break;
	case MiniMC::Model::RegType::CPU:   frames.front().setValueOfRegister (v,std::move(value));break;
	case MiniMC::Model::RegType::Meta: metas->set(v,std::move(value));break;
	case MiniMC::Model::RegType::Persistent: throw MiniMC::Support::Exception ("Persistent registers not implemented yet");  
	default:
	  std::unreachable();
	}
	
      }

      auto searchForRecordWithSymbol (const MiniMC::Model::Symbol& s) const -> const ActivationRecord<Value>*  {
	for (auto& a : std::ranges::reverse_view (frames)) {
	  if (a.isActiveRegister(s)) {
	    return &a;
	  }
	}
	return nullptr;
      }

      
      std::vector<ActivationRecord<Value>> frames;
      std::shared_ptr<MiniMC::Model::VariableMap<Value>> metas;
    };

    template<class Value>
    class StaticContext {
    public:
      StaticContext (MiniMC::Model::SymbolTable<Value>&& map = MiniMC::Model::SymbolTable<Value> {}) : symbmap(std::move(map)){}

      void addSymbol (MiniMC::Model::Symbol symb, Value val) {
	symbmap.emplace (symb,val);
      }

      bool hasSymbol (MiniMC::Model::Symbol symb) {
	return symbmap.count (symb);
      }
      
      
      auto at (const MiniMC::Model::Symbol& s) const {
	return symbmap.at(s);
      }
      
    private:
      MiniMC::Model::SymbolTable<Value> symbmap;
    };
    
    template<class Value,class Memory>
    class EvaluationContext {
    public:
      EvaluationContext (ActivationStack<Value>& values, ActivationRecord<Value>& pers, Memory& memory,StaticContext<Value>& scontext) : values(values),persistent(pers),memory(memory),scontext(scontext) {}
    public:
      Value lookupRegister (const MiniMC::Model::Register& reg) const  {
	if (reg.getRegType () == MiniMC::Model::RegType::Persistent) {
	  return persistent.getValueOfRegister (reg);
	}
	return values.lookupRegister(reg);
      }

      Value lookupSymbol (MiniMC::Model::Symbol s) const {
	if (scontext.hasSymbol(s))
	  return scontext.at(s);
	else {
	  auto record = values.searchForRecordWithSymbol (s);
	  if (record) {
	    return record->getValue (s);
	  }
	}
	MiniMC::Support::Localiser loc {"Cannot localise symbol %1%"};
	throw MiniMC::Support::Exception (loc.format(s));
      }
      
      void saveValue(const MiniMC::Model::Register& reg, Value&& value)  {
	if (reg.getRegType () == MiniMC::Model::RegType::Persistent) {
	  return persistent.setValueOfRegister  (reg,std::move(value));
	}
	else 
	  values.saveValue(reg,std::move(value));
      }

      Value load (const Value::Pointer p, const MiniMC::Model::Type& t) const {
	return memory.load (p,t);
      }
      
    private:
      ActivationStack<Value>& values;
      ActivationRecord<Value>& persistent; 
      Memory& memory;
      StaticContext<Value>& scontext;
    };

    template<class Value>
    class DummyRegisterStore {
    public:  
      DummyRegisterStore (StaticContext<Value>& scontext) : scontext(scontext) {}

      Value lookupRegister (const MiniMC::Model::Register& ) const  {
	throw MiniMC::Support::Exception ("No registers to load fraom");
        
      }

      
      void saveValue(const MiniMC::Model::Register&, Value&&)  {
	throw MiniMC::Support::Exception ("Cannot save register");
      }

      Value load (const Value::Pointer, const MiniMC::Model::Type&) const {
	throw MiniMC::Support::Exception {"Not implemented"};
      }

      Value lookupSymbol (MiniMC::Model::Symbol s) const {
	return scontext.at(s);
      }

    private:
      StaticContext<Value>& scontext;
      
      
    };
    
   
    
    template<class Value,MiniMC::VMT::Memory<Value> Mem>
    class StateMixin : public MiniMC::CPA::LocationInfo   {
    public:
      StateMixin (std::vector<ActivationStack<Value>>&& stacks,
		  Mem&& mem,
		  ActivationRecord<Value>&& persistent,
		  std::shared_ptr<StaticContext<Value> >&& scontext = nullptr
		  ) : stacks(std::move(stacks)),
		      persistent(std::move(persistent)),
		      memory(std::move(mem)),
		      scontext(std::move(scontext))
      {}

      StateMixin (StateMixin&&) = default;
      StateMixin (const StateMixin&) = default;
      
      
      template<class Operations>
      static StateMixin createInitialState (const MiniMC::CPA::InitialiseDescr& descr,Operations&& ops,Mem&& heap) {
	std::vector<ActivationStack<Value>> stack;
	auto _scontext = std::make_shared<MiniMC::CPA::Common::StaticContext<Value>> ();
	ActivationRecord<Value> persistent {descr.getProgram().getPersistentRegs ().getTotalRegisters(),nullptr,nullptr};
	for (auto& v : descr.getProgram().getPersistentRegs().getRegisters()) {
	  persistent.setValueOfRegister(v,ops.defaultValue (*v.getType()));
	 }
	for (auto& f : descr.getEntries()) {
          auto& vstack = f.getFunction()->getRegisterDescr();
	  
	  ActivationStack<Value> cs {descr.getProgram().getCPURegs(),descr.getProgram().getMetaRegs()};
	  cs.push (f.getFunction()->getCFA().getInitialLocation (),nullptr);


	  
	  
	  EvaluationContext<Value,Mem> regstore {cs,persistent,heap,*_scontext};
	  for (auto& v : vstack.getRegisters()) {
            regstore.saveValue  (v,ops.defaultValue (*v.getType ()));
	  }

	  for (auto& reg : descr.getProgram().getCPURegs().getRegisters()) {
	    auto val = ops.defaultValue (*reg.getType ());
	    regstore.saveValue (reg,std::move(val));
	  }

	  for (auto& reg : descr.getProgram().getMetaRegs().getRegisters()) {
	    auto val = ops.defaultValue (*reg.getType ());
	    regstore.saveValue (reg,std::move(val));
	  }

	  auto pit = f.getParams ().begin ();
	  auto rit = f.getFunction()->getParameters().begin ();
	  auto eval = MiniMC::VMT::makeEvaluator<Value> (regstore,ops);
	  for (; pit != f.getParams ().end ();++pit,++rit) {
	    //TODO: Updatee this 
	    regstore.saveValue  (**rit,eval.Eval (**pit));
	  } 
	  
          stack.push_back(cs);
	  
        }

	auto eval = MiniMC::VMT::makeEvaluator<Value> (DummyRegisterStore<Value>{*_scontext},ops);
	heap.createHeapLayout (descr.getHeap (),*_scontext);
	
	for (auto& b : descr.getHeap ().blocks()) {
	  if (b.value) {
	    Value ptr = eval.Eval (MiniMC::Model::Pointer (b.baseobj));
            Value valueToStor = eval.Eval(*b.value);
	    Value::visit (MiniMC::Support::Overload {
		[&heap,&_scontext]<typename K>(const Value::Pointer& ptr, const K& value) requires (!std::is_same_v<K,typename Value::Bool>) {
		  
		  heap.store (ptr,value);
		},
		[](const auto&, const auto&) {
		    throw MiniMC::Support::Exception ("Error");
		},
		  
		  
		  },
	      ptr,
	      valueToStor
	      );
	    }
	}
	
	return StateMixin {std::move(stack),std::move(heap),std::move(persistent),std::move(_scontext)}; 
      }
      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	for (auto& vl : stacks) {
	  hash << vl;
	}
	hash << memory;
	return hash;
      }

      auto& getProc(std::size_t i) { return stacks.at(i); }
      auto& getMemory() { return memory; }
      
      auto& getProc(std::size_t i) const { return stacks.at(i); }
      auto& getMemory() const { return memory; }
      
      //LocationInfo
      size_t nbOfProcesses() const override {return stacks.size();}
      bool isActive(size_t id) const override {return !getProc(id).activeRecord().isCPU();}
      MiniMC::Model::Location& getLocation(proc_id id) const override   {return *getProc(id).activeRecord().getLocation();}
      
      auto makeEvaluationContext (proc_id id) const {
	return EvaluationContext (const_cast<ActivationStack<Value>&>(getProc(id)),const_cast<ActivationRecord<Value>&>(persistent),const_cast<Mem&>(memory),*scontext);
      }
      
    private:
      std::vector<ActivationStack<Value> > stacks;
      ActivationRecord<Value> persistent;
      Mem memory;
      std::shared_ptr<StaticContext<Value> > scontext;
    };
    
    
    template<class T,MiniMC::VMT::RegisterStore<T> Eval, MiniMC::VMT::Memory<T> Mem,MiniMC::VMT::PathControl<T> PathC,MiniMC::VMT::StackControl stackC>  
    struct VMState {
      VMState (Mem& m, PathC& path, stackC& stack,Eval&& vlook) : memory(m),control(path),scontrol(stack),lookup(std::move(vlook)) {}
      auto& getValueLookup () {return lookup;}
      auto& getMemory () {return memory;}
      void  setMemory (Mem&& m) {memory = std::move(m);}
      
      auto& getPathControl ()  {return control;}
      auto& getStackControl ()  {return scontrol;}
    private:
      Mem& memory;
      PathC& control;
      stackC& scontrol;
      Eval lookup;
    };
    
        
  }
  }
}

#endif
