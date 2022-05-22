#ifndef _CPA_QUERY__
#define _CPA_QUERY__

#include "util/array.hpp"
#include "model/variables.hpp"
#include "vm/concrete/concrete.hpp"

#include <iosfwd>
#include <memory>



namespace MiniMC {
  namespace CPA {
    class QueryExpr {
    public:
      
      virtual ~QueryExpr () {}
      virtual std::ostream& output (std::ostream&) const = 0;
    };

    using QueryExpr_ptr = std::unique_ptr<QueryExpr>;
    
    class QueryBuilder {
    public:
      virtual ~QueryBuilder () {}
      virtual QueryExpr_ptr buildValue (MiniMC::proc_t, const MiniMC::Model::Value_ptr&) const = 0; 
    };

    class QueryEvaluator {
      public:
      virtual ~QueryEvaluator () {}
      virtual MiniMC::VMT::Concrete::ConcreteVMVal evaluate (const QueryExpr&) const = 0;
    };

    inline std::ostream& operator<< (std::ostream& os, const QueryExpr& e) {
      return e.output (os);
    }
    
  }
}

#endif
