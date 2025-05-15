#ifndef _SUPPORT_SMT__
#define _SUPPORT_SMT__

#include "smt/context.hpp"
#include "minimc/host/types.hpp"
#include "minimc/model/variables.hpp"
#include <iostream>
#include <memory>
#include <expected>
#include <iostream>
#include <bit>

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

      
      
      template<class Iterator>
      Iterator extractByte (Iterator iter, MiniMC::BV8& b) {
	b = 0;
	for (int i = 0; i < 8; ++iter,++i) {
	  if (*iter) {
	    b |= 1 << i;
	  }
	}
	return iter;
      }

      
      
      template<class Iterator,class T>
      Iterator extract (Iterator iter, T& b) {
	union {
	  T val;
	  MiniMC::BV8 d[sizeof(T)];
	  
	} data;
	
	for (unsigned int i = 0; i < sizeof(data.d); ++i) {
	  iter = extractByte(iter,data.d[i]);
	}
	b = data.val;
	return iter;
      }

      
      template<class Iterator,class EIterator>
      void extractBytes (Iterator it, Iterator end, EIterator dest) {
	for (; it != end; ++dest,++it ) {
	  
	  MiniMC::BV8 buf{0};
	  it = extractByte (it,buf);
	  *dest = buf;
	  
	  
	}
      }

      
      class Translator   {
      public:
	Translator (SMTLib::Context_ptr context) : context(context) {}
	SMTLib::Term_ptr Translate (const MiniMC::Model::Value& v) const {
	  SMTLib::Term_ptr res = cached (v);
	  if (!res) {
	    res = MiniMC::Model::visitValue<SMTLib::Term_ptr>(*this,v);
	    assert(res);
	    cache(v,res);
	  }
	  return res;
	}
	
	SMTLib::Term_ptr operator() (const MiniMC::Model::I8Integer&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::I16Integer&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::I32Integer&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::I64Integer&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::Bool&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::Pointer&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::Pointer32&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::AggregateConstant&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::Register&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::Undef&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::SymbolicConstant&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::StoreExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::LoadExpr&) const;
	
	SMTLib::Term_ptr operator() (const MiniMC::Model::Ptr32ToPtrExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::PtrToPtr32Expr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::AddExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::LogAndExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::SubExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::MulExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::UDivExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::SDivExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::LShlExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::LShrExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::AShrExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::AndExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::OrExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::XorExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::SGtExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::UGtExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::SGeExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::UGeExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::SLtExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::ULtExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::SLeExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::ULeExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::EqExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::NEqExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::NotExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::LogNotExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::TruncExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::BitCastExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::ZExtExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::SExtExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::PtrToIntExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::IntToPtrExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::IntToBoolExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::PtrAddExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::PtrSubExpr&) const;
	
	SMTLib::Term_ptr operator() (const MiniMC::Model::ExtractValueExpr&) const;
	SMTLib::Term_ptr operator() (const MiniMC::Model::InsertValueExpr&) const;
	
	
	
      private:
	SMTLib::Context_ptr context;
	SMTLib::Term_ptr cached (const MiniMC::Model::Value& v) const {
	  if (terms.count(&v))
	    return terms.at(&v);
	  return nullptr;
	}
	SMTLib::Term_ptr cache (const MiniMC::Model::Value& v, SMTLib::Term_ptr t) const {
	  terms.emplace(&v,t);
	  return t;
	}
	mutable std::unordered_map<const MiniMC::Model::Value*,SMTLib::Term_ptr> terms;
	mutable std::size_t next{0};
      };
      
    } // namespace SMT
  }   // namespace Support

} // namespace MiniMC
#endif
