
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <llvm/IRPrinter/IRPrintingPasses.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Value.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/DCE.h>
#include <llvm/Transforms/Scalar/LoopRotation.h>
#include <llvm/Transforms/Scalar/LoopUnrollPass.h>
#include <llvm/Transforms/Scalar/SCCP.h>
#include <llvm/Transforms/Scalar/SROA.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Utils/LCSSA.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Utils/UnrollLoop.h>

#include <llvm/Analysis/LoopInfo.h>

#include <llvm/PassRegistry.h>


#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>

#include <unordered_set>

#include "llvmpasses.hpp"
#include "minimc/loaders/loader.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/model/program.hpp"
#include "minimc/support/feedback.hpp"
#include "minimc/support/localisation.hpp"

#include "context.hpp"
#include "minimc/model/builder.hpp"
namespace MiniMC {
  namespace Loaders {

    class FunctionNotDefined : public MiniMC::Support::WarningMessage {
    public:
      FunctionNotDefined (const std::string& funcname) : funcname(funcname) {}
      MiniMC::IO::ostream& to_string (MiniMC::IO::ostream& os) const override {
	return os << loc.format (funcname);
      }
      
    private:
      std::string funcname;
      MiniMC::Support::Localiser loc{"Function '%1%' only declared in LLVM assembly. Treating it as a non-determinstic function"};
    };
    
    MiniMC::Model::TypeID getTypeID(llvm::Type* type);
    
    
    class LLVMLoader : public LoaderDirect {
    public:
      LLVMLoader()  : LoaderDirect("LLVM") {

	addOption<IntOption>("stack", "StackSize", &stacksize);
	addOption<VecStringOption>("entry", "Entry point function", &entry);
	addOption<BoolOption>("disable_promote_pass", "Disable the promotion of allocas to registers", &disablePromotion);
	addOption<BoolOption>("print", "Print LLVM module to stderr", &printLLVMPass);
	
      }
      LoadResult loadFromFile(const std::string& file, MiniMC::Support::Messager& mess) override {
	std::fstream str;
        str.open(file);
        std::string ir((std::istreambuf_iterator<char>(str)), (std::istreambuf_iterator<char>()));
        std::unique_ptr<llvm::MemoryBuffer> buffer = llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(ir));
        return readFromBuffer(buffer,  mess);
      }

      LoadResult loadFromString(const std::string& inp,MiniMC::Support::Messager& mess) override {
        std::stringstream str;
        str.str(inp);
        std::string ir((std::istreambuf_iterator<char>(str)), (std::istreambuf_iterator<char>()));
        std::unique_ptr<llvm::MemoryBuffer> buffer = llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(ir));
        return readFromBuffer(buffer, mess);
      }

      auto createFunctionWorkList(llvm::Module& module) {
        std::unordered_set<llvm::Function*> functions;
        for (auto& F : module) {
          functions.insert(&F);
          for (auto& B : F) {
            for (auto& I : B) {
              if (auto cinst = llvm::dyn_cast<llvm::CallInst>(&I)) {
		if (!cinst->isIndirectCall())
		  functions.insert(cinst->getCalledFunction());
              }
            }
          }
        }
        return functions;
      }

      void loadGlobals(GLoadContext& lcontext, MiniMC::Model::Program& prgm, llvm::Module& module, MiniMC::Support::Messager&) {
	std::vector<MiniMC::Model::Instruction> instr;

        MiniMC::Model::func_t fid = 0;
        for (auto& F : createFunctionWorkList(module)) {
	  std::string fname = F->getName().str();
	  auto fsymbol = prgm.getRootFrame ().makeSymbol (fname);
	  function2symb.emplace (F,fsymbol);
	  auto& Func = *F;
          auto ptr = MiniMC::Model::SymbolicConstant::make(fsymbol);
          ptr->setType(MiniMC::Model::PointerType::get());
          lcontext.addValue(&Func, ptr);
          MiniMC::Model::offset_t lid = 0;
          for (auto& BB : Func) {
            auto ptr = MiniMC::Model::Pointer::make(MiniMC::Model::pointer_t::makeLocationPointer(fid, lid));
            ptr->setType(MiniMC::Model::PointerType::get());
            lcontext.addValue(&BB, ptr);
            lid++;
          }
          fid++;
        }

        for (auto g = module.global_begin(); g != module.global_end(); ++g) {
	  auto pointTySize = lcontext.computeSizeInBytes(g->getValueType());
	  MiniMC::Model::Value_ptr val = nullptr;
	  if (g->hasInitializer()) {
	    val = lcontext.findValue(g->getInitializer());
          }
	  auto symbol =  prgm.getRootFrame().makeFresh (g->getName ().str());
	  prgm.getHeapLayout().addBlock(symbol, MiniMC::Model::pointer_t::makeHeapPointer (++nextHeap,0),pointTySize,lcontext.getHeapMem(),val);
	  auto gvar = MiniMC::Model::SymbolicConstant::make(symbol);
	  gvar->setType(MiniMC::Model::PointerType::get());
          lcontext.addValue(&(*g), gvar);
	  
        }
 
      }

      void instantiateFunction(llvm::Function& F, GLoadContext& lcontext, MiniMC::Model::Program& prgm, MiniMC::Support::Messager& mess) {

        auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();
        std::string fname = F.getName().str();
        auto frame = prgm.getRootFrame().create(fname);
        MiniMC::Model::CFA cfg;
        std::vector<MiniMC::Model::Symbol> params;
        MiniMC::Model::RegisterDescr variablestack;
        MiniMC::Model::LocationInfoCreator locinfoc(frame);
   
        auto sp_mem = variablestack.addRegister(frame.makeFresh("sp_mem"), MiniMC::Model::PointerType::get());
	
        LoadContext load{lcontext, variablestack, sp, sp_mem, frame};
        auto returnTy = load.getType(F.getReturnType());

        auto makeVariable = [&load, &frame](auto val) {
          if (!load.hasValue(val)) {
            auto type = load.getType(val->getType());
	    std::string name{val->getName().str()};
	    MiniMC::Model::Symbol symb = (name != "") ? frame.makeSymbol (name) : frame.makeFresh ();
            auto reg = load.getStack().addRegister(symb, type);
            auto symb_constant = MiniMC::Model::SymbolicConstant::make(symb);
	    symb_constant->setType(reg->getType());
	    load.addValue(val, symb_constant);
          }
          return load.findValue(val);
        };

        auto makeVar = [&load, makeVariable](auto op) {
          const llvm::Constant* oop = llvm::dyn_cast<const llvm::Constant>(op);
          auto lltype = op->getType();
          if (lltype->isLabelTy() ||
              lltype->isVoidTy())
            return;
          auto type = load.getType(op->getType());
          if (oop) {
            return;
          } else {
            makeVariable(op);
          }
        };
        for (auto itt = F.arg_begin(); itt != F.arg_end(); itt++) {
          auto lltype = itt->getType();
          auto type = load.getType(lltype);
          params.push_back(std::static_pointer_cast<MiniMC::Model::SymbolicConstant>(makeVariable(itt))->getValue());
        }

        if (F.isDeclaration()) {
          mess << FunctionNotDefined {F.getName().str()};
	  
          auto iinit = locinfoc.make();
          auto init = cfg.makeLocation(frame.makeSymbol("init"), iinit);
          auto einit = locinfoc.make();
          auto end = cfg.makeLocation(frame.makeSymbol("exit"), einit);

          cfg.setInitial(init);
          {
            MiniMC::Model::EdgeBuilder edgebuilder{cfg, init, end, frame};
            if (returnTy->getTypeID() != MiniMC::Model::TypeID::Void) {
	      edgebuilder.addInstr<MiniMC::Model::InstructionCode::Ret>(MiniMC::Model::Undef::make(returnTy));
            }
	    
            else {
              edgebuilder.addInstr<MiniMC::Model::InstructionCode::RetVoid>();
            }
          }

          prgm.addFunction(function2symb.at (&F), params, returnTy,  std::move(cfg), F.isVarArg(), frame);
        }

        else {
          for (const llvm::BasicBlock& bb : F) {
            for (auto& inst : bb) {
              makeVar(&inst);
              auto ops = inst.getNumOperands();
              for (std::size_t i = 0; i < ops; i++) {
                auto op = inst.getOperand(i);
                makeVar(op);
              }
            }
          }

          std::unordered_map<llvm::BasicBlock*, MiniMC::Model::Location_ptr> locmap;
          std::vector<llvm::BasicBlock*> waiting;
          auto enqueue = [&frame, &locinfoc, &cfg, &locmap, &waiting, &source_loc](llvm::BasicBlock* BB) -> MiniMC::Model::Location_ptr {
            if (locmap.count(BB)) {
              return locmap.at(BB);
            } else {
              auto info = locinfoc.make();
              auto location = cfg.makeLocation(frame.makeSymbol(BB->getName().str()+std::string("_enter")), info);
              locmap.insert(std::make_pair(BB, location));
              waiting.push_back(BB);
              return location;
            }
          };

          auto buildphi = [&load](llvm::BasicBlock* from, llvm::BasicBlock* to, auto&& builder) {
            for (auto& phi : to->phis()) {
              auto ass = load.findValue(&phi);
              auto incoming = load.findValue(phi.getIncomingValueForBlock(from));
              builder.template addInstr<MiniMC::Model::InstructionCode::Assign>(ass, incoming);
            }
          };

          auto& BB = F.getEntryBlock();
          auto entry = enqueue(&BB);

	  //Set up pre-header for storing sp in sp_mem
	  auto init = cfg.makeLocation (frame.makeFresh (),entry->getInfo());
	  {
	    MiniMC::Model::EdgeBuilder edgebuilder{cfg, init, entry, frame};
	    edgebuilder.template addInstr<MiniMC::Model::InstructionCode::Assign>(sp_mem, sp );
	  }
	  cfg.setInitial(init);
          
          InstructionTranslator translate{load};

          while (waiting.size()) {
            auto cur_bb = waiting.back();
            waiting.pop_back();
            auto from = locmap.at(cur_bb);
            auto to = cfg.makeLocation(frame.makeFresh(cur_bb->getName().str()+std::string("_end")), from->getInfo());
            MiniMC::Model::EdgeBuilder edgebuilder{cfg, from, to, frame};
            auto term = cur_bb->getTerminator();

            for (llvm::Instruction& inst : *cur_bb) {

              if (llvm::isa<llvm::PHINode>(inst) || term == &inst) {
                continue;
              }

              translate(&inst, edgebuilder);
	    }
	    
	    if (term->getOpcode() == llvm::Instruction::Br) {

	      auto brterm = llvm::dyn_cast<llvm::BranchInst>(term);
	      if (brterm->isUnconditional()) {
		auto succ = enqueue(term->getSuccessor(0));
		MiniMC::Model::EdgeBuilder builder{cfg, to, succ, frame};
		builder.setPhi ();
		buildphi(cur_bb, term->getSuccessor(0), builder);
	      } else {
		auto cond = load.findValue(brterm->getCondition());
		{
		  auto ttloc = enqueue(term->getSuccessor(0));
		  MiniMC::Model::EdgeBuilder builder {cfg, to, ttloc, frame};
		  builder.addInstr<MiniMC::Model::InstructionCode::Assume>(cond).setPhi ();;
		  buildphi(cur_bb, term->getSuccessor(0), builder);
		}
		
		{
		  auto ffloc = enqueue(term->getSuccessor(1));
		  MiniMC::Model::EdgeBuilder builder{cfg, to, ffloc, frame};
		  builder.addInstr<MiniMC::Model::InstructionCode::NegAssume>(cond).setPhi ();
		  buildphi(cur_bb, term->getSuccessor(1), builder);
		}
	      }
	    }
	    
	    else if (term->getOpcode() == llvm::Instruction::IndirectBr) {
	      auto brterm = llvm::dyn_cast<llvm::IndirectBrInst>(term);
	      std::size_t dests = brterm->getNumDestinations();
	      auto value = load.findValue(brterm->getAddress());
	      for (std::size_t i = 0; i < dests; ++i) {
		auto splitloc = cfg.makeLocation(frame.makeFresh(), to->getInfo());
		auto dest = enqueue(brterm->getDestination(i));
		auto valComp = load.findValue(brterm->getDestination(i));
		auto btype = MiniMC::Model::BoolType::get();
		auto cond = load.getStack().addRegister(frame.makeFresh(), btype);
		
		MiniMC::Model::EdgeBuilder{cfg, to, splitloc, frame}.addInstr<MiniMC::Model::InstructionCode::Eq>(cond,
														       value,
														       valComp)
		  .addInstr<MiniMC::Model::InstructionCode::Assume>(cond);
		buildphi(cur_bb, brterm->getDestination(i), MiniMC::Model::EdgeBuilder{cfg, splitloc, dest, frame,true});
	      }
	    }
	    else if (term->getOpcode() == llvm::Instruction::Ret) {
	      translate(term, edgebuilder);
	    }
	  }
	
	  
          prgm.addFunction(function2symb.at(&F), params, returnTy,  std::move(cfg), F.isVarArg(), frame);
        }
      }

      void instantiateFunctions(GLoadContext& lcontext, MiniMC::Model::Program& prgm, llvm::Module& module,  MiniMC::Support::Messager& mess) {
        std::unordered_set<llvm::Function*> functions = createFunctionWorkList(module);
        for (auto& F : functions) {
          instantiateFunction(*F, lcontext, prgm,mess);
        }
      }

      void llvmModifications(llvm::Module& module, MiniMC::Support::Messager&) {

        llvm::legacy::PassManager lpm;
        lpm.add(llvm::createLowerSwitchPass());
        lpm.run(module);
        llvm::PassBuilder PB;

        llvm::LoopAnalysisManager lam;
        llvm::FunctionAnalysisManager fam;
        llvm::CGSCCAnalysisManager cgam;
        llvm::ModuleAnalysisManager mam;
	
        llvm::LoopPassManager loopmanager;
        llvm::FunctionPassManager funcmanagerllvm;
        llvm::FunctionPassManager funcmanager;
        llvm::ModulePassManager mpm;

        PB.registerFunctionAnalyses(fam);
        PB.registerModuleAnalyses(mam);
        PB.registerLoopAnalyses(lam);
        PB.registerCGSCCAnalyses(cgam);
        PB.crossRegisterProxies(lam, fam, cgam, mam);

        funcmanagerllvm.addPass(ConstExprRemover());
        funcmanagerllvm.addPass(RemoveUnusedInstructions());
        if (!disablePromotion)
          funcmanager.addPass(llvm::PromotePass());
        funcmanagerllvm.addPass(GetElementPtrSimplifier());
        funcmanagerllvm.addPass(InstructionNamer());
        mpm.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(funcmanagerllvm)));
        mpm.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(funcmanager)));
        if (printLLVMPass)
          mpm.addPass(llvm::PrintModulePass(llvm::errs()));

        mpm.run(module, mam);
      }

      MiniMC::Model::Symbol createEntryPoint(std::size_t stacksize, MiniMC::Model::Program& program, const MiniMC::Model::Symbol function_symb, std::vector<MiniMC::Model::Value_ptr>&&, const MiniMC::Model::Register_ptr& sp_reg, const MiniMC::Model::Value_ptr& heap_mem) {
      static std::size_t nb = 0;
      auto function = std::get<MiniMC::Model::Function_ptr> (function_symb.getUserData ());
      const std::string name = MiniMC::Support::Localiser("__minimc__entry_%1%-%2%").format(function_symb.getName(), ++nb);
      auto frame = program.getRootFrame().create(name);
      MiniMC::Model::CFA cfg;
      MiniMC::Model::RegisterDescr vstack;
      MiniMC::Model::LocationInfoCreator locinf(frame);

      auto funcpointer = MiniMC::Model::SymbolicConstant::make(function_symb);
      funcpointer->setType(MiniMC::Model::PointerType::get());
      auto iinfo = locinf.make();
      auto init = cfg.makeLocation(frame.makeFresh("init"), iinfo);
      auto einfo = locinf.make();

      auto end = cfg.makeLocation(frame.makeFresh("end"), einfo);

      cfg.setInitial(init);

      std::vector<MiniMC::Model::Value_ptr> params;
      MiniMC::Model::Value_ptr result = nullptr;
      auto stack_symb = program.getRootFrame().makeFresh (name+std::string("_stack"));
      program.getHeapLayout().addBlock(stack_symb,MiniMC::Model::pointer_t::makeHeapPointer (++nextHeap,0),stacksize,heap_mem);
      MiniMC::Model::Value_ptr sp = MiniMC::Model::SymbolicConstant::make(stack_symb);
      sp->setType(MiniMC::Model::PointerType::get());

      MiniMC::Model::Value_ptr stacksize_p = MiniMC::Model::I64Integer::make(stacksize);
      MiniMC::Model::Value_ptr nb_skips = MiniMC::Model::I64Integer::make (1);
      
      
      
      auto restype = function->getReturnType();
      if (restype->getTypeID() != MiniMC::Model::TypeID::Void) {
        result = vstack.addRegister(frame.makeFresh(), restype);
      }
      {
	MiniMC::Model::EdgeBuilder builder{cfg, init, end, frame};
	builder.addInstr<MiniMC::Model::InstructionCode::PtrAdd>(sp_reg, sp,stacksize_p,nb_skips);
	builder.addInstr<MiniMC::Model::InstructionCode::Call>(result, funcpointer, params);
      }
      auto entry_symb = program.getRootFrame().makeSymbol (name);
      program.addFunction(entry_symb, {},
			  MiniMC::Model::VoidType::get(),
                          std::move(cfg),
			  false,
			  frame);
      
      return entry_symb;
      }
      
      
      void setupEntryPoints(MiniMC::Model::Program& prgm, MiniMC::Model::Value_ptr heap_mem) {
        for (const auto& e : entry) {
	  
	  if (auto symbol = prgm.getRootFrame().resolve (e)) {
	    //auto func = prgm.getFunction(symbol.value());
	    auto entry = createEntryPoint(stacksize, prgm, symbol.value(), {},sp,heap_mem);
	    
	    prgm.addEntryPoint(entry);
	  }
        }
      }

      virtual LoadResult readFromBuffer(std::unique_ptr<llvm::MemoryBuffer>& buffer, MiniMC::Support::Messager& mess) {
        MiniMC::Model::Program prgm;
	sp = prgm.getCPURegs().addRegister(prgm.getRootFrame().makeFresh("sp"), MiniMC::Model::PointerType::get());
	auto heap_mem = prgm.getPersistentRegs().addRegister(prgm.getRootFrame().makeFresh("heap_mem"), MiniMC::Model::MemoryType::get());
        GLoadContext lcontext{heap_mem};

        llvm::SMDiagnostic diag;
        std::unique_ptr<llvm::LLVMContext> context = std::make_unique<llvm::LLVMContext>();
        std::unique_ptr<llvm::Module> module = parseIR(*buffer, diag, *context);
        if (!module) {
          return std::unexpected{MiniMC::Loaders::Error::LoadFailed};
        }

        llvmModifications(*module,mess);
        loadGlobals(lcontext, prgm, *module,mess);
        instantiateFunctions(lcontext, prgm, *module, mess);

        setupEntryPoints(prgm,heap_mem);

        return prgm;
      }

    private:
      std::size_t stacksize{200};
      std::vector<std::string> entry;
      bool disablePromotion;
      bool printLLVMPass;
      MiniMC::Model::Register_ptr sp;
      std::unordered_map<llvm::Function*,MiniMC::Model::Symbol> function2symb;
      std::size_t nextHeap{0};
      
    };

    class LLVMLoadRegistrar : public LoaderRegistrar {
    public:
      LLVMLoadRegistrar() : LoaderRegistrar("LLVM") {
    }

      Loader_ptr makeLoader() override {
        return std::make_shared<LLVMLoader>();
      }
    };

    static LLVMLoadRegistrar llvmloadregistrar;

  } // namespace Loaders
} // namespace MiniMC
