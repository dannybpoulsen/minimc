#ifndef _CPA_COMMON__
#define _CPA_COMMON__

#include "minimc/vm/vmt.hpp"
#include "minimc/cpa/state.hpp"
#include "minimc/cpa/interface.hpp"

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
      
      
      MiniMC::Model::VariableMap<Value> values;
      MiniMC::Model::Value_ptr ret{nullptr};
      MiniMC::Model::Location_ptr loc;
    };
    
    template <class Value>
    struct ActivationStack {
      ActivationStack(const MiniMC::Model::RegisterDescr& cpuregs)  {
	frames.push_back(ActivationRecord<Value>{cpuregs.getTotalRegisters(),nullptr,nullptr});
      } 
      
      ActivationStack(const ActivationStack&) = default;
      
      auto pop()  {
	if(frames.size () > 1) {
	  auto retval = frames.back().ret;
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

      auto& back () {return frames.back ();}
      auto& back () const {return frames.back ();}
      auto& cpus () {return frames.front();}
      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	for (auto& vl : frames) {
	  hash << vl;
	}
	return hash;
      }

      auto getDepth () const {return frames.size();}
      
      
      std::vector<ActivationRecord<Value>> frames;
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
	case MiniMC::Model::RegType::CPU: return values.cpus().values[reg];
	case MiniMC::Model::RegType::Meta: return metas[reg];
	default:
	  std::unreachable();
	  
	}
      }
      
      void saveValue(const MiniMC::Model::Register& v, Value&& value)  {
	switch (v.getRegType ()) {
	case MiniMC::Model::RegType::Local: values.back().values.set (v,std::move(value));break;
	case MiniMC::Model::RegType::CPU:   values.cpus ().values.set (v,std::move(value));break;
	case MiniMC::Model::RegType::Meta: metas.set(v,std::move(value));break;
	  
	default:
	  std::unreachable();
	}
	
      }

      Value lookupRegisterViaSymbol (const MiniMC::Model::Symbol& symbol) {
	auto it = values.rbegin ();
	auto end = values.rend ();
	for (; it != end; ++it) {
	  ActivationRecord<Value>& cur = *it;
	  auto& regs = cur.loc->getLocationInfo().getRegisters();
	  if (regs.hasSymbol (symbol)) {
	    return cur.values[regs.getRegister (symbol)];
	  }
	}

	throw MiniMC::Support::Exception ("HH");
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
      
      void saveValue(const MiniMC::Model::Register&, Value&&)  {
	throw MiniMC::Support::Exception ("Cannot save register");
      }
      
      
    };
    
   
    
    template<class Value,MiniMC::VMT::Memory<Value> Mem>
    class StateMixin : public MiniMC::CPA::LocationInfo   {
    public:
      StateMixin (std::vector<ActivationStack<Value>>&& stacks,
		  Mem&& mem) : stacks(std::move(stacks)),
			       memory(std::move(mem)) {}

      StateMixin (StateMixin&&) = default;
      StateMixin (const StateMixin&) = default;
      
      
      template<class Operations>
      static StateMixin createInitialState (const MiniMC::CPA::InitialiseDescr& descr,Operations&& ops,Mem&& heap) {
	std::vector<ActivationStack<Value>> stack;
	for (auto& f : descr.getEntries()) {
          auto& vstack = f.getFunction()->getRegisterDescr();
	  
	  ActivationStack<Value> cs {descr.getProgram().getCPURegs()};
	  cs.push (f.getFunction()->getCFA().getInitialLocation (),nullptr);
	  MiniMC::Model::VariableMap<Value> metas{1};
	  RegisterStore<Value> regstore {cs,metas};
	  for (auto& v : vstack.getRegisters()) {
            regstore.saveValue  (v,ops.defaultValue (*v.getType ()));
	  }

	  for (auto& reg : descr.getProgram().getCPURegs().getRegisters()) {
	    auto val = ops.defaultValue (*reg.getType ());
	    regstore.saveValue (reg,std::move(val));
	  }
	  

	  auto pit = f.getParams ().begin ();
	  auto rit = f.getFunction()->getParameters().begin ();
	  MiniMC::VMT::Evaluator<Value,RegisterStore<Value>,Operations,Mem> eval {ops,regstore,heap};
	  for (; pit != f.getParams ().end ();++pit,++rit) {
	    //TODO: Updatee this 
	    regstore.saveValue  (**rit,eval.Eval (**pit));
	  } 
	  
          stack.push_back(cs);
	  
        }

	MiniMC::VMT::Evaluator<Value,DummyRegisterStore<Value>,Operations,Mem> eval {ops,DummyRegisterStore<Value>{},heap};
	heap.createHeapLayout (descr.getHeap ());
	
	for (auto& b : descr.getHeap ()) {
	  if (b.value) {
	    Value ptr = eval.Eval (MiniMC::Model::Pointer (b.baseobj));
            Value valueToStor = eval.Eval(*b.value);
	    Value::visit (MiniMC::Support::Overload {
		[&heap]<typename K>(const Value::Pointer& ptr, const K& value) requires (!std::is_same_v<K,typename Value::Bool>) {
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
	
	return StateMixin {std::move(stack),std::move(heap)}; 
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
      bool isActive(size_t id) const override {return !getProc(id).back().isCPU();}
      MiniMC::Model::Location& getLocation(proc_id id) const override   {return *getProc(id).back().getLocation();}
      
      
    private:
      std::vector<ActivationStack<Value> > stacks;
      Mem memory;
    };
    
    
    template<class T,MiniMC::VMT::RegisterStore<T> Eval, MiniMC::VMT::Memory<T> Mem,MiniMC::VMT::PathControl<T> PathC,MiniMC::VMT::StackControl stackC>  
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
    
        
  }
  }
}

#endif
