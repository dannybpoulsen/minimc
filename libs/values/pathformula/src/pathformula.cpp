#include "minimc/model/valuevisitor.hpp"
#include "minimc/vm/value.hpp"
#include "smt/builder.hpp"
#include "minimc/smt/smtconstruction.hpp"
#include "minimc/values/pathformula/pathformula.hpp"


#include "smt/solver.hpp"
#include "smt/context.hpp"

#include "minimc/smt/smt.hpp"
#include <sstream>
namespace MiniMC {
  namespace VMT {
    namespace Pathformula {
      static std::size_t next = 0;
      Value Operations::defaultValue(const MiniMC::Model::Type& t) const {
	return unboundValue (t);
      }
	
      Value Operations::unboundValue(const MiniMC::Model::Type& t) const {
        std::stringstream str;
        str << "Var" << ++next;
        switch (t.getTypeID()) {
          case MiniMC::Model::TypeID::Bool:
            return BoolValue(builder.makeVar(builder.makeSort(SMTLib::SortKind::Bool, {}), str.str()));
	case MiniMC::Model::TypeID::Pointer32:
	  return Pointer32Value(builder.makeVar(builder.makeBVSort(32), str.str()));
          
	  
          case MiniMC::Model::TypeID::Pointer:
            return PointerValue(builder.makeVar(builder.makeBVSort(64), str.str()));
          case MiniMC::Model::TypeID::I8:
            return I8Value(builder.makeVar(builder.makeBVSort(8), str.str()));
          case MiniMC::Model::TypeID::I16:
            return I16Value(builder.makeVar(builder.makeBVSort(16), str.str()));
	  case MiniMC::Model::TypeID::I32:
            return I32Value(builder.makeVar(builder.makeBVSort(32), str.str()));
          case MiniMC::Model::TypeID::I64:
            return I64Value(builder.makeVar(builder.makeBVSort(64), str.str()));
	  case MiniMC::Model::TypeID::Aggregate:
            return AggregateValue(builder.makeVar(builder.makeBVSort(8 * t.getSize()), str.str()), t.getSize());
	  case MiniMC::Model::TypeID::Memory:
            return MemoryValue::construct_empty_memory(builder);
	    
	  default:
            break;
        }
        throw MiniMC::Support::Exception("Erro");
      }
      
      
      
      Value::I8 Operations::create(const MiniMC::Model::I8Integer& val) const { return I8Value(builder.makeBVIntConst(val.getValue(), 8)); }
      Value::I16 Operations::create(const MiniMC::Model::I16Integer& val) const { return I16Value(builder.makeBVIntConst(val.getValue(), 16)); }
      Value::I32 Operations::create(const MiniMC::Model::I32Integer& val) const { return I32Value(builder.makeBVIntConst(val.getValue(), 32)); }
      Value::I64 Operations::create(const MiniMC::Model::I64Integer& val) const {  return I64Value(builder.makeBVIntConst(val.getValue(), 64)); }
      Value::Bool Operations::create(const MiniMC::Model::Bool& val) const { return BoolValue(builder.makeBoolConst(val.getValue())); }
      Value::Pointer Operations::create(const MiniMC::Model::Pointer& val) const {
	auto pointer = val.getValue ();
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	chainer << builder.makeBVIntConst(pointer.segment, sizeof(pointer.segment)*8)
		<< builder.makeBVIntConst(pointer.base, sizeof(pointer.base)*8)
		<< builder.makeBVIntConst(pointer.offset, sizeof(pointer.offset)*8);
	return PointerValue(chainer.getTerm ());
	} 

      Value::Pointer32 Operations::create(const MiniMC::Model::Pointer32& val) const { 
	auto pointer = val.getValue ();
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	chainer << builder.makeBVIntConst(pointer.segment, sizeof(pointer.segment)*8)
		  << builder.makeBVIntConst(pointer.base, sizeof(pointer.base)*8)
		  << builder.makeBVIntConst(pointer.offset, sizeof(pointer.offset)*8);
	  return Value::Pointer32(chainer.getTerm ());
	}
      Value Operations::create(const MiniMC::Model::AggregateConstant& val) const {
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	for (auto byte : val.getData().get_direct_access()) {
	  chainer >> (builder.makeBVIntConst(byte, 8));
	}
	return AggregateValue(chainer.getTerm(), val.getSize());
      }
      std::generator<Value> Operations::create(const MiniMC::Model::Undef& val) const {
	co_yield unboundValue(*val.getType());
      }

      std::generator<Value::I8> Memory::loadBytes(const MemoryValue& mem, const typename Value::Pointer& startAddr, std::size_t bytes) const  {
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> concat(builder);
	for (size_t i = 0; i < bytes; ++i) {
	  auto ones = builder->makeBVIntConst(i, Value::Pointer::intbitsize());
	  auto curind = builder->buildTerm(SMTLib::Ops::BVAdd, {startAddr.getTerm (), ones});
	  co_yield Value::I8{builder->buildTerm(SMTLib::Ops::Select, {mem.getMemVar (), curind})};
	  
	}
      }
      
      
      Value Memory::load(const MemoryValue& mem, const typename Value::Pointer& startAddr, const MiniMC::Model::Type& t) const {
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> concat(builder);
	for (size_t i = 0; i < t.getSize (); ++i) {
	  auto ones = builder->makeBVIntConst(i, Value::Pointer::intbitsize());
	  auto curind = builder->buildTerm(SMTLib::Ops::BVAdd, {startAddr.getTerm (), ones});
	  if (t.getTypeID () == MiniMC::Model::TypeID::Aggregate)
	    concat >> builder->buildTerm(SMTLib::Ops::Select, {mem.getMemVar (), curind});
	  else {
	    concat >> builder->buildTerm(SMTLib::Ops::Select, {mem.getMemVar (), curind});
	  }
	}
	switch (t.getTypeID ()) {
	case MiniMC::Model::TypeID::Bool:
	  return Value::Bool{concat.getTerm()};
	case MiniMC::Model::TypeID::I8:
	  return Value::I8{concat.getTerm()};
	case MiniMC::Model::TypeID::I16:
	  return Value::I16{concat.getTerm()};
	case MiniMC::Model::TypeID::I32:
	  return Value::I32{concat.getTerm()};
	case MiniMC::Model::TypeID::I64:
	  return Value::I64{concat.getTerm()};
	case MiniMC::Model::TypeID::Pointer:
	  return Value::Pointer{concat.getTerm()};
	case MiniMC::Model::TypeID::Pointer32:
	  return Value::Pointer32{concat.getTerm()};
	case MiniMC::Model::TypeID::Aggregate:
	  return Value::Aggregate{concat.getTerm(),t.getSize ()};
	case MiniMC::Model::TypeID::Void:
	default:
	  throw MiniMC::Support::Exception ("Float and DOuble unsupported");
	}
	
      }

      

      MemoryValue  Memory::allocate(const MemoryValue& mem, const Value::Pointer& , const Value::I64&) {
	return MemoryValue (mem.getNextBlock()+1,mem.getMemVar());
      }
      
      Value::Pointer  Memory::find_space(const MemoryValue& mem, const Value::I64&) {
	MiniMC::Util::PointerHelper helper {builder};
	return helper.makeHeapPointer (mem.getNextBlock(),0);
      }
	

      MemoryValue MemoryValue::construct_empty_memory (SMTLib::TermBuilder& builder) {
	auto arr_sort = builder.makeSort(
					  SMTLib::SortKind::Array, {builder.makeBVSort(Value::Pointer::intbitsize()),
								   builder.makeBVSort(8)});
	auto mem_var = builder.makeVar(arr_sort, "Mem_mm");
	return MemoryValue (0,mem_var);
      }
      
      Memory::Memory (SMTLib::TermBuilder& b) : builder(&b) {
	/*auto arr_sort = builder->makeSort(
	  SMTLib::SortKind::Array, {builder->makeBVSort(Value::Pointer::intbitsize()),
	  builder->makeBVSort(8)});
	  auto mem_var = builder->makeVar(arr_sort, "Mem");
	  mem = MemoryValue (0,mem_var);*/
      }

      template<std::size_t PtrWidth>
      SMTLib::Term_ptr write(size_t bytes, SMTLib::TermBuilder& t, const SMTLib::Term_ptr& arr, const SMTLib::Term_ptr& startInd, const SMTLib::Term_ptr& content) {
	auto carr = arr;
        for (size_t i = 0; i < bytes; ++i) {
          auto ones = t.makeBVIntConst(bytes - 1 - i,  PtrWidth);
          auto curind = t.buildTerm(SMTLib::Ops::BVAdd, {startInd, ones});
          auto curbyte = t.buildTerm(SMTLib::Ops::Extract, {content}, {i * 8 + 7, i * 8});
	  carr = t.buildTerm(SMTLib::Ops::Store, {carr, curind, curbyte});
        }
	assert(carr);
        return carr;
      }

      template<std::size_t PtrWidth>
      SMTLib::Term_ptr writeAggr(size_t bytes, SMTLib::TermBuilder& t, const SMTLib::Term_ptr& arr, const SMTLib::Term_ptr& startInd, const SMTLib::Term_ptr& content) {
	auto carr = arr;
        for (size_t i = 0; i < bytes; ++i) {
          auto ones = t.makeBVIntConst(i,  PtrWidth);
          auto curind = t.buildTerm(SMTLib::Ops::BVAdd, {startInd, ones});
          auto curbyte = t.buildTerm(SMTLib::Ops::Extract, {content}, {i * 8 + 7, i * 8});
	  carr = t.buildTerm(SMTLib::Ops::Store, {carr, curind, curbyte});
        }
	assert(carr);
        return carr;
      }

      
      MemoryValue Memory::store(const MemoryValue& mem, const Value::Pointer& ptr, const Value::I8& val) const  {
	auto old_mem_var = mem.getMemVar ();
	auto mem_var = write<Value::Pointer::intbitsize()> (val.size(),*builder,old_mem_var,ptr.getTerm (),val.getTerm ());
	return 	MemoryValue(mem.getNextBlock(),mem_var);
      
      }

      MemoryValue  Memory::store(const MemoryValue& mem,const Value::Pointer& ptr, const Value::I16& val) const{
	auto old_mem_var = mem.getMemVar ();
	auto mem_var = write<Value::Pointer::intbitsize()> (val.size(),*builder,old_mem_var,ptr.getTerm (),val.getTerm ());
	return MemoryValue(mem.getNextBlock(),mem_var);
      }
      
      MemoryValue Memory::store(const MemoryValue& mem,const Value::Pointer& ptr, const Value::I32& val)  const{
	auto old_mem_var = mem.getMemVar ();
	auto mem_var = write<Value::Pointer::intbitsize()> (val.size(),*builder,old_mem_var,ptr.getTerm (),val.getTerm ());
	return MemoryValue(mem.getNextBlock(),mem_var);
      }

      MemoryValue Memory::store(const MemoryValue& mem,const Value::Pointer& ptr, const Value::I64& val)  const{
	auto old_mem_var = mem.getMemVar ();
	auto mem_var = write<Value::Pointer::intbitsize()> (val.size(),*builder,old_mem_var,ptr.getTerm (),val.getTerm ());
	return MemoryValue(mem.getNextBlock(),mem_var);
      }

      MemoryValue Memory::store(const MemoryValue& mem,const Value::Pointer& ptr, const Value::Pointer& val) const{
	auto old_mem_var = mem.getMemVar ();
	auto mem_var = write<Value::Pointer::intbitsize()> (val.size(),*builder,old_mem_var,ptr.getTerm (),val.getTerm ());
	return MemoryValue(mem.getNextBlock(),mem_var);
      }

      MemoryValue Memory::store(const MemoryValue& mem,const Value::Pointer& ptr, const Value::Pointer32& val) const{
	auto old_mem_var = mem.getMemVar ();
	auto mem_var = write<Value::Pointer::intbitsize()> (val.size(),*builder,old_mem_var,ptr.getTerm (),val.getTerm ());
	return MemoryValue(mem.getNextBlock(),mem_var);
      }

      MemoryValue Memory::store(const MemoryValue& mem,const Value::Pointer& ptr, const Value::Aggregate& val) const {
	auto old_mem_var = mem.getMemVar ();
	auto mem_var = writeAggr<Value::Pointer::intbitsize()> (val.size(),*builder,old_mem_var,ptr.getTerm (),val.getTerm ());
	return MemoryValue(mem.getNextBlock(),mem_var);
      }
      
      std::ostream& MemoryValue::output(std::ostream& os) const {
	if (mem_var)
	  return mem_var->output(os);
	else
	  return os << "Uninit Mem";
      }
      

      template <typename v,MiniMC::Model::TypeID id>
      std::ostream& TValue<v,id>::output(std::ostream& os) const {
        return term->output(os);
      }

      

      
      

      template<class T,MiniMC::Model::TypeID id>
      T TValue<T,id>::interpretValue (const SMTLib::Solver& solver) const {
	
	if constexpr (MiniMC::Model::is_pointer_v<T>) {
	  //T res;
	  // std::memset (&pointer,0,sizeof(MiniMC::pointer_t));

	  auto ires = std::get<SMTLib::bitvector> (solver.getModelValue(term));
	  decltype(T::segment) seg{0};
	  decltype(T::offset) offset{0};
	  decltype(T::base) base{0};
	  
		
	  auto iter = MiniMC::Support::SMT::extract (ires.begin(),offset);
	  iter = MiniMC::Support::SMT::extract (iter,base);
	  iter = MiniMC::Support::SMT::extract (iter,seg);
	  /*res.segment = seg;
	  res.offset = offset;
	  res.base = base;
	  return res;*/
	  return T{seg,base,offset};
	  
	}
	else if constexpr (std::is_same_v<T,MiniMC::Util::Array>) {
	  MiniMC::Util::Array res{size()};
	  
	  auto aggrres = std::get<SMTLib::bitvector>(solver.getModelValue(term));
	  auto iter = aggrres.begin();
	  for (size_t i = 0; i < size(); i++) {
	    MiniMC::BV8 buf;
	    iter = MiniMC::Support::SMT::extractByte (iter,buf);
	    res.get_direct_access ()[i] = buf;
	  }
	  return res;
	  
								    
	}

	else if constexpr (std::is_same_v<T,MiniMC::BV8> ||
			   std::is_same_v<T,MiniMC::BV16> ||
			   std::is_same_v<T,MiniMC::BV32> ||
			   std::is_same_v<T,MiniMC::BV64>) {

	  T res{0};
	  
	  auto ires = std::get<SMTLib::bitvector>(solver.getModelValue(term));
	  MiniMC::Support::SMT::extract(ires.begin(),res);
	  return res;
	}
	
	if constexpr (std::is_same_v<T, bool>)
	  {
	    auto bres = std::get<bool>(solver.getModelValue(term));
	    return bres;
	  }

	else 
	  throw MiniMC::Support::Exception ("Not Implemented yet");
	
      }
      
      
      template class TValue<bool,MiniMC::Model::TypeID::Bool>;
      template class TValue<MiniMC::Model::pointer64_t,MiniMC::Model::TypeID::Pointer>;
      template class TValue<MiniMC::Model::pointer32_t,MiniMC::Model::TypeID::Pointer32>;
      template class TValue<MiniMC::BV8,MiniMC::Model::TypeID::I8>;
      template class TValue<MiniMC::BV16,MiniMC::Model::TypeID::I16>;
      template class TValue<MiniMC::BV32,MiniMC::Model::TypeID::I32>;
      template class TValue<MiniMC::BV64,MiniMC::Model::TypeID::I64>;
      template class TValue<MiniMC::Util::Array,MiniMC::Model::TypeID::Aggregate>;
      
      void ConstraintSolver::addConstraint (Value::Bool b) {
	smtsolver.assert_formula (b.getTerm ());
      }

      MiniMC::VMT::Feasibility ConstraintSolver::check () const  {

	switch (smtsolver.check_sat()) {
	case SMTLib::Result::Satis:
	  return MiniMC::VMT::Feasibility::Feasible;
	case SMTLib::Result::NSatis:
	  return MiniMC::VMT::Feasibility::Infeasible;
	default:
	  return MiniMC::VMT::Feasibility::Unknown;
	}
      }
      
      MiniMC::Model::Constant_ptr ConstraintSolver::eval (const Value& v) const {
	return MiniMC::VMT::Pathformula::Value::visit (MiniMC::Support::Overload {
	      [this](MiniMC::VMT::Pathformula::Value::I8& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I8Integer::make (val.interpretValue (smtsolver));},
	      [this](MiniMC::VMT::Pathformula::Value::I16& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I16Integer::make  (val.interpretValue (smtsolver));},
	      [this](MiniMC::VMT::Pathformula::Value::I32& val) ->MiniMC::Model::Constant_ptr { return MiniMC::Model::I32Integer::make  (val.interpretValue (smtsolver));},
	      [this](MiniMC::VMT::Pathformula::Value::I64& val) ->MiniMC::Model::Constant_ptr{return MiniMC::Model::I64Integer::make (val.interpretValue (smtsolver));},
	      [this](MiniMC::VMT::Pathformula::Value::Pointer& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::Pointer::make  (val.interpretValue (smtsolver));},
	      [this](MiniMC::VMT::Pathformula::Value::Pointer32& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::Pointer32::make  (val.interpretValue (smtsolver));},
	      [this](MiniMC::VMT::Pathformula::Value::Bool& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::Bool::make  (val.interpretValue (smtsolver));},
	      [this](MiniMC::VMT::Pathformula::Value::Aggregate& val) ->MiniMC::Model::Constant_ptr {
		auto res = val.interpretValue (smtsolver);
		return MiniMC::Model::AggregateConstant::make  (std::move(res));;
	      },
	      [](MiniMC::VMT::Pathformula::Value::Memory&) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I8Integer::make  (0);}
		
		},v
	  );
	
	
      }
      

      ValueDefinition::ValueDefinition (MiniMC::Support::SMT::SMTDescr fact) : context(fact.makeContext()) {
      }
      Operations ValueDefinition::ops () const {
	return Operations{context->getBuilder()};
      }
      Memory ValueDefinition::memops () const { return {context->getBuilder()};}
      ConstraintSolver ValueDefinition::solver() const {return ConstraintSolver{context->getSolver()};}
      
      
    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC
