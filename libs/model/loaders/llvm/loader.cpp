
#include <fstream>
#include <sstream>
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
#include <llvm/IR/IRPrintingPasses.h>
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
#include "support/pointer.hpp"
#include "llvminstuctioncreator.hpp"


namespace MiniMC {
  namespace Loaders {

    MiniMC::Model::Value_ptr makeConstant (llvm::Value* val, Types& tt, MiniMC::Model::ConstantFactory_ptr& fac, Val2ValMap& map) {
      auto constant = llvm::dyn_cast<llvm::Constant> (val);
      assert(constant);
      auto ltype = constant->getType ();
      if (ltype->isIntegerTy ()) {
		llvm::ConstantInt* csti = llvm::dyn_cast<llvm::ConstantInt> (constant);
		assert(csti);
		auto type = tt.getType(csti->getType());
		auto cst = fac->makeIntegerConstant(csti->getZExtValue (),type);
		return cst;
      }
      else if (ltype->isStructTy() || ltype->isArrayTy () ) {
		
		auto cstAggr = llvm::dyn_cast<llvm::ConstantDataSequential> (val);
		if (cstAggr) {
		  MiniMC::Model::ConstantFactory::aggr_input vals;
		  const size_t oper = cstAggr->getNumElements ();
		  for (size_t i = 0; i < oper;++i) {
			auto elem = cstAggr->getElementAsConstant(i);
			auto nconstant = makeConstant(elem,tt,fac,map);
			assert(nconstant->isConstant ());
			vals.push_back(std::static_pointer_cast<MiniMC::Model::Constant> (nconstant));
		  }
		  auto type = tt.getType (constant->getType ());
		  auto cst = fac->makeAggregateConstant (vals,ltype->isArrayTy ());
		  cst->setType (type);
		  return cst;
		}
		
		auto cstAggr2 = llvm::dyn_cast<llvm::ConstantAggregate> (val);
		if (cstAggr2) {
		  MiniMC::Model::ConstantFactory::noncompile_aggr_input vals;
		  const size_t oper = cstAggr2->getNumOperands ();
		  for (size_t i = 0; i < oper;++i) {
			auto elem = cstAggr2->getOperand(i);
			vals.push_back(makeConstant(elem,tt,fac,map));
		  }
		  auto type = tt.getType (constant->getType ());
		  auto cst = fac->makeAggregateConstantNonCompile (vals,ltype->isArrayTy ());
		  cst->setType (type);
		  return cst;
		  
		  
		  //assert(false && "FAil");
		}
		
		  
      }
      else if (llvm::isa<llvm::Function> (val) ||
			   llvm::isa<llvm::GlobalVariable> (val)
			   ) {
		return map.at (val);
      }
      else if (llvm::BlockAddress* block = llvm::dyn_cast<llvm::BlockAddress> (val)) {
		return map.at (block->getBasicBlock ());
      }
      else if (ltype->isPointerTy ()) {
		throw MiniMC::Support::Exception ("Pointer Not Quite there");
      }
      throw MiniMC::Support::Exception ("Error");
    }
    

    
    MiniMC::Model::Value_ptr findValue (llvm::Value* val, Val2ValMap& values, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      llvm::Constant* cst = llvm::dyn_cast<llvm::Constant> (val);
      if (cst) 
		return makeConstant (cst,tt,cfac,values);
      else {
		return values.at(val); 
      }
    }
    
        
    MiniMC::Model::Variable_ptr makeVariable (const llvm::Value* val, const std::string& name, MiniMC::Model::Type_ptr& type, MiniMC::Model::VariableStackDescr_ptr& stack,Val2ValMap& values) {
      if (!values.count (val)) {
		auto newVar = stack->addVariable (name,type);
		values[val] = newVar;
      }
      return std::static_pointer_cast<MiniMC::Model::Variable> (values[val]);
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
		std::string fname = F.getName().str();
		fname = fname+".";
		for (auto &Arg : F.args())
		  if (!Arg.hasName()) 
			Arg.setName(fname+"arg");
		  else 
			Arg.setName(fname+Arg.getName());
		for (llvm::BasicBlock &BB : F) {
		  if (!BB.hasName())
			BB.setName(fname+"bb");
		  else {
			BB.setName(fname+BB.getName());
		  }
		  for (llvm::Instruction &I : BB)
			if (!I.hasName())
			  I.setName(fname+"tmp");
			else
			  I.setName(fname+I.getName());
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

    struct RemoveUnusedInstructions : public llvm::PassInfoMixin<RemoveUnusedInstructions>
    {
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

    struct GlobalConstructor : public llvm::PassInfoMixin<GlobalConstructor> {
	  
      GlobalConstructor (MiniMC::Model::Program_ptr& prgm,
						 MiniMC::Model::TypeFactory_ptr& tfac,
						 MiniMC::Model::ConstantFactory_ptr& cfac,
						 Val2ValMap& values
						 ) : prgm(prgm),cfactory(cfac),tfactory(tfac),values(values) {
      }
    public:
      llvm::PreservedAnalyses run(llvm::Module &F, llvm::ModuleAnalysisManager& AM) {
		std::vector<MiniMC::Model::Instruction> instr;
		Types tt;
		tt.tfac = tfactory;

		MiniMC::func_t fid = 0;
		for (auto& Func : F) {
		  auto ptr = cfactory->makeFunctionPointer  (fid);
		  ptr->setType (tfactory->makePointerType ());
		  values.insert(std::make_pair (&Func,ptr));
		  
		  MiniMC::offset_t lid = 0;
		  for (auto& BB : Func) {
			auto ptr = cfactory->makeLocationPointer  (fid,lid);
			ptr->setType (tfactory->makePointerType ());
			values.insert (std::make_pair(&BB,ptr));
			lid++;
		  }
		  fid++;
		}

		auto gstack = prgm->getGlobals ().get();
		for (auto& g : F.getGlobalList()) {
		  auto lltype = g.getType ();
		  auto type = getType (lltype,tfactory);
		  auto gvar = makeVariable (&g,g.getName().str(),type,gstack,values);
		  gvar->setGlobal ();
		  auto pointTy = getType (g.getValueType(),tfactory);
		  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::FindSpace> spaceb;
		  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Malloc> mallocb;
		  auto sizeType = tt.tfac->makeIntegerType (64); 
		  auto size = cfactory->makeIntegerConstant(pointTy->getSize(),sizeType);
		  size->setType (sizeType);
		  spaceb.setResult (gvar);
		  spaceb.setSize (size);
		  mallocb.setPointer (gvar);
		  mallocb.setSize (size);
		  instr.push_back (spaceb.BuildInstruction ());
		  instr.push_back (mallocb.BuildInstruction ());
		  if (g.hasInitializer ()) {
			auto val = findValue (g.getInitializer (),values,tt,cfactory);
			MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Store> store;
			store.setValue (val);
			store.setAddress (gvar);
			instr.push_back (store.BuildInstruction ());
		  }
		  
		}
		if (instr.size()) {
		  MiniMC::Model::InstructionStream str (instr);
		  prgm->setInitialiser (str);
		}
	
		return llvm::PreservedAnalyses::all();
      }
    private:
      MiniMC::Model::Program_ptr& prgm;
      MiniMC::Model::TypeFactory_ptr& tfactory;
      MiniMC::Model::ConstantFactory_ptr& cfactory;
      Val2ValMap& values;
	  
    };
    
    struct Constructor : public llvm::PassInfoMixin<Constructor> {
      
      Constructor (MiniMC::Model::Program_ptr& prgm,
				   MiniMC::Model::TypeFactory_ptr& tfac,
				   MiniMC::Model::ConstantFactory_ptr& cfac,
				   Val2ValMap& val
				   ) : prgm(prgm),cfactory(cfac),tfactory(tfac),values(val) {
      }
      llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager& AM) {
		Types tt;
		tt.tfac = tfactory;
		std::string fname =  F.getName().str();
		auto cfg  = std::make_shared<MiniMC::Model::CFG> ();
		std::vector<gsl::not_null<MiniMC::Model::Variable_ptr>> params;
		auto variablestack =  prgm->makeVariableStack ();
		tt.stack = variablestack;
		using inserter = std::back_insert_iterator< std::vector<gsl::not_null<MiniMC::Model::Variable_ptr>>>;
		pickVariables <inserter> (F,variablestack,std::back_inserter(params));
		auto f = prgm->addFunction (F.getName().str(),params,tt.getType(F.getReturnType ()),variablestack,cfg);
		std::unordered_map<llvm::BasicBlock*,MiniMC::Model::Location_ptr> locmap;
	
		for (llvm::BasicBlock &BB : F) {
		  auto loc = cfg->makeLocation(BB.getName().str());
		  locmap.insert (std::make_pair(&BB,loc)); 
		}
	
	
		
		auto& entry = F.getEntryBlock ();
		cfg->setInitial (locmap.at(&entry));
		for (llvm::BasicBlock &BB : F) {
		  auto loc = locmap.at(&BB);
		  auto term = BB.getTerminator ();
		  std::vector<MiniMC::Model::Instruction> insts;
		  for (llvm::Instruction& inst : BB) {
			if (llvm::isa<llvm::PHINode> (inst)) {
			  continue;
			}
			if (&inst!=term) {
			  addInstruction (&inst,insts,tt);
			}
			if (llvm::isa<llvm::CallInst> (inst)) {
			  loc->set<MiniMC::Model::Location::Attributes::CallPlace> ();
			  auto mloc = cfg->makeLocation (loc->getName () + ":AC");
			  auto edge = cfg->makeEdge (loc,mloc,prgm);
			  if (insts.size())
				edge->template setAttribute<MiniMC::Model::AttributeType::Instructions> (insts);
			  insts.clear();
			  loc = mloc;
			}
		  }
		  
		  if (insts.size()) {
			auto mloc = cfg->makeLocation (loc->getName () + ":S");
			auto edge = cfg->makeEdge (loc,mloc,prgm);
			edge->template setAttribute<MiniMC::Model::AttributeType::Instructions> (insts);
			loc = mloc;
			insts.clear();
		  }
		  
		  if (term) {
			assert(!insts.size ());
			if( term->getOpcode () == llvm::Instruction::Br) {
		  
			  auto brterm = llvm::dyn_cast<llvm::BranchInst> (term);
			  if (brterm->isUnconditional ()) {
				std::vector<MiniMC::Model::Instruction> insts;
				auto succ = term->getSuccessor (0);
				auto succloc =  buildPhiEdge (&BB,succ,*cfg,tt,locmap);
				//locmap.at(succ);
				auto edge = cfg->makeEdge (loc,succloc,prgm);
				if (insts.size())
				  edge->template setAttribute<MiniMC::Model::AttributeType::Instructions> (insts);
			  }
			  else {
				auto cond = findValue (brterm->getCondition(),values,tt,cfactory);
				auto ttloc = buildPhiEdge (&BB,term->getSuccessor (0),*cfg,tt,locmap);
				auto ffloc = buildPhiEdge (&BB,term->getSuccessor (1),*cfg,tt,locmap);
				auto edge = cfg->makeEdge (loc,ttloc,prgm);
				edge->setAttribute<MiniMC::Model::AttributeType::Guard> (MiniMC::Model::Guard (cond,false));
				edge = cfg->makeEdge (loc,ffloc,prgm);
				edge->setAttribute<MiniMC::Model::AttributeType::Guard> (MiniMC::Model::Guard (cond,true));
			  }
			}
			else if ( term->getOpcode () == llvm::Instruction::IndirectBr) {
			  auto brterm = llvm::dyn_cast<llvm::IndirectBrInst> (term);
			  auto splitloc = cfg->makeLocation ("Indirect");
			  std::size_t dests = brterm->getNumDestinations ();
			  auto value = findValue (brterm->getAddress(),values,tt,cfactory);
			  for (std::size_t i = 0; i < dests;++i) {
				auto valComp = findValue (brterm->getDestination (i),values,tt,cfactory);
				auto btype = tfactory->makeBoolType ();
				auto cond = variablestack->addVariable ("",btype);
				MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::PtrEq> builder;
				builder.setLeft (value);
				builder.setRight (valComp);
				builder.setRes (cond);
				auto ttloc = buildPhiEdge (&BB,brterm->getDestination (i),*cfg,tt,locmap);
				auto edge = cfg->makeEdge (splitloc,ttloc,prgm);
				edge->setAttribute <MiniMC::Model::AttributeType::Guard> (MiniMC::Model::Guard (cond,false));
				insts.push_back (builder.BuildInstruction ());
			  }
			  //splitloc->set<MiniMC::Model::Location::Attributes::LoopEntry> ();
			  auto nedge = cfg->makeEdge (loc,splitloc,prgm);
			  nedge->setAttribute <MiniMC::Model::AttributeType::Instructions> (insts);
			}
		    
			else if( term->getOpcode () == llvm::Instruction::Ret) {
			  std::vector<MiniMC::Model::Instruction> insts;
			  addInstruction (term,insts,tt);
			  auto succloc = cfg->makeLocation (fname+"."+"Term");
			  auto edge = cfg->makeEdge (loc,succloc,prgm);
			  edge->template setAttribute<MiniMC::Model::AttributeType::Instructions> (insts);
			}
		    
		  }
		}		
		f->takeOwnsership ();
		return llvm::PreservedAnalyses::all();
      }

      MiniMC::Model::Location_ptr buildPhiEdge (llvm::BasicBlock* from, llvm::BasicBlock* to, MiniMC::Model::CFG& cfg,Types& tt, std::unordered_map<llvm::BasicBlock*,MiniMC::Model::Location_ptr>& locmap) {
		std::vector<MiniMC::Model::Instruction> insts;
		static std::unordered_map<llvm::BasicBlock*,int> phiNum;
		for (auto& phi : to->phis ()) {
		  auto ass = findValue (&phi,values,tt,cfactory);
		  auto incoming = findValue (phi.getIncomingValueForBlock(from),values,tt,cfactory);
		  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Assign> builder;
		  builder.setValue(incoming);
		  builder.setResult (ass);
		  insts.push_back (builder.BuildInstruction());
		}
		auto loc = locmap.at(to);
		if (insts.size()) {
		  auto nloc = cfg.makeLocation (to->getName().str()+":PHI:"+std::to_string (phiNum[to]++));
		  auto edge = cfg.makeEdge (nloc,loc,prgm);
		  edge->setAttribute<MiniMC::Model::AttributeType::Instructions> (MiniMC::Model::InstructionStream (insts,true));
		  return nloc;
		}
		else {
		  return loc;
		}
      }
      
      template<class Inserter>
      void pickVariables (const llvm::Function& func,MiniMC::Model::VariableStackDescr_ptr stack, Inserter in ) {
		for (auto itt = func.arg_begin();itt!=func.arg_end(); itt++) {
		  auto lltype = itt->getType ();
		  auto type = getType (lltype,tfactory);
		  in = makeVariable (itt,itt->getName().str(),type,stack,values);
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
		if (lltype->isLabelTy () ||
			lltype->isVoidTy ()
			)
		  return ;
		auto type = getType (op->getType(),tfactory);
		if (oop) {
		  return ;
		}
		else {
		  makeVariable (op,op->getName().str(),type,stack,values);
		}
      }
	  
#define SUPPORTEDLLVM							\
      X(Add)									\
      X(Sub)									\
      X(Mul)									\
      X(UDiv)									\
      X(SDiv)									\
      X(Shl)									\
      X(LShr)									\
      X(AShr)									\
      X(And)									\
      X(Or)										\
      X(Xor)									\
      X(ICmp)									\
      X(Trunc)									\
      X(ZExt)									\
      X(SExt)									\
      X(PtrToInt)								\
      X(IntToPtr)								\
      X(BitCast)								\
      X(Load)									\
      X(Store)									\
      X(Alloca)									\
      X(GetElementPtr)							\
      X(InsertValue)							\
		X(ExtractValue)							\
      X(Call)									\
      X(Ret)									\
	  
	  
	  
	  
	  
      void addInstruction (llvm::Instruction* inst, std::vector<MiniMC::Model::Instruction>& insts,Types& tt) {
		switch (inst->getOpcode()) {
#define X(TT)															\
		  case llvm::Instruction::TT:									\
			translateAndAddInstruction<llvm::Instruction::TT> (inst,values,insts,tt,cfactory); \
			break;
		  SUPPORTEDLLVM
		default:
		  throw MiniMC::Support::Exception ("Unsupported operation");
		}
	    
	    
      }
			       
	  
    private:
      MiniMC::Model::Program_ptr& prgm;
      MiniMC::Model::TypeFactory_ptr& tfactory;
      MiniMC::Model::ConstantFactory_ptr& cfactory;
      Val2ValMap& values;
	  
    };
	
    class LLVMLoader  {
	public:
      virtual MiniMC::Model::Program_ptr loadFromFile (const std::string& file, MiniMC::Model::TypeFactory_ptr& tfac, MiniMC::Model::ConstantFactory_ptr& cfac) {
		std::fstream str;
		str.open (file);
		std::string ir((std::istreambuf_iterator<char>(str)), (std::istreambuf_iterator<char>()));
		std::unique_ptr<llvm::MemoryBuffer> buffer= llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(ir));
		return readFromBuffer (buffer,tfac,cfac);
	  }

	  virtual MiniMC::Model::Program_ptr loadFromString (const std::string& inp, MiniMC::Model::TypeFactory_ptr& tfac, MiniMC::Model::ConstantFactory_ptr& cfac) {
		std::stringstream str;
		str.str(inp);
		std::string ir((std::istreambuf_iterator<char>(str)), (std::istreambuf_iterator<char>()));
		std::unique_ptr<llvm::MemoryBuffer> buffer= llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(ir));
		return readFromBuffer (buffer,tfac,cfac);
	  }
	  
	  virtual MiniMC::Model::Program_ptr readFromBuffer (std::unique_ptr<llvm::MemoryBuffer>& buffer, MiniMC::Model::TypeFactory_ptr& tfac, MiniMC::Model::ConstantFactory_ptr& cfac) {	
		auto prgm = std::make_shared<MiniMC::Model::Program> (tfac,cfac);
		
		llvm::SMDiagnostic diag;
		std::unique_ptr<llvm::LLVMContext> context = std::make_unique<llvm::LLVMContext> ();
		std::unique_ptr<llvm::Module> module = parseIR(*buffer, diag,*context);
		
		llvm::PassBuilder PB;
		
		llvm::LoopAnalysisManager lam;
		llvm::FunctionAnalysisManager fam;
		llvm::CGSCCAnalysisManager cgam;
		llvm::ModuleAnalysisManager mam;
		
		llvm::LoopPassManager loopmanager;
		llvm::FunctionPassManager funcmanagerllvm;
		llvm::FunctionPassManager funcmanager;
		llvm::ModulePassManager mpm;
		Val2ValMap values;
		
		PB.registerFunctionAnalyses (fam);
		PB.registerModuleAnalyses (mam);
		PB.registerLoopAnalyses(lam);
		PB.registerCGSCCAnalyses(cgam);
		PB.crossRegisterProxies(lam, fam, cgam, mam);
		
		funcmanagerllvm.addPass (ConstExprRemover());
		funcmanagerllvm.addPass (RemoveUnusedInstructions());
		funcmanager.addPass (llvm::PromotePass());
		funcmanagerllvm.addPass (GetElementPtrSimplifier());
		funcmanagerllvm.addPass (InstructionNamer());
		mpm.addPass (llvm::createModuleToFunctionPassAdaptor(std::move(funcmanagerllvm)));	
		//mpm.addPass (llvm::PrintModulePass (llvm::errs()));
		
		mpm.addPass (GlobalConstructor (prgm,tfac,cfac,values));	
		
		funcmanager.addPass (Constructor(prgm,tfac,cfac,values));
		mpm.addPass (llvm::createModuleToFunctionPassAdaptor(std::move(funcmanager)));
		
		
		mpm.run (*module,mam);
		
		return prgm;
      }
    };
    
    
	template<>
	MiniMC::Model::Program_ptr loadFromFile<Type::LLVM,BaseLoadOptions > (const std::string& file, BaseLoadOptions loadOptions) {
	  return LLVMLoader{}.loadFromFile (file,loadOptions.tfactory,loadOptions.cfactory);
	}

	template<>
	MiniMC::Model::Program_ptr loadFromString<Type::LLVM,BaseLoadOptions > (const std::string& inp, BaseLoadOptions loadOptions) {
	  return LLVMLoader{}.loadFromString (inp,loadOptions.tfactory,loadOptions.cfactory);
	}
	
    
  }
}
