#ifndef _CPA_QUERY__
#define _CPA_QUERY__

#include "minimc/model/variables.hpp"
#include "minimc/io/ostream.hpp"

#include <iosfwd>
#include <memory>


namespace MiniMC {
  namespace CPA {
    class QueryExpr {
    public:
      virtual ~QueryExpr () {}
      virtual MiniMC::IO::ostream& output (MiniMC::IO::ostream
					   &) const = 0;
    };

    template<class T>
    class TQuery : public QueryExpr {
    public:
      TQuery (T t):value(t) {}
      
      MiniMC::IO::ostream& output (MiniMC::IO::ostream& os) const override {
	std::stringstream str;
	str << value;
	return os << str.str();
      } 
      
      auto getValue () const {return value;}
	
    private:
      T value;
    };
    
    using QueryExpr_ptr = std::unique_ptr<QueryExpr>;
    
    class QueryBuilder {
    public:
      virtual ~QueryBuilder () {}
      virtual QueryExpr_ptr buildValue (MiniMC::Model::proc_t, const MiniMC::Model::Value&) const = 0; 
    };

    class QueryEvaluator {
      public:
      virtual ~QueryEvaluator () {}
      virtual MiniMC::Model::Constant_ptr evaluate (const QueryExpr&) const = 0;
    };

    inline MiniMC::IO::ostream& operator<< (MiniMC::IO::ostream& os, const QueryExpr& e) {
      return e.output (os);
    }
    
  }
}

#endif
