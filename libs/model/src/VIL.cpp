#include "minimc/model/VIL.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/variables.hpp"


namespace MiniMC {
  namespace Model {
    namespace VIL {


      StatementBuilder& StatementBuilder::Assign (MiniMC::Model::Register_ptr r, MiniMC::Model::SourceInfo sinfo) {
	auto assign_expr = get();
	auto instr = MiniMC::Model::makeInstruction (MiniMC::Model::InstructionCode::Assign,{r,assign_expr});
	stmts.push (std::make_shared<InstructionStatement> (instr,sinfo));
	return *this;
      }
      
      StatementBuilder& StatementBuilder::RetVoid (MiniMC::Model::SourceInfo sinfo) {
	auto instr = MiniMC::Model::makeInstruction (MiniMC::Model::InstructionCode::RetVoid,{});
	stmts.push (std::make_shared<InstructionStatement> (instr,sinfo));
	return *this;
      }
      StatementBuilder& StatementBuilder::Ret (MiniMC::Model::SourceInfo sinfo ) {
	auto ret_expr = get();
	auto instr = MiniMC::Model::makeInstruction (MiniMC::Model::InstructionCode::Ret,{ret_expr});
	stmts.push (std::make_shared<InstructionStatement> (instr,sinfo));
	return *this;
      }
      
      StatementBuilder& StatementBuilder::Assume (MiniMC::Model::SourceInfo sinfo) {
	auto ret_expr = get();
	auto instr = MiniMC::Model::makeInstruction (MiniMC::Model::InstructionCode::Assume,{ret_expr});
	stmts.push (std::make_shared<InstructionStatement> (instr,sinfo));
	return *this;
      }

      StatementBuilder& StatementBuilder::Assert (MiniMC::Model::SourceInfo sinfo) {
	auto ret_expr = get();
	auto instr = MiniMC::Model::makeInstruction (MiniMC::Model::InstructionCode::Assert,{ret_expr});
	stmts.push (std::make_shared<InstructionStatement> (instr,sinfo));
	return *this;
      }
      
      StatementBuilder& StatementBuilder::Loop (MiniMC::Model::SourceInfo sinfo) {
	auto cond = get();
	auto stmt = stmts.top ();stmts.pop();
	stmts.push (std::make_shared<LoopStatement> (stmt,cond,sinfo));
	return *this;
      }

      StatementBuilder& StatementBuilder::Sequence (MiniMC::Model::SourceInfo sinfo) {
	auto s = stmts.top ();stmts.pop();
	auto f = stmts.top ();stmts.pop();
	
	stmts.push (std::make_shared<SequenceStatement> (f,s,sinfo));
	return *this;
      }
      
      StatementBuilder& StatementBuilder::If (MiniMC::Model::SourceInfo sinfo) {
	auto cond = get();
	auto false_ = stmts.top ();stmts.pop();
	auto true_ = stmts.top ();stmts.pop();

	static_cast<MiniMC::Model::ExpressionBuilder&> (*this) << cond;
	(Assume(sinfo) << true_).Sequence() ;
	
	(static_cast<MiniMC::Model::ExpressionBuilder&> (*this) << cond).LogNot();
	(Assume(sinfo) << false_).Sequence() ;
	
	return Branch (2,sinfo);
	//return *this;
      }
      StatementBuilder& StatementBuilder::Branch (std::size_t nb, MiniMC::Model::SourceInfo sinfo){
	std::vector<Statement_ptr> statements;
	for (std::size_t i = 0; i < 2; ++i) {
	  statements.push_back (stmts.top());
	  stmts.pop();
	}
	stmts.push (std::make_shared<BranchStatement> (statements,sinfo));
	return *this;
      }
      StatementBuilder& StatementBuilder::Call (MiniMC::Model::Register_ptr r,  std::size_t nbArgs, MiniMC::Model::SourceInfo sinfo) {
	std::vector<Value_ptr> args;
	auto func = get();
	
	args.push_back(r);
	args.push_back(func);
	for (std::size_t i = 0; i < nbArgs; ++nbArgs) {
	  args.push_back (get());
	}
	auto instr = MiniMC::Model::makeInstruction (MiniMC::Model::InstructionCode::Call,args);

	stmts.push (std::make_shared<InstructionStatement> (instr,sinfo));
	return *this;
      }
      
    }
  }
}
