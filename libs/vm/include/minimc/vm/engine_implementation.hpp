#ifndef _ENGINE_IMPLE__

#define _ENGINE_IMPLE__

#include "minimc/model/cfg.hpp"
#include "minimc/model/valuevisitor.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/vm/value.hpp"
#include "minimc/vm/vmt.hpp"
#include "minimc/support/overload.hpp"

#include <iostream>

namespace MiniMC {
  namespace VMT {



    template<typename T, Ops<T> Operations, MemoryOperations<T> MemControl>
    struct Engine<T,Operations,MemControl>::Impl {
    private:
      const MiniMC::Model::Program& prgm;
      Operations operations;
      MemControl memcontrol;
      typename T::Bool asserts;
      typename T::Bool assumes;
      
      
    public:
      Impl (Operations&& operations, MemControl&& memcontrol, const MiniMC::Model::Program& prgm) : prgm(prgm),operations(operations),memcontrol(memcontrol) {}
      
      template<RegisterStore<T> Regstore>
      auto makeEvaluator (Regstore store) {
	return MiniMC::VMT::makeEvaluator<T> (store,operations);
      }

      void reset (T::Bool pathform) {
	asserts = operations.create (MiniMC::Model::Bool(true));
	assumes = pathform;
      }

      auto getAssertions () const {return asserts;}
      auto getAssumes () const {return assumes;}
      void addAssertion (T::Bool b) {asserts = operations.BoolAnd (asserts,b);}
      
      template<class State>
      void addAssumption (State& st, T::Bool b) {
	st.setPathform (operations.BoolAnd (st.getPathform (),b));			
      }
      
      template <class Value>
      auto castPtrToAppropriateInteger(const Value& v) {
        if constexpr (std::is_same_v<Value, typename T::Pointer>) {
          return operations.template PtrToInt<typename T::I64>(v);
        } else if constexpr (std::is_same_v<Value, typename T::Pointer32>) {
          return operations.template Ptr32ToInt<typename T::I32>(v);
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
	  addAssertion (obj);
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
          auto op1 = eval.Eval(*content.op1);
          state->makeEvaluationContext(id).saveValue(res, std::move(op1));
          co_yield state;;
        }

        else if constexpr (op == MiniMC::Model::VMInstructionCode::Call) {
	  auto& content = instr.getOps();
	  auto& scontrol = state->getStackControl(id);
          assert(content.function->isConstant());
	  
          auto func = MiniMC::Model::visitValue<MiniMC::Model::Function_ptr>(
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
										   return std::get<MiniMC::Model::Function_wptr>(symb.getUserData()).lock();
										 },
										 MiniMC::Support::Error<MiniMC::Model::Function_ptr> {}
									     },
									     *content.function
									     );

	  std::vector<T> params;
	  if (func->isVarArgs()) {
	    throw MiniMC::Support::Exception("Vararg functions are not supported");
	  }
	  
	    
	  auto inserter = std::back_inserter(params);
	  std::for_each(content.params.begin(), content.params.end(), [&inserter, &state,&eval](auto& v) { inserter = eval.Eval(*v); });
	  
	  
	  auto res = content.res;
	  scontrol.push(func->getCFA().getInitialLocation(),  res);
	  
	  auto it = params.begin();
	  for (auto& p : func->getParameters()) {
	    state->makeEvaluationContext(id).saveValue(*p, std::move(*it));
	    ++it;
	  }

	
	  co_yield state;
	}

        else if constexpr (op == MiniMC::Model::VMInstructionCode::Ret) {
	  auto& content = instr.getOps();
	  auto ret = eval.Eval(*content.value);
	  auto ret_reg = state->getStackControl(id).pop();
	  if (ret_reg) {
	    state->makeEvaluationContext(id).saveValue (ret_reg->asRegister (),std::move(ret));
	  }
	  co_yield state;
	}

        else if constexpr (op == MiniMC::Model::VMInstructionCode::RetVoid) {
	  state->getStackControl(id).pop();
	  co_yield state;
        }

        else if constexpr (op == MiniMC::Model::VMInstructionCode::Skip) {
          co_yield state;
        }

        else if constexpr (op == MiniMC::Model::VMInstructionCode::NonDet) {
	  auto& content = instr.getOps();
          auto& res = content.res->asRegister ();
	  auto ret = eval.Eval(*MiniMC::Model::Undef::make(res.getType()));
          state->makeEvaluationContext(id).saveValue(res, std::move(ret));
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
	
        if constexpr (op == MiniMC::Model::VMInstructionCode::InsertValue) {
          auto val_v = content.insertee;

          auto aggr = eval.Eval(*content.aggregate);
          auto value = eval.Eval(*content.insertee);

	  T::visit (MiniMC::Support::Overload {
	      [this,&eval,&state,&res,&offset,id](const typename T::Aggregate& aggr,const typename T::Aggregate& value) {
		state->makeEvaluationContext(id).saveValue(res, operations.template InsertAggregateValue(aggr, offset, value));
	      },
		[this,&eval,&state,&res,&offset,id]<typename K>(const typename T::Aggregate& aggr,const K& value) requires (!MiniMC::VMT::MemoryC<T,K>) {
		state->makeEvaluationContext(id).saveValue(res, operations.template InsertBaseValue(aggr, offset, value));
	      },
	      MiniMC::Support::Error<void> {}
	    }
 	    ,
	    aggr,
	    value);
	  co_yield state;
        }

        else if constexpr (op == MiniMC::Model::VMInstructionCode::ExtractValue) {
	  typename T::Aggregate aggr = T::visit (MiniMC::Support::Overload {
	      [](const typename T::Aggregate& aggr) {return aggr;},
	      MiniMC::Support::Error<typename T::Aggregate> {}	
		},
	    eval.Eval(*content.aggregate)
	    );
	  
          switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              state->makeEvaluationContext(id).saveValue(res, operations.template ExtractBaseValue<typename T::I8>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I16:
              state->makeEvaluationContext(id).saveValue(res, operations.template ExtractBaseValue<typename T::I16>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I32:
              state->makeEvaluationContext(id).saveValue(res, operations.template ExtractBaseValue<typename T::I32>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I64:
              state->makeEvaluationContext(id).saveValue(res, operations.template ExtractBaseValue<typename T::I64>(aggr, offset));
              break;

            case MiniMC::Model::TypeID::Pointer:
              state->makeEvaluationContext(id).saveValue(res, operations.template ExtractBaseValue<typename T::Pointer>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::Aggregate:
              state->makeEvaluationContext(id).saveValue(res, operations.ExtractAggregateValue(aggr, offset, res.getType()->getSize()));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Extract");
          }
	  co_yield state;
        }
	else
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
	for (auto cstate : worklist)
	  for (auto state :  it->visit ([this,&cstate,id](auto& t) {return _impl->template runInstruction (t, *cstate,id);})) {
	    newlist.push_back (state);
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
