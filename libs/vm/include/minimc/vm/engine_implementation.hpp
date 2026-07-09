#ifndef _ENGINE_IMPLE__

#define _ENGINE_IMPLE__

#include "minimc/model/cfg.hpp"
#include "minimc/model/valuevisitor.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/vm/value.hpp"
#include "minimc/vm/vmt.hpp"
#include "minimc/support/overload.hpp"
#include "vmt.hpp"

#include <iostream>
#include <ranges>
namespace MiniMC {
  namespace VMT {



    template<typename T, Ops<T> Operations, MemoryOperations<T> MemControl>
    struct Engine<T,Operations,MemControl>::Impl {
    private:
      const MiniMC::Model::Program& prgm;
      Operations operations;
      MemControl memcontrol;
      
    public:
      Impl (Operations&& operations, MemControl&& memcontrol, const MiniMC::Model::Program& prgm) : prgm(prgm),operations(operations),memcontrol(memcontrol) {}
      
      template<RegisterStore<T> Regstore>
      auto makeEvaluator (Regstore store) {
	return MiniMC::VMT::makeEvaluator<T> (store,operations);
      }


      
      template<class State>
      void addAssumption (State& st, T::Bool b) {
	st.setPathform (operations.BoolAnd (st.getPathform (),b));			
      }
      
      template <class Value>
      auto castPtrToAppropriateInteger(const Value& v) {
        if constexpr (std::is_same_v<Value, typename T::Pointer>) {
          return operations.template BitCast<typename T::I64>(v);
        } else if constexpr (std::is_same_v<Value, typename T::Pointer32>) {
          return operations.template BitCast<typename T::I32>(v);
        } else {
          return v;
        }
      }
      
      template <class I,class State>
      std::generator<std::shared_ptr<State>> runInstruction(const I&, State&,MiniMC::Model::proc_t)  {
	throw NotImplemented<I::getOpcode()> ();
      }
      
      
      template <class I,VMState<T> State>
      std::generator<std::shared_ptr<State>> runInstruction(const I& instr, State& ostate, MiniMC::Model::proc_t id) requires MiniMC::Model::isAssertAssume_v<I>       {
	auto state = ostate.lcopy ();
	
	constexpr auto op = instr.getOpcode ();
	auto& content = instr.getOps();
	auto eval = makeEvaluator (state->makeEvaluationContext (id));
	
	auto obj = T::visit (MiniMC::Support::Overload {
	    [](const T::Bool& b) {return b;},
	    MiniMC::Support::Error<typename T::Bool>{}
	  },
	  eval.Eval(*content.expr)
	  );
	
	if constexpr (op == MiniMC::Model::VMInstructionCode::Assume) {
	  addAssumption (*state,obj);
	  if (obj.boolState () != TriBool::False)
	    co_yield state;
	} 
	else if constexpr (op == MiniMC::Model::VMInstructionCode::Assert) {
	  if (obj.boolState () == TriBool::False) {
	    auto nstate = ostate.lcopy ();
	    addAssumption (*nstate,operations.BoolNegate(obj));
	    nstate->setFlag (MiniMC::VMT::FlagType::AssertViolated);
	    co_yield nstate;
	  }

	  else if  (obj.boolState () == TriBool::True) {
	    auto nstate = ostate.lcopy ();
	    addAssumption (*nstate,obj);
	    co_yield nstate;
	  }
	  else {
	    auto tstate = ostate.lcopy ();
	    addAssumption (*tstate,obj);
	    co_yield tstate;

	    auto fstate = ostate.lcopy ();
	    fstate->setFlag (MiniMC::VMT::FlagType::AssertViolated);
	    
	    addAssumption (*fstate,operations.BoolNegate(obj));
	    co_yield fstate;
	  
	    
	  }
	  if (obj.boolState () != TriBool::False)
	    co_yield state;
	} else
	  throw NotImplemented<op>();
      
    }


      template <class I,VMState<T> State>
      std::generator<std::shared_ptr<State>> runInstruction(const I& instr, State& ostate, MiniMC::Model::proc_t id)
        requires MiniMC::Model::isInternal_v<I>
      {
	auto state = ostate.lcopy();
	constexpr auto op = I::getOpcode ();
        auto eval = makeEvaluator (state->makeEvaluationContext (id));
	

        if constexpr (op == MiniMC::Model::VMInstructionCode::Assign ) {
	  auto& content = instr.getOps();
	  auto& res = content.res->asRegister ();
	  for (auto op1 : eval.MEval (*content.op1)) {
	    auto nstate = state->lcopy();
	    nstate->makeEvaluationContext(id).saveValue(res, std::move(op1));
	    co_yield nstate;;
	  }
  
        }

        else if constexpr (op == MiniMC::Model::VMInstructionCode::Call) {
	  auto& content = instr.getOps();
	  auto& scontrol = state->getStackControl(id);
	  MiniMC::Model::Value_ptr function;
          if (content.function->isConstant()) {
	    function = content.function;
	  }
	  else {
	    auto solver = state->constraint_solver();
	    solver.push ();
	    solver.addConstraint (state->getPathform ());
	    
	    if (solver.check () == MiniMC::VMT::Feasibility::Feasible) {
	      auto ff = eval.Eval(*content.function);
	      function = solver.eval (ff);
	      T::visit (MiniMC::Support::Overload {
		  [this,&state](T::Pointer l,T::Pointer r) { 
		    addAssumption (*state,operations.Eq (castPtrToAppropriateInteger(l),castPtrToAppropriateInteger(r)));
		  },
		  [this,&state](T::Pointer32 l,T::Pointer32 r) { 
		    addAssumption (*state,operations.Eq (castPtrToAppropriateInteger(l),castPtrToAppropriateInteger(r)));
		  },
		  MiniMC::Support::Error<void>{}
		    
		    },ff,eval.Eval(*function)
		);
	      solver.pop ();
	    }
	  }
	  auto func = MiniMC::Model::visitValue(
									     MiniMC::Support::Overload{
									       [this](const MiniMC::Model::Pointer& t) -> MiniMC::Model::Function_ptr {
										 auto loadPtr = t.getValue();
										 auto func = prgm.getFunction(loadPtr.base);
										 return func;
									       },
										 [this](const MiniMC::Model::Pointer32& t) -> MiniMC::Model::Function_ptr {
										   auto loadPtr = t.getValue();
										   auto func = prgm.getFunction(loadPtr.base);
										   return func;
										   										 },
										 [this,&eval](const MiniMC::Model::SymbolicConstant& t) -> MiniMC::Model::Function_ptr {
										   auto symb = t.getValue();
										   return std::get<MiniMC::Model::Function_ptr>(symb.getUserData());
										 },
										 MiniMC::Support::Error<MiniMC::Model::Function_ptr> {}
									     },
									     *function
									     );
	  
	  if (func->isVarArgs()) {
	    throw MiniMC::Support::Exception("Vararg functions are not supported");
	  }

	  
	  std::vector<T> params;
	  auto inserter = std::back_inserter(params);
	  if (content.params.size() != func->getParameters().size()) {
	    throw MiniMC::Support::Exception ("Inconsistent number of parameters between call and function definition"); 
	  }
	  for (auto [formal,actual] : std::ranges::views::zip (func->getParameters(),content.params)) {
	    if (*actual->getType () != *std::get<MiniMC::Model::Register_wptr> (formal.getUserData()).lock()->getType()) {
	      throw MiniMC::Support::Exception ("Inconsistent types at call site");
	    }
	    inserter = eval.Eval (*actual);
	  }
	  
	  
	  auto res = content.res;
	  scontrol.push(func->getCFA().getInitialLocation(),  res);

	  for (auto& r : func->getRegisterDescr().getRegisters()) {
	    state->makeEvaluationContext(id).saveValue(r, operations.defaultValue(*r.getType()));
	    
	  }

          for (auto [formal, actual] : std::ranges::views::zip(func->getParameters(), params)) {
	    auto reg = std::get<MiniMC::Model::Register_wptr> (formal.getUserData()).lock();
	    state->makeEvaluationContext(id).saveValue(*reg, std::move(actual));
	  }
	  
	  
	  co_yield state;
	}
	
        else if constexpr (op == MiniMC::Model::VMInstructionCode::Ret) {
	  auto& content = instr.getOps();
	  for (auto ret : eval.MEval(*content.value)) {
	    auto nstate = state->lcopy();
	    auto ret_reg = nstate->getStackControl(id).pop();
	    if (ret_reg) {
	      nstate->makeEvaluationContext(id).saveValue (ret_reg->asRegister (),std::move(ret));
	      
	    }
	    co_yield nstate;
	  }
	}
	
        else if constexpr (op == MiniMC::Model::VMInstructionCode::RetVoid) {
	  state->getStackControl(id).pop();
	  co_yield state;
        }

        else if constexpr (op == MiniMC::Model::VMInstructionCode::Skip) {
          co_yield state;
        }

        else {
	  throw NotImplemented<op>();
        }
      }

      template <class I,VMState<T> State>
      std::generator<std::shared_ptr<State>> runInstruction(const I& instr, State& ostate, MiniMC::Model::proc_t id)
        requires MiniMC::Model::isAggregate_v<I> 
      {
	auto state = ostate.lcopy ();
	auto eval = makeEvaluator (state->makeEvaluationContext (id));
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = content.res->asRegister ();
        assert(content.offset->isConstant());
        MiniMC::BV64 offset{0};

        offset = MiniMC::Model::visitValue<MiniMC::BV64>(MiniMC::Support::Overload{
	      [](const MiniMC::Model::I16Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const MiniMC::Model::I32Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const MiniMC::Model::I64Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      MiniMC::Support::Error<MiniMC::BV64> {}
	  },
	  *content.offset);
	
        
          throw NotImplemented<op>();
	
	
      }
      
      };
       
    template <class Value,Ops<Value> Operations,MemoryOperations<Value> MemControl>
    template<VMState<Value> State>
    std::generator<std::shared_ptr<State>> Engine<Value,Operations,MemControl>::execute(const MiniMC::Model::Instruction& instr,

											const State& wstate, MiniMC::Model::proc_t id) {
      auto nstate = wstate.lcopy ();
      for (auto state : instr.visit ([this,&wstate,id](auto& t) {return _impl->template runInstruction (t, *wstate,id);}))
	co_yield state;
      
    }
    
    template <class Value,Ops<Value> Operations,MemoryOperations<Value> MemControl> 
    template<VMState<Value> State>
    std::generator<std::shared_ptr<State>> Engine<Value,Operations,MemControl>::execute(const MiniMC::Model::InstructionStream& instr,
								       const State& wstate, MiniMC::Model::proc_t id) {
 
      auto end = instr.end();
      auto it = instr.begin();
      auto nstate = wstate.lcopy ();
      std::vector<std::shared_ptr<State>> worklist;
      worklist.push_back (nstate);
      for (it = instr.begin(); it != end;  ++it) {
	std::vector<std::shared_ptr<State>> newlist;
	for (auto cstate : worklist) {
	  if (cstate->isSet (MiniMC::VMT::FlagType::AssertViolated)) {
	    newlist.push_back (cstate);
	  }
	  else {
	    for (auto state :  it->visit ([this,&cstate,id](auto& t) {return _impl->template runInstruction (t, *cstate,id);})) {
	      newlist.push_back (state);
	    }
	  }
	}
	std::swap(worklist,newlist);
	newlist.clear();
      }
      for (auto state : worklist)
	co_yield state;
    }
    

    template <class Value,Ops<Value> Operations,MemoryOperations<Value> MemControl>
    Engine<Value,Operations,MemControl>::Engine (Operations&& ops,MemControl&& memcontrol,const MiniMC::Model::Program& prgm)   {
      _impl = std::make_unique<Impl> (std::move(ops),std::move(memcontrol),prgm);
    }

    template <class Value,Ops<Value> Operations,MemoryOperations<Value> MemControl>
    Engine<Value,Operations,MemControl>::~Engine () {}
    
  } // namespace VMT
} // namespace MiniMC

#endif
