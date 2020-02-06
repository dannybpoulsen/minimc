#ifndef _INSTRUCTIONS__
#define _INSTRUCTIONS__
#include <cassert>
#include <ostream>
#include "model/variables.hpp"

namespace MiniMC {
  namespace Model {
#define TACOPS								\
    X(Add)								\
    X(Sub)								\
    X(Mul)								\
    X(UDiv)								\
    X(SDiv)								\
    X(Shl)								\
    X(LShr)								\
    X(AShr)								\
    X(And)								\
    X(Or)								\
    X(Xor)
	
#define COMPARISONS							\
    X(ICMP_SGT)								\
    X(ICMP_UGT)								\
    X(ICMP_SGE)								\
    X(ICMP_UGE)								\
    X(ICMP_SLT)								\
    X(ICMP_ULT)								\
    X(ICMP_SLE)								\
    X(ICMP_ULE)								\
    X(ICMP_EQ)								\
    X(ICMP_NEQ)								\
    
    
#define POINTEROPS				\
    X(PtrAdd)					\

#define AGGREGATEOPS				\
    X(ExtractValue)				\
    X(InsertValue)				\
    X(InsertValueFromConst)			\
    
    
#define CASTOPS					\
    X(Trunc)					\
    X(ZExt)					\
    X(SExt)					\
    X(PtrToInt)					\
    X(IntToPtr)					\
    X(BitCast)					\
    

#define MEMORY					\
    X(Alloca)					\
    X(Store)					\
    X(Load)					\

#define INTERNAL				\
    X(Skip)					\
    X(Call)					\
    X(Assign)					\
    X(Ret)					\
    X(RetVoid)					\
    X(NonDet)					\
    X(Assert)					\
    X(StackRestore)				\
    X(StackSave)				\
    
    enum class InstructionCode {
#define X(OP)					\
				OP,
				TACOPS
				COMPARISONS
				CASTOPS
				MEMORY
				INTERNAL
				POINTEROPS
				AGGREGATEOPS
#undef X
    };

    inline std::ostream& operator<< (std::ostream& os, const InstructionCode& c) {
      switch (c) {
#define X(OP)					\
	case InstructionCode::OP:		\
	  return os << #OP;			
		TACOPS
	  COMPARISONS
	  MEMORY
	  CASTOPS
	  INTERNAL
	  AGGREGATEOPS
#undef X
      default:
	return os << "Unknown";
	
      }	
    }
    
    template<InstructionCode>
    struct InstructionData{
      static const bool isTAC = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const std::size_t operands = 0;
      static const bool hasResVar = false;
    };

#define X(OP)							\
    template<>							\
    struct InstructionData<InstructionCode::OP>{		\
      static const bool isTAC = true;				\
      static const bool isMemory = false;			\
      static const bool isComparison = false;			\
      static const bool isCast = false;				\
      static const bool isPointer = false;			\
      static const bool isAggregate = false;			\
      static const std::size_t operands = 2;			\
      static const bool hasResVar = true;			\
    };
    TACOPS
#undef X

#define X(OP)											\
    template<>											\
    struct InstructionData<InstructionCode::OP>{		\
      static const bool isTAC = false;					\
      static const bool isMemory = false;				\
      static const bool isComparison = true;				\
      static const bool isCast = false;					\
      static const bool isPointer = false;				\
      static const bool isAggregate = false;			\
      static const std::size_t operands = 2;			\
      static const bool hasResVar = true;				\
    };
    COMPARISONS
#undef X
	
#define X(OP)							\
    template<>							\
    struct InstructionData<InstructionCode::OP>{		\
      static const bool isTAC = false;				\
      static const bool isComparison = false;			\
      static const bool isMemory = false;			\
      static const bool isCast = true;				\
      static const bool isPointer = false;			\
      static const bool isAggregate = false;			\
      static const std::size_t operands = 1;			\
      static const bool hasResVar = true;			\
    };
    CASTOPS
#undef X

    template<>							
    struct InstructionData<InstructionCode::ExtractValue>{		
      static const bool isTAC = false;
	  static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;				
      static const bool isPointer = false;			
      static const bool isAggregate = true;
      static const std::size_t operands = 3;			
      static const bool hasResVar = true;			
    };
    
    template<>							
    struct InstructionData<InstructionCode::InsertValue>{		
      static const bool isTAC = false;
	  static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;				
      static const bool isPointer = false;			
      static const bool isAggregate = true;
      static const std::size_t operands = 3;			
      static const bool hasResVar = true;			
    };
    
    template<>							
    struct InstructionData<InstructionCode::InsertValueFromConst>{		
      static const bool isTAC = false;
	  static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;				
      static const bool isPointer = false;			
      static const bool isAggregate = true;
      static const std::size_t operands = 0;			
      static const bool hasResVar = true;			
    };

    template<>							
    struct InstructionData<InstructionCode::PtrAdd>{		
      static const bool isTAC = false;
	  static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;				
      static const bool isPointer = true;			
      static const bool isAggregate = false;			
      static const std::size_t operands = 4;			
      static const bool hasResVar = true;			
    };


    
    template<>						
    struct InstructionData<InstructionCode::Alloca> {		
      static const bool isTAC = false;
	  static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;			
      static const std::size_t operands = 1;			
      static const bool hasResVar = true;			
    };

    template<>						
    struct InstructionData<InstructionCode::Skip> {		
      static const bool isTAC = false;
	  static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const std::size_t operands = 0;			
      static const bool hasResVar = false;			
    };
    
    template<>						
    struct InstructionData<InstructionCode::NonDet> {		
      static const bool isTAC = false;
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const std::size_t operands = 0;			
      static const bool hasResVar = true;			
    };

    template<>						
    struct InstructionData<InstructionCode::Assert> {		
      static const bool isTAC = false;
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = true;			
    };

    template<>						
    struct InstructionData<InstructionCode::StackSave> {		
      static const bool isTAC = false;
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const std::size_t operands = 0;			
      static const bool hasResVar = true;			
    };

    template<>						
    struct InstructionData<InstructionCode::StackRestore> {		
      static const bool isTAC = false;
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = false;			
    };
    
    template<>						
    struct InstructionData<InstructionCode::Load>{		
      static const bool isTAC = false;
	  static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = true;			
    };
    
    template<>						
    struct InstructionData<InstructionCode::Store>{		
      static const bool isTAC = false;
	  static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const std::size_t operands = 2;			
      static const bool hasResVar = false;			
    };

    class Function;
    using Function_ptr = std::shared_ptr<Function>;
    
    struct Instruction {
    public:
      Instruction (InstructionCode code, std::initializer_list<Value_ptr> ops) : opcode(code),
										 ops(ops),
										 parent(nullptr)
      {}
      Instruction (InstructionCode code, std::vector<Value_ptr> ops) : opcode(code),
								       ops(ops),
								        parent(nullptr)
      {}
      void replace (const Instruction& i) {
	opcode = i.opcode;
	ops = i.ops;
      }
      auto getOpcode () const {return opcode;}
      auto& getOps () const {return ops;}
      auto& getOp (std::size_t i) const {return ops.at(i);}
      auto getNbOps () const {return ops.size();}
      std::ostream& output (std::ostream& os) const; /*{
	os << "(" << opcode;
	for (auto& op : ops) {
	  os << " " << *op; 
	}
	return os << ")";
      }*/
      const Function_ptr& getFunction () const {return parent;}
      void setFunction (const Function_ptr& par) {parent = par;}
    private:
      InstructionCode opcode;
      std::vector<Value_ptr> ops;
      Function_ptr parent;
    };

    inline std::ostream& operator<< (std::ostream& os, const Instruction& inst) {
      return inst.output(os);
    }
    
    template<InstructionCode i,class T = void>
    class InstHelper;

    template<InstructionCode i,class T = void>
    class InstBuilder;

    template<InstructionCode i,class T = void>
    struct Formatter {
      static std::ostream& output (std::ostream& os, const Instruction&) { return os << "??";} 
    };

    
    
    template<InstructionCode i>
    class InstHelper<i,typename std::enable_if<InstructionData<i>::isTAC>::type> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
      auto getLeftOp () const {return inst.getOp(1);}
      auto getRightOp () const {return inst.getOp(2);}
      
      private:
      const Instruction& inst;
    };

    template<InstructionCode i>
    class InstBuilder<i,typename std::enable_if<InstructionData<i>::isTAC>::type> {
    public:
      void setRes (const Value_ptr& ptr) {res = ptr;}
      void setLeft (const Value_ptr& ptr) {left = ptr;}
      void setRight (const Value_ptr& ptr) {right = ptr;}
      Instruction BuildInstruction () {
		assert(res);
		assert(left);
		assert(right);
		return Instruction (i,{res,left,right});
      }
    private:
      Value_ptr res;
      Value_ptr left;
      Value_ptr right;
    };

	template<InstructionCode i> 
    struct Formatter<i,typename std::enable_if<InstructionData<i>::isTAC>::type> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
		InstHelper<i> h (inst);
		return os << *h.getResult () << " = " << i << " " << *h.getLeftOp () << " " << *h.getRightOp ();
      } 
    };
	
	template<InstructionCode i>
    class InstBuilder<i,typename std::enable_if<InstructionData<i>::isComparison>::type> {
    public:
      void setRes (const Value_ptr& ptr) {res = ptr;}
      void setLeft (const Value_ptr& ptr) {left = ptr;}
      void setRight (const Value_ptr& ptr) {right = ptr;}
      Instruction BuildInstruction () {
		assert(res);
		assert(left);
		assert(right);
		return Instruction (i,{res,left,right});
      }
    private:
      Value_ptr res;
      Value_ptr left;
      Value_ptr right;
    };

	template<InstructionCode i>
    class InstHelper<i,typename std::enable_if<InstructionData<i>::isComparison>::type> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
      auto getLeftOp () const {return inst.getOp(1);}
      auto getRightOp () const {return inst.getOp(2);}
      
      private:
      const Instruction& inst;
    };
	
    template<InstructionCode i> 
    struct Formatter<i,typename std::enable_if<InstructionData<i>::isComparison>::type> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<i> h (inst);
	return os << *h.getResult () << " = " << i << " " <<  *h.getLeftOp () << " " << *h.getRightOp ();
      } 
    };

    template<InstructionCode i>
    class InstHelper<i,typename std::enable_if<InstructionData<i>::isCast>::type> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
      auto& getCastee () const {return inst.getOp(1);}
      
    private:
      const Instruction& inst;
    };

    
    template<InstructionCode i>
    class InstBuilder<i,typename std::enable_if<InstructionData<i>::isCast>::type> {
    public:
      void setRes (const Value_ptr& ptr) {res = ptr;}
      void setCastee (const Value_ptr& ptr) {castee = ptr;}
      Instruction BuildInstruction () {
	assert(res);
	assert(castee);
	return Instruction (i,{res,castee});
      }
    private:
      Value_ptr res;
      Value_ptr castee;
    };
    
    template<InstructionCode i> 
    struct Formatter<i,typename std::enable_if<InstructionData<i>::isCast>::type> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<i> h (inst);
	return os << *h.getResult () << " = " << i << *h.getCastee ();
      } 
    };
    
    template<>
    class InstHelper<InstructionCode::Alloca,void> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
      auto& getSize () const {return inst.getOp(1);}
      
      private:
      const Instruction& inst;
    };


    template<>
    class InstBuilder<InstructionCode::Alloca,void> {
    public:
      void setRes (const Value_ptr& ptr) {res = ptr;}
      void setSize (const Value_ptr& ptr) {size = ptr;}
      Instruction BuildInstruction () {
	assert(res);
	assert(size);
	return Instruction (InstructionCode::Alloca,{res,size});
      }
    private:
      Value_ptr res;
      Value_ptr size;
    };

    template<> 
    struct Formatter<InstructionCode::Alloca,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::Alloca> h (inst);
	return os << *h.getResult () << " = " << InstructionCode::Alloca << *h.getSize ();
      } 
    };
    
    template<>
    class InstHelper<InstructionCode::Skip,void> {
    public:
      
      InstHelper (Instruction& inst) : inst(inst) {}
    private:
      Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::Skip,void> {
    public:
      Instruction BuildInstruction () {
	return Instruction (InstructionCode::Skip,{});
      }
    
    };

    template<> 
    struct Formatter<InstructionCode::Skip,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::Alloca> h (inst);
	return os <<  InstructionCode::Skip;
      } 
    };


    //
    template<>
    class InstHelper<InstructionCode::NonDet,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
    private:
      const Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::NonDet,void> {
    public:
      Instruction BuildInstruction () {
	return Instruction (InstructionCode::NonDet,{res});
      }
     
      void setResult (const Value_ptr& p) {res = p;}
      
    private:
      Value_ptr res = nullptr;
    
    };

    template<> 
    struct Formatter<InstructionCode::NonDet,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::NonDet> h (inst);
	return os << *h.getResult() << "=" <<  InstructionCode::NonDet;
      } 
    };
    //

    template<>
    class InstHelper<InstructionCode::StackSave,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
    private:
      const Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::StackSave,void> {
    public:
      Instruction BuildInstruction () {
	return Instruction (InstructionCode::StackSave,{res});
      }
     
      void setResult (const Value_ptr& p) {res = p;}
      
    private:
      Value_ptr res = nullptr;
    
    };

    template<> 
    struct Formatter<InstructionCode::StackSave,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::StackSave> h (inst);
	return os << *h.getResult() << "=" <<  InstructionCode::StackSave;
      } 
    };
    
    //

    template<>
    class InstHelper<InstructionCode::StackRestore,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getValue () const {return inst.getOp(0);}
    private:
      const Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::StackRestore,void> {
    public:
      Instruction BuildInstruction () {
	return Instruction (InstructionCode::StackRestore,{res});
      }
      
      void setValue (const Value_ptr& p) {res = p;}
      
    private:
      Value_ptr res = nullptr;
    
    };

    template<> 
    struct Formatter<InstructionCode::StackRestore,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::StackRestore> h (inst);
	return os <<  InstructionCode::StackRestore << "( " << *h.getValue() << " )";
      } 
    };
    


    //
    template<>
    class InstHelper<InstructionCode::Assert,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getAssert () const {return inst.getOp(0);}
    private:
      const Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::Assert,void> {
    public:
      Instruction BuildInstruction () {
	return Instruction (InstructionCode::Assert,{ass});
      }
     
      void setAssert (const Value_ptr& p) {ass = p;}
      
    private:
      Value_ptr ass = nullptr;
    
    };

    template<> 
    struct Formatter<InstructionCode::Assert,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::Assert> h (inst);
	return os << InstructionCode::Assert << "(" << *h.getAssert() << ")"  ;
      } 
    };
    //
    
    template<>
    class InstHelper<InstructionCode::Call,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {
	nbparams = std::static_pointer_cast<MiniMC::Model::IntegerConstant> (inst.getOp (1))->getValue();
	
      }
      
      auto nbParams ()  const {return nbparams;}
      auto getFunctionPtr () {return inst.getOp (0);}
      auto getParam (std::size_t p) {return inst.getOp (3+p);}
      auto getRes () {
	return inst.getOp (2);
      }
      
    private:
      const Instruction& inst;
      std::size_t nbparams;
      
    };
    
    template<>
    class InstBuilder<InstructionCode::Call,void> {
    public:
      Instruction BuildInstruction () {
	std::vector<Value_ptr> values;
	values.push_back (func);
	values.push_back (nbParameters);
	values.push_back(res);
	std::copy(params.begin(),params.end(),std::back_inserter(values));
	return Instruction (InstructionCode::Call,values);
      }

      void setFunctionPtr (const Value_ptr& func) {
	this->func = func;
      }

      void setRes (const Value_ptr& res) {
	this->res = res;
      }

      void setNbParamters (const Value_ptr& p) {
	assert(p->isConstant ());
	this->nbParameters = p;
      }

      void addParam (const Value_ptr& p) {
	params.push_back (p);
      }
      
      
    private:
      Value_ptr func;
      Value_ptr nbParameters;
      Value_ptr res = nullptr;
      std::vector<Value_ptr> params;
      
    };
    
    template<> 
    struct Formatter<InstructionCode::Call,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	//InstHelper<InstructionCode::Call> h (inst);
	return os <<  InstructionCode::Call;
      } 
    };
    
    
    
    template<>
    class InstHelper<InstructionCode::PtrAdd,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getValue () const {return inst.getOp(0);}
      auto& getAddress () const {return inst.getOp(1);}
      auto& getSkipSize () const {return inst.getOp(2);}
      auto& getResult () const {return inst.getOp(3);}
    private:
      const Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::PtrAdd,void> {
    public:
      void setSkipSize (const Value_ptr& ptr) {skipSize = ptr;}
      void setValue (const Value_ptr& ptr) {value = ptr;}
      void setAddress (const Value_ptr& ptr) {address = ptr;}
      void setResult (const Value_ptr& ptr) {result = ptr;}
      Instruction BuildInstruction () {
	return Instruction (InstructionCode::PtrAdd,{value,address,skipSize,result});
      }
      
    private:
      Value_ptr skipSize;
      Value_ptr value;
      Value_ptr address;
      Value_ptr result;
    };

    template<> 
    struct Formatter<InstructionCode::PtrAdd,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::PtrAdd> h (inst);
	return os << *h.getResult () << " = " << *h.getAddress () << " + " << *h.getSkipSize() << "*"<<*h.getValue ();
      } 
    };
    
    
    template<>
    class InstHelper<InstructionCode::ExtractValue,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getAggregate () const {return inst.getOp (0);}
      auto& getOffset () const {return inst.getOp (1);}
      auto& getResult () const {return inst.getOp (2);}
      
    private:
      const Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::ExtractValue,void> {
    public:
      void setAggregate (const Value_ptr& aggr) {aggregate = aggr;}
      void setOffset (const Value_ptr& ptr) {offset = ptr;}
      void setResult (const Value_ptr& ptr) {res = ptr;}
      Instruction BuildInstruction () {
	return Instruction (InstructionCode::ExtractValue,{aggregate,offset,res});
      }
      
    private:
      Value_ptr aggregate;
      Value_ptr offset;
      Value_ptr res;
    };

    template<> 
    struct Formatter<InstructionCode::ExtractValue,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::ExtractValue> h (inst);
	return os << *h.getResult () << " = " << *h.getAggregate () << " [ " << *h.getOffset() << " ] ";
      } 
    };
    

    template<>
    class InstHelper<InstructionCode::Assign,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getValue () const  {return inst.getOp (1);} 
      auto& getResult () const {return inst.getOp (0);}
      
    private:
      const Instruction& inst;
    };

    template<> 
    struct Formatter<InstructionCode::Assign,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::Assign> h (inst);
	return os << *h.getResult () << " = " << *h.getValue ();
      } 
    };
    
    template<>
    class InstBuilder<InstructionCode::Assign,void> {
    public:
      void setValue (const Value_ptr& aggr) {value = aggr;}
      void setResult (const Value_ptr& ptr) {res = ptr;}
      Instruction BuildInstruction () {
	return Instruction (InstructionCode::Assign,{res,value});
      }
      
    private:
      Value_ptr value;
      Value_ptr res;
    };


    template<>
    class InstHelper<InstructionCode::RetVoid,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      
    private:
      const Instruction& inst;
    };
    
    template<>
    class InstBuilder<InstructionCode::RetVoid,void> {
    public:
      Instruction BuildInstruction () {
	return Instruction (InstructionCode::RetVoid,{});
      }
    };
    
    template<> 
    struct Formatter<InstructionCode::RetVoid,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::RetVoid> h (inst);
	return os << "RetVoid";
      } 
    };

    //
    template<>
    class InstHelper<InstructionCode::Ret,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getValue () const {return inst.getOp(0);}
    private:
      const Instruction& inst;
    };
    
    template<>
    class InstBuilder<InstructionCode::Ret,void> {
    public:
      Instruction BuildInstruction () {
	return Instruction (InstructionCode::Ret,{value});
      }

      void setRetValue (const Value_ptr& value)  {
	this->value = value;
      }

    private:
      Value_ptr value;
    };
    
    template<> 
    struct Formatter<InstructionCode::Ret,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::Ret> h (inst);
	return os << "Ret " << *h.getValue ();
      } 
    };


    //
    template<>
    class InstHelper<InstructionCode::InsertValue,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getAggregate () const  {return inst.getOp (0);}
      auto& getOffset () const {return inst.getOp (1);}
      auto& getInsertee () const {return inst.getOp (3);} 
      auto& getResult () const {return inst.getOp (2);}
      
    private:
      const Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::InsertValue,void> {
    public:
      void setAggregate (const Value_ptr& aggr) {aggregate = aggr;}
      void setOffset (const Value_ptr& ptr) {offset = ptr;}
      void setResult (const Value_ptr& ptr) {res = res;}
      void setInsertee (const Value_ptr& ptr)  {insertee = ptr;}
      Instruction BuildInstruction () {
	return Instruction (InstructionCode::PtrAdd,{aggregate,offset,res,insertee});
      }
      
    private:
      Value_ptr aggregate;
      Value_ptr offset;
      Value_ptr insertee;
      Value_ptr res;
    };

    template<>
    class InstHelper<InstructionCode::InsertValueFromConst,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getAggregate () const  {return inst.getOp (0);}
      size_t nbOps () const {return inst.getNbOps () - 3;}
      auto& getOp (size_t op) const {return inst.getOp (op+3);}
      auto& getInsertee () const {return inst.getOp (2);} 
      auto& getResult () const {return inst.getOp (1);}
      
    private:
      const Instruction& inst;
    };
    
    template<>
    class InstBuilder<InstructionCode::InsertValueFromConst,void> {
    public:
      void setAggregate (const Value_ptr& aggr) {aggregate = aggr;}
      void addOps (const Value_ptr& ptr) {consts.push_back(ptr);}
      void setResult (const Value_ptr& ptr) {res = res;}
      void setInsertee (const Value_ptr& ptr)  {insertee = ptr;}
      Instruction BuildInstruction () {
	std::vector<Value_ptr> vals ({aggregate,res,insertee});
	std::copy (consts.begin(),consts.end(),std::back_inserter (vals));
	return Instruction (InstructionCode::InsertValueFromConst,vals);
      }
      
    private:
      Value_ptr aggregate;
      Value_ptr insertee;
      Value_ptr res;
      std::vector<Value_ptr> consts;
    };

    template<> 
    struct Formatter<InstructionCode::InsertValue,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::InsertValue> h (inst);
	return os << h.getResult () << " = " << *h.getAggregate () << " [ " << *h.getOffset() << " ]:= " << *h.getInsertee ();
      } 
    };
    
    template<>
    class InstHelper<InstructionCode::Store,void> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getValue () const {return inst.getOp(0);}
      auto& getAddress () const {return inst.getOp(1);}
      
      private:
      const Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::Store,void> {
    public:
      void setValue (const Value_ptr& ptr) {value = ptr;}
      void setAddress (const Value_ptr& ptr) {address = ptr;}
      Instruction BuildInstruction () {
	assert(value);
	assert(address);
	return Instruction (InstructionCode::Store,{value,address});
      }
    private:
      Value_ptr value;
      Value_ptr address;
    };

    template<> 
    struct Formatter<InstructionCode::Store,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::PtrAdd> h (inst);
	return os << "*" <<*h.getAddress () << " = " << *h.getValue ();
      } 
    };
    
    template<>
    class InstHelper<InstructionCode::Load,void> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {
      }
      auto& getResult () const {return inst.getOp(0);}
      auto& getAddress () const {return inst.getOp(1);}
      
      private:
      const Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::Load,void> {
    public:
      void setRes (const Value_ptr& ptr) {res = ptr;}
      void setAddress (const Value_ptr& ptr) {address = ptr;}
      Instruction BuildInstruction () {
	assert(res);
	assert(address);
	return Instruction (InstructionCode::Load,{res,address});
      }
    private:
      Value_ptr res;
      Value_ptr address;
    };

    template<> 
    struct Formatter<InstructionCode::Load,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
	InstHelper<InstructionCode::Load> h (inst);
	return os  << *h.getResult () << " = *" << *h.getAddress ();
      } 
    };
    
    template<InstructionCode code>
    InstHelper<code> makeHelper (Instruction& inst) {
      assert(inst.getOpcode() == code);
      return InstHelper<code,void> (inst);
    }
    

    using instructionstream = std::vector<Instruction>;

    inline std::ostream& operator<< (std::ostream& os, const instructionstream& str) {
      os << "{";
      for (auto& i : str) {
	os << i << std::endl;
      }
      return os <<"}";
      
    }

    inline std::ostream& Instruction::output (std::ostream& os) const {
      switch (getOpcode () ) {
#define X(OP)								\
	case InstructionCode::OP:					\
	  return Formatter<InstructionCode::OP>::output (os,*this);	\
	  break;							
	  TACOPS
	  COMPARISONS
	CASTOPS
	  MEMORY
	  INTERNAL
	  POINTEROPS
	  AGGREGATEOPS
#undef X
	    }
      return os << "??";
    }
    
  }
}

#endif
