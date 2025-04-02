
#include "minimc/model/cfg.hpp"
#include "minimc/model/source.hpp"
#include "minimc/model/builder.hpp"

#include "whiley/ast.hpp"
#include "compiler.hpp"

#include <unordered_map>
#include <memory>

namespace MiniMC {
  namespace Loaders {
    namespace whiley {

      struct Compiler::Internal {
	MiniMC::Model::Program prgm;
	MiniMC::Model::CFA cfa;
	MiniMC::Model::Frame frame;
	MiniMC::Model::Location_ptr start;
	MiniMC::Model::Location_ptr end;
	MiniMC::Model::Value_ptr expr;
	MiniMC::Model::Value_ptr heap_pointer;
	MiniMC::Model::Value_ptr heap_mem;
	
	std::unordered_map<std::string,MiniMC::Model::Register_ptr> vars;
	std::unique_ptr<MiniMC::Model::LocationInfoCreator> locinfo;
      };
	
      Compiler::Compiler ()  {}

      Compiler::~Compiler() {}

      
      MiniMC::Model::Program Compiler::compile (const ::Whiley::Program& prgm) {
	_internal = std::make_unique<Internal> ();

	//Make variables
	auto type = MiniMC::Model::I8Type::get();//tfac->makeIntegerType (8);
	auto rootFrame = _internal->prgm.getRootFrame();
	
	_internal->heap_mem = _internal->prgm.getPersistentRegs().addRegister (rootFrame.makeSymbol ("mem"),MiniMC::Model::MemoryType::get());
	
	
	auto heap = rootFrame.makeSymbol ("heap");
	_internal->prgm.getHeapLayout ().addBlock (heap,MiniMC::Model::pointer_t::makeHeapPointer(0,0),256,_internal->heap_mem);
	_internal->heap_pointer = MiniMC::Model::SymbolicConstant::make (heap);
	_internal->heap_pointer->setType(MiniMC::Model::PointerType::get());
	
	auto& register_descr =  _internal->prgm.getCPURegs(); 
	for (auto& var : prgm.getVars ()) {
	  std::string name = var.getName();
	  auto symbol = rootFrame.makeSymbol (name);
	  auto reg = register_descr.addRegister (std::move(symbol),type);
	  _internal->vars.emplace(name,reg);
	}

	
	_internal->frame = rootFrame.create ("_main");
	auto func_name = rootFrame.makeSymbol ("_main");
	MiniMC::Model::RegisterDescr descr;
	_internal->locinfo = std::make_unique<MiniMC::Model::LocationInfoCreator> (descr,_internal->frame);
	_internal->start = _internal->cfa.makeLocation (_internal->frame.makeFresh ("start"),_internal->locinfo->make({}));
	_internal->end =  _internal->cfa.makeLocation (_internal->frame.makeFresh ("end"),_internal->locinfo->make({}));
	_internal->cfa.setInitial (_internal->start);
	prgm.getStmt().accept (*this);

	_internal->prgm.addFunction (func_name,{},MiniMC::Model::VoidType::get(),std::move(descr),std::move(_internal->cfa),false,_internal->frame); 
	
	_internal->prgm.addEntryPoint (func_name);
	
	return std::move(_internal->prgm);
      }
	
      void Compiler::visitIdentifier (const Whiley::Identifier& id)  {
	_internal->expr = _internal->vars.at(id.getName());
      }

      void Compiler::visitNumberExpression (const Whiley::NumberExpression& n )  {
	_internal->expr = MiniMC::Model::I8Integer::make(n.getValue());
      } 

      void Compiler::visitDerefExpression (const Whiley::DerefExpression& a)  {
	a.getMem().accept (*this);
	auto convert_loc = std::make_shared<MiniMC::Model::ZExtExpr> (_internal->expr,MiniMC::Model::I64Type::get());
	auto ones = MiniMC::Model::I64Integer::make(1);//,MiniMC::Model::TypeID::I64);
	auto ptr = std::make_shared<MiniMC::Model::PtrAddExpr> (_internal->heap_pointer,convert_loc,ones); 
	
	_internal->expr = std::make_shared<MiniMC::Model::LoadExpr> (_internal->heap_mem,ptr,MiniMC::Model::I8Type::get());
      }

      
      void Compiler::visitCastExpression (const Whiley::CastExpression& a)  {
	a.getExpression().accept(*this);
      }

      void Compiler::visitUndefExpression (const Whiley::UndefExpression& a)  {
	_internal->expr = MiniMC::Model::Undef::make(MiniMC::Model::I8Type::get());//cfac->makeUndef (MiniMC::Model::TypeID::I8);
      }
      
      void Compiler::visitBinaryExpression (const Whiley::BinaryExpression& be)  {
	be.getLeft ().accept (*this);
	auto le = _internal->expr;
	be.getRight ().accept (*this);
	auto right = _internal->expr;
	bool _signed = be.getLeft().getType () == Whiley::Type::SI8 ||
	  be.getRight().getType () == Whiley::Type::SI8;   
	switch (be.getOp ()) {
	case Whiley::BinOps::Add:
	  _internal->expr = std::make_shared<MiniMC::Model::AddExpr> (std::move(le),std::move(right));
	  break;
	case Whiley::BinOps::Sub:
	  _internal->expr = std::make_shared<MiniMC::Model::SubExpr> (std::move(le),std::move(right));
	  break;
	case Whiley::BinOps::Mul:
	  _internal->expr = std::make_shared<MiniMC::Model::MulExpr> (std::move(le),std::move(right));
	  break;
	case Whiley::BinOps::Div:
	  if (_signed)
	    _internal->expr = std::make_shared<MiniMC::Model::SDivExpr> (std::move(le),std::move(right));
	  else
	    _internal->expr = std::make_shared<MiniMC::Model::UDivExpr> (std::move(le),std::move(right));
	  
	  break;
	case Whiley::BinOps::LEq:
	  if (_signed)
	    _internal->expr = std::make_shared<MiniMC::Model::SLEExpr> (std::move(le),std::move(right));
	  else
	    _internal->expr = std::make_shared<MiniMC::Model::ULEExpr> (std::move(le),std::move(right));
	  
	  break;
	case Whiley::BinOps::GEq:
	  if (_signed)
	    _internal->expr = std::make_shared<MiniMC::Model::SGEExpr> (std::move(le),std::move(right));
	  else
	    _internal->expr = std::make_shared<MiniMC::Model::UGEExpr> (std::move(le),std::move(right));
	  break;
	case Whiley::BinOps::Lt:
	  if (_signed)
	    _internal->expr = std::make_shared<MiniMC::Model::SLtExpr> (std::move(le),std::move(right));
	  else
	    _internal->expr = std::make_shared<MiniMC::Model::ULtExpr> (std::move(le),std::move(right));
	  
	  break;
	case Whiley::BinOps::Gt:
	  if(_signed)
	    _internal->expr = std::make_shared<MiniMC::Model::SGtExpr> (std::move(le),std::move(right));
	  else
	    _internal->expr = std::make_shared<MiniMC::Model::UGtExpr> (std::move(le),std::move(right));
	  
	  break;
      case Whiley::BinOps::Eq:
	_internal->expr = std::make_shared<MiniMC::Model::EqExpr> (std::move(le),std::move(right));
	break;
	case Whiley::BinOps::NEq:
	  _internal->expr = std::make_shared<MiniMC::Model::NEqExpr> (std::move(le),std::move(right));
	  break;
	  
	
	}
      }
        
	
      void Compiler::visitAssignStatement (const Whiley::AssignStatement& ass)  {
	_internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	auto reg = _internal->vars.at(ass.getAssignName());
	ass.getExpression ().accept(*this);
      
	MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};

	builder.addInstr<MiniMC::Model::InstructionCode::Assign> (reg,_internal->expr);
	
      } 
      void Compiler::visitAssertStatement (const Whiley::AssertStatement& a)  {
	_internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	a.getExpression().accept (*this);

	
	MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};

	builder.addInstr<MiniMC::Model::InstructionCode::Assert> (_internal->expr);
      } 
      void Compiler::visitAssumeStatement (const Whiley::AssumeStatement& a)  {
	_internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	a.getExpression().accept (*this);

	
	MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};

	builder.addInstr<MiniMC::Model::InstructionCode::Assume> (_internal->expr);
      
	
      } 
            
      void Compiler::visitIfStatement (const Whiley::IfStatement& iff )  {
	iff.getCondition ().accept(*this);
	auto cond = _internal->expr; 
	auto if_branch = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	auto else_branch = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	auto done_loc = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	auto start = _internal->start;
	{
	  MiniMC::Model::EdgeBuilder builder {_internal->cfa,start,if_branch,_internal->frame,false};
	  builder.addInstr<MiniMC::Model::InstructionCode::Assume>(cond);
	  _internal->start = if_branch;
	  iff.getIfBody().accept(*this);
	  MiniMC::Model::EdgeBuilder builder2 {_internal->cfa,_internal->end,done_loc,_internal->frame,false};
	}

	{
	  MiniMC::Model::EdgeBuilder builder {_internal->cfa,start,else_branch,_internal->frame,false};
	  builder.addInstr<MiniMC::Model::InstructionCode::Assume>(std::make_shared<MiniMC::Model::LogNotExpr> (cond));
	  _internal->start = else_branch;
	  iff.getElseBody().accept(*this);
	  MiniMC::Model::EdgeBuilder builder2 {_internal->cfa,_internal->end,done_loc,_internal->frame,false};
	}
	
	
	
	_internal->end  = done_loc;
	  
      } 
      void Compiler::visitSkipStatement (const Whiley::SkipStatement& )  {
	_internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};
	
      } 
      void Compiler::visitWhileStatement (const Whiley::WhileStatement& w )  {
	w.getCondition().accept(*this);
	
	auto exec_loop  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	
	auto loop_done  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	
	
	{
	  MiniMC::Model::EdgeBuilder exec_builder {_internal->cfa,_internal->start,exec_loop,_internal->frame,false};
	  exec_builder.addInstr<MiniMC::Model::InstructionCode::Assume> (_internal->expr);
	}
	  
	{
	  
	  MiniMC::Model::EdgeBuilder non_exec_builder {_internal->cfa,_internal->start,loop_done,_internal->frame,false};
	  non_exec_builder.addInstr<MiniMC::Model::InstructionCode::Assume> (std::make_shared<MiniMC::Model::LogNotExpr> (_internal->expr));
	}
	auto loop_head = _internal->start;
	_internal->start = exec_loop;
	w.getBody().accept(*this);
	{
	  MiniMC::Model::EdgeBuilder jump_back_builder {_internal->cfa,_internal->end,loop_head,_internal->frame,false};
	}
	
	_internal->end  = loop_done;
	
      } 
      void Compiler::visitSequenceStatement (const Whiley::SequenceStatement& s)  {
	s.getFirst().accept (*this);
	_internal->start = _internal->end;
	s.getSecond().accept (*this);
      } 
      void Compiler::visitMemAssignStatement (const Whiley::MemAssignStatement& a) {
	_internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	
	
	
	MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};
	a.getMemLoc().accept (*this);
	auto convert_loc = std::make_shared<MiniMC::Model::ZExtExpr> (_internal->expr,MiniMC::Model::I64Type::get());
	auto ones = MiniMC::Model::I64Integer::make(1);//cfac->makeIntegerConstant (1,MiniMC::Model::TypeID::I64);
	auto ptr = std::make_shared<MiniMC::Model::PtrAddExpr> (_internal->heap_pointer,convert_loc,ones); 
	a.getExpression ().accept (*this);

	builder.addInstr<MiniMC::Model::InstructionCode::Store> (_internal->heap_mem,_internal->heap_mem,ptr,_internal->expr);
	
      }

    }
  }
}
