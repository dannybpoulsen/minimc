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
      class QExpr : public MiniMC::CPA::QueryExpr {
      public:
	QExpr (MiniMC::VMT::Concrete::Value&& val) : value(std::move(val)) {}
	std::ostream& output (std::ostream& os) const override {
	  return os << value; 
	}

	auto getValue () const {return value;}
	
      private:
	MiniMC::VMT::Concrete::Value value;
      };

      class MConcretizer : public MiniMC::CPA::Solver {
      public:
        MConcretizer()  {}
	MiniMC::CPA::Solver::Feasibility isFeasible() const override { return Feasibility::Feasible; }
	MiniMC::Model::Constant_ptr evaluate (const QueryExpr& expr) const override {
	  auto& ref = static_cast<const QExpr&> (expr);
	  return MiniMC::VMT::Concrete::Value::visit (MiniMC::Support::Overload {
	        [](MiniMC::VMT::Concrete::Value::I8& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::I8Integer> (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::I16& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::I16Integer> (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::I32& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::I32Integer> (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::I64& val) ->MiniMC::Model::Constant_ptr{return std::make_shared<MiniMC::Model::I64Integer> (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::Pointer& val) ->MiniMC::Model::Constant_ptr{return std::make_shared<MiniMC::Model::Pointer> (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::Pointer32& val) ->MiniMC::Model::Constant_ptr{return std::make_shared<MiniMC::Model::Pointer32> (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::Bool& val) ->MiniMC::Model::Constant_ptr{return std::make_shared<MiniMC::Model::Bool> (val.getValue ());},
		[](MiniMC::VMT::Concrete::Value::Aggregate& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::AggregateConstant> (val.getValue ());},
		  },
	    ref.getValue ()
	    
	    );
	}
	
	
      };

      
      
      struct Transferer::Internal {
	Internal (const MiniMC::Model::Program& prgm) : engine(MiniMC::VMT::Concrete::Operations{},
							       prgm),
							metas(prgm.getMetaRegs().getTotalRegisters())
	{}
	MiniMC::VMT::Concrete::ConcreteEngine engine;
	MiniMC::Model::VariableMap<MiniMC::VMT::Concrete::Value> metas;
	
      };
      
      Transferer::Transferer (const MiniMC::Model::Program& p) : _internal(new Internal (p)) {}
      Transferer::~Transferer () {}
      
      class State : public MiniMC::CPA::DataState,
		    private MiniMC::CPA::QueryBuilder
      {
      public:
        State(MiniMC::CPA::Common::StateMixin<MiniMC::VMT::Concrete::Value,MiniMC::VMT::Concrete::Memory>&& internal) : mixin(std::move(internal)) {
        }

	
	State(const State&)  = default;
	
	
        virtual MiniMC::Hash::hash_t hash() const override {
	  return mixin.hash ();
	}
	
        virtual std::shared_ptr<MiniMC::CPA::DataState> copy() const override {
	  return std::make_shared<State>(*this); 
	}

	
	
	auto& getProc(std::size_t i) { return mixin.getProc(i); }
        auto& getHeap() { return mixin.getMemory (); }
	
        auto& getProc(std::size_t i) const { return mixin.getProc (i); }
        auto& getHeap() const { return mixin.getMemory (); }
	
	
        virtual const Solver_ptr getConcretizer() const override { return std::make_shared<MConcretizer> ();}

	//QueryBuilder
	QueryExpr_ptr buildValue (MiniMC::Model::proc_t p, const MiniMC::Model::Value_ptr& val) const override {
	  if (p >= mixin.nbOfProcesses ()) {
	    throw MiniMC::Support::Exception ("Not enough processes");
	  }
	  MiniMC::Model::VariableMap<MiniMC::VMT::Concrete::Value> metas{1};
	  MiniMC::VMT::Concrete::ValueLookup lookup{MiniMC::VMT::Concrete::ValueCreator{},{const_cast<MiniMC::VMT::Concrete::ActivationStack&> (mixin.getProc(p)),metas }};
	  return std::make_unique<QExpr> (lookup.lookupValue(*val));
	    
	}
	
	const QueryBuilder& getBuilder () const override  {return *this;}
	
	const MiniMC::CPA::LocationInfo& getLocationState () const {return mixin;}
	
	
      private:
	MiniMC::CPA::Common::StateMixin<MiniMC::VMT::Concrete::Value,MiniMC::VMT::Concrete::Memory> mixin;
      };

      
      MiniMC::CPA::DataState_ptr CPA::makeInitialState(const InitialiseDescr& descr) {
	MiniMC::VMT::Concrete::Memory mem;
	return std::make_shared<State> (MiniMC::CPA::Common::StateMixin<MiniMC::VMT::Concrete::Value,MiniMC::VMT::Concrete::Memory>::createInitialState<MiniMC::VMT::Concrete::ValueCreator>(descr,MiniMC::VMT::Concrete::ValueCreator{},std::move(mem)));
      }

      MiniMC::CPA::State_ptr<DataState> Transferer::doTransfer(const MiniMC::CPA::DataState& s, const MiniMC::CPA::Transition& t )  {
	const MiniMC::Model::Edge& e = *t.edge;
	proc_id id = t.proc;
	
	auto resstate = s.copy();
        auto& nstate = static_cast<MiniMC::CPA::Concrete::State&>(*resstate);

	if (nstate.getProc(id).back ().getLocation () != e.getFrom ())
	  return nullptr;
	nstate.getProc(id).back().setLocation (e.getTo ());
	
	MiniMC::VMT::Status status  = MiniMC::VMT::Status::Ok;
	  
	MiniMC::VMT::Concrete::PathControl control;
	MiniMC::VMT::Concrete::ValueLookup lookup (MiniMC::VMT::Concrete::ValueCreator{},{nstate.getProc (id),_internal->metas});
	
	
	MiniMC::VMT::Concrete::ConcreteVMState newvm {nstate.getHeap (),control,nstate.getProc(id),lookup};
	auto& instr = e.getInstructions();
	status = _internal->engine.execute(instr,newvm);
	
	if (status == MiniMC::VMT::Status::Ok)
	  return resstate;
	else {
	  
	  return nullptr;

	}
      }

    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC
