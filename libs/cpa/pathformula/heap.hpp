#ifndef _pathheap__
#define _pathheap__

#include <memory>
#include "support/feedback.hpp"
#include "support/pointer.hpp"
#include "util/ssamap.hpp"
#include "cpa/interface.hpp"
#include "smt/context.hpp"
#include "smt/solver.hpp"


namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
      struct HeapEntry {
	HeapEntry (SMTLib::Term_ptr cont, std::size_t s)  : content(cont),size(s) {} 
	SMTLib::Term_ptr content;
	std::size_t size; //Only fixed size allocations supported for now
      };

      template<SMTLib::Ops op>
      struct Chainer {
	Chainer (SMTLib::TermBuilder* builder) : builder (builder) {}

	Chainer& operator<< (SMTLib::Term_ptr nterm) {
	  if (!term)
	    term = nterm;
	  else {
	    term = builder->buildTerm (op,{term,nterm});
	  }
	  return *this;
	}

	auto getTerm () const {return term;}
	
	SMTLib::Term_ptr term= nullptr;
	SMTLib::TermBuilder* builder;
      };
      
      struct PointerHelper {
	PointerHelper (SMTLib::TermBuilder* b) : builder(b) {}

	SMTLib::Term_ptr isStackConstraint (SMTLib::Term_ptr& term) {
	  //Extract first eight bits;
	  auto stack_seg = builder->buildTerm (SMTLib::Ops::Extract,{term},{63,63-sizeof(MiniMC::seg_t)*8});
	  auto stack_id = builder->makeBVIntConst (static_cast<MiniMC::seg_t> (MiniMC::Support::PointerType::Stack),8);
	  return builder->buildTerm (SMTLib::Ops::Equal,{stack_seg,stack_id});
	}

	SMTLib::Term_ptr baseValue (SMTLib::Term_ptr& term) {
	  return builder->buildTerm (SMTLib::Ops::Extract,{term},{47,32}); 
	}

	SMTLib::Term_ptr offsetValue (SMTLib::Term_ptr& term) {
	  return builder->buildTerm (SMTLib::Ops::Extract,{term},{31,0}); 
	}
	
	SMTLib::Term_ptr makeStackPointer (MiniMC::base_t base,MiniMC::offset_t) {

	  auto stack_term = builder->makeBVIntConst (static_cast<MiniMC::seg_t> (MiniMC::Support::PointerType::Stack),8*sizeof(MiniMC::seg_t));
	  auto zero_term = builder->makeBVIntConst (0,8*sizeof(MiniMC::int8_t));
	  
	  auto base_term = builder->makeBVIntConst (base,8*sizeof(MiniMC::base_t));
	  auto offset_term = builder->makeBVIntConst (base,8*sizeof(MiniMC::offset_t));

	  return (Chainer<SMTLib::Ops::Concat>{builder} <<  stack_term << zero_term << base_term << offset_term ).getTerm ();
	  
	}
	  
	SMTLib::TermBuilder* builder;
      };

      SMTLib::Term_ptr readFromArr (size_t bytes, SMTLib::TermBuilder& t, const SMTLib::Term_ptr&  arr, const SMTLib::Term_ptr&  startInd) {
	Chainer<SMTLib::Ops::Concat> concat (&t);
	for (size_t i = 0; i < bytes;++i) {
	  auto ones = t.makeBVIntConst (i,32);
	  auto curind = t.buildTerm (SMTLib::Ops::BVAdd,{startInd,ones});
	  concat << t.buildTerm (SMTLib::Ops::Select,{arr,curind});
	}
	
	return concat.getTerm ();
      }
      
      SMTLib::Term_ptr writeToArr (size_t bytes, SMTLib::TermBuilder& t, const SMTLib::Term_ptr&  arr, const SMTLib::Term_ptr&  startInd,SMTLib::Term_ptr content) {
	auto carr = arr;
	for (size_t i = 0; i < bytes;++i) {
	  auto ones = t.makeBVIntConst (bytes-1-i,32);
	  auto curind = t.buildTerm (SMTLib::Ops::BVAdd,{startInd,ones});
	  auto curbyte = t.buildTerm (SMTLib::Ops::Extract,{content},{i*8+7,i*8});
	  
	  carr = t.buildTerm (SMTLib::Ops::Store,{carr,curind,curbyte});
	}
	assert(carr);
	//std::cerr << *carr << std::endl;
	return carr;
      }
       
      class Heap {
      public:
	void free (SMTLib::Term_ptr pointer) {
	  //Ignore for now
	}

	void  write (SMTLib::Term_ptr content,SMTLib::Term_ptr pointer,std::size_t bytes, SMTLib::TermBuilder& builder) {
	  if (entries.size () == 0) {
	    //DO nothing atm
	  }
	  else {
	    for (base_t i = 0; i < entries.size (); i++) {
	      PointerHelper phelper (&builder);
	      auto comp = builder.buildTerm (SMTLib::Ops::Equal, {phelper.baseValue (pointer), builder.makeBVIntConst (i,8*sizeof(MiniMC::base_t))});
	      auto updArr = writeToArr (bytes,builder,entries[i].content,phelper.offsetValue(pointer),content);
	      auto newcontent = builder.buildTerm (SMTLib::Ops::ITE, {comp,updArr,entries[i].content});
	      entries[i].content = newcontent;
	    }
	    
	  }
	}

	SMTLib::Term_ptr  read (SMTLib::Term_ptr pointer,std::size_t bytes,SMTLib::TermBuilder& builder) const  {
	  auto sort = builder.makeBVSort (bytes*8);
	  
	    
	  auto res = builder.makeVar (sort, "UndefRead");
	  
	  PointerHelper phelper (&builder);
	  auto offset = phelper.offsetValue(pointer);
	  auto base = phelper.baseValue (pointer);
	  for (base_t i = 0; i < entries.size(); i++) {
	    auto comp = builder.buildTerm (SMTLib::Ops::Equal, {base, builder.makeBVIntConst (i,8*sizeof(MiniMC::base_t))});
	    auto read = readFromArr (bytes,builder,entries.at(i).content,offset); 
	    res = builder.buildTerm (SMTLib::Ops::ITE, {comp,read,res});
	  }
	  return res;
	}

	SMTLib::Term_ptr allocate (MiniMC::uint64_t size,SMTLib::TermBuilder& builder) {
	  auto res = PointerHelper {&builder}.makeStackPointer (entries.size(),0);
	  auto arr_sort = builder.makeSort (
					     SMTLib::SortKind::Array,{
					       builder.makeBVSort (32),
					       builder.makeBVSort (8)}
					    );
	  auto content = builder.makeVar (arr_sort,"CC");
	  entries.emplace_back (content,size);
	  return res;
	}
	
      private:
	std::vector<HeapEntry> entries;
      };
      
    }
  }
}


#endif
