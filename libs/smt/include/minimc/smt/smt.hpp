#ifndef _SUPPORT_SMT__
#define _SUPPORT_SMT__

#include "smt/context.hpp"
#include "minimc/host/types.hpp"
#include "minimc/model/variables.hpp"
#include <iostream>
#include <memory>
#include <expected>


namespace MiniMC {
  namespace Support {
    namespace SMT {

      
      struct SMTDescr {
        SMTDescr(SMTLib::SMTBackendRegistrar* r = nullptr) : r(r) {}
	SMTDescr (const SMTDescr&) = default;
        std::string name() const; 
        std::string descr() const; 
	SMTLib::Context_ptr makeContext () const;
      private:
	SMTLib::SMTBackendRegistrar* r;
      };
      enum class Error {
	NoSolver
      };
      
      class SMTSolverRepository {
      public:
	static SMTSolverRepository& get ();
	
	std::expected<SMTDescr,Error> getBackend (const std::string& i ) {
	  for (auto& r : descr) {
	    if (r.name () == i) {
	      return r;
	    }
	  }
	  return std::unexpected (Error::NoSolver);
	}
	
	auto& getSMTBackends () const {
	  return descr;
	}
      private:
	SMTSolverRepository ();
	std::vector<SMTDescr> descr;
      };

      
      
      template<class Iterator,class EIterator>
      void extractBytes (Iterator it, Iterator end, EIterator dest) {
	std::size_t bitscounted = 0;
	for (; it != end; ++it,++bitscounted ) {
	  if (*it) {
	    std::size_t bytenumber = bitscounted / 8;
	    std::size_t bit = bitscounted % 8;
	    
	    *(dest+bytenumber) |= (1 << bit);
	  }
	  
	}
      }

      class Translator   {
      public:
	Translator (SMTLib::Context_ptr context) : context(context) {}
	SMTLib::Term_ptr Translate (const MiniMC::Model::Value& v) {
	  SMTLib::Term_ptr res = cached (v);
	  if (!res) {
	    res = MiniMC::Model::visitValue<SMTLib::Term_ptr>(*this,v);
	    assert(res);
	    cache(v,res);
	  }
	  return res;
	}
	
	SMTLib::Term_ptr operator() (const MiniMC::Model::I8Integer&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::I16Integer&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::I32Integer&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::I64Integer&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::Bool&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::Pointer&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::Pointer32&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::AggregateConstant&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::Register&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::Undef&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::SymbolicConstant&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::StoreExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::LoadExpr&);
	
	SMTLib::Term_ptr operator() (const MiniMC::Model::Ptr32ToPtrExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::PtrToPtr32Expr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::AddExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::LogAndExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::SubExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::MulExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::UDivExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::SDivExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::LShlExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::LShrExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::AShrExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::AndExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::OrExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::XorExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::SGtExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::UGtExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::SGeExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::UGeExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::SLtExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::ULtExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::SLeExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::ULeExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::EqExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::NEqExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::NotExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::LogNotExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::TruncExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::BitCastExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::ZExtExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::SExtExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::PtrToIntExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::IntToPtrExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::IntToBoolExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::PtrAddExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::PtrSubExpr&);
	
	SMTLib::Term_ptr operator() (const MiniMC::Model::ExtractValueExpr&);
	SMTLib::Term_ptr operator() (const MiniMC::Model::InsertValueExpr&);
	
	
	
      private:
	SMTLib::Context_ptr context;
	SMTLib::Term_ptr cached (const MiniMC::Model::Value& v) const {
	  if (terms.count(&v))
	    return terms.at(&v);
	  return nullptr;
	}
	SMTLib::Term_ptr cache (const MiniMC::Model::Value& v, SMTLib::Term_ptr t) {
	  terms.emplace(&v,t);
	  return t;
	}
	std::unordered_map<const MiniMC::Model::Value*,SMTLib::Term_ptr> terms;
	std::size_t next{0};
      };
      
    } // namespace SMT
  }   // namespace Support

} // namespace MiniMC
#endif
