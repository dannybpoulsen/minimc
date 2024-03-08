#include "minimc/model/valuevisitor.hpp"
#include "smt/builder.hpp"
#include "minimc/smt/smtconstruction.hpp"
#include "pathvm/pathformua.hpp"
#include "pathvm/value.hpp"
#include "smt/solver.hpp"
#include "minimc/smt/smt.hpp"
#include <sstream>

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {
      static std::size_t next = 0;
      Value ValueCreator::defaultValue(const MiniMC::Model::Type& t) const {
	return unboundValue (t);
      }
	
      Value ValueCreator::unboundValue(const MiniMC::Model::Type& t) const {
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
	    
          default:
            break;
        }
        throw MiniMC::Support::Exception("Erro");
      }
      
      
      
      Value ValueCreator::create(const MiniMC::Model::I8Integer& val) const { return I8Value(builder.makeBVIntConst(val.getValue(), 8)); }
      Value ValueCreator::create(const MiniMC::Model::I16Integer& val) const { return I16Value(builder.makeBVIntConst(val.getValue(), 16)); }
      Value ValueCreator::create(const MiniMC::Model::I32Integer& val) const { return I32Value(builder.makeBVIntConst(val.getValue(), 32)); }
      Value ValueCreator::create(const MiniMC::Model::I64Integer& val) const { return I64Value(builder.makeBVIntConst(val.getValue(), 64)); }
      Value ValueCreator::create(const MiniMC::Model::Bool& val) const { return BoolValue(builder.makeBoolConst(val.getValue())); }
      Value ValueCreator::create(const MiniMC::Model::Pointer& val) const { 
	auto pointer = val.getValue ();
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	chainer << builder.makeBVIntConst(pointer.segment, sizeof(pointer.segment)*8)
		<< builder.makeBVIntConst(pointer.base, sizeof(pointer.base)*8)
		<< builder.makeBVIntConst(pointer.offset, sizeof(pointer.offset)*8);
	return PointerValue(chainer.getTerm ());
	} 

      Value ValueCreator::create(const MiniMC::Model::Pointer32& val) const { 
	auto pointer = val.getValue ();
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	chainer << builder.makeBVIntConst(pointer.segment, sizeof(pointer.segment)*8)
		  << builder.makeBVIntConst(pointer.base, sizeof(pointer.base)*8)
		  << builder.makeBVIntConst(pointer.offset, sizeof(pointer.offset)*8);
	  return Value::Pointer32(chainer.getTerm ());
	}
      Value ValueCreator::create(const MiniMC::Model::AggregateConstant& val) const {
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> chainer{&builder};
	for (auto byte : val.getData()) {
	  chainer >> (builder.makeBVIntConst(byte, 8));
	}
	return AggregateValue(chainer.getTerm(), val.getSize());
      }
      Value ValueCreator::create(const MiniMC::Model::Undef& val) const { return unboundValue(*val.getType()); }
      
      
      /*Value ValueLookupBase::lookupValue(const MiniMC::Model::Value& v) const {
	return MiniMC::Model::visitValue(
            MiniMC::Model::Overload{
	      [this](const MiniMC::Model::Register& val) -> Value {
		return lookupRegisterValue (val);
	      },
	      [this](const auto& v) -> Value {
		return creator.create(v);
	      }
	    },
            v);
      }
      */
      Value Memory::load(const typename Value::Pointer& startAddr, const MiniMC::Model::Type_ptr& t) const {
	
	MiniMC::Util::Chainer<SMTLib::Ops::Concat> concat(&builder);
        for (size_t i = 0; i < t->getSize (); ++i) {
          auto ones = builder.makeBVIntConst(i, Value::Pointer::intbitsize());
          auto curind = builder.buildTerm(SMTLib::Ops::BVAdd, {startAddr.getTerm (), ones});
          concat << builder.buildTerm(SMTLib::Ops::Select, {mem_var, curind});
        }
	switch (t->getTypeID ()) {
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
	  return Value::Aggregate{concat.getTerm(),t->getSize ()};
	case MiniMC::Model::TypeID::Float:
	case MiniMC::Model::TypeID::Double:
	case MiniMC::Model::TypeID::Void:
	default:
	  throw MiniMC::Support::Exception ("Float and DOuble unsupported");
	}
	
      }

      Value::Pointer Memory::alloca(const Value::I64&) {
	MiniMC::Util::PointerHelper helper {&builder};
	auto stack_pointer = helper.makeHeapPointer (++next_block,0);
        return Value::Pointer(std::move(stack_pointer));
      }

      Memory::Memory (SMTLib::TermBuilder& b) : builder(b) {
	auto arr_sort = builder.makeSort(
					 SMTLib::SortKind::Array, {builder.makeBVSort(Value::Pointer::intbitsize()),
								   builder.makeBVSort(8)});
	mem_var = builder.makeVar(arr_sort, "Mem");
      }

      void Memory::createHeapLayout(const MiniMC::Model::HeapLayout& hl) {
	//For now we don't need to do anything special...except run thorough all blocks and ensure out start block number is less than the one used by the heap
	for (auto& block : hl) {
	  auto baseobj = MiniMC::Model::getBase(block.baseobj);
	  next_block = (baseobj > next_block) ? baseobj + 1 : next_block;
	}

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

      
      void Memory::store(const Value::Pointer& ptr, const Value::I8& val)  {
	mem_var = write<Value::Pointer::intbitsize()> (val.size(),builder,mem_var,ptr.getTerm (),val.getTerm ());
      }

      void Memory::store(const Value::Pointer& ptr, const Value::I16& val) {
	mem_var = write<Value::Pointer::intbitsize()> (val.size(),builder,mem_var,ptr.getTerm (),val.getTerm ());
      }

      void Memory::store(const Value::Pointer& ptr, const Value::I32& val)  {
	mem_var = write<Value::Pointer::intbitsize()> (val.size(),builder,mem_var,ptr.getTerm (),val.getTerm ());
      }

      void Memory::store(const Value::Pointer& ptr, const Value::I64& val)  {
	mem_var = write<Value::Pointer::intbitsize()> (val.size(),builder,mem_var,ptr.getTerm (),val.getTerm ());
      }

      void Memory::store(const Value::Pointer& ptr, const Value::Pointer& val) {
	mem_var = write<Value::Pointer::intbitsize()> (val.size(),builder,mem_var,ptr.getTerm (),val.getTerm ());
      }

      void Memory::store(const Value::Pointer& ptr, const Value::Pointer32& val) {
	mem_var = write<Value::Pointer::intbitsize()> (val.size(),builder,mem_var,ptr.getTerm (),val.getTerm ());
      }

      void Memory::store(const Value::Pointer& ptr, const Value::Aggregate& val) {
	mem_var = writeAggr<Value::Pointer::intbitsize()> (val.size(),builder,mem_var,ptr.getTerm (),val.getTerm ());
      }
      
      
      PathControl::PathControl(SMTLib::TermBuilder& builder) : builder(builder) {
        assump = nullptr;
      }

      TriBool PathControl::addAssumption(const Value::Bool& b) {
        if (assump)
          assump = builder.buildTerm(SMTLib::Ops::And, {assump, b.getTerm()});
        else
          assump = b.getTerm();
        return TriBool::Unk;
      }

      TriBool PathControl::addAssert(const Value::Bool& b) {
	if (asserts)
          asserts = builder.buildTerm(SMTLib::Ops::And, {asserts, b.getTerm()});
        else
          asserts = b.getTerm();
	return TriBool::Unk;
      }

      template <typename v>
      std::ostream& TValue<v>::output(std::ostream& os) const {
        return term->output(os);
      }

      template<class T>
      T TValue<T>::interpretValue (const SMTLib::Solver& solver) const {
	if constexpr (MiniMC::Model::is_pointer_v<T>) {
	  T pointer;
	  // std::memset (&pointer,0,sizeof(MiniMC::pointer_t));
	  
	  auto pointerres = std::get<SMTLib::bitvector>(solver.getModelValue(term));
	  assert(sizeof(T) == pointerres.size() / 8);
	  auto beginoff = pointerres.begin()+((sizeof(T)-offsetof(T,offset)-sizeof(pointer.offset)))*8;
	  auto segoff = pointerres.begin()+((sizeof(T)-offsetof(T,segment)-sizeof(pointer.segment)))*8;
	  auto baseoff = pointerres.begin()+((sizeof(T)-offsetof(T,base)-sizeof(pointer.base)))*8;
	  
	  
	  MiniMC::Support::SMT::extractBytes(beginoff, beginoff+sizeof(pointer.offset)*8, reinterpret_cast<MiniMC::BV8*>(&pointer.offset));
	  MiniMC::Support::SMT::extractBytes(segoff, segoff+sizeof(pointer.segment)*8, reinterpret_cast<MiniMC::BV8*>(&pointer.segment));
	  MiniMC::Support::SMT::extractBytes(baseoff, baseoff+sizeof(pointer.base)*8, reinterpret_cast<MiniMC::BV8*>(&pointer.base));
                
	  return pointer;
	}
	else if constexpr (std::is_same_v<T,MiniMC::Util::Array>) {
	  MiniMC::Util::Array res{size()};
	  
	  auto aggrres = std::get<SMTLib::bitvector>(solver.getModelValue(term));
	  MiniMC::Support::SMT::extractBytes(aggrres.begin(), aggrres.end(), res.begin());
	  return res;
	  
	}

	else if constexpr (std::is_same_v<T,MiniMC::BV8> ||
			   std::is_same_v<T,MiniMC::BV16> ||
			   std::is_same_v<T,MiniMC::BV32> ||
			   std::is_same_v<T,MiniMC::BV64>) {

	  T res{0};
	  
	  auto ires = std::get<SMTLib::bitvector>(solver.getModelValue(term));
	  assert(sizeof(T) == ires.size() / 8);
	  MiniMC::Support::SMT::extractBytes(ires.begin(), ires.end(), reinterpret_cast<MiniMC::BV8*>(&res));
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
      
      
      template class TValue<bool>;
      template class TValue<MiniMC::Model::pointer64_t>;
      template class TValue<MiniMC::Model::pointer32_t>;
      template class TValue<MiniMC::BV8>;
      template class TValue<MiniMC::BV16>;
      template class TValue<MiniMC::BV32>;
      template class TValue<MiniMC::BV64>;
      template class TValue<MiniMC::Util::Array>;

    } // namespace Pathformula
  }   // namespace VMT
} // namespace MiniMC
