
#include "minimc/model/cfg.hpp"
#include "minimc/model/source.hpp"
#include "minimc/model/builder.hpp"

#include "minimc/model/symbol.hpp"
#include "minimc/model/types.hpp"
#include "minimc/model/variables.hpp"
#include "whiley/ast.hpp"
#include "compiler.hpp"

#include <unordered_map>
#include <memory>
#include <variant>

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
	
	//std::unordered_map<std::string,MiniMC::Model::Register_ptr> vars;
	std::unique_ptr<MiniMC::Model::LocationInfoCreator> locinfo;
      };
	
      Compiler::Compiler ()  {}

      Compiler::~Compiler() {}

      auto makeType (Whiley::Type t) {
	switch(t) {
	case Whiley::Type::SI8:
	case Whiley::Type::UI8:
	  return MiniMC::Model::I8Type::get();
	case Whiley::Type::SI16:
	case Whiley::Type::UI16:
	  return MiniMC::Model::I16Type::get();
	case Whiley::Type::SI32:
	case Whiley::Type::UI32:
	  return MiniMC::Model::I32Type::get();
	case Whiley::Type::SI64:
	case Whiley::Type::UI64:
	  return MiniMC::Model::I64Type::get();
	
	case Whiley::Type::Pointer:
	    return MiniMC::Model::PointerType::get();
	
	default:
	  return MiniMC::Model::I8Type::get();
	}
      };
      

      
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
	
	std::vector<MiniMC::Model::Register_ptr> params;

	auto main_func_frame = rootFrame.create ("_main");
	auto func_name = rootFrame.makeSymbol ("_main");
	MiniMC::Model::RegisterDescr descr;
	
	_internal->locinfo = std::make_unique<MiniMC::Model::LocationInfoCreator> (descr,main_func_frame);
	_internal->start = _internal->cfa.makeLocation (_internal->frame.makeFresh ("_iniit"),_internal->locinfo->make({}));
	_internal->end =  _internal->cfa.makeLocation (_internal->frame.makeFresh ("_end"),_internal->locinfo->make({}));
	auto end_init = _internal->end;
	_internal->cfa.setInitial (_internal->start);
	{
	  MiniMC::Model::EdgeBuilder edgebuilder {_internal->cfa,_internal->start,_internal->end,main_func_frame,false};
	
	
	  for (auto var : prgm.getVars ()) {
	    std::string name = var.getName();
	    MiniMC::Model::Frame frame = rootFrame;
	    auto symbol = rootFrame.makeSymbol (name);
	    auto reg = _internal->prgm.getCPURegs().addRegister (std::move(symbol),makeType(var.getType()));  
	    if (var.isParamter()) {
	      auto psymbol = main_func_frame.makeSymbol (name);
	      auto preg = descr.addRegister (std::move(psymbol),makeType(var.getType()));
	      params.push_back (preg);
	      edgebuilder.addInstr<MiniMC::Model::InstructionCode::Assign> (reg,preg);
	    }
	  }
	}
	auto main_func_cfa = std::move(_internal->cfa);
	auto loc_info = std::move(_internal->locinfo);
	
	_internal->frame = rootFrame;
	for (auto var : prgm.getFrame().getLocalSymbols()) {
	  if (std::holds_alternative<Whiley::Function_ptr> (var.getUserData()))
	    _internal->frame.makeSymbol (var.getName());
	}

	for (auto var : prgm.getFrame().getLocalSymbols()) {
	  if (std::holds_alternative<Whiley::Function_ptr> (var.getUserData()))
	    writeFunction(var);
	}
	
	
	
	_internal->frame = main_func_frame;
	_internal->start = end_init;
	_internal->locinfo = std::move(loc_info);
	_internal->cfa = std::move(main_func_cfa);
	_internal->end =  _internal->cfa.makeLocation (_internal->frame.makeFresh ("end"),_internal->locinfo->make({}));
	
	
	
	prgm.getStmt().accept (*this);

	_internal->prgm.addFunction (func_name,params,MiniMC::Model::VoidType::get(),std::move(descr),std::move(_internal->cfa),false,_internal->frame); 
	
	_internal->prgm.addEntryPoint (func_name);
	
	return std::move(_internal->prgm);
      }

      void Compiler::writeFunction (Whiley::Symbol symb) {
	std::vector<MiniMC::Model::Register_ptr> params;
	MiniMC::Model::Symbol func_name;
	_internal->frame.resolve (symb.getName(),func_name);
	_internal->frame = _internal->frame.create(symb.getName());
	MiniMC::Model::RegisterDescr descr;
	auto wh_func = std::get<Whiley::Function_ptr> (symb.getUserData());

	for (auto s : wh_func->getFrame().getLocalSymbols()) {
	  std::visit (Whiley::overloaded {
	      [&s,this,&descr](const Whiley::VarDecl& decl) {
		std::string name = s.getName();
		auto symbol = _internal->frame.makeSymbol (name);
		auto reg = descr.addRegister (std::move(symbol),makeType(decl.type));
	      },
		[&s,this,&descr](const Whiley::ParamDecl& decl) {
		std::string name = s.getName();
		auto symbol = _internal->frame.makeSymbol (name);
		auto reg = descr.addRegister (std::move(symbol),makeType(decl.type));
		},
		[] (auto&) {}
	    },
	    s.getUserData()
	    );
	}

	for (auto h : wh_func->getParams()) {
	  MiniMC::Model::Symbol s;
	  _internal->frame.resolve(h.getName(),s);
	  params.push_back(std::get<MiniMC::Model::Register_wptr> (s.getUserData()).lock());
	}
	
	_internal->locinfo = std::make_unique<MiniMC::Model::LocationInfoCreator> (descr,_internal->frame);
	_internal->start = _internal->cfa.makeLocation (_internal->frame.makeFresh ("start"),_internal->locinfo->make({}));
	_internal->end =  _internal->cfa.makeLocation (_internal->frame.makeFresh ("end"),_internal->locinfo->make({}));
	_internal->cfa.setInitial (_internal->start);
	wh_func->getStmt()->accept (*this);

	_internal->prgm.addFunction (func_name,params,makeType(wh_func->returns()),std::move(descr),std::move(_internal->cfa),false,_internal->frame); 
	
	_internal->frame = _internal->frame.close();
      }
      
      void Compiler::visitIdentifier (const Whiley::Identifier& id)  {
	MiniMC::Model::Symbol symb;
	if (_internal->frame.resolve(id.getName(),symb)) {
	  auto reg = std::get<MiniMC::Model::Register_wptr> (symb.getUserData()).lock();
	  
	  _internal->expr = reg;//MiniMC::Model::makeExpr<MiniMC::Model::SymbolicConstant> (symb);
	}
	//_internal->expr = _internal->vars.at(id.getName());
      }

      void Compiler::visitNumberExpression (const Whiley::NumberExpression& n )  {
	_internal->expr = MiniMC::Model::I64Integer::make(n.getValue());
      } 

      void Compiler::visitAllocStatement (const Whiley::AllocStatement& alloc)  {
	_internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};
	MiniMC::Model::Symbol symb;
	if (_internal->frame.resolve(alloc.getAssignName(),symb)) {
	  auto reg = std::get<MiniMC::Model::Register_wptr> (symb.getUserData()).lock();
	  alloc.getExpression().accept (*this);
	  
	  builder.addInstr<MiniMC::Model::InstructionCode::Assign> (reg,std::make_shared<MiniMC::Model::FindSpaceExpr> (_internal->heap_mem,_internal->expr));
	  builder.addInstr<MiniMC::Model::InstructionCode::Assume> (std::make_shared<MiniMC::Model::CheckFreeExpr> (_internal->heap_mem,reg,_internal->expr));
	  builder.addInstr<MiniMC::Model::InstructionCode::Assign> (_internal->heap_mem,std::make_shared<MiniMC::Model::AllocExpr> (_internal->heap_mem,reg,_internal->expr));
	}
	
	
      }

      void Compiler::visitFreeStatement (const Whiley::FreeStatement& free)  {
	_internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	free.getExpression().accept (*this);
	  
	
	MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};
	builder.addInstr<MiniMC::Model::InstructionCode::Assign> (_internal->heap_mem,std::make_shared<MiniMC::Model::FreeExpr> (_internal->heap_mem,_internal->expr));

      }
      
      
      void Compiler::visitDerefExpression (const Whiley::DerefExpression& a)  {
	a.getMem().accept (*this);
	/*auto convert_loc = std::make_shared<MiniMC::Model::ZExtExpr> (_internal->expr,MiniMC::Model::I64Type::get());*/
	//auto ptr = std::make_shared<MiniMC::Model::PtrAddExpr> (_internal->heap_pointer,_internal->expr); 
	
	_internal->expr = std::make_shared<MiniMC::Model::LoadExpr> (_internal->heap_mem,_internal->expr,makeType(a.getLoadType()));
      }

      
      void Compiler::visitCastExpression (const Whiley::CastExpression& a)  {
	a.getExpression().accept(*this);
	if (Whiley::bytesize(a.getType ()) == Whiley::bytesize(a.getExpression().getType()))
	  return;
	else {
	  MiniMC::Model::ExpressionBuilder builder;
	  builder << _internal->expr;
	  builder << makeType(a.getType());
	  if (Whiley::bytesize(a.getType ()) > Whiley::bytesize(a.getExpression().getType())) {
	    if (isSigned (a.getType()))
	      builder.SExt ();
	    else
	      builder.ZExt ();
	    
	  }
	  else
	    builder.Trunc();
	  _internal->expr = builder.get();
	}
      }

      void Compiler::visitUndefExpression (const Whiley::UndefExpression& a)  {
	_internal->expr = MiniMC::Model::Undef::make(makeType(a.getType()));//cfac->makeUndef (MiniMC::Model::TypeID::I8);
      }

      
      void Compiler::visitBinaryExpression (const Whiley::BinaryExpression& be)  {
	be.getLeft ().accept (*this);
	auto le = _internal->expr;
	be.getRight ().accept (*this);
	auto right = _internal->expr;
	bool _signed = isSigned(be.getLeft().getType ()) ||
	  isSigned(be.getRight().getType ());
	MiniMC::Model::ExpressionBuilder builder;
	builder << le << right;
	switch (be.getOp ()) {
	case Whiley::BinOps::Add:
	  if (be.getLeft().getType() == Whiley::Type::Pointer) {
	    //_internal->expr = std::make_shared<MiniMC::Model::PtrAddExpr> (std::move(le),std::move(right));
	    builder.PtrAdd ();
	  }
	  else {
	    builder.Add();
	  }
	    break;
	case Whiley::BinOps::Sub:
	  builder.Sub();
	  break;
	case Whiley::BinOps::Mul:
	  builder.Mul();
	  break;
	case Whiley::BinOps::Xor:
	  builder.Xor();
	  break;
	case Whiley::BinOps::Or:
	  builder.Or();
	  break;
	case Whiley::BinOps::And:
	  builder.And();
	  break;
	case Whiley::BinOps::LShl:
	  builder.LShl();
	  break;
	case Whiley::BinOps::Mod:
	  throw MiniMC::Support::Exception {"Modulo operations not supported in MiniMC"};
	break;
	case Whiley::BinOps::Div:
	  if (_signed)
	    builder.SDiv();
	  
	  else
	    builder.UDiv();
	  break;
	  case Whiley::BinOps::LEq:
	    if (_signed)
	      //_internal->expr = std::make_shared<MiniMC::Model::SLeExpr> (std::move(le),std::move(right));
	      builder.SLe().pushI8Type().ZExt();
	    else
	      //_internal->expr = std::make_shared<MiniMC::Model::ULeExpr> (std::move(le),std::move(right));
	      builder.ULe().pushI8Type().ZExt();
	    break;
	    case Whiley::BinOps::GEq:
	      if (_signed)
		builder.SGe().pushI8Type().ZExt();
		//_internal->expr = std::make_shared<MiniMC::Model::SGeExpr> (std::move(le),std::move(right));
	      else
		builder.UGe().pushI8Type().ZExt();
	      //_internal->expr = std::make_shared<MiniMC::Model::UGeExpr> (std::move(le),std::move(right));
	      break;
	      case Whiley::BinOps::Lt:
		if (_signed)
		  //_internal->expr = std::make_shared<MiniMC::Model::SLtExpr> (std::move(le),std::move(right));
		  builder.SLt().pushI8Type().ZExt();
		else
		    builder.ULt().pushI8Type().ZExt();
		
		//_internal->expr = std::make_shared<MiniMC::Model::ULtExpr> (std::move(le),std::move(right));
	  
		break;
		case Whiley::BinOps::Gt:
		  if(_signed)
		    //_internal->expr = std::make_shared<MiniMC::Model::SGtExpr> (std::move(le),std::move(right));
		    builder.SGt().pushI8Type().ZExt();
		  else
		    //_internal->expr = std::make_shared<MiniMC::Model::UGtExpr> (std::move(le),std::move(right));
		    builder.UGt().pushI8Type().ZExt();
		  
		  break;
		  case Whiley::BinOps::Eq:
		    //_internal->expr = std::make_shared<MiniMC::Model::EqExpr> (std::move(le),std::move(right));
		    builder.Eq().pushI8Type().ZExt();
		    break;
		    case Whiley::BinOps::NEq:
		      //_internal->expr = std::make_shared<MiniMC::Model::NEqExpr> (std::move(le),std::move(right));
		      builder.NEq().pushI8Type().ZExt();
		      break;
		      
	  
	}
	_internal->expr = builder.get();
    }
        
	
    void Compiler::visitAssignStatement (const Whiley::AssignStatement& ass)  {
      _internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
      MiniMC::Model::Symbol symb;
      _internal->frame.resolve (ass.getAssignName(),symb);
      auto reg = std::get<MiniMC::Model::Register_wptr> (symb.getUserData()).lock();
      ass.getExpression ().accept(*this);
      
      MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};

      builder.addInstr<MiniMC::Model::InstructionCode::Assign> (reg,_internal->expr);
	
    }

    void Compiler::visitIncrementDecrementStatement (const Whiley::IncrementDecrementStatement& ass)  {
      _internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
      MiniMC::Model::Symbol symb;
      _internal->frame.resolve (ass.getIncrementee(),symb);
      auto reg = std::get<MiniMC::Model::Register_wptr> (symb.getUserData()).lock();
      //ass.getExpression ().accept(*this);
      MiniMC::Model::ExpressionBuilder exprbuilder;
      exprbuilder << reg;
      switch (reg->getType()->getTypeID()) {
      case MiniMC::Model::TypeID::I8:
	exprbuilder.I8 (1);
	break;
      case MiniMC::Model::TypeID::I16:
	exprbuilder.I16 (1);
	break;
      case MiniMC::Model::TypeID::I32:
	exprbuilder.I32 (1);
	break;
      case MiniMC::Model::TypeID::I64:
	exprbuilder.I64 (1);
	break;
      default:
	throw MiniMC::Support::Exception ("Can't increment expression");
      }
      exprbuilder.Add ();
      
      MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};

      builder.addInstr<MiniMC::Model::InstructionCode::Assign> (reg,exprbuilder.get());
	
    }
      
    void Compiler::visitAssertStatement (const Whiley::AssertStatement& a)  {
      _internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
      a.getExpression().accept (*this);

      MiniMC::Model::ExpressionBuilder ebuilder;
      ebuilder << _internal->expr;
      ebuilder.pushBoolType();
      ebuilder.IntToBool();
      MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};

      builder.addInstr<MiniMC::Model::InstructionCode::Assert> (ebuilder.get());
    } 
    void Compiler::visitAssumeStatement (const Whiley::AssumeStatement& a)  {
      _internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
      a.getExpression().accept (*this);

	
      MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};

      MiniMC::Model::ExpressionBuilder ebuilder;
      ebuilder << _internal->expr;
      ebuilder.pushBoolType();
      ebuilder.IntToBool();
      
      builder.addInstr<MiniMC::Model::InstructionCode::Assume> (ebuilder.get());
      
	
    } 

    void Compiler::visitChooseStatement (const Whiley::ChooseStatement& iff )  {
      auto start = _internal->start;
      auto done_loc = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	
      for (auto& stmt : iff.getStatements()) {
	auto nstart = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	  
	{
	  MiniMC::Model::EdgeBuilder  b {_internal->cfa,start,nstart,_internal->frame,false};
	}
	  
	  _internal->start = nstart;
	  stmt->accept(*this);
	  MiniMC::Model::EdgeBuilder builder2 {_internal->cfa,_internal->end,done_loc,_internal->frame,false};
	}
	_internal->end = done_loc;
      }
      void Compiler::visitIfStatement (const Whiley::IfStatement& iff )  {
	iff.getCondition ().accept(*this);
	auto cond = std::make_shared<MiniMC::Model::IntToBoolExpr>(_internal->expr,MiniMC::Model::BoolType::get()); 
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

	auto cond = std::make_shared<MiniMC::Model::IntToBoolExpr>(_internal->expr,MiniMC::Model::BoolType::get());
	
	auto exec_loop  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	
	auto loop_done  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	
	
	{
	  MiniMC::Model::EdgeBuilder exec_builder {_internal->cfa,_internal->start,exec_loop,_internal->frame,false};
	  exec_builder.addInstr<MiniMC::Model::InstructionCode::Assume> (cond);
	}
	  
	{
	  
	  MiniMC::Model::EdgeBuilder non_exec_builder {_internal->cfa,_internal->start,loop_done,_internal->frame,false};
	  non_exec_builder.addInstr<MiniMC::Model::InstructionCode::Assume> (std::make_shared<MiniMC::Model::LogNotExpr> (cond));
	}
	auto loop_head = _internal->start;
	_internal->start = exec_loop;
	w.getBody().accept(*this);
	{
	  MiniMC::Model::EdgeBuilder jump_back_builder {_internal->cfa,_internal->end,loop_head,_internal->frame,false};
	}
	
	_internal->end  = loop_done;
	
      }

      void Compiler::visitReturnStatement (const Whiley::ReturnStatement& ret)   {
	_internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};
	ret.getExpr().accept(*this);
	builder.addInstr<MiniMC::Model::InstructionCode::Ret> (_internal->expr);
      }
      
      void Compiler::visitCallStatement (const Whiley::CallStatement& c)  {
	_internal->end  = _internal->cfa.makeLocation (_internal->frame.makeFresh(),_internal->locinfo->make ({}));
	MiniMC::Model::EdgeBuilder builder {_internal->cfa,_internal->start,_internal->end,_internal->frame,false};
	MiniMC::Model::Symbol symb;
	MiniMC::Model::Symbol func_symb;

	MiniMC::Model::Register_ptr reg = nullptr;
	if (_internal->frame.resolve (c.assignname(),symb))
	  reg = std::get<MiniMC::Model::Register_wptr>(symb.getUserData()).lock();
	
	if (_internal->frame.resolve (c.funcname(),func_symb)) {
	  
	  std::vector<MiniMC::Model::Value_ptr> params;

	  for (auto& a : c.parameters()) {
	    a->accept (*this);
	    params.push_back(_internal->expr);
	  }

	  auto symb_expr = MiniMC::Model::makeExpr<MiniMC::Model::SymbolicConstant> (func_symb); 
	  
	  builder.addInstr<MiniMC::Model::InstructionCode::Call> (reg,symb_expr,params);
	}
	
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
	/*auto convert_loc = std::make_shared<MiniMC::Model::ZExtExpr> (_internal->expr,MiniMC::Model::I64Type::get());*/
	auto ptr = _internal->expr;//std::make_shared<MiniMC::Model::PtrAddExpr> (_internal->heap_pointer,_internal->expr); 
	a.getExpression ().accept (*this);

	builder.addInstr<MiniMC::Model::InstructionCode::Store> (_internal->heap_mem,_internal->heap_mem,ptr,_internal->expr);
	
      }

    }
  }
}
