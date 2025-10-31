#include "minimc/model/cfg.hpp"
#include "whiley/ast.hpp"

namespace MiniMC {
  namespace Loaders {
    namespace whiley {
      class Compiler : private Whiley::NodeVisitor {
      public:
	Compiler ();
	~Compiler();
	MiniMC::Model::Program compile (const ::Whiley::Program& prgm );	
	void visitIdentifier (const Whiley::Identifier&) override ;
	void visitNumberExpression (const Whiley::NumberExpression& ) override ; 
	void visitDerefExpression (const Whiley::DerefExpression& ) override ; // 
	void visitCastExpression (const Whiley::CastExpression& ) override ; // 
	void visitUndefExpression (const Whiley::UndefExpression& ) override ; //
	void visitBinaryExpression (const Whiley::BinaryExpression& ) override ;  
	void visitAssignStatement (const Whiley::AssignStatement& ) override ; 
	void visitAssertStatement (const Whiley::AssertStatement& ) override ; 
	void visitAssumeStatement (const Whiley::AssumeStatement& ) override ; 
	
	void visitIfStatement (const Whiley::IfStatement& ) override ; 
	void visitSkipStatement (const Whiley::SkipStatement& ) override ; 
	void visitWhileStatement (const Whiley::WhileStatement& ) override ; 
	void visitSequenceStatement (const Whiley::SequenceStatement& ) override ; 
	void visitChooseStatement (const Whiley::ChooseStatement& ) override ; 
	
	void visitMemAssignStatement (const Whiley::MemAssignStatement&) override;

      private:
	
	struct Internal;
	std::unique_ptr<Internal> _internal;
        };
    }
  }
}
