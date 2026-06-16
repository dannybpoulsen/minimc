#ifndef _MINIMC_VIL__
#define _MINIMC_VIL__

#include <memory>
#include <vector>

#include "minimc/io/ostream.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/model/source.hpp"
#include "minimc/model/instructions.hpp"


namespace MiniMC {
  namespace Model {
    namespace VIL {

      class Statement;
      class InstructionStatement;
      class SequenceStatement;
      class BranchStatement;
      class LoopStatement;
      class InstructionSequence;
      
      class StatementVisitor {
      public:
	virtual ~StatementVisitor() {}
	virtual void visitInstructionStatement (const InstructionStatement&) = 0;
	virtual void visitInstructionSequence (const InstructionSequence&) = 0;
	virtual void visitSequenceStatement (const SequenceStatement&) = 0;
	virtual void visitBranchStatement (const BranchStatement&) = 0;
	virtual void visitLoopStatement (const LoopStatement&) = 0;	
	
      };

      
      
      class Statement {
      public:
	Statement (MiniMC::Model::SourceInfo sinfo) : source(sinfo) {}
	~Statement () {}
	virtual MiniMC::IO::ostream& output (MiniMC::IO::ostream& ) const = 0;
	virtual void accept (StatementVisitor& ) const = 0;
	auto getSource () const {return source;}
      private:
	MiniMC::Model::SourceInfo source;
      };

      using Statement_ptr = std::shared_ptr<Statement>;
      
      class InstructionStatement : public Statement {
      public:
	InstructionStatement (MiniMC::Model::Instruction instruction, MiniMC::Model::SourceInfo sinfo) : Statement(sinfo),instruction(instruction) {}
	virtual MiniMC::IO::ostream& output (MiniMC::IO::ostream& o) const {return o << instruction;} 
	void accept (StatementVisitor& sv) const override {sv.visitInstructionStatement (*this);}
	Instruction getInstruction () const {return instruction;}
      private:
	MiniMC::Model::Instruction instruction;
      };

      class InstructionSequence : public Statement {
      public:
	InstructionSequence (MiniMC::Model::InstructionStream instruction,  MiniMC::Model::SourceInfo sinfo) : Statement(sinfo),instruction(instruction) {}
	virtual MiniMC::IO::ostream& output (MiniMC::IO::ostream& o) const {return o << instruction;} 
	void accept (StatementVisitor& sv) const override {sv.visitInstructionSequence (*this);}
	const InstructionStream& getInstructions () const {return instruction;}
      private:
	MiniMC::Model::InstructionStream instruction;
      };
      
      class SequenceStatement : public Statement {
      public:
	SequenceStatement (Statement_ptr l, Statement_ptr r, MiniMC::Model::SourceInfo sinfo) : Statement(sinfo),left(std::move(l)),right(std::move(r)) {}

	virtual MiniMC::IO::ostream& output (MiniMC::IO::ostream& o) const {
	  left-> output(o);
	  o << " ";
	  right->output(o);
	  return o;
	} 

	void accept (StatementVisitor& sv) const override {sv.visitSequenceStatement (*this);}

	auto& getLeft () const {return *left;}
	auto& getRight () const {return *right;}
	
	
      private:
	Statement_ptr left;
	Statement_ptr right;
      };
      
      class BranchStatement : public Statement {
      public:
	BranchStatement (std::vector<Statement_ptr> stmts,  MiniMC::Model::SourceInfo sinfo) : Statement(sinfo),stmts(std::move(stmts)) {}

	virtual MiniMC::IO::ostream& output (MiniMC::IO::ostream& o) const {
	  o << "[ ";
	  for (auto& s : stmts) {
	    o<< "{";
	    s -> output(o) << "} ";
	  }
	  return o << "]";
	} 

	void accept (StatementVisitor& sv) const override {sv.visitBranchStatement (*this);}
	
	auto& getBranches () const {return stmts;}
	
      private:
	std::vector<Statement_ptr> stmts;
      };

      class LoopStatement : public Statement {
      public:
	LoopStatement (Statement_ptr stmt, MiniMC::Model::Value_ptr val, MiniMC::Model::SourceInfo info) : Statement(info), stmt(std::move(stmt)), condition(std::move(val)) {}
	virtual MiniMC::IO::ostream& output (MiniMC::IO::ostream& o) const {
	  o << " < " << *condition << " > {";
	  stmt->output(o) << "}";
	  return o;
	}

	void accept (StatementVisitor& sv) const override {sv.visitLoopStatement (*this);}

	auto& getStatement() const {return stmt;}
	auto& getCondition () const {return condition;}
	
      private:
	Statement_ptr stmt;
	MiniMC::Model::Value_ptr condition;
      };
      
      class StatementBuilder : public MiniMC::Model::ExpressionBuilder {
      public:
	Statement_ptr getStatement() { auto st = stmts.top();stmts.pop();return st;}
	StatementBuilder& Assign (MiniMC::Model::Register_ptr r, MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& RetVoid (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Ret (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Assume (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Assert (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& InstrSequence (MiniMC::Model::SourceInfo sinfo = {});
	
	StatementBuilder& Loop (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& If (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Branch (std::size_t, MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Call (MiniMC::Model::Register_ptr, std::size_t, MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& Skip (MiniMC::Model::SourceInfo sinfo = {});
	
	StatementBuilder& Sequence (MiniMC::Model::SourceInfo sinfo = {});
	StatementBuilder& operator<< (Statement_ptr ptr) {stmts.push(ptr);return *this;}
      private:
	
	std::stack<Statement_ptr> stmts;
	std::vector<MiniMC::Model::Instruction> instrs;
      };

      class VILtoCFA : private StatementVisitor {
      public:
	~VILtoCFA ();
	MiniMC::Model::CFA convert (const Statement&, MiniMC::Model::Frame);
	void visitInstructionStatement (const InstructionStatement&) override;
	void visitSequenceStatement (const SequenceStatement&) override;
	void visitBranchStatement (const BranchStatement&) override;
	void visitLoopStatement (const LoopStatement&) override;	
	void visitInstructionSequence (const InstructionSequence&) override;
      private:
	struct Internal;
	Internal* _internal;
      };
      
    }
  }
}

#endif 
