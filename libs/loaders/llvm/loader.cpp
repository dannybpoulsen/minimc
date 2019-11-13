
#include <fstream>
#include <unordered_map>

#include <llvm/Support/CommandLine.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/AssemblyAnnotationWriter.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Scalar/LoopUnrollPass.h>
#include <llvm/Transforms/Scalar/DCE.h>
#include <llvm/Transforms/Scalar/SCCP.h>
#include <llvm/Transforms/Scalar/LoopRotation.h>
#include <llvm/Transforms/Scalar/SROA.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Utils/UnrollLoop.h>
#include <llvm/Transforms/Utils/LCSSA.h>
#include <llvm/Analysis/LoopInfo.h>

#include <llvm/PassRegistry.h>

#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/MemoryBuffer.h>

#include "support/exceptions.hpp"
#include "loaders/loader.hpp"
#include "model/cfg.hpp"
#include "llvminstuctioncreator.hpp"

namespace MiniMC {
  namespace Loaders {

    
    MiniMC::Model::Value_ptr findValue (llvm::Value* val, Val2ValMap& values, Types& tt ) {
      llvm::Constant* cst = llvm::dyn_cast<llvm::Constant> (val);
      if (cst && !llvm::isa<llvm::Function> (val)) 
	return makeConstant (cst,tt);
      else {
	return values.at(val); 
      }
    }
    
    MiniMC::Model::Type_ptr getType (llvm::Type* type, MiniMC::Model::TypeFactory_ptr& tfactory);
    uint32_t computeSizeInBytes (llvm::Type* ty,MiniMC::Model::TypeFactory_ptr& tfactory) {
      if (ty -> isArrayTy ()) {
	return ty->getArrayNumElements ()*computeSizeInBytes(ty->getArrayElementType (),tfactory);
      }
      
      else if (ty->isStructTy ()) {
	auto it = static_cast<llvm::StructType*> (ty);
	std::size_t size = 0;
	for (std::size_t i = 0;i < it->getNumElements ();++i) {
	  size += computeSizeInBytes (it->getElementType (i),tfactory);
	}
	return size;
      }
      else {
	return getType (ty,tfactory)->getSize();
      }
      throw MiniMC::Support::Exception ("Can't calculate size of type");
    }
    
    MiniMC::Model::Type_ptr getType (llvm::Type* type, MiniMC::Model::TypeFactory_ptr& tfactory) {
      if (type->isVoidTy ()) {
	return tfactory->makeVoidType();
      }
      else if (type->isPointerTy ()) {
	return tfactory->makePointerType();
      }
      
      else if (type->isIntegerTy ()) {
	unsigned  bits = type->getIntegerBitWidth ();
	if (bits == 1) {
	  return tfactory->makeBoolType();
	}
	else if (bits <= 8)
	  return tfactory->makeIntegerType(8);
	else if (bits <=16)
	  return tfactory->makeIntegerType(16);
	else if (bits <=32)
	  return tfactory->makeIntegerType(32);
	else if (bits <=64)
	  return tfactory->makeIntegerType(64);
      }
      else if (type->isStructTy()) {
	return tfactory->makeStructType (computeSizeInBytes(type,tfactory));
      }
      
      else if (type->isArrayTy()) {
	return tfactory->makeArrayType (computeSizeInBytes(type,tfactory));
      }
      
      
      throw MiniMC::Support::Exception ("Unknown Type");
      return nullptr;
    }
    
    struct InstructionNamer : public llvm::PassInfoMixin<InstructionNamer> {
      llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager&) {
	for (auto &Arg : F.args())
	  if (!Arg.hasName())
	    Arg.setName("arg");
	
	for (llvm::BasicBlock &BB : F) {
	  if (!BB.hasName())
	    BB.setName("bb");
	  
	  for (llvm::Instruction &I : BB)
	    if (!I.hasName() && !I.getType()->isVoidTy())
	      I.setName("tmp");
	}
	return llvm::PreservedAnalyses::all();
      }
    };

    struct ConstExprRemover : public llvm::PassInfoMixin<ConstExprRemover> {
	llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager&) {
	  
	  for (llvm::BasicBlock &BB : F) {
		for (llvm::Instruction &I : BB) {
		  handle(&I);
		}
	  }
	  return llvm::PreservedAnalyses::none();
	}
	
	void handle (llvm::Instruction* inst) {
	  auto ops = inst->getNumOperands ();
	  for (std::size_t i = 0; i < ops; i++) {
		auto op = inst->getOperand (i);
		llvm::ConstantExpr* oop = nullptr;
		if (( oop = llvm::dyn_cast<llvm::ConstantExpr> (op))) {
		  auto ninst = oop->getAsInstruction ();
		  ninst->insertBefore (inst);
		  inst->setOperand (i,ninst);
		  handle (ninst);
		}
	  }
	}
  };

  struct RemoveUnusedInstructions : public llvm::PassInfoMixin<RemoveUnusedInstructions> {
	llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager&) {
	  bool changed = true;
	  do {
		changed = false;
		for (llvm::BasicBlock &BB : F) {
		  for (auto I = BB.begin(), E = BB.end(); I != E;  ++I) {
		    llvm::Instruction& inst = *I;
		    if(!inst.getNumUses() &&
		       deleteable (&inst)
		       ) {
		      I = inst.eraseFromParent();
		      changed = true;
			}
		    
		  }
		  
		  
		}
	  }while (changed);
	  
	  
	  return llvm::PreservedAnalyses::none();
	}

    
    bool deleteable (llvm::Instruction* inst) {
      switch (inst->getOpcode()) {
      case llvm::Instruction::Store:
      case llvm::Instruction::Call:
      case llvm::Instruction::Unreachable:
	return false;
      default:
	return !inst->isTerminator ();
      }
    }
    
  };
    
    struct GetElementPtrSimplifier : public llvm::PassInfoMixin<InstructionNamer> {
      llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager&) {
	llvm::LLVMContext& context = F.getContext();
	auto i32Type = llvm::IntegerType::getInt32Ty(context);
	auto zero = llvm::ConstantInt::get (i32Type,0);
	for (llvm::BasicBlock &BB : F) {
	  llvm::BasicBlock::iterator BI = BB.begin();
	  llvm::BasicBlock::iterator BE =  BB.end();
	  while (BI != BE) {
	    auto& I = *BI++;
	    //WOrking here
	    if (I.getOpcode () == llvm::Instruction::GetElementPtr) {
	      llvm::GetElementPtrInst* inst = llvm::dyn_cast<llvm::GetElementPtrInst> (&I);
	      llvm::Value* indexList[1] = {inst->getOperand(1)};
	      auto prev = llvm::GetElementPtrInst::Create (nullptr,inst->getOperand(0),llvm::ArrayRef<llvm::Value*> (indexList,1),"_gep__",inst);
	      
	      auto type_iter = llvm::gep_type_begin(*inst);
	      ++type_iter;
	      const std::size_t E = inst->getNumOperands ();
	      for (std::size_t oper = 2; oper < E; ++oper,++type_iter) {
		auto resType = type_iter.getIndexedType();
		llvm::Value* indexList[2] = {zero,inst->getOperand(oper)};
		prev = llvm::GetElementPtrInst::Create (nullptr,prev,llvm::ArrayRef<llvm::Value*> (indexList,2),"_gep__",inst);
	      }
	      I.replaceAllUsesWith (prev);
	      I.eraseFromParent ();
	    }
	  }
	  
	  
	}
	return llvm::PreservedAnalyses::none();
      }
    };
    
    struct Constructor : public llvm::PassInfoMixin<InstructionNamer> {
      
      Constructor (MiniMC::Model::Program_ptr& prgm,
		   MiniMC::Model::TypeFactory_ptr& tfac
		   ) : prgm(prgm),tfactory(tfac) {
      }
	  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager&) {
		auto cfg  = std::make_shared<MiniMC::Model::CFG> ();
		std::unordered_map<llvm::BasicBlock*,MiniMC::Model::Location_ptr> locmap;
		for (llvm::BasicBlock &BB : F) {
		  locmap.insert (std::make_pair(&BB,cfg->makeLocation(BB.getName())));
		}

		Types tt;
		tt.tfac = tfactory;
		
		std::vector<gsl::not_null<MiniMC::Model::Variable_ptr>> params;
		auto variablestack =  prgm->makeVariableStack ();
		pickVariables (F,variablestack);
		auto& entry = F.getEntryBlock ();
		cfg->setInitial (locmap.at(&entry));
		for (llvm::BasicBlock &BB : F) {
		  auto loc = locmap.at(&BB);
		  auto term = BB.getTerminator ();
		  std::vector<MiniMC::Model::Instruction> insts;
		  for (llvm::Instruction& inst : BB) {
		    if (&inst!=term) {
		      addInstruction (&inst,insts,tt);
		    }
		    if (llvm::isa<llvm::CallInst> (inst)) {
		      auto mloc = cfg->makeLocation (loc->getName () + ":Call");
		      cfg->makeEdge (loc,mloc,insts,nullptr,prgm);
		      insts.clear();
		      loc = mloc;
		    }
		  }

		  if (insts.size()) {
		    auto mloc = cfg->makeLocation (loc->getName () + ":");
		    cfg->makeEdge (loc,mloc,insts,nullptr,prgm);
		    loc = mloc;
		  }
		  
		  if (term) {
		    insts.clear();
		    if( term->getOpcode () == llvm::Instruction::Br) {
		      
		      auto brterm = llvm::dyn_cast<llvm::BranchInst> (term);
		      if (brterm->isUnconditional ()) {
			std::vector<MiniMC::Model::Instruction> insts;
			auto succ = term->getSuccessor (0);
			auto succloc = locmap.at(succ);
			cfg->makeEdge (loc,succloc,insts,nullptr,prgm);
		      }
		      else {
			auto cond = findValue (brterm->getCondition(),values,tt);
			auto ttloc = locmap.at(term->getSuccessor (0));
			auto ffloc = locmap.at(term->getSuccessor (1));
			cfg->makeEdge (loc,ttloc,insts,cond,prgm);
			cfg->makeEdge (loc,ffloc,insts,cond,prgm,true);
		      }
		    }

		    if( term->getOpcode () == llvm::Instruction::Ret) {
		      std::vector<MiniMC::Model::Instruction> insts;
		      auto succloc = cfg->makeLocation ("Term");
		      cfg->makeEdge (loc,succloc,insts,nullptr,prgm);
		    }
		    
		  }
		}
		auto f = prgm->addFunction (F.getName(),params,variablestack,cfg);

		auto id = f->getID ();
		auto ptr = std::make_shared<MiniMC::Model::IntegerConstant> (id);
		values.insert (std::make_pair(&F,ptr));

		prgm->addEntryPoint (f);
		return llvm::PreservedAnalyses::all();
	  }

	  	  
	  void pickVariables (const llvm::Function& func,MiniMC::Model::VariableStackDescr_ptr stack) {
		for (auto itt = func.arg_begin();itt!=func.arg_end(); itt++) {
		  auto lltype = itt->getType ();
		  auto type = getType (lltype,tfactory);
		  makeVariable (itt,itt->getName(),type,stack);
		}
		
		for (const llvm::BasicBlock& bb : func) {
		  for (auto& inst : bb) {
		    makeVar (&inst,stack);
		    auto ops = inst.getNumOperands ();
		    for (std::size_t i = 0; i < ops; i++) {
		      auto op = inst.getOperand (i);
		      makeVar (op,stack);
		    }
		    
		    
		    
		  }
		}
	  }

	  void makeVar (const llvm::Value* op,MiniMC::Model::VariableStackDescr_ptr stack ) {
	    const llvm::Constant* oop = llvm::dyn_cast<const llvm::Constant> (op);
	    auto lltype = op->getType();
	    if (lltype->isLabelTy ())
	      return ;
	    auto type = getType (op->getType(),tfactory);
	    if (oop) {
	      return ;
	    }
	    else {
	      makeVariable (op,op->getName(),type,stack);
	    }
	  }
	  
	  void makeVariable (const llvm::Value* val, const std::string& name, MiniMC::Model::Type_ptr& type, MiniMC::Model::VariableStackDescr_ptr& stack) {
	    if (!values.count (val)) {
	      auto newVar = stack->addVariable (name,type);
	      values[val] = newVar;
	    }
	  }

#define SUPPORTEDLLVM				\
	  X(Add)				\
	  X(Sub)				\
	  X(Mul)				\
	  X(UDiv)				\
	  X(SDiv)				\
	  X(Shl)				\
	  X(LShr)				\
	  X(AShr)				\
	  X(And)				\
	  X(Or)					\
	  X(Xor)				\
	  X(ICmp)				\
	  X(Trunc)				\
	  X(ZExt)				\
	  X(SExt)				\
	  X(PtrToInt)				\
	  X(IntToPtr)				\
	  X(BitCast)				\
	  X(Load)				\
	  X(Store)				\
	  X(Alloca)				\
	  X(GetElementPtr)			\
	  X(InsertValue)			\
	  X(ExtractValue)			\
	  X(Call)				\
	  X(Ret)				\
	  
	  
	  
	  
	  
	  void addInstruction (llvm::Instruction* inst, std::vector<MiniMC::Model::Instruction>& insts,Types& tt) {
	    switch (inst->getOpcode()) {
#define X(TT)					\
	      case llvm::Instruction::TT:	\
		translateAndAddInstruction<llvm::Instruction::TT> (inst,values,insts,tt);\
		break;
	      SUPPORTEDLLVM
	    default:
	      throw MiniMC::Support::Exception ("Unsupported operation");
	    }
	    
	    
	  }
			       
	  
	private:
	  MiniMC::Model::Program_ptr& prgm;
	  MiniMC::Model::TypeFactory_ptr& tfactory;
	  Val2ValMap values;
	  
	};
	
    class LLVMLoader : public Loader {
      virtual MiniMC::Model::Program_ptr loadFromFile (const std::string& file, MiniMC::Model::TypeFactory_ptr& tfac) {
	auto prgm = std::make_shared<MiniMC::Model::Program> ();
	std::fstream str;
	str.open (file);
	std::string ir((std::istreambuf_iterator<char>(str)), (std::istreambuf_iterator<char>()));
	std::unique_ptr<llvm::MemoryBuffer> buffer= llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(ir));
 
	llvm::SMDiagnostic diag;
	std::unique_ptr<llvm::LLVMContext> context = std::make_unique<llvm::LLVMContext> ();
	std::unique_ptr<llvm::Module> module = parseIR(*buffer, diag,*context);
	
	llvm::PassBuilder PB;
	
	llvm::LoopAnalysisManager lam;
	llvm::FunctionAnalysisManager fam;
	llvm::CGSCCAnalysisManager cgam;
	llvm::ModuleAnalysisManager mam;
		
	llvm::LoopPassManager loopmanager;
	llvm::FunctionPassManager funcmanager;
	llvm::FunctionPassManager funcmanagerafterinit;
	llvm::ModulePassManager mpm;
	
	PB.registerFunctionAnalyses (fam);
	PB.registerModuleAnalyses (mam);
	PB.registerLoopAnalyses(lam);
	PB.registerCGSCCAnalyses(cgam);
	PB.crossRegisterProxies(lam, fam, cgam, mam);

	funcmanager.addPass (ConstExprRemover());
	funcmanager.addPass (RemoveUnusedInstructions());
	funcmanager.addPass (InstructionNamer());
	funcmanager.addPass (GetElementPtrSimplifier());
	funcmanager.addPass (Constructor(prgm,tfac));
	
	
	mpm.addPass (llvm::createModuleToFunctionPassAdaptor(std::move(funcmanager)));
		
	mpm.run (*module,mam);
	
	return prgm;
      }
    };
    
    template<>
    std::unique_ptr<Loader> makeLoader<Type::LLVM> () {
      return std::make_unique<LLVMLoader> ();
    }
    
  }
}
