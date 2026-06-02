#ifndef _MINIMC_VIL__
#define _MINIMC_VIL__

#include <memory>
#include <vector>


#include "minimc/model/source.hpp"
#include "minimc/model/instructions.hpp"


namespace MiniMC {
  namespace Model {
    namespace VIL {
      class Statement {
      public:
	Statement (MiniMC::Model::SourceInfo sinfo) : source(sinfo) {}
	~Statement () {}
      private:
	MiniMC::Model::SourceInfo source;
      };

      using Statement_ptr = std::shared_ptr<Statement>;
      
      class InstructionStatement : public Statement {
      public:
	InstructionStatement (MiniMC::Model::Instruction instruction, MiniMC::Model::SourceInfo sinfo) : Statement(sinfo),instruction(instruction) {} 
      private:
	MiniMC::Model::Instruction instruction;
      };

      class SequenceStatement : public Statement {
      public:
	SequenceStatement (Statement_ptr l, Statement_ptr r, MiniMC::Model::SourceInfo sinfo) : Statement(sinfo),left(std::move(l)),right(std::move(r)) {}
	
      private:
	Statement_ptr left;
	Statement_ptr right;
      };
      
      class BranchStatement : public Statement {
      public:
	BranchStatement (std::vector<Statement_ptr> stmts,  MiniMC::Model::SourceInfo sinfo) : Statement(sinfo),stmts(std::move(stmts)) {}

      private:
	std::vector<Statement_ptr> stmts;
      };

      class LoopStatement : public Statement {
      public:
	LoopStatement (Statement_ptr stmt, MiniMC::Model::Value_ptr val, MiniMC::Model::SourceInfo info) : Statement(info), stmt(std::move(stmt)), condition(std::move(val)) {}
      private:
	Statement_ptr stmt;
	MiniMC::Model::Value_ptr condition;
      };
      
      class StatementBuilder : public MiniMC::Model::ExpressionBuilder {
      public:
	StatementBuilder& Assign (MiniMC::Model::Register_ptr r, MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& RetVoid (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Ret (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Assume (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Assert (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Loop (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& If (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Branch (std::size_t, MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Call (MiniMC::Model::Register_ptr, std::size_t, MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Sequence (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& operator<< (Statement_ptr ptr) {stmts.push(ptr);return *this;}
      private:
	
	std::stack<Statement_ptr> stmts;
      };
      
    }
  }
}

#endif 
