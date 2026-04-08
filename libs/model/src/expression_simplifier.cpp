#include "minimc/model/variables.hpp"
#include "minimc/host/operataions.hpp"
#include "minimc/host/casts.hpp"

namespace MiniMC {
  namespace Model {



    
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::LogAndExpr& i) const {
      auto l = Simplify (i.op1());
      auto r = Simplify (i.op2());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [](MiniMC::Model::Bool& ll,MiniMC::Model::Bool& rr)->MiniMC::Model::Value_ptr  {
	    return MiniMC::Model::Bool::make (ll.getValue() && rr.getValue());
	  },
	  [&l,&r](auto&, auto&) {
	    return MiniMC::Model::makeExpr<MiniMC::Model::LogAndExpr> (l,r);
	  }
	    },
	*l,*r);
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::NotExpr& i) const {
      return i.shared_from_this();
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::LogNotExpr& i) const {return i.shared_from_this();}

    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::PtrAddExpr& i) const {
      auto ptr = Simplify (i.ptr());
      auto skipsize = Simplify (i.skipsize());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [](MiniMC::Model::Pointer& p,MiniMC::Model::I64Integer& skip)->MiniMC::Model::Value_ptr  {
	    return MiniMC::Model::Pointer::make (p.getValue().add (skip.getValue ()));
	    },
	    [&ptr,&skipsize](auto&, auto&) {
	      return MiniMC::Model::makeExpr<MiniMC::Model::PtrAddExpr> (ptr,skipsize);}
	    },
	*ptr,*skipsize);
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::PtrSubExpr& i) const {
        auto ptr = Simplify (i.ptr());
      auto skipsize = Simplify (i.skipsize());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [](MiniMC::Model::Pointer& p,MiniMC::Model::I64Integer& skip)->MiniMC::Model::Value_ptr  {
	    return MiniMC::Model::Pointer::make (p.getValue().sub (skip.getValue ()));
	    },
	    [&ptr,&skipsize](auto&, auto&) {
	      return MiniMC::Model::makeExpr<MiniMC::Model::PtrSubExpr> (ptr,skipsize);}
	    },
	*ptr,*skipsize);
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::ExtractValueExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::ExtractValueExpr> (Simplify (i.aggregate()),Simplify (i.offset()),i.getExtractType());
    }

    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::AllocExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::AllocExpr> (Simplify (i.memory()),Simplify (i.pointer()),Simplify(i.size()));
    }

    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::FreeExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::FreeExpr> (Simplify (i.memory()),Simplify (i.pointer()));
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::CheckFreeExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::CheckFreeExpr> (Simplify (i.memory()),Simplify (i.pointer()),Simplify(i.size()));
    }

    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::ValidPointerExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::ValidPointerExpr> (Simplify (i.memory()),Simplify (i.pointer()));
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::FindSpaceExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::FindSpaceExpr> (Simplify (i.memory()),Simplify(i.size()));
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::InsertValueExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::InsertValueExpr> (Simplify (i.aggregate()),Simplify(i.offset()),Simplify (i.insertee()));
      
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::BitCastExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::BitCastExpr> (Simplify (i.op1()),i.toType());
      
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::PtrToIntExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::PtrToIntExpr> (Simplify (i.op1()),i.toType());
     
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::IntToPtrExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::IntToPtrExpr> (Simplify (i.op1()),i.toType());
      
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::StoreExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::StoreExpr> (Simplify(i.storeto()),Simplify (i.addr()),Simplify(i.storee()));
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::LoadExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::LoadExpr> (Simplify(i.mem()),Simplify (i.addr()),i.loadType());
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::IntToBoolExpr& i) const {return i.shared_from_this();}
    
    
  }
}
