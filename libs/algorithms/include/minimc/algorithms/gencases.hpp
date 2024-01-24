
#include "minimc/cpa/interface.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/smt/smt.hpp"
#include "minimc/support/feedback.hpp"

#include <iosfwd>
#include <memory>
#include <vector>

namespace MiniMC {
  namespace Algorithms {
    namespace GenCases {
      class TestCase {
      public:
	TestCase () {}
	TestCase (std::vector<MiniMC::Model::Value_ptr>&& v) : _values(std::move(v)) {}
	auto& values () const {return _values;}
      private:
	std::vector<MiniMC::Model::Value_ptr> _values;
      };

      class TestCaseGenResult {
      public:
	TestCaseGenResult (std::vector<MiniMC::Model::Register_ptr> params) : _vars(std::move(params)) {}
	void addTestCase (TestCase&& casee){
	  if (casee.values().size () != _vars.size ()) {
	    throw MiniMC::Support::Exception ("Error. Inconsistent amount of values for test case");
	  }
	  else
	    _cases.push_back (std::move(casee));
	}

	auto& vars () const {return _vars;}
	auto& cases () const {return _cases;}
	
      private:
	std::vector<MiniMC::Model::Register_ptr> _vars;
	std::vector<TestCase> _cases;
      };

      class TestCaseGenerator {
      public:
	TestCaseGenerator (MiniMC::Model::Program& prgm);
	~TestCaseGenerator ();
	[[nodiscard]] TestCaseGenResult generate  (MiniMC::Support::Messager& mess,
				     MiniMC::Model::Function_ptr& function,
				     MiniMC::Support::SMT::SMTDescr smt
				     );
	
	
	
      private:
	struct Internal;
	std::unique_ptr<Internal> _internal;
      };
      
      
    }
  }
}
