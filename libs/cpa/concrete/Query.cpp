#include <memory>
#include <iostream>

#include "minimc/cpa/concrete.hpp"
#include "cpa/common.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/support/overload.hpp"
#include "concvm/concrete.hpp"
#include "concvm/value.hpp"
#include "concvm/operations.hpp"


namespace MiniMC {
  namespace CPA {
    namespace Concrete {

      
      
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Concrete::Value>;
      using ActivationStack = MiniMC::CPA::Common::ActivationStack<MiniMC::VMT::Concrete::Value>;
      using ConcreteVMState = MiniMC::CPA::Common::VMState<MiniMC::VMT::Concrete::Value,MiniMC::CPA::Common::EvaluationContext<MiniMC::VMT::Concrete::Value,MiniMC::VMT::Concrete::Memory>,ActivationStack>;
      
      //ConcreteVMState 
      using ConcreteEngine = MiniMC::VMT::Engine<MiniMC::VMT::Concrete::Value, MiniMC::VMT::Concrete::Operations, MiniMC::VMT::Concrete::Memory>;
      
      
      using QExpr = TQuery<MiniMC::VMT::Concrete::Value>;
      
      class MConcretizer : public MiniMC::CPA::Solver {
      public:
        MConcretizer()  {}
	MiniMC::CPA::Solver::Feasibility isFeasible() const override { return Feasibility::Feasible; }
	MiniMC::Model::Constant_ptr evaluate (const QueryExpr& expr) const override {
	  auto& ref = static_cast<const QExpr&> (expr);
	  return MiniMC::VMT::Concrete::Value::visit (MiniMC::Support::Overload {
	      [](MiniMC::VMT::Concrete::Value::I8& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I8Integer::make (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::I16& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I16Integer::make  (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::I32& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I32Integer::make (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::I64& val) ->MiniMC::Model::Constant_ptr{return MiniMC::Model::I64Integer::make  (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::Pointer& val) ->MiniMC::Model::Constant_ptr{return MiniMC::Model::Pointer::make (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::Pointer32& val) ->MiniMC::Model::Constant_ptr{return MiniMC::Model::Pointer32::make (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::Bool& val) ->MiniMC::Model::Constant_ptr{return MiniMC::Model::Bool::make (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::Aggregate& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::AggregateConstant::make  (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::Memory&) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I8Integer::make (0);}
		  },
	    ref.getValue ()
	    
	    );
	}
	
	
      };

      
      
      struct Transferer::Internal {
	Internal (const MiniMC::Model::Program& prgm) : engine(MiniMC::VMT::Concrete::Operations{},
							       MiniMC::VMT::Concrete::Memory{},
							       prgm),
							metas(prgm.getMetaRegs().getTotalRegisters())
	{}
	ConcreteEngine engine;
	MiniMC::Model::VariableMap<MiniMC::VMT::Concrete::Value> metas;
	
      };
      
      Transferer::Transferer (const MiniMC::Model::Program& p) : _internal(new Internal (p)) {}
      Transferer::~Transferer () {}
      
      class State : public MiniMC::CPA::State,
		    private MiniMC::CPA::QueryBuilder
      {
      public:
        State(MiniMC::CPA::Common::StateMixin<MiniMC::VMT::Concrete::Value>&& internal) : mixin(std::move(internal)) {
        }

	
	State(const State&)  = default;
	
	
        virtual MiniMC::Hash::hash_t hash() const override {
	  return mixin.hash ();
	}
	
        virtual State_ptr copy() const override {
	  return makeState<State>(*this); 
	}

	
	
	auto& getProc(std::size_t i) { return mixin.getProc(i); }
        
        auto& getProc(std::size_t i) const { return mixin.getProc (i); }
        
	 
        virtual const Solver_ptr getConcretizer() const override { return std::make_shared<MConcretizer> ();}

	auto makeEvaluationContext (proc_id id) const {return mixin.makeEvaluationContext(id,MiniMC::VMT::Concrete::Memory{});}
	
	//QueryBuilder
	QueryExpr_ptr buildValue (MiniMC::Model::proc_t p, const MiniMC::Model::Value& val) const override {
	  if (p >= mixin.nbOfProcesses ()) {
	    throw MiniMC::Support::Exception ("Not enough processes");
	  }
	  MiniMC::VMT::Evaluator<MiniMC::VMT::Concrete::Value
				 ,MiniMC::CPA::Common::EvaluationContext<MiniMC::VMT::Concrete::Value,
									 MiniMC::VMT::Concrete::Memory>,
				 
				 MiniMC::VMT::Concrete::Operations> eval (
																								       MiniMC::VMT::Concrete::Operations{},
																								       makeEvaluationContext(p) 														       );
	  return std::make_unique<QExpr> (eval.Eval(val));
	    
	}
	
	const QueryBuilder& getBuilder () const override  {return *this;}
	
	const MiniMC::CPA::LocationInfo& getLocationState () const {return mixin;}
	
	
      private:
	MiniMC::CPA::Common::StateMixin<MiniMC::VMT::Concrete::Value> mixin;
	};

      
	MiniMC::CPA::State_ptr CPA::makeInitialState(const InitialiseDescr& descr) {
	  return makeState<State> (MiniMC::CPA::Common::StateMixin<MiniMC::VMT::Concrete::Value>::createInitialState(descr,MiniMC::VMT::Concrete::Operations{},MiniMC::VMT::Concrete::Memory{}));
	}
      
      MiniMC::CPA::State_ptr Transferer::doTransfer(const MiniMC::CPA::State& s, const MiniMC::CPA::Transition& t )  {
	const MiniMC::Model::Edge& e = *t.edge;
	proc_id id = t.proc;
	
	auto resstate = s.copy();
        auto& nstate = static_cast<MiniMC::CPA::Concrete::State&>(*resstate);

	if (nstate.getProc(id).activeRecord ().getLocation () != e.getFrom ())
	  return nullptr;
	nstate.getProc(id).activeRecord().setLocation (e.getTo ());
	
	
	
	ConcreteVMState newvm {nstate.getProc(id),nstate.makeEvaluationContext (id)};
	auto& instr = e.getInstructions();
	auto res = _internal->engine.execute(instr,newvm);
	
	if (res.status == MiniMC::VMT::Status::Ok)
	  return resstate;
	else {
	  
	  return nullptr;

	}
      }

    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC
