#ifndef _CPA_COMMON__
#define _CPA_COMMON__

#include "minimc/support/overload.hpp"
#include "minimc/cpa/query.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/vm/vmt.hpp"
#include "minimc/vm/value.hpp"
#include "minimc/util/valuemap.hpp"
#include "minimc/cpa/state.hpp"
#include "minimc/cpa/interface.hpp"
#include <memory>
#include <ranges>
#include <bitset>
#include <type_traits>

namespace MiniMC {
  namespace CPA {
  namespace Common {
    template<class Value>
    struct ActivationRecord {
      ActivationRecord(std::size_t size,
                       std::size_t meta_size,
                       const MiniMC::Model::Register_ptr& ret,
                       MiniMC::Model::Location_ptr l,
                       MiniMC::Model::Frame frame)
          : values(size), metas(meta_size), ret(ret), loc(l), frame(std::move(frame)) {}

      ActivationRecord(const ActivationRecord&) = default;
      ActivationRecord& operator= (ActivationRecord&&) =default;
      ActivationRecord& operator= (const ActivationRecord&) =default;
      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	hash << values << metas << ret.get() << loc.get();
	return hash;
      }

      auto& getLocation () const {return loc;}
      void setLocation (MiniMC::Model::Location_ptr l)  {loc = l;}
      bool isCPU() const {return (loc == nullptr) ;}
      
      auto returnRegister () const {return ret;}
      auto getValueOfRegister (const MiniMC::Model::Register& r) const {
	if (r.getRegType() == MiniMC::Model::RegType::Meta)
	  return metas[r];
	return values[r];
      }

      auto isActiveRegister (const MiniMC::Model::Symbol& r) const {
	return frame.hasSymbol(r);
      }

      void setValueOfRegister (const MiniMC::Model::Register& r,Value&& v) {
	if (r.getRegType() == MiniMC::Model::RegType::Meta)
	  metas.set(r,std::move(v));
	else
	  values.set(r,std::move(v));
      }
      
      
    private:
      MiniMC::Model::VariableMap<Value> values;
      MiniMC::Model::VariableMap<Value> metas;
      MiniMC::Model::Register_ptr ret{nullptr};
      MiniMC::Model::Location_ptr loc;
      MiniMC::Model::Frame frame;
    };
    
    template <class Value>
    struct ActivationStack {
      ActivationStack(const MiniMC::Model::Frame& frame) {
	frames.push_back(ActivationRecord<Value>{
	    frame.numberOfRegisters(MiniMC::Model::RegType::CPU),
	    frame.numberOfRegisters(MiniMC::Model::RegType::Meta), nullptr, nullptr, frame});
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
      
      void push(MiniMC::Model::Location_ptr loc, const MiniMC::Model::Register_ptr& ret) {
        const auto& frame = loc->getInfo().getFrame();
        frames.push_back(ActivationRecord<Value>{
            frame.numberOfRegisters(MiniMC::Model::RegType::Local),
            frame.numberOfRegisters(MiniMC::Model::RegType::Meta),
            ret, loc, frame});
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
	case MiniMC::Model::RegType::Meta: {
	  auto record = searchForRecordWithSymbol(reg.getSymbol());
	  if (!record)
	    throw MiniMC::Support::Exception("Meta register has no active frame");
	  return record->getValueOfRegister(reg);
	}
	case MiniMC::Model::RegType::Persistent:
	  throw MiniMC::Support::Exception ("Persistent registers cannot be accessed through an activation record");
		  
	default:
	  std::unreachable();
	  
	}
      }
      
      void saveValue(const MiniMC::Model::Register& v, Value&& value)  {
	switch (v.getRegType ()) {
	case MiniMC::Model::RegType::Local: frames.back().setValueOfRegister (v,std::move(value));break;
	case MiniMC::Model::RegType::CPU:   frames.front().setValueOfRegister (v,std::move(value));break;
	case MiniMC::Model::RegType::Meta: {
	  auto record = searchForRecordWithSymbol(v.getSymbol());
	  if (!record)
	    throw MiniMC::Support::Exception("Meta register has no active frame");
	  record->setValueOfRegister(v,std::move(value));
	  break;
	}
	case MiniMC::Model::RegType::Persistent:
	  throw MiniMC::Support::Exception ("Persistent registers cannot be accessed through an activation record");
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

      auto searchForRecordWithSymbol (const MiniMC::Model::Symbol& s) -> ActivationRecord<Value>* {
	for (auto& a : std::ranges::reverse_view (frames)) {
	  if (a.isActiveRegister(s)) {
	    return &a;
	  }
	}
	return nullptr;
      }

      std::vector<ActivationRecord<Value>> frames;
    };

    template <class StackControl,class Operations>
    class DefaultInitializingStackControl {
    public:
      DefaultInitializingStackControl ( StackControl& s, Operations op) : sc(s),ops(op) {}
      auto pop() ->MiniMC::Model::Register_ptr  {
	return sc.pop();
      }
      
      void push(MiniMC::Model::Location_ptr loc, const MiniMC::Model::Register_ptr& ret) {
        sc.push(loc, ret);
	const auto& frame = loc->getInfo().getFrame();
        for (auto[_,reg] : frame.local_registers()) {
	  sc.saveValue(*reg, ops.defaultValue(*reg->getType()));
	    
	}
      }
      
    private:
      StackControl& sc;
      Operations ops;
    };
      
    
    template<class Value>
    class RegisterAccessView {
    public:
      RegisterAccessView (ActivationStack<Value>& values, MiniMC::Model::VariableMap<Value>& persistent)
          : values(&values), persistent(persistent) {}
      explicit RegisterAccessView (MiniMC::Model::VariableMap<Value>& persistent)
          : values(nullptr), persistent(persistent) {}

      Value lookupRegister (const MiniMC::Model::Register& reg) const {
	if (reg.getRegType () == MiniMC::Model::RegType::Persistent)
	  return persistent[reg];
	if (values)
	  return values->lookupRegister(reg);
	throw MiniMC::Support::Exception ("No registers to load fraom");
      }

      void saveValue (const MiniMC::Model::Register& reg, Value&& value) {
	if (reg.getRegType () == MiniMC::Model::RegType::Persistent)
	  return persistent.set(reg,std::move(value));
	if (values)
	  return values->saveValue(reg,std::move(value));
	throw MiniMC::Support::Exception ("No registers to save to");
      }

    private:
      ActivationStack<Value>* values;
      MiniMC::Model::VariableMap<Value>& persistent;
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
      EvaluationContext (ActivationStack<Value>& values,
                         MiniMC::Model::VariableMap<Value>& persistent,
                         MemControl memorycontrol,
                         StaticContext<Value>& scontext)
          : values(values), registers(values, persistent), memcontrol(memorycontrol), scontext(scontext) {}
    public:
      Value lookupRegister (const MiniMC::Model::Register& reg) const  {
	return registers.lookupRegister(reg);
      }

      Value lookupSymbol (MiniMC::Model::Symbol s) const {

	auto val = std::visit (MiniMC::Support::Overload {
	    [this,&s](MiniMC::Model::Register_wptr& r) ->std::optional<Value>{
	      auto record = values.searchForRecordWithSymbol (s);
	      if (record)
		return record->getValueOfRegister(*r.lock());
	      return std::nullopt;
	    },
	      [&s,this](auto& )->std::optional<Value> {
		if (scontext.hasSymbol (s))
		  return scontext.at(s);
		return std::nullopt;
	      }
	      },
	  s.getUserData()
	  );
	
	if (val)
	  return val.value();
	MiniMC::Support::Localiser loc {"Cannot localise symbol %1%"};
	throw MiniMC::Support::Exception (loc.format(s));
      }
      
      void saveValue(const MiniMC::Model::Register& reg, Value&& value)  {
	registers.saveValue(reg,std::move(value));
      }

      Value load (const Value::Pointer p, const Value::Memory& m, const MiniMC::Model::Type& t) const {
	return memcontrol.load (m,p,t);
      }

      
      Value find_space (const Value::Memory m, const Value::I64& s) const {
	return memcontrol.find_space (m,s);
      }

      Value check_free (const Value::Memory& m, const Value::Pointer& p, const Value::I64& s) const {
	return memcontrol.checkFree(m,p,s);
      }

      Value valid_pointer (const Value::Memory& m, const Value::Pointer& p) const {
	return memcontrol.valid_pointer(m,p);
      }
      
      
      Value allocate (const Value::Memory m, Value::Pointer ptr, const Value::I64& s) const {
	return memcontrol.allocate (m,ptr,s);
      }

      Value free (const Value::Memory m, Value::Pointer ptr) const {
	return memcontrol.free (m,ptr);
      }
      
      std::generator<typename Value::I8> loadBytes (const Value::Pointer p, const Value::Memory& m , std::size_t s) const {
	co_yield std::ranges::elements_of(memcontrol.loadBytes (m,p,s)); 
      }
      
      Value::Memory store (const Value::Memory& m,const Value::Pointer p,  const Value& t) const {
	return Value::visit (
			     MiniMC::Support::Overload {
			       [&m,&p,this] (const Value::I8& v) {
				 return memcontrol.store (m,p,v); 
			       },
				 MiniMC::Support::Error<typename Value::Memory>{}
			       },
			     t);
      }
      
    private:
      ActivationStack<Value>& values;
      RegisterAccessView<Value> registers;
      MemControl memcontrol;
      StaticContext<Value>& scontext;
    };

    template<class Value>
    class RegisterSymbolView {
    public:  
      RegisterSymbolView (StaticContext<Value>& scontext, MiniMC::Model::VariableMap<Value>& persistent)
          : scontext(scontext), registers(persistent) {}

      Value lookupRegister (const MiniMC::Model::Register& r) const  {
	return registers.lookupRegister(r);
      }

      
      void saveValue(const MiniMC::Model::Register& r, Value&& v)  {
	registers.saveValue(r,std::move(v));
      }

      Value load (const Value::Pointer, const Value::Memory&, const MiniMC::Model::Type&) const {
	throw MiniMC::Support::Exception {"Not implemented"};
      }

      std::generator<typename Value::I8> loadBytes (const Value::Pointer, const Value::Memory&, std::size_t) const {
	throw MiniMC::Support::Exception {"Not implemented"};
      }
      
      
      Value lookupSymbol (MiniMC::Model::Symbol s) const {
	return scontext.at(s);
      }
      
      Value::Memory store (const Value::Memory& ,const Value::Pointer ,  const Value&) const {
	throw MiniMC::Support::Exception {"Not implemented"};
	
      }

      Value find_space (const Value::Memory&, const Value::I64&) const {
	throw MiniMC::Support::Exception {"Not implemented"};
      }

      Value check_free (const Value::Memory&, const Value::Pointer&, const Value::I64&) const {
	throw MiniMC::Support::Exception {"Not implemented"};
      }

      Value valid_pointer (const Value::Memory&, const Value::Pointer&) const {
	throw MiniMC::Support::Exception {"Not implemented"};
      }
      
      
      Value allocate (const Value::Memory&, const Value::Pointer&, const Value::I64&) const {
	throw MiniMC::Support::Exception {"Not implemented"};
      }

      Value free (const Value::Memory&, const Value::Pointer&) const {
	throw MiniMC::Support::Exception {"Not implemented"};
      }
      
      
    private:
      StaticContext<Value>& scontext;
      RegisterAccessView<Value> registers;
      
    };
    
   
    class Flags {
    public:
      Flags () {}
      
      Flags(const Flags& ) = default;
      Flags( Flags&& ) = default;
      
      auto& operator|= (MiniMC::VMT::FlagType flag) {
	sets.set(static_cast<int> (flag));
	return *this;
      }
      
      auto& unset (MiniMC::VMT::FlagType flag) {
	sets.reset(static_cast<int> (flag));
	return *this;
      }
      
      
      
      auto operator& (MiniMC::VMT::FlagType flag) const {
	return sets.test (static_cast<int> (flag));
      }

      auto hash () const  {
	return std::hash<std::bitset<1>>{}(sets);
      }
      
    private:
      std::bitset<1> sets;
    };




    template<class Value>
    class StateMixin : public MiniMC::CPA::LocationInfo   {
    public:
      StateMixin (std::vector<ActivationStack<Value>>&& stacks,
		  MiniMC::Model::VariableMap<Value>&& persistent,
		  Value::Bool pathformula,
		  std::shared_ptr<StaticContext<Value> >&& scontext = nullptr
		  ) : stacks(std::move(stacks)),
		      persistent(std::move(persistent)),
		      scontext(std::move(scontext)),
		      pathform(pathformula)
      {}

      StateMixin (StateMixin&&) = default;
      StateMixin (const StateMixin&) = default;
      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	for (auto& vl : stacks) {
	  hash << vl;
	}
	hash << persistent;
	hash << getPathform ();
	hash << flags;
	return hash;
      }

      auto& getProc(std::size_t i) { return stacks.at(i); }
      
      auto& getProc(std::size_t i) const { return stacks.at(i); }
      
      //LocationInfo
      size_t nbOfProcesses() const override {return stacks.size();}
      bool isActive(size_t id) const override {return !getProc(id).activeRecord().isCPU()
	  && getLocation(id).hasOutgoingEdge()
	  ;}
      MiniMC::Model::Location& getLocation(proc_id id) const override   {return *getProc(id).activeRecord().getLocation();}

      template<MiniMC::VMT::MemoryOperations<Value> MemControl>
      auto makeEvaluationContext (proc_id id,MemControl&& memcontrol) const {
	return EvaluationContext<Value,MemControl> (const_cast<ActivationStack<Value>&>(getProc(id)),const_cast<MiniMC::Model::VariableMap<Value>&>(persistent),std::move(memcontrol),*scontext);
      }
      
      
      Value::Bool getPathform () const { return pathform;}
      void setPathform (Value::Bool&& p ) { pathform = std::move(p);}
      void setFlag (MiniMC::VMT::FlagType flag)  {flags |= flag; }
      bool isSet (MiniMC::VMT::FlagType flag) const   {return flags & flag; }
      
    private:
      std::vector<ActivationStack<Value> > stacks;
      MiniMC::Model::VariableMap<Value> persistent;
      std::shared_ptr<StaticContext<Value> > scontext;
      Value::Bool pathform;
      Flags flags;
    };

      
        
    template<class Value,MiniMC::VMT::ConstraintSolver<Value> Constraintsolver>
    class Solver : public MiniMC::CPA::Solver {
    public:
      Solver (Constraintsolver&& solver) : solver(std::move(solver)) {
	solver.push ();
      }

      ~Solver ()  {
	solver.pop ();
      }
      
      MiniMC::CPA::Solver::Feasibility isFeasible() const override {
	switch (solver.check ()) {
	case MiniMC::VMT::Feasibility::Feasible: return Feasibility::Feasible;
	case MiniMC::VMT::Feasibility::Infeasible: return Feasibility::Infeasible;

	case MiniMC::VMT::Feasibility::Unknown:
	default:
	  return Feasibility::Unknown;
	  
	}
      }

      void addConstraint (Value::Bool b) {
	solver.addConstraint (b);
      }
      
      MiniMC::Model::Constant_ptr evaluate (const QueryExpr& expr) const override {
	if (solver.check() == MiniMC::VMT::Feasibility::Feasible) {
	  auto& ref = static_cast<const TQuery<Value>&> (expr);
	  return solver.eval (ref.getValue());
	}
	else
	  throw MiniMC::Support::Exception("Cannot evaluate on infeasible states");
      }
	
    private:
      Constraintsolver solver;
    };

    template<MiniMC::VMT::ValueDefinition ValDef>
    class CPAState : public State,
                     private QueryBuilder
    {
    public:
      CPAState (StateMixin<typename ValDef::Val>&& m,ValDef valdef) : mixin(std::move(m)),valuedefinition(std::move(valdef)) {}
      CPAState (const CPAState&) = default;
      
      virtual MiniMC::Hash::hash_t hash() const override {
	return mixin.hash ();
      }
      
      virtual std::shared_ptr<CPAState<ValDef>>  lcopy() const  {
	return makeState<CPAState<ValDef>>(*this); 
      }

      virtual State_ptr copy() const override {
	return makeState<CPAState<ValDef>>(*this); 
      }
      
      auto& getProc(std::size_t i) { return mixin.getProc(i); }
      auto& getProc(std::size_t i) const { return mixin.getProc (i); }
      
      auto makeEvaluationContext (proc_id id) const {return mixin.makeEvaluationContext(id,valuedefinition.memops());}
      auto getStackControl (proc_id id)  {return DefaultInitializingStackControl (mixin.getProc(id),valuedefinition.ops());}
      auto getValueLookup (proc_id id)  {return mixin.getProc(id);}
      
      
      //QueryBuilder
      QueryExpr_ptr buildValue (MiniMC::Model::proc_t p, const MiniMC::Model::Value& val) const override {
	if (p >= mixin.nbOfProcesses ()) {
	  throw MiniMC::Support::Exception ("Not enough processes");
	}
	MiniMC::VMT::MultiEvaluator<ValDef,decltype(this->makeEvaluationContext(1))> eval (
	    valuedefinition,makeEvaluationContext(p));
	return std::make_unique<TQuery<typename ValDef::Val>> (eval.Eval(val));
	
      }
      
      const QueryBuilder& getBuilder () const override  {return *this;}
      
      const MiniMC::CPA::LocationInfo& getLocationState () const {return mixin;}
      
      virtual const Solver_ptr getConcretizer() const override {
	auto solver = valuedefinition.solver();
	auto ssolver = std::make_unique<Solver<typename ValDef::Val,decltype(valuedefinition.solver())>> (std::move(solver));
	ssolver->addConstraint (getPathform());
	return std::move(ssolver);
      }

      virtual const Solver_ptr getConcretizer(MiniMC::VMT::SolverOptions sopts) const override {
	auto solver = valuedefinition.solver(sopts);
	auto ssolver = std::make_unique<Solver<typename ValDef::Val,decltype(valuedefinition.solver(sopts))>> (std::move(solver));
	ssolver->addConstraint (getPathform());
	return std::move(ssolver);
      }
      
      
      auto constraint_solver() const {
	return valuedefinition.solver();
      }
      
      ValDef::Val::Bool getPathform () const { return mixin.getPathform();}
      void setPathform (ValDef::Val::Bool&& p ) { mixin.setPathform(std::move(p));}
      void setFlag (MiniMC::VMT::FlagType t) override  {mixin.setFlag(t);}
      bool isSet (MiniMC::VMT::FlagType flag) const override  {return mixin.isSet(flag); }
      
    private:
      StateMixin<typename ValDef::Val> mixin;
      ValDef valuedefinition;
    };


    template <MiniMC::VMT::ValueDefinition ValDef>
    class StateBuilder : public MiniMC::CPA::StateBuilder{
      using Value = ValDef::Val;
    public:
      StateBuilder(ValDef d) : valdef(d) {
	path = valdef.ops().create(MiniMC::Model::Bool(true));
        }
      MiniMC::CPA::State_ptr build() override{
        return makeState<CPAState<ValDef>> (StateMixin<typename ValDef::Val> {std::move(stack),std::move(persistent),path,std::move(_scontext)},valdef);
	
      }

      MiniMC::CPA::StateBuilder& addPersistentRegisters(const MiniMC::Model::Frame& frame) override {
        persistent = MiniMC::Model::VariableMap<typename ValDef::Val>{
	    frame.numberOfRegisters(MiniMC::Model::RegType::Persistent)};
	for (auto [_, reg] : frame.local_registers(MiniMC::Model::RegType::Persistent)) {
	  persistent.set(*reg,valdef.ops().defaultValue (*reg->getType()));
	}
	return *this;
      }

      MiniMC::CPA::StateBuilder& addHeapBlock(const MiniMC::Model::HeapBlock& block) override {
	RegisterSymbolView<Value> regstore{*_scontext,persistent};
	auto eval = MiniMC::VMT::makeEvaluator (valdef,regstore);
	auto ptr = eval.Eval (*MiniMC::Model::Pointer::make (block.baseobj));
	auto size = eval.Eval (*MiniMC::Model::I64Integer::make (block.size));
        Value::visit(
            MiniMC::Support::Overload{
                [this, &block, &regstore](const Value::Pointer& ptr, const Value::I64& size, const Value::Memory& mem) {
                  auto mem2 = valdef.memops().allocate(mem, ptr, size);
                  _scontext->addSymbol(block.symbol, ptr);
                  regstore.saveValue(*block.heap_register, Value{mem2});
                },
                MiniMC::Support::Error<void>{}},
            ptr, size, eval.Eval(*block.heap_register)
		      );
	
	if (block.value) {
	  Value ptr = eval.Eval (*MiniMC::Model::Pointer::make (block.baseobj));
	  Value valueToStor = eval.Eval(*block.value);
	  
	  Value::visit (MiniMC::Support::Overload {
	      [this,&block,&regstore]<typename K>(const Value::Pointer& ptr, const K& value, const Value::Memory& mem) requires (!std::is_same_v<K,typename Value::Bool> && !std::is_same_v<K,typename Value::Memory>) {
		auto ones = valdef.ops().create (MiniMC::Model::I64Integer{1});
		auto ptr_c = ptr;
		for (auto by : valdef.ops().bytes(value)) {
		  auto mem2 = valdef.memops().store (mem,ptr_c,by);
		  ptr_c = valdef.ops().PtrAdd (ptr_c,ones);
		  regstore.saveValue (*block.heap_register,Value{mem2});
		}
		
	      },
		[](const auto&, const auto&,const auto& ) {
		  throw MiniMC::Support::Exception ("Error");
		},
		
		
		},
	    ptr,
	    valueToStor,
	    eval.Eval(*block.heap_register)
	    
	    );
        }
	return *this;
      }
      
      MiniMC::CPA::StateBuilder& addThread(const MiniMC::Model::Function& f, const MiniMC::Model::Frame& frame, std::vector<MiniMC::Model::Value_ptr> params  = {}) override{
	auto memops = valdef.memops();
	auto ops = valdef.ops();
	
        ActivationStack<Value> cs{frame};
	DefaultInitializingStackControl{cs, ops}.push (f.getCFA().getInitialLocation(),nullptr);
        

        EvaluationContext<Value,decltype(memops)> regstore {cs,persistent,valdef.memops(),*_scontext};
	for (auto [_, reg] : frame.local_registers(MiniMC::Model::RegType::CPU)) {
	  auto val = ops.defaultValue (*reg->getType ());
	  regstore.saveValue (*reg,std::move(val));
        }
	for (auto [_, reg] : frame.local_registers(MiniMC::Model::RegType::Meta)) {
	  auto val = ops.defaultValue (*reg->getType ());
	  regstore.saveValue (*reg,std::move(val));
        }

        
	auto eval = MiniMC::VMT::makeEvaluator (valdef,regstore);
        for (auto [formal, act] : std::views::zip(f.getParameters(), params)) {
	  auto reg =  std::get<MiniMC::Model::Register_ptr> (formal.getUserData());            
	  regstore.saveValue  (*reg,eval.Eval (*act));
	}          
        stack.push_back(cs);
	return *this;
      }

      MiniMC::CPA::StateBuilder& addConstraint(MiniMC::Model::Value& b) {
	RegisterSymbolView<Value> regstore{*_scontext,persistent};
        auto eval = MiniMC::VMT::makeEvaluator(valdef, regstore);
	auto converted = eval.Eval (b);
        Value::visit(
            MiniMC::Support::Overload{
	      [this](const Value::Bool& v) { path = valdef.ops().BoolAnd(path, v); },
		MiniMC::Support::Error<void>{}
	    },
	    converted
		     );                
		                 
        return *this;        
      }        
      
    private:
      ValDef valdef;      
      std::vector<ActivationStack<Value>> stack;
      MiniMC::Model::VariableMap<Value> persistent{0};
      std::shared_ptr<MiniMC::CPA::Common::StaticContext<Value>> _scontext = std::make_shared<MiniMC::CPA::Common::StaticContext<Value>>();
      Value::Bool path;
    };
    
    template<MiniMC::VMT::ValueDefinition ValDef>
    class Transferer : public MiniMC::CPA::Transfer {
    public:
      Transferer(ValDef def,const MiniMC::Model::Program& prgm) : def(std::move(def)),engine(this->def,prgm) {}
      std::generator<State_ptr>  doTransfer(const MiniMC::CPA::State& s, const MiniMC::CPA::Transition& t )  {
	
	const MiniMC::Model::Edge& e = *t.edge;
	proc_id id = t.proc;
	
	auto resstate = s.copy();
        auto& nstate = static_cast<CPAState<ValDef>&>(*resstate);
	
	if (nstate.getProc(id).activeRecord ().getLocation () == e.getFrom ()) {
	  nstate.getProc(id).activeRecord().setLocation (e.getTo ());
	  

	  auto& instr = e.getInstructions();
	  for (auto t :  engine.execute(instr,nstate,id)) {
	    if (e.getTo ()->getInfo().getFlags().isSet (MiniMC::Model::Attributes::AssertViolated))
	      t->setFlag (MiniMC::VMT::FlagType::AssertViolated);
	    co_yield t;
	  }
	  
	}
      }
    private:
      ValDef def; 
      MiniMC::VMT::Engine<ValDef> engine;
    };


    template<VMT::ValueDefinition ValDef>
    struct CPA : public ICPA {
    public:
      template<class...Args>
      CPA (Args... args) : valdef(args...) {}
      State_ptr makeInitialState(const InitialiseDescr& descr) override {
	StateBuilder<ValDef> builder{valdef};
        builder.addPersistentRegisters(descr.getProgram().getRootFrame());
	
        for (auto& block : descr.getHeap().blocks()) {
	  builder.addHeapBlock(block);
	}
	
	for (auto& f : descr.getEntries()) {
          builder.addThread(*f.getFunction(),
                            descr.getProgram().getRootFrame(),
			    f.getParams());
	  
	}          
        
	return builder.build ();
      }

      
      
      virtual Transferer_ptr makeTransfer(const MiniMC::Model::Program& prgm ) const {return std::make_shared<Transferer<ValDef>> (valdef,prgm);}
    private:
      ValDef valdef;
    };
    
  }
  }
}

#endif
