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
	Internal (const MiniMC::Model::Program& prgm) : engine(MiniMC::VMT::Concrete::ConcreteEngine::OperationsT{},
							       prgm),
							metas(prgm.getMetaRegs().getTotalRegisters())
	{}
	MiniMC::VMT::Concrete::ConcreteEngine engine;
	MiniMC::Model::VariableMap<MiniMC::VMT::Concrete::Value> metas;
	
      };
      
      Transferer::Transferer (const MiniMC::Model::Program& p) : _internal(new Internal (p)) {}
      Transferer::~Transferer () {}
      
      class State : public MiniMC::CPA::DataState,
		    private MiniMC::CPA::QueryBuilder,
		    private MiniMC::CPA::LocationInfo
      {
      public:
        State( const std::vector<MiniMC::VMT::Concrete::ActivationStack>& var, MiniMC::VMT::Concrete::Memory& mem) :  proc_vars(var),heap(mem) {
        }

	
        virtual MiniMC::Hash::hash_t hash() const override {
	  MiniMC::Hash::Hasher hash;
	  for (auto& vl : proc_vars) {
	    hash << vl;
	  }
	  hash << heap;
	  return hash;
	}
	
        virtual std::shared_ptr<MiniMC::CPA::DataState> copy() const override {

	  std::vector<MiniMC::VMT::Concrete::ActivationStack> proc_vars2{proc_vars};
	  MiniMC::VMT::Concrete::Memory heap2(heap);
	  return std::make_shared<State>(proc_vars2,heap2); 
	}

	
	
        auto& getProc(std::size_t i) { return proc_vars.at(i); }
        auto& getHeap() { return heap; }
	
        auto& getProc(std::size_t i) const { return proc_vars.at(i); }
        auto& getHeap() const { return heap; }

        virtual const Solver_ptr getConcretizer() const override { return std::make_shared<MConcretizer> ();}

	//QueryBuilder
	QueryExpr_ptr buildValue (MiniMC::Model::proc_t p, const MiniMC::Model::Value_ptr& val) const override {
	  if (p >= proc_vars.size ()) {
	    throw MiniMC::Support::Exception ("Not enough processes");
	  }
	  MiniMC::Model::VariableMap<MiniMC::VMT::Concrete::Value> metas{1};
	  MiniMC::VMT::Concrete::ValueLookup lookup{MiniMC::VMT::Concrete::ValueCreator{},{const_cast<MiniMC::VMT::Concrete::ActivationStack&> (proc_vars.at(p)),metas }};
	  return std::make_unique<QExpr> (lookup.lookupValue(*val));
	  
	}
	
	const QueryBuilder& getBuilder () const override  {return *this;}
	
	MiniMC::Model::Location& getLocation(proc_id id) const override   {return *getProc(id).back().getLocation();}
	size_t nbOfProcesses() const override {return proc_vars.size();}
	bool isActive(size_t id) const override {return getProc(id).getDepth();}
	const MiniMC::CPA::LocationInfo& getLocationState () const {return *this;}

	
      private:
        std::vector<MiniMC::VMT::Concrete::ActivationStack> proc_vars;
	MiniMC::VMT::Concrete::Memory heap;
      };

      
      MiniMC::CPA::DataState_ptr CPA::makeInitialState(const InitialiseDescr& descr) {
        std::vector<MiniMC::VMT::Concrete::ActivationStack> stack;
	for (auto& f : descr.getEntries()) {
          auto& vstack = f.getFunction()->getRegisterDescr();
	  MiniMC::Model::VariableMap<MiniMC::VMT::Concrete::Value> gvalues {descr.getProgram().getCPURegs().getTotalRegisters ()};
	  MiniMC::Model::VariableMap<MiniMC::VMT::Concrete::Value> values{vstack.getTotalRegisters ()};
	  MiniMC::VMT::Concrete::ActivationRecord sf {std::move(values),nullptr,f.getFunction()->getCFA().getInitialLocation ()};
	  MiniMC::VMT::Concrete::ActivationStack cs {std::move(gvalues),std::move(sf)};
	  MiniMC::Model::VariableMap<MiniMC::VMT::Concrete::Value> metas{1};
	  MiniMC::VMT::Concrete::ValueLookup lookup {MiniMC::VMT::Concrete::ValueCreator{},{cs,metas}};
	  for (auto& v : vstack.getRegisters()) {
            lookup.saveValue  (*v,lookup.defaultValue (*v->getType ()));
	  }

	  for (auto& reg : descr.getProgram().getCPURegs().getRegisters()) {
	    auto val = lookup.defaultValue (*reg->getType ());
	    lookup.saveValue (*reg,std::move(val));
	  }
	  
	  
	  auto pit = f.getParams ().begin ();
	  auto rit = f.getFunction()->getParameters().begin ();
	  for (; pit != f.getParams ().end ();++pit,++rit) {
	    lookup.saveValue  (**rit,lookup.lookupValue (**pit));
	  } 
	  
          stack.push_back(cs);
	  
        }
	MiniMC::VMT::Concrete::ValueLookupNoRegister lookup{MiniMC::VMT::Concrete::ValueCreator{}};
	MiniMC::VMT::Concrete::Memory heap;
	heap.createHeapLayout (descr.getHeap ());
	
	for (auto& b : descr.getHeap ()) {
	  if (b.value) {
	    VMT::Concrete::Value ptr = lookup.lookupValue (MiniMC::Model::Pointer (b.baseobj));
            VMT::Concrete::Value valueToStor = lookup.lookupValue(*b.value);
	    VMT::Concrete::Value::visit (MiniMC::Support::Overload {
		
		[&heap]<typename K>(VMT::Concrete::Value::Pointer& ptr, K& value) requires (!std::is_same_v<K,VMT::Concrete::Value::Bool>) {
		  heap.store (ptr,value);
		},
		  [](auto&, auto&) {
		    throw MiniMC::Support::Exception ("Error");
		  },
		  
		  
		  },
	      ptr,
	      valueToStor
	      );
	    }
	}
	
	auto state = std::make_shared<State>(stack,heap);

	
        return state;
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
	MiniMC::VMT::Concrete::StackControl scontrol {nstate.getProc (id)};
	MiniMC::VMT::Concrete::ValueLookup lookup (MiniMC::VMT::Concrete::ValueCreator{},{nstate.getProc (id),_internal->metas});
	
	
	MiniMC::VMT::Concrete::ConcreteVMState newvm {nstate.getHeap (),control,scontrol,lookup};
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
