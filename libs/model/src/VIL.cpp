#include "minimc/model/VIL.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/source.hpp"
#include "minimc/model/variables.hpp"
#include <algorithm>


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
	return *this;
      }
      StatementBuilder& StatementBuilder::Branch (std::size_t nb, MiniMC::Model::SourceInfo sinfo){
	std::vector<Statement_ptr> statements;
	for (std::size_t i = 0; i < nb; ++i) {
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

      VILtoCFA::~VILtoCFA () {}
      
      struct VILtoCFA::Internal {
	Internal (MiniMC::Model::Frame frame) : frame(frame) {}
	MiniMC::Model::Frame frame;
	MiniMC::Model::CFA cfa;
	MiniMC::Model::LocationInfoCreator infc;
	MiniMC::Model::Location_ptr start;
	MiniMC::Model::Location_ptr end;
      };

      MiniMC::Model::CFA VILtoCFA::convert (const Statement& s, MiniMC::Model::Frame f) {
	Internal loc{f};
	_internal=&loc;
	loc.start = loc.cfa.makeLocation (f.makeFresh("Start"),loc.infc.make());
	loc.cfa.setInitial (loc.start);
	s.accept(*this);
	
	return std::move(loc.cfa);
      }
									  
	
      
      void VILtoCFA::visitInstructionStatement (const InstructionStatement& s) {
	MiniMC::Model::InfoResetter reset (_internal->infc);
	_internal->infc.setSource (s.getSource());
	MiniMC::Model::InstructionStream stream;
	stream.add (s.getInstruction());
	_internal->end = _internal->cfa.makeLocation (_internal->frame.makeFresh (),_internal->infc.make ());
	_internal->cfa.makeEdge (_internal->start,_internal->end,std::move(stream),false);
	//_internal->start = _internal->cfa.makeLocation (
      }

      void VILtoCFA::visitSequenceStatement (const SequenceStatement& s)  {
	s.getLeft().accept(*this);
	_internal->start = _internal->end;
	s.getRight().accept(*this);
	
      }

      void VILtoCFA::visitBranchStatement (const BranchStatement& bs)  {
	MiniMC::Model::InfoResetter reset (_internal->infc);
	auto start = _internal->start;
	_internal->infc.setSource (bs.getSource());

	std::vector<Location_ptr> ends;

	for (auto& b :  bs.getBranches()) {
	  
	  _internal->start = start;
	  b->accept(*this);
	  ends.push_back(_internal->end);
	  
	}
	_internal->end =  _internal->cfa.makeLocation (_internal->frame.makeFresh (),_internal->infc.make ());

	for (auto& e : ends) {
	  _internal->cfa.makeEdge (e,_internal->end,{});
	}
	
      }

      void VILtoCFA::visitLoopStatement (const LoopStatement& ls) {
	MiniMC::Model::InfoResetter reset (_internal->infc);
	_internal->infc.setSource (ls.getSource());

	auto start = _internal->start;
	auto cont = _internal->cfa.makeLocation (_internal->frame.makeFresh (),_internal->infc.make ());
	auto done = _internal->cfa.makeLocation (_internal->frame.makeFresh (),_internal->infc.make ());
	
	
	_internal->start = cont;
	ls.getStatement()->accept(*this);
	_internal->cfa.makeEdge (_internal->end,start,{});
	
	
	{
	  MiniMC::Model::InstructionStream stream;
	  stream.add<MiniMC::Model::VMInstructionCode::Assume> (ls.getCondition());
	  _internal->cfa.makeEdge (start,cont,std::move(stream));
	}
	{
	  MiniMC::Model::InstructionStream stream;
	  MiniMC::Model::ExpressionBuilder builder;
	  stream.add<MiniMC::Model::VMInstructionCode::Assume> ((builder << ls.getCondition()).LogNot().get());
	  _internal->cfa.makeEdge (start,done ,std::move(stream));
	  _internal->end = done;
	}
	
      }	
      
      
    }
  }
}
