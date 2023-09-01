
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <llvm/IR/AssemblyAnnotationWriter.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/GetElementPtrTypeIterator.h>
#include <llvm/IR/IRPrintingPasses.h>
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

#include <functional>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <type_traits>
#include <unordered_set>
#include <variant>

#include "llvmpasses.hpp"
#include "loaders/loader.hpp"
#include "model/cfg.hpp"
#include "support/exceptions.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"

#include "context.hpp"
#include "model/builder.hpp"

namespace MiniMC {
  namespace Loaders {

    class FunctionNotDefined : public MiniMC::Support::WarningMessage {
    public:
      FunctionNotDefined (const std::string& funcname) : funcname(funcname) {}
      std::ostream& to_string (std::ostream& os) const override {
	return os << loc.format (funcname);
      }
      
    private:
      std::string funcname;
      MiniMC::Support::Localiser loc{"Function '%1%' only declared in LLVM assembly. Treating it as a non-determinstic function"};
    };
    
    MiniMC::Model::TypeID getTypeID(llvm::Type* type);
    
    
    class LLVMLoader : public Loader {
    public:
      LLVMLoader(std::size_t stacksize,
                 std::vector<std::string> entry,
                 bool disablePromotion,
                 bool printPass) : stacksize(stacksize), entry(entry), disablePromotion(disablePromotion), printLLVMPass(printPass) {}
      MiniMC::Model::Program loadFromFile(const std::string& file, MiniMC::Model::TypeFactory_ptr& tfac, Model::ConstantFactory_ptr& cfac, MiniMC::Support::Messager& mess) override {
	std::fstream str;
        str.open(file);
        std::string ir((std::istreambuf_iterator<char>(str)), (std::istreambuf_iterator<char>()));
        std::unique_ptr<llvm::MemoryBuffer> buffer = llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(ir));
        return readFromBuffer(buffer, tfac, cfac,mess);
      }

      MiniMC::Model::Program loadFromString(const std::string& inp, MiniMC::Model::TypeFactory_ptr& tfac, Model::ConstantFactory_ptr& cfac,MiniMC::Support::Messager& mess) override {
        std::stringstream str;
        str.str(inp);
        std::string ir((std::istreambuf_iterator<char>(str)), (std::istreambuf_iterator<char>()));
        std::unique_ptr<llvm::MemoryBuffer> buffer = llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(ir));
        return readFromBuffer(buffer, tfac, cfac,mess);
      }

      auto createFunctionWorkList(llvm::Module& module) {
        std::unordered_set<llvm::Function*> functions;
        for (auto& F : module) {
          functions.insert(&F);
          for (auto& B : F) {
            for (auto& I : B) {
              if (auto cinst = llvm::dyn_cast<llvm::CallInst>(&I)) {
                functions.insert(cinst->getCalledFunction());
              }
            }
          }
        }
        return functions;
      }

      void loadGlobals(GLoadContext& lcontext, MiniMC::Model::Program& prgm, llvm::Module& module, MiniMC::Support::Messager&) {
        std::vector<MiniMC::Model::Instruction> instr;

        MiniMC::func_t fid = 0;
        for (auto& F : createFunctionWorkList(module)) {
	  std::string fname = F->getName().str();
	  auto fsymbol = prgm.getRootFrame ().makeSymbol (fname);
	  function2symb.emplace (F,fsymbol);
	  auto& Func = *F;
          auto ptr = lcontext.getConstantFactory().makeSymbolicConstant(fsymbol);
          ptr->setType(lcontext.getTypeFactory().makePointerType());
          lcontext.addValue(&Func, ptr);
          MiniMC::offset_t lid = 0;
          for (auto& BB : Func) {
            auto ptr = lcontext.getConstantFactory().makeLocationPointer(fid, lid);
            ptr->setType(lcontext.getTypeFactory().makePointerType());
            lcontext.addValue(&BB, ptr);
            lid++;
          }
          fid++;
        }

        for (auto& g : module.getGlobalList()) {
          auto pointTySize = lcontext.computeSizeInBytes(g.getValueType());
	  
	  auto the_pointer = prgm.getHeapLayout().addBlock(MiniMC::pointer_t::makeHeapPointer (++nextHeap,0),pointTySize);
	  auto gvar = lcontext.getConstantFactory().makeHeapPointer(MiniMC::getBase(the_pointer),MiniMC::getOffset (the_pointer));
          lcontext.addValue(&g, gvar);
          if (g.hasInitializer()) {
            auto val = lcontext.findValue(g.getInitializer());
            instr.push_back(MiniMC::Model::Instruction::make<MiniMC::Model::InstructionCode::Store>(gvar, val));
          }
        }
        if (instr.size()) {
          MiniMC::Model::InstructionStream str(instr);
          prgm.setInitialiser(str);
        }
      }

      void instantiateFunction(llvm::Function& F, GLoadContext& lcontext, MiniMC::Model::Program& prgm, MiniMC::Support::Messager& mess) {

        auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();
        std::string fname = F.getName().str();
        auto frame = prgm.getRootFrame().create(fname);
        MiniMC::Model::CFA cfg;
        std::vector<MiniMC::Model::Register_ptr> params;
        MiniMC::Model::RegisterDescr variablestack;
        MiniMC::Model::LocationInfoCreator locinfoc(variablestack);
   
        auto sp_mem = variablestack.addRegister(frame.makeFresh("sp_mem"), lcontext.getTypeFactory().makePointerType());

        LoadContext load{lcontext, variablestack, sp, sp_mem, frame};
        auto returnTy = load.getType(F.getReturnType());

        auto makeVariable = [&load, &frame](auto val) {
          if (!load.hasValue(val)) {
            auto type = load.getType(val->getType());
            load.addValue(val, load.getStack().addRegister(frame.makeSymbol(val->getName().str()), type));
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
          params.push_back(std::static_pointer_cast<MiniMC::Model::Register>(makeVariable(itt)));
        }

        if (F.isDeclaration()) {
          mess << FunctionNotDefined {F.getName().str()};

          auto iinit = locinfoc.make(MiniMC::Model::LocFlags{}, *source_loc);
          auto init = cfg.makeLocation(frame.makeSymbol("init"), iinit);
          auto einit = locinfoc.make(MiniMC::Model::LocFlags{}, *source_loc);
          auto end = cfg.makeLocation(frame.makeSymbol("exit"), einit);

          cfg.setInitial(init);
          {
            MiniMC::Model::EdgeBuilder edgebuilder{cfg, init, end, frame};
            if (returnTy->getTypeID() != MiniMC::Model::TypeID::Void) {
              std::size_t bitwidth = returnTy->getSize(); // inst->getType()->getIntegerBitWidth();
              MiniMC::Model::Value_ptr min, max;

              switch (bitwidth) {
	      case 1:
		min = prgm.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV8>::min(), MiniMC::Model::TypeID::I8);
		max = prgm.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV8>::max(), MiniMC::Model::TypeID::I8);
		break;
	      case 2:
		min = prgm.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV16>::min(), MiniMC::Model::TypeID::I16);
		max = prgm.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV16>::max(), MiniMC::Model::TypeID::I16);
		break;
	      case 4:
		min = prgm.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV32>::min(), MiniMC::Model::TypeID::I32);
		max = prgm.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV32>::max(), MiniMC::Model::TypeID::I32);
		break;
	      case 8:
		min = prgm.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV64>::min(), MiniMC::Model::TypeID::I64);
		max = prgm.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV64>::max(), MiniMC::Model::TypeID::I64);
		break;
	      default:
		throw MiniMC::Support::Exception("Error");
              }

              auto retVar = variablestack.addRegister(frame.makeFresh(), returnTy);

              edgebuilder.addInstr<MiniMC::Model::InstructionCode::NonDet>(retVar, min, max);
              edgebuilder.addInstr<MiniMC::Model::InstructionCode::Ret>(retVar);

            }

            else {
              edgebuilder.addInstr<MiniMC::Model::InstructionCode::RetVoid>();
            }
          }

          prgm.addFunction(function2symb.at (&F), params, returnTy, std::move(variablestack), std::move(cfg), F.isVarArg(), frame);
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
              auto info = locinfoc.make(MiniMC::Model::LocFlags{}, *source_loc);
              auto location = cfg.makeLocation(frame.makeSymbol(BB->getName().str()), info);
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
          cfg.setInitial(entry);
          InstructionTranslator translate{load};

          while (waiting.size()) {
            auto cur_bb = waiting.back();
            waiting.pop_back();
            auto from = locmap.at(cur_bb);
            auto to = cfg.makeLocation(frame.makeFresh(), from->getInfo());
            MiniMC::Model::EdgeBuilder edgebuilder{cfg, from, to, frame};
            auto term = cur_bb->getTerminator();

            for (llvm::Instruction& inst : *cur_bb) {

              if (llvm::isa<llvm::PHINode>(inst) || term == &inst) {
                continue;
              }

              translate(&inst, edgebuilder);
            }

            if (term) {
              if (term->getOpcode() == llvm::Instruction::Br) {

                auto brterm = llvm::dyn_cast<llvm::BranchInst>(term);
                if (brterm->isUnconditional()) {
                  auto succ = enqueue(term->getSuccessor(0));
                  MiniMC::Model::EdgeBuilder<true> builder{cfg, to, succ, frame};
                  buildphi(cur_bb, term->getSuccessor(0), builder);
                } else {
                  auto cond = load.findValue(brterm->getCondition());
                  {
                    auto ttloc = enqueue(term->getSuccessor(0));
                    auto ttloc_tmp = cfg.makeLocation(frame.makeFresh(), to->getInfo());
                    MiniMC::Model::EdgeBuilder{cfg, to, ttloc_tmp, frame}.addInstr<MiniMC::Model::InstructionCode::Assume>(cond);
                    buildphi(cur_bb, term->getSuccessor(0), MiniMC::Model::EdgeBuilder<true>{cfg, ttloc_tmp, ttloc, frame});
                  }

                  {
                    auto ffloc = enqueue(term->getSuccessor(1));
                    auto ffloc_tmp = cfg.makeLocation(frame.makeFresh(), to->getInfo());
                    MiniMC::Model::EdgeBuilder{cfg, to, ffloc_tmp, frame}.addInstr<MiniMC::Model::InstructionCode::NegAssume>(cond);
                    buildphi(cur_bb, term->getSuccessor(1), MiniMC::Model::EdgeBuilder<true>{cfg, ffloc_tmp, ffloc, frame});
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
                  auto btype = load.getTypeFactory().makeBoolType();
                  auto cond = load.getStack().addRegister(frame.makeFresh(), btype);

                  MiniMC::Model::EdgeBuilder{cfg, to, splitloc, frame}.addInstr<MiniMC::Model::InstructionCode::PtrEq>(cond,
		      value,
		      valComp)
		    .addInstr<MiniMC::Model::InstructionCode::Assume>(cond);
                  buildphi(cur_bb, brterm->getDestination(i), MiniMC::Model::EdgeBuilder<true>{cfg, splitloc, dest, frame});
                }
              } else if (term->getOpcode() == llvm::Instruction::Ret) {
                translate(term, edgebuilder);
              }
            }
          }
	  
          prgm.addFunction(function2symb.at(&F), params, returnTy, std::move(variablestack), std::move(cfg), F.isVarArg(), frame);
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

      MiniMC::Model::Function_ptr createEntryPoint(std::size_t stacksize, MiniMC::Model::Program& program, MiniMC::Model::Function_ptr function, std::vector<MiniMC::Model::Value_ptr>&&, const MiniMC::Model::Register_ptr& sp_reg) {
      static std::size_t nb = 0;
      const std::string name = MiniMC::Support::Localiser("__minimc__entry_%1%-%2%").format(function->getSymbol(), ++nb);
      auto frame = program.getRootFrame().create(name);
      MiniMC::Model::CFA cfg;
      MiniMC::Model::RegisterDescr vstack;
      MiniMC::Model::LocationInfoCreator locinf(vstack);

      auto funcpointer = program.getConstantFactory().makeSymbolicConstant(function->getSymbol());
      funcpointer->setType(program.getTypeFactory().makePointerType());
      auto iinfo = locinf.make({});
      auto init = cfg.makeLocation(frame.makeFresh("init"), iinfo);
      auto einfo = locinf.make({});

      auto end = cfg.makeLocation(frame.makeFresh("end"), einfo);

      cfg.setInitial(init);

      std::vector<MiniMC::Model::Value_ptr> params;
      MiniMC::Model::Value_ptr result = nullptr;
      auto the_pointer = program.getHeapLayout().addBlock(MiniMC::pointer_t::makeHeapPointer (++nextHeap,0),stacksize);
      MiniMC::Model::Value_ptr sp = program.getConstantFactory().makeHeapPointer(MiniMC::getBase (the_pointer),MiniMC::getOffset (the_pointer));
      sp->setType(program.getTypeFactory().makePointerType());

      MiniMC::Model::Value_ptr stacksize_p = program.getConstantFactory().makeIntegerConstant (stacksize,MiniMC::Model::TypeID::I64);
      MiniMC::Model::Value_ptr nb_skips = program.getConstantFactory().makeIntegerConstant (1,MiniMC::Model::TypeID::I64);
      
      
      
      auto restype = function->getReturnType();
      if (restype->getTypeID() != MiniMC::Model::TypeID::Void) {
        result = vstack.addRegister(frame.makeFresh(), restype);
      }
      {
	MiniMC::Model::EdgeBuilder builder{cfg, init, end, frame};
	builder.addInstr<MiniMC::Model::InstructionCode::PtrAdd>(sp_reg, sp,stacksize_p,nb_skips);
	builder.addInstr<MiniMC::Model::InstructionCode::Call>(result, funcpointer, params);
      }
      return program.addFunction(program.getRootFrame().makeSymbol (name), {},
                                 program.getTypeFactory().makeVoidType(),
                                 std::move(vstack),
                                 std::move(cfg),
                                 false,
                                 frame);
    }
      
      
      void setupEntryPoints(MiniMC::Model::Program& prgm) {
        for (const auto& e : entry) {
          auto func = prgm.getFunction(e);
          auto entry = createEntryPoint(stacksize, prgm, func, {},sp);
	  
          prgm.addEntryPoint(entry->getSymbol().getName());
        }
      }

      virtual MiniMC::Model::Program readFromBuffer(std::unique_ptr<llvm::MemoryBuffer>& buffer, MiniMC::Model::TypeFactory_ptr& tfac, MiniMC::Model::ConstantFactory_ptr& cfac, MiniMC::Support::Messager& mess) {
        MiniMC::Model::Program prgm {tfac, cfac};
	
        sp = prgm.getCPURegs().addRegister(prgm.getRootFrame().makeFresh("sp"), tfac->makePointerType());
        GLoadContext lcontext{*cfac, *tfac};

        llvm::SMDiagnostic diag;
        std::unique_ptr<llvm::LLVMContext> context = std::make_unique<llvm::LLVMContext>();
        std::unique_ptr<llvm::Module> module = parseIR(*buffer, diag, *context);
        if (!module) {
          throw LoadError{};
        }

        llvmModifications(*module,mess);
        loadGlobals(lcontext, prgm, *module,mess);
        instantiateFunctions(lcontext, prgm, *module, mess);

        setupEntryPoints(prgm);

        return prgm;
      }

    private:
      std::size_t stacksize;
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
        addOption<IntOption>("stack", "StackSize", 200uL);
        addOption<VecStringOption>("entry", "Entry point function", std::vector<std::string>{});
        addOption<BoolOption>("disable_promote_pass", "Disable the promotion of allocas to registers", false);
        addOption<BoolOption>("print", "Print LLVM module to stderr", false);
      }

      Loader_ptr makeLoader() override {
        auto stacksize = getOption<IntOption>(0).value;
        auto entry = getOption<VecStringOption>(1).value;
        auto disablePromotion = getOption<BoolOption>(2).value;
        auto printPass = getOption<BoolOption>(3).value;
        return std::make_unique<LLVMLoader>(stacksize, entry, disablePromotion, printPass);
      }
    };

    static LLVMLoadRegistrar llvmloadregistrar;

  } // namespace Loaders
} // namespace MiniMC
