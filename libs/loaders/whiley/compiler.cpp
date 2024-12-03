
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
	std::unordered_map<std::string,MiniMC::Model::Register_ptr> vars;
	std::unique_ptr<MiniMC::Model::LocationInfoCreator> locinfo;
      };
	
      Compiler::Compiler (	MiniMC::Model::TypeFactory_ptr tfac,
				MiniMC::Model::ConstantFactory_ptr cfac) : tfac(std::move(tfac)),cfac(std::move(cfac)) {}

      Compiler::~Compiler() {}

      
      MiniMC::Model::Program Compiler::compile (const ::Whiley::Program& prgm) {
	_internal = std::make_unique<Internal> ();

	//Make variables
	auto type = tfac->makeIntegerType (8);
	auto rootFrame = _internal->prgm.getRootFrame();
	
	auto heap = rootFrame.makeSymbol ("heap");
	_internal->prgm.getHeapLayout ().addBlock (heap,MiniMC::Model::pointer_t::makeHeapPointer(0,0),256);
	_internal->heap_pointer = cfac->makeSymbolicConstant (heap);
	_internal->heap_pointer->setType(tfac->makePointerType ());
	
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
	_internal->locinfo = std::make_unique<MiniMC::Model::LocationInfoCreator> (descr);
	_internal->start = _internal->cfa.makeLocation (_internal->frame.makeFresh ("start"),_internal->locinfo->make({}));
	_internal->end =  _internal->cfa.makeLocation (_internal->frame.makeFresh ("end"),_internal->locinfo->make({}));
	_internal->cfa.setInitial (_internal->start);
	prgm.getStmt().accept (*this);

	_internal->prgm.addFunction (func_name,{},tfac->makeVoidType(),std::move(descr),std::move(_internal->cfa),false,_internal->frame); 
	
	_internal->prgm.addEntryPoint (func_name);
	
	return std::move(_internal->prgm);
      }
	
      void Compiler::visitIdentifier (const Whiley::Identifier& id)  {
	_internal->expr = _internal->vars.at(id.getName());
      }

      void Compiler::visitNumberExpression (const Whiley::NumberExpression& n )  {
	_internal->expr = cfac->makeIntegerConstant (n.getValue(),MiniMC::Model::TypeID::I8);
      } 

      void Compiler::visitDerefExpression (const Whiley::DerefExpression& a)  {
	a.getMem().accept (*this);
	auto convert_loc = std::make_shared<MiniMC::Model::ZExtExpr> (_internal->expr,tfac->makeIntegerType (64));
	auto ones = cfac->makeIntegerConstant (1,MiniMC::Model::TypeID::I64);
	auto ptr = std::make_shared<MiniMC::Model::PtrAddExpr> (_internal->heap_pointer,convert_loc,ones); 
	
	_internal->expr = std::make_shared<MiniMC::Model::LoadExpr> (ptr,tfac->makeIntegerType (8));
      }

      
      void Compiler::visitCastExpression (const Whiley::CastExpression& a)  {
	a.getExpression().accept(*this);
	auto expr = _internal->expr;
	if (a.getType () != a.getExpression ().getType()) {
	  auto one = cfac->makeIntegerConstant (1,MiniMC::Model::TypeID::I8);
	  if (a.getType () == Whiley::Type::SI8) {
	    //Signed to unsigned
	    auto sub = std::make_shared<MiniMC::Model::SubExpr> (expr,one);
	    _internal->expr = std::make_shared<MiniMC::Model::NotExpr> (sub);
	  }

	  else  {
	    //unsigned to nsigned
	    auto negexpr = std::make_shared<MiniMC::Model::NotExpr> (expr);
	    _internal->expr = std::make_shared<MiniMC::Model::AddExpr> (negexpr,one);
	  }
	  
	  
	}
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
	    _internal->expr = std::make_shared<MiniMC::Model::ICMP_SLEExpr> (std::move(le),std::move(right));
	  else
	    _internal->expr = std::make_shared<MiniMC::Model::ICMP_ULEExpr> (std::move(le),std::move(right));
	  
	  break;
	case Whiley::BinOps::GEq:
	  if (_signed)
	    _internal->expr = std::make_shared<MiniMC::Model::ICMP_SGEExpr> (std::move(le),std::move(right));
	  else
	    _internal->expr = std::make_shared<MiniMC::Model::ICMP_UGEExpr> (std::move(le),std::move(right));
	  break;
	case Whiley::BinOps::Lt:
	  if (_signed)
	    _internal->expr = std::make_shared<MiniMC::Model::ICMP_SLTExpr> (std::move(le),std::move(right));
	  else
	    _internal->expr = std::make_shared<MiniMC::Model::ICMP_ULTExpr> (std::move(le),std::move(right));
	  
	  break;
	case Whiley::BinOps::Gt:
	  if(_signed)
	    _internal->expr = std::make_shared<MiniMC::Model::ICMP_SGTExpr> (std::move(le),std::move(right));
	  else
	    _internal->expr = std::make_shared<MiniMC::Model::ICMP_UGTExpr> (std::move(le),std::move(right));
	  
	  break;
      case Whiley::BinOps::Eq:
	_internal->expr = std::make_shared<MiniMC::Model::ICMP_EQExpr> (std::move(le),std::move(right));
	break;
	case Whiley::BinOps::NEq:
	  _internal->expr = std::make_shared<MiniMC::Model::ICMP_NEQExpr> (std::move(le),std::move(right));
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
      void Compiler::visitNonDetAssignStatement (const Whiley::NonDetAssignStatement& nd)  {
      	_internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	auto reg = _internal->vars.at(nd.getAssignName());
	auto undef = cfac->makeUndef (MiniMC::Model::TypeID::I8);
	MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};
	builder.addInstr<MiniMC::Model::InstructionCode::Assign> (reg,undef);
      } 
      
      void Compiler::visitIfStatement (const Whiley::IfStatement& )  {
	_internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};
	
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
	auto convert_loc = std::make_shared<MiniMC::Model::ZExtExpr> (_internal->expr,tfac->makeIntegerType (64));
	auto ones = cfac->makeIntegerConstant (1,MiniMC::Model::TypeID::I64);
	auto ptr = std::make_shared<MiniMC::Model::PtrAddExpr> (_internal->heap_pointer,convert_loc,ones); 
	a.getExpression ().accept (*this);

	builder.addInstr<MiniMC::Model::InstructionCode::Store> (ptr,_internal->expr);
	
      }

    }
  }
}
