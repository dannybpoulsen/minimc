#ifndef _CPA_COMMON__
#define _CPA_COMMON__

#include "minimc/model/variables.hpp"
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
    
    template<class Value,MiniMC::VMT::MemoryOperations<Value> MemControl>
    class EvaluationContext {
    public:
      EvaluationContext (ActivationStack<Value>& values, ActivationRecord<Value>& pers, MemControl memorycontrol,StaticContext<Value>& scontext) : values(values),persistent(pers),memcontrol(memorycontrol),scontext(scontext) {}
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

      Value load (const Value::Pointer p, const Value::Memory& m, const MiniMC::Model::Type& t) const {
	return memcontrol.load (m,p,t);
      }

      Value store (const Value::Memory& m,const Value::Pointer p,  const Value& t) const {
	return Value::visit (
			     MiniMC::Support::Overload {
			       [&m,&p,this]<typename T> (const T& v)  requires (!MiniMC::VMT::Boolean<Value,T> && !MiniMC::VMT::MemoryC<Value,T>) {
				 return Value{memcontrol.store (m,p,v)}; 
			       },
				 MiniMC::Support::Error<Value>{}
			       },
			     t);
      }
      
    private:
      ActivationStack<Value>& values;
      ActivationRecord<Value>& persistent;
      MemControl memcontrol;
      StaticContext<Value>& scontext;
    };

    template<class Value>
    class DummyRegisterStore {
    public:  
      DummyRegisterStore (StaticContext<Value>& scontext, ActivationRecord<Value>& persistent) : scontext(scontext), persistent(persistent) {}

      Value lookupRegister (const MiniMC::Model::Register& r) const  {
	switch(r.getRegType ()) {
	case MiniMC::Model::RegType::Persistent:
	  return persistent.getValueOfRegister(r);
	default:
	  throw MiniMC::Support::Exception ("No registers to load fraom");
	  
	}
      }

      
      void saveValue(const MiniMC::Model::Register& r, Value&& v)  {
	switch(r.getRegType ()) {
	case MiniMC::Model::RegType::Persistent:
	  return persistent.setValueOfRegister(r,std::move(v));
	default:
	  throw MiniMC::Support::Exception ("No registers to save to");
	  
	}
      }

      Value load (const Value::Pointer, const Value::Memory&, const MiniMC::Model::Type&) const {
	throw MiniMC::Support::Exception {"Not implemented"};
      }

      Value lookupSymbol (MiniMC::Model::Symbol s) const {
	return scontext.at(s);
      }

      Value store (const Value::Memory& ,const Value::Pointer ,  const Value&) const {
	throw MiniMC::Support::Exception {"Not implemented"};
      
      }
      
    private:
      StaticContext<Value>& scontext;
      ActivationRecord<Value>& persistent;
      
    };
    
   
    
    template<class Value>
    class StateMixin : public MiniMC::CPA::LocationInfo   {
    public:
      StateMixin (std::vector<ActivationStack<Value>>&& stacks,
		  ActivationRecord<Value>&& persistent,
		  std::shared_ptr<StaticContext<Value> >&& scontext = nullptr
		  ) : stacks(std::move(stacks)),
		      persistent(std::move(persistent)),
		      scontext(std::move(scontext))
      {}

      StateMixin (StateMixin&&) = default;
      StateMixin (const StateMixin&) = default;
      
      
      template<class Operations,MiniMC::VMT::MemoryOperations<Value> MemControl>
      static StateMixin createInitialState (const MiniMC::CPA::InitialiseDescr& descr,Operations&& ops, MemControl&& memcontrol) {
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


	  
	  
	  EvaluationContext<Value,MemControl> regstore {cs,persistent,memcontrol,*_scontext};
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

	DummyRegisterStore<Value> regstore{*_scontext,persistent};
	auto eval = MiniMC::VMT::makeEvaluator<Value> (regstore,ops);
	
	for (auto& b : descr.getHeap ().blocks()) {
	  //Allocate block here
	  auto ptr = eval.Eval (*MiniMC::Model::Pointer::make (b.baseobj));
	  auto size = eval.Eval (*MiniMC::Model::I64Integer::make (b.size));
	  
	  Value::visit (
			MiniMC::Support::Overload {
			  [&_scontext,&b,&memcontrol,&regstore](const Value::Pointer& ptr, const Value::I64& size, const Value::Memory& mem)  {
			    auto mem2 = memcontrol.allocate (mem,ptr,size);
			    _scontext->addSymbol (b.symbol,ptr);
			    regstore.saveValue (b.heap_register->asRegister(),Value{mem2});
			  },
			    [](const auto&, const auto&, const auto&) {
			      throw MiniMC::Support::Exception ("Error");
			    }
			    },
			ptr,size,eval.Eval(*b.heap_register)
			);
	  
	  if (b.value) {
	    Value ptr = eval.Eval (*MiniMC::Model::Pointer::make (b.baseobj));
            Value valueToStor = eval.Eval(*b.value);
	    
	    Value::visit (MiniMC::Support::Overload {
		[&b,&_scontext,&memcontrol,&regstore]<typename K>(const Value::Pointer& ptr, const K& value, const Value::Memory& mem) requires (!std::is_same_v<K,typename Value::Bool> && !std::is_same_v<K,typename Value::Memory>) {
		  
		  auto mem2 = memcontrol.store (mem,ptr,value);
		  regstore.saveValue (b.heap_register->asRegister(),Value{mem2});
			  
		},
		  [](const auto&, const auto&,const auto& ) {
		    throw MiniMC::Support::Exception ("Error");
		},
		  
		  
		  },
	      ptr,
	      valueToStor,
	      eval.Eval(*b.heap_register)
	      
	      );
	    }
	}
	
	return StateMixin {std::move(stack),std::move(persistent),std::move(_scontext)}; 
      }
      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	for (auto& vl : stacks) {
	  hash << vl;
	}
	return hash;
      }

      auto& getProc(std::size_t i) { return stacks.at(i); }
      
      auto& getProc(std::size_t i) const { return stacks.at(i); }
      
      //LocationInfo
      size_t nbOfProcesses() const override {return stacks.size();}
      bool isActive(size_t id) const override {return !getProc(id).activeRecord().isCPU();}
      MiniMC::Model::Location& getLocation(proc_id id) const override   {return *getProc(id).activeRecord().getLocation();}

      template<MiniMC::VMT::MemoryOperations<Value> MemControl>
      auto makeEvaluationContext (proc_id id,MemControl&& memcontrol) const {
	return EvaluationContext<Value,MemControl> (const_cast<ActivationStack<Value>&>(getProc(id)),const_cast<ActivationRecord<Value>&>(persistent),std::move(memcontrol),*scontext);
      }
      
    private:
      std::vector<ActivationStack<Value> > stacks;
      ActivationRecord<Value> persistent;
      std::shared_ptr<StaticContext<Value> > scontext;
    };
    
    
    template<class T,MiniMC::VMT::RegisterStore<T> Eval, MiniMC::VMT::StackControl stackC>  
    struct VMState {
      VMState (stackC& stack,Eval&& vlook) : scontrol(stack),lookup(std::move(vlook)) {}
      auto& getValueLookup () {return lookup;}
      
      auto& getStackControl ()  {return scontrol;}
    private:
      stackC& scontrol;
      Eval lookup;
    };
    
        
  }
  }
}

#endif
