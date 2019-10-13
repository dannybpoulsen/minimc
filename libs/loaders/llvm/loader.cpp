#include <fstream>
#include <unordered_map>

#include <llvm/Support/raw_os_ostream.h>
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
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/MemoryBuffer.h>

#include "support/exceptions.hpp"
#include "loaders/loader.hpp"
#include "model/cfg.hpp"
#include "llvminstuctioncreator.hpp"
namespace MiniMC {
  namespace Loaders {

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

	struct Constructor : public llvm::PassInfoMixin<InstructionNamer> {
	 
	  Constructor (MiniMC::Model::Program_ptr& prgm,
				   MiniMC::Model::TypeFactory_ptr& tfac
				   ) : prgm(prgm) {
		tbool = tfac->makeBoolType ();
		i8 = tfac->makeIntegerType (8);
		i16 = tfac->makeIntegerType (16);
		i32 = tfac->makeIntegerType (32);
		i64 = tfac->makeIntegerType (32);
		pointer = tfac->makePointerType ();
		voidtype = tfac->makeVoidType ();
	  }
	  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager&) {
		auto cfg  = std::make_shared<MiniMC::Model::CFG> ();
		std::unordered_map<llvm::BasicBlock*,MiniMC::Model::Location_ptr> locmap;
		for (llvm::BasicBlock &BB : F) {
			locmap.insert (std::make_pair(&BB,cfg->makeLocation(BB.getName())));
		}

		std::vector<MiniMC::Model::Instruction> inst;
		std::vector<gsl::not_null<MiniMC::Model::Variable_ptr>> params;
		auto variablestack =  prgm->makeVariableStack ();
		pickVariables (F,variablestack);
		auto& entry = F.getEntryBlock ();
		cfg->setInitial (locmap.at(&entry));
		for (llvm::BasicBlock &BB : F) {
		  auto loc = locmap.at(&BB);
		  auto term = BB.getTerminator ();
		  if (term) {
			auto nb = term->getNumSuccessors ();
			for (unsigned i = 0; i < nb; i++) {
			  auto succ = term->getSuccessor (i);
			  auto succloc = locmap.at(succ);
			  cfg->makeEdge (loc,succloc,inst,nullptr);
			}
		  }
		}
		auto f = prgm->addFunction (F.getName(),params,variablestack,cfg);
		prgm->addEntryPoint (f);
		return llvm::PreservedAnalyses::all();
	  }

	  MiniMC::Model::Type_ptr getType (llvm::Type* type) {
		if (type->isVoidTy ()) {
		  return voidtype;
		}
		else if (type->isPointerTy ()) {
		  return pointer;
		}

		else if (type->isIntegerTy ()) {
		  unsigned  bits = type->getIntegerBitWidth ();
		  if (bits == 1) {
			return tbool;
		  }
		  else if (bits <= 8)
			return i8;
		  else if (bits <=16)
			return i16;
		  else if (bits <=32)
			return i32;
		  else if (bits <=64)
			return i64;
		}
		throw MiniMC::Support::Exception ("Unknown Type");
		return nullptr;
	  }
	  
	  void pickVariables (const llvm::Function& func,MiniMC::Model::VariableStackDescr_ptr stack) {
		for (auto itt = func.arg_begin();itt!=func.arg_end(); itt++) {
		  auto lltype = itt->getType ();
		  auto type = getType (lltype);
		  makeVariable (itt,itt->getName(),type,stack);
		}
		
		for (const llvm::BasicBlock& bb : func) {
		  for (auto& inst : bb) {
			auto ops = inst.getNumOperands ();
			for (std::size_t i = 0; i < ops; i++) {
			  auto op = inst.getOperand (i);
			  llvm::Constant* oop = llvm::dyn_cast<llvm::Constant> (op);
			  auto lltype = op->getType();
			  if (lltype->isLabelTy ())
				continue;
			  auto type = getType (op->getType());
			  if (oop) {
				continue;
				//throw MiniMC::Support::Exception ("ERRROR");
			  }
			  else {
				makeVariable (op,op->getName(),type,stack);
			  }
			}
			

			  
		  }
		}
	  }

	  void makeVariable (const llvm::Value* val, const std::string& name, MiniMC::Model::Type_ptr& type, MiniMC::Model::VariableStackDescr_ptr& stack) {
		if (!values.count (val)) {
		  auto newVar = stack->addVariable (name,type);
		  values[val] = newVar;
		}
	  }
	  
	private:
	  MiniMC::Model::Program_ptr& prgm;
	  MiniMC::Model::TypeFactory_ptr tfactory;
	  MiniMC::Model::Type_ptr tbool;
	  MiniMC::Model::Type_ptr i8;
	  MiniMC::Model::Type_ptr i16;
	  MiniMC::Model::Type_ptr i32;
	  MiniMC::Model::Type_ptr i64;
	  MiniMC::Model::Type_ptr pointer;
	  MiniMC::Model::Type_ptr voidtype;
	  std::unordered_map<const llvm::Value*,MiniMC::Model::Variable_ptr> values;
	  
	};
	
	class LLVMLoader : public Loader {
	  virtual MiniMC::Model::Program_ptr loadFromFile (const std::string& file, MiniMC::Model::TypeFactory_ptr& tfac) {
		auto prgm = std::make_unique<MiniMC::Model::Program> ();
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

		funcmanager.addPass (InstructionNamer());
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
