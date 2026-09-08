
#include "minimc/model/VIL.hpp"
#include "minimc/model/builder.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/model/source.hpp"

#include "compiler.hpp"
#include "minimc/model/symbol.hpp"
#include "minimc/model/types.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/support/exceptions.hpp"
#include "whiley/ast.hpp"

#include <sstream>
#include <memory>
#include <unordered_map>
#include <variant>
#include "minimc/io/ostream.hpp"

namespace MiniMC {
  namespace Loaders {
    namespace whiley {

      class SourceLocation : public MiniMC::Model::SourceInfo::SourceData{
      public:
	SourceLocation (Whiley::location_t l) : loc(l) {}
	virtual std::ostream& out(std::ostream& os) {return os << loc;}
	virtual MiniMC::IO::ostream& output(MiniMC::IO::ostream& os) const {
	  return os << loc.begin.line << ":" << loc.begin.col; 
	}
	
      private:
	Whiley::location_t loc;
      };

      struct Compiler::Internal {
        MiniMC::Model::Program prgm;
        MiniMC::Model::Frame frame;
	
        MiniMC::Model::Register_ptr heap_mem;
	
	MiniMC::Model::VIL::StatementBuilder builder;
        
      };

      Compiler::Compiler() {}

      Compiler::~Compiler() {}

      auto makeType(Whiley::Type t) {
        switch (t) {
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
      
      MiniMC::Model::Program Compiler::compile(const ::Whiley::Program& prgm) {
	_internal = std::make_unique<Internal>();

	
	_internal->frame = _internal->prgm.getRootFrame();;
	_internal->heap_mem = _internal->prgm.getPersistentRegs().addRegister(_internal->frame.makeSymbol("mem"), MiniMC::Model::MemoryType::get());
	for (auto var : prgm.getVars()) {
	  std::string name = var.getName();
	  auto symbol = _internal->frame.makeSymbol(name);
	  auto reg = _internal->prgm.getCPURegs().addRegister(std::move(symbol), makeType(var.getType()));
	  
	}
	
	
	
        for (auto var : prgm.getFrame().getLocalSymbols()) {
          if (std::holds_alternative<Whiley::Function_ptr>(var.getUserData()))
            _internal->frame.makeSymbol(var.getName());
        }

	for (auto var : prgm.getFrame().getLocalSymbols()) {
          if (std::holds_alternative<Whiley::Function_ptr>(var.getUserData()))
            writeFunction(var);
        }

	
	MiniMC::Model::RegisterDescr descr;
        std::vector<MiniMC::Model::Symbol> params;
	
        auto main_func_symbol = _internal->frame.makeSymbol("_main");
	_internal->frame = _internal->frame.create("_main");
	
	for (auto var : prgm.getVars()) {
	  std::string name = var.getName();
	  
	  auto symb = _internal->frame.resolve(name).value();
	  auto reg = std::get<MiniMC::Model::Register_ptr> (symb.getUserData());
	  if (var.isParamter()) {
	    auto psymbol = _internal->frame.makeFresh(name);
	    auto preg = descr.addRegister(psymbol, makeType(var.getType()));
	    params.push_back(psymbol);
	    static_cast<MiniMC::Model::ExpressionBuilder&>(_internal->builder) << preg;
	    _internal->builder.Assign(reg);
	  }
	  
	}

	prgm.getStmt().accept (*this);
	auto cfa = MiniMC::Model::VIL::VILtoCFA{}.convert(*_internal->builder.getStatement(),_internal->frame);
	_internal->prgm.addFunction(main_func_symbol, params, MiniMC::Model::VoidType::get(), std::move(descr), std::move(cfa), false, _internal->frame);
	_internal->prgm.addEntryPoint(main_func_symbol);
        return std::move(_internal->prgm);
      }

      void Compiler::writeFunction(Whiley::Symbol symb) {
        auto func_name = _internal->frame.resolve(symb.getName()).value();
	MiniMC::Model::RegisterDescr descr;
	
        _internal->frame = _internal->frame.create(symb.getName());
	auto wh_func = std::get<Whiley::Function_ptr>(symb.getUserData());
	for (auto s : wh_func->getFrame().getLocalSymbols()) {
          std::visit(Whiley::overloaded{
	      [&s, this, &descr](const Whiley::VarDecl& decl) {
		std::string name = s.getName();
		auto symbol = _internal->frame.makeSymbol(name);
		auto reg = descr.addRegister(std::move(symbol), makeType(decl.type));
	      },
		[&s, this, &descr](const Whiley::ParamDecl& decl) {
		  std::string name = s.getName();
		  auto symbol = _internal->frame.makeSymbol(name);
		  auto reg = descr.addRegister(std::move(symbol), makeType(decl.type));
		},
		[](auto&) {}},
	    s.getUserData());
        }

	std::vector<MiniMC::Model::Symbol> params;
        for (auto h : wh_func->getParams()) {
          auto s = _internal->frame.resolve(h.getName()).value();
          params.push_back(s);
        }
	wh_func->getStmt()->accept(*this);
	auto cfa = MiniMC::Model::VIL::VILtoCFA{}.convert(*_internal->builder.getStatement(),_internal->frame);
	_internal->prgm.addFunction(func_name, params, makeType(wh_func->returns()), std::move(descr), std::move(cfa), false, _internal->frame);
	
	
	_internal->frame = _internal->frame.close();
	
      }

      void Compiler::visitIdentifier(const Whiley::Identifier& id) {
	
	if (auto symb = _internal->frame.resolve(id.getName())) {
	  
	  auto reg = std::get<MiniMC::Model::Register_ptr>(symb.value().getUserData());
	  
	  auto& exprbuilder = static_cast<MiniMC::Model::ExpressionBuilder&> (_internal->builder);
	  exprbuilder << reg;
	  
        }

	else {
	  _internal->builder.I64(0);
	}

	
      }

      void Compiler::visitNumberExpression(const Whiley::NumberExpression& n) {
	_internal->builder.I64(n.getValue());
      }

      void Compiler::visitAllocStatement(const Whiley::AllocStatement& alloc) {
        auto& builder = static_cast<MiniMC::Model::ExpressionBuilder&> (_internal->builder); 
	alloc.getExpression().accept(*this);
	auto expr = builder.get();
	if (auto symb = _internal->frame.resolve(alloc.getAssignName())) {
	  auto reg = std::get<MiniMC::Model::Register_ptr>(symb.value().getUserData());
	  builder << _internal->heap_mem << expr;;
          builder.FindSpace();
	  _internal->builder.Assign (reg);
	  
	  builder << _internal->heap_mem << reg <<expr;;
	  builder.CheckFree();
	  _internal->builder.Assume();
	  
	  builder << _internal->heap_mem << reg << expr;
	  builder.Alloc();
	  _internal->builder.Assign(_internal->heap_mem).InstrSequence();
	  
	  
	  
        }
      }

      void Compiler::visitFreeStatement(const Whiley::FreeStatement& free) {
	auto& builder = static_cast<MiniMC::Model::ExpressionBuilder&> (_internal->builder); 
	builder << _internal->heap_mem;;
	free.getExpression().accept(*this);
	builder.Free();
	_internal->builder.Assign (_internal->heap_mem).InstrSequence();
	
      }

      void Compiler::visitDerefExpression(const Whiley::DerefExpression& a) {
	auto& builder = static_cast<MiniMC::Model::ExpressionBuilder&> (_internal->builder); 
	builder << _internal->heap_mem;
	a.getMem().accept(*this);
	builder << makeType(a.getLoadType());
	builder.Load();
      }

      void Compiler::visitCastExpression(const Whiley::CastExpression& a) {
	auto& builder = static_cast<MiniMC::Model::ExpressionBuilder&> (_internal->builder);
	a.getExpression().accept(*this);
        if (Whiley::bytesize(a.getType()) == Whiley::bytesize(a.getExpression().getType()))
          return;
        else {
          builder << makeType(a.getType());
          if (Whiley::bytesize(a.getType()) > Whiley::bytesize(a.getExpression().getType())) {
            if (isSigned(a.getType()))
              builder.SExt();
            else
              builder.ZExt();

          } else
            builder.Trunc();
        }
      }

      void Compiler::visitUndefExpression(const Whiley::UndefExpression& a) {
	static_cast<MiniMC::Model::ExpressionBuilder&> (_internal->builder) << MiniMC::Model::Undef::make(makeType(a.getType()));
      }

      void Compiler::visitBinaryExpression(const Whiley::BinaryExpression& be) {
	auto& builder = static_cast<MiniMC::Model::ExpressionBuilder&> (_internal->builder);
	be.getLeft().accept(*this);
        be.getRight().accept(*this);
        bool _signed = isSigned(be.getLeft().getType()) ||
                       isSigned(be.getRight().getType());
        switch (be.getOp()) {
          case Whiley::BinOps::Add:
            if (be.getLeft().getType() == Whiley::Type::Pointer) {
              builder.PtrAdd();
            } else {
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
            throw MiniMC::Support::Exception{"Modulo operations not supported in MiniMC"};
            break;
          case Whiley::BinOps::Div:
            if (_signed)
              builder.SDiv();

            else
              builder.UDiv();
            break;
          case Whiley::BinOps::LEq:
            if (_signed)
              builder.SLe().pushI8Type().ZExt();
            else
              builder.ULe().pushI8Type().ZExt();
            break;
          case Whiley::BinOps::GEq:
            if (_signed)
              builder.SGe().pushI8Type().ZExt();
            else
              builder.UGe().pushI8Type().ZExt();
            break;
          case Whiley::BinOps::Lt:
            if (_signed)
              builder.SLt().pushI8Type().ZExt();
            else
              builder.ULt().pushI8Type().ZExt();

            break;
          case Whiley::BinOps::Gt:
            if (_signed)
              builder.SGt().pushI8Type().ZExt();
            else
              builder.UGt().pushI8Type().ZExt();

            break;
	case Whiley::BinOps::Eq:
            builder.Eq().pushI8Type().ZExt();
            break;
          case Whiley::BinOps::NEq:
            builder.NEq().pushI8Type().ZExt();
            break;
        }
      }

      void Compiler::visitAssignStatement(const Whiley::AssignStatement& ass) {
        auto symb = _internal->frame.resolve(ass.getAssignName()).value();
        auto reg = std::get<MiniMC::Model::Register_ptr>(symb.getUserData());
        ass.getExpression().accept(*this);
	
        _internal->builder.Assign (reg).InstrSequence();
      }

      void Compiler::visitIncrementDecrementStatement(const Whiley::IncrementDecrementStatement& ass) {
	auto& exprbuilder = static_cast<MiniMC::Model::ExpressionBuilder&> (_internal->builder);
	
        auto symb = _internal->frame.resolve(ass.getIncrementee()).value();
        auto reg = std::get<MiniMC::Model::Register_ptr>(symb.getUserData());
        exprbuilder << reg;
        switch (reg->getType()->getTypeID()) {
          case MiniMC::Model::TypeID::I8:
            exprbuilder.I8(1);
            break;
          case MiniMC::Model::TypeID::I16:
            exprbuilder.I16(1);
            break;
          case MiniMC::Model::TypeID::I32:
            exprbuilder.I32(1);
            break;
          case MiniMC::Model::TypeID::I64:
            exprbuilder.I64(1);
            break;
          default:
            throw MiniMC::Support::Exception("Can't increment expression");
        }
        exprbuilder.Add();
	_internal->builder.Assign(reg).InstrSequence();
      }

      void Compiler::visitAssertStatement(const Whiley::AssertStatement& a) {
	a.getExpression().accept(*this);
	_internal->builder.pushBoolType();
	_internal->builder.IntToBool ();
	_internal->builder.Assert().InstrSequence();
      }
      void Compiler::visitAssumeStatement(const Whiley::AssumeStatement& a) {
	a.getExpression().accept(*this);
	_internal->builder.pushBoolType();
	_internal->builder.IntToBool ();
	_internal->builder.Assume().InstrSequence();
      }

      void Compiler::visitChooseStatement(const Whiley::ChooseStatement& iff) {
	std::size_t i = 0;
	for (auto& s : iff.getStatements()) {
	  s->accept(*this);
	  i++;
	}
	_internal->builder.Branch (i);
      }
      void Compiler::visitIfStatement(const Whiley::IfStatement& iff) {
	iff.getCondition().accept(*this);
	_internal->builder.pushBoolType();
	_internal->builder.IntToBool ();
        iff.getIfBody().accept(*this);
	iff.getElseBody().accept(*this);
	_internal->builder.If ();
      }
      void Compiler::visitSkipStatement(const Whiley::SkipStatement&) {
	_internal->builder.Skip().InstrSequence();
      }
      void Compiler::visitWhileStatement(const Whiley::WhileStatement& w) {
	w.getCondition().accept(*this);
	_internal->builder.pushBoolType();
	_internal->builder.IntToBool ();
	w.getBody().accept(*this);
	_internal->builder.Loop();
      }

      void Compiler::visitReturnStatement(const Whiley::ReturnStatement& ret) {
	ret.getExpr().accept(*this);
	_internal->builder.Ret ().InstrSequence();
      }

      void Compiler::visitAtomicStatement(const Whiley::AtomicStatement& c) {
	c.getStmt().accept(*this);
	_internal->builder.Atomic();
      }
      
      void Compiler::visitCallStatement(const Whiley::CallStatement& c) {
	auto& exprbuilder = static_cast<MiniMC::Model::ExpressionBuilder&> (_internal->builder);
	MiniMC::Model::Symbol symb;
        MiniMC::Model::Symbol func_symb;
	
	MiniMC::Model::Register_ptr reg = nullptr;
        if (auto symb = _internal->frame.resolve(c.assignname()))
          reg = std::get<MiniMC::Model::Register_ptr>(symb.value().getUserData());
	
        if (auto func_symb = _internal->frame.resolve(c.funcname())) {
	  auto symb_expr = MiniMC::Model::makeExpr<MiniMC::Model::SymbolicConstant>(func_symb.value());
	  std::size_t params=0;
	  for (auto& a : c.parameters()) {
	    a->accept(*this);   
	    params++;
	  }
	  exprbuilder <<  symb_expr;
	  _internal->builder.Call(reg,params);
	}
      }

      void Compiler::visitSequenceStatement(const Whiley::SequenceStatement& s) {
	s.getFirst().accept(*this);
        s.getSecond().accept(*this);
	_internal->builder.Sequence();
	
      }

      void Compiler::visitMemAssignStatement(const Whiley::MemAssignStatement& a) {
	auto& exprbuilder = static_cast<MiniMC::Model::ExpressionBuilder&> (_internal->builder);
	exprbuilder << _internal->heap_mem;
	a.getMemLoc().accept(*this);
	a.getExpression().accept(*this);
	exprbuilder.Store();
	_internal->builder.Assign(_internal->heap_mem).InstrSequence();
      }


    } // namespace whiley
  } // namespace Loaders
} // namespace MiniMC
