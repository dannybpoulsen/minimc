 /**
 * @file   instructions.hpp
 * @author Danny Bøgsted Poulsen <caramon@homemachine>
 * @date   Sun Apr 19 11:30:31 2020
 * 
 * @brief  
 * 
 * 
 */

#ifndef _INSTRUCTIONS__
#define _INSTRUCTIONS__
#include <cassert>
#include <ostream>
#include "model/variables.hpp"

namespace MiniMC {
  namespace Model {
#define TACOPS									\
    X(Add)										\
    X(Sub)										\
    X(Mul)										\
    X(UDiv)										\
    X(SDiv)										\
    X(Shl)										\
    X(LShr)										\
    X(AShr)										\
    X(And)										\
    X(Or)										\
    X(Xor)

#define UNARYOPS								\
    X(Not)										\
    
    
#define COMPARISONS								\
    X(ICMP_SGT)									\
    X(ICMP_UGT)									\
    X(ICMP_SGE)									\
    X(ICMP_UGE)									\
    X(ICMP_SLT)									\
    X(ICMP_ULT)									\
    X(ICMP_SLE)									\
    X(ICMP_ULE)									\
    X(ICMP_EQ)									\
    X(ICMP_NEQ)									\
    
#define POINTEROPS								\
    X(PtrAdd)									\
    X(PtrEq)									\
    
#define AGGREGATEOPS							\
    X(ExtractValue)								\
    X(InsertValue)								\
    X(InsertValueFromConst)						\
    
    
#define CASTOPS									\
    X(Trunc)									\
    X(ZExt)										\
    X(SExt)										\
    X(PtrToInt)									\
    X(IntToPtr)									\
    X(BitCast)									\
    X(BoolZExt)									\
    X(BoolSExt)									\
    X(IntToBool)								\
	

#define MEMORY									\
    X(Alloca)									\
    X(FindSpace)								\
    X(Malloc)									\
    X(Free)										\
    X(Store)									\
    X(Load)										\
	X(ExtendObj)								\
	
	
#define INTERNAL								\
    X(Skip)										\
    X(Call)										\
    X(Assign)									\
    X(Ret)										\
    X(RetVoid)									\
    X(NonDet)									\
    X(Assert)									\
    X(Assume)									\
    X(NegAssume)								\
    X(StackRestore)								\
    X(StackSave)								\
    X(MemCpy)									\
    X(Uniform)									\

#define PREDICATES								\
    X(PRED_ICMP_SGT)							\
    X(PRED_ICMP_UGT)							\
    X(PRED_ICMP_SGE)							\
    X(PRED_ICMP_UGE)							\
    X(PRED_ICMP_SLT)							\
    X(PRED_ICMP_ULT)							\
    X(PRED_ICMP_SLE)							\
    X(PRED_ICMP_ULE)							\
    X(PRED_ICMP_EQ)								\
    X(PRED_ICMP_NEQ)							\

#define OPERATIONS								\
    TACOPS										\
    UNARYOPS									\
    COMPARISONS									\
    CASTOPS										\
    MEMORY										\
    INTERNAL									\
    POINTEROPS									\
    AGGREGATEOPS								\
    PREDICATES									\
    
	
    enum class InstructionCode {
#define X(OP)									\
	  OP,
	  OPERATIONS
#undef X
    };

    inline std::ostream& operator<< (std::ostream& os, const InstructionCode& c) {
      switch (c) {
#define X(OP)									\
		case InstructionCode::OP:				\
		  return os << #OP;			
		OPERATIONS
#undef X
      default:
		return os << "Unknown";
	
      }	
    }

	
    template<InstructionCode i>
    struct InstructionData{
      static const bool isTAC = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 0;
      static const bool hasResVar = false;
    };

#define X(OP)										\
    template<>										\
    struct InstructionData<InstructionCode::OP>{	\
      static const bool isTAC = true;				\
      static const bool isMemory = false;			\
      static const bool isUnary =false;				\
      static const bool isComparison = false;		\
      static const bool isCast = false;				\
      static const bool isPointer = false;			\
      static const bool isPredicate = false;		\
	  static const bool isAggregate = false;		\
	  static const std::size_t operands = 2;		\
      static const bool hasResVar = true;			\
    };
    TACOPS
#undef X



#define X(OP)										\
    template<>										\
    struct InstructionData<InstructionCode::OP>{	\
	  static const bool isTAC = false;				\
	  static const bool isUnary = true;				\
	  static const bool isMemory = false;			\
	  static const bool isComparison = false;		\
	  static const bool isCast = false;				\
	  static const bool isPointer = false;			\
	  static const bool isAggregate = false;		\
	  static const bool isPredicate = false;		\
	  static const std::size_t operands = 1;		\
	  static const bool hasResVar = true;			\
    };
    UNARYOPS
#undef X


#define X(OP)										\
    template<>										\
    struct InstructionData<InstructionCode::OP>{	\
	  static const bool isTAC = false;				\
	  static const bool isUnary = false;			\
	  static const bool isMemory = false;			\
	  static const bool isComparison = false;		\
	  static const bool isCast = false;				\
	  static const bool isPointer = false;			\
	  static const bool isAggregate = false;		\
	  static const bool isPredicate = true;			\
	  static const std::size_t operands = 2;		\
	  static const bool hasResVar = false;			\
    };
    PREDICATES
#undef X

    
#define X(OP)										\
    template<>										\
    struct InstructionData<InstructionCode::OP>{	\
      static const bool isTAC = false;				\
      static const bool isUnary = false;			\
      static const bool isMemory = false;			\
      static const bool isComparison = true;		\
      static const bool isCast = false;				\
      static const bool isPointer = false;			\
      static const bool isAggregate = false;		\
      static const bool isPredicate = false;		\
      static const std::size_t operands = 2;		\
      static const bool hasResVar = true;			\
    };
    COMPARISONS
#undef X
	
#define X(OP)										\
    template<>										\
    struct InstructionData<InstructionCode::OP>{	\
      static const bool isTAC = false;				\
      static const bool isUnary = false;			\
      static const bool isComparison = false;		\
      static const bool isMemory = false;			\
      static const bool isCast = true;				\
      static const bool isPointer = false;			\
      static const bool isAggregate = false;		\
      static const bool isPredicate = false;		\
	  static const std::size_t operands = 1;		\
      static const bool hasResVar = true;			\
    };
    CASTOPS
#undef X

    template<>							
    struct InstructionData<InstructionCode::ExtractValue>{		
      static const bool isTAC = false;
      static const bool isUnary = false;		\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;				
      static const bool isPointer = false;			
      static const bool isAggregate = true;
      static const bool isPredicate = false;
      static const std::size_t operands = 3;			
      static const bool hasResVar = true;			
    };
    
    template<>							
    struct InstructionData<InstructionCode::InsertValue>{		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;				
      static const bool isPointer = false;			
      static const bool isAggregate = true;
      static const bool isPredicate = false;
      static const std::size_t operands = 3;			
      static const bool hasResVar = true;			
    };
    
    template<>							
    struct InstructionData<InstructionCode::InsertValueFromConst>{		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;				
      static const bool isPointer = false;			
      static const bool isAggregate = true;
      static const bool isPredicate = false;
      static const std::size_t operands = 0;			
      static const bool hasResVar = true;			
    };

    template<>							
    struct InstructionData<InstructionCode::PtrAdd>{		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;				
      static const bool isPointer = true;			
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 4;			
      static const bool hasResVar = true;			
    };

    template<>							
    struct InstructionData<InstructionCode::PtrEq>{		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = true;
      static const bool isMemory = false;			
      static const bool isCast = false;				
      static const bool isPointer = true;			
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 2;			
      static const bool hasResVar = true;			
    };
    
    template<>						
    struct InstructionData<InstructionCode::Alloca> {		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = true;			
    };

	template<>						
    struct InstructionData<InstructionCode::ExtendObj> {
	  static const bool isTAC = false;
	  static const bool isUnary =false;			\
	  static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 2;			
      static const bool hasResVar = true;			
    };
	
    template<>						
    struct InstructionData<InstructionCode::Malloc> {		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 2;			
      static const bool hasResVar = false;			
    };

    template<>						
    struct InstructionData<InstructionCode::Free> {		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = false;			
    };
    
    template<>						
    struct InstructionData<InstructionCode::FindSpace> {		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = true;			
    };
    
    template<>						
    struct InstructionData<InstructionCode::Skip> {		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 0;			
      static const bool hasResVar = false;			
    };
    
    template<>						
    struct InstructionData<InstructionCode::NonDet> {		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 0;			
      static const bool hasResVar = true;			
    };

    template<>						
    struct InstructionData<InstructionCode::Assert> {		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = false;			
    };

    template<>						
    struct InstructionData<InstructionCode::Assume> {		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = false;			
    };

    template<>						
    struct InstructionData<InstructionCode::NegAssume> {		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = false;			
    };
    
    template<>						
    struct InstructionData<InstructionCode::StackSave> {		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 0;			
      static const bool hasResVar = true;			
    };

    template<>						
    struct InstructionData<InstructionCode::StackRestore> {		
      static const bool isTAC = false;
      static const bool isUnary =false;			\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = false;			
    };

    template<>						
    struct InstructionData<InstructionCode::MemCpy> {		
      static const bool isTAC = false;
      static const bool isUnary =false;		\
      static const bool isComparison = false;
      static const bool isMemory = false;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 3;			
      static const bool hasResVar = false;			
    };
    
    template<>						
    struct InstructionData<InstructionCode::Load>{		
      static const bool isTAC = false;
      static const bool isUnary =false;		\
      static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = true;			
    };
    
    template<>						
    struct InstructionData<InstructionCode::Store>{		
      static const bool isTAC = false;
      static const bool isUnary =false;		\
      static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 2;			
      static const bool hasResVar = false;			
    };

    template<>						
    struct InstructionData<InstructionCode::Uniform>{		
      static const bool isTAC = false;
      static const bool isUnary =false;		\
      static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 2;			
      static const bool hasResVar = true;			
    };

    template<>						
    struct InstructionData<InstructionCode::Assign>{		
      static const bool isTAC = false;
      static const bool isUnary =false;		\
      static const bool isComparison = false;
      static const bool isMemory = true;			
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;			
      static const bool hasResVar = true;			
    };

    class Function;
    using Function_ptr = std::shared_ptr<Function>;

    /**
     *  \brief Container for instruction codes and their paramaters
     * 
     * The Instruction class holds its operands internally in  a vector. 
     * The operands are accesible through the getOp function. 
     * Since the order of the operands are important in the rest of MiniMC, you should never 
     * instantiate Instruction objects yourself but rather use the InstBuilder classes. Furthermore access to operand should be done using the the InstHelper structs.
     */											
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

      /**
       * Replace this instruction with contents of \p i.
       */
      void replace (const Instruction& i) {
		opcode = i.opcode;
		ops = i.ops;
      }

      /**
       * \returns InstructionCode of this Instruction
       */
      auto getOpcode () const {return opcode;}

      /**
       * \returns std::vector<Value_ptr>& with all operands op this Instruction. 
       */
      auto& getOps () const {return ops;}

      auto& getOp (std::size_t i) const {return ops.at(i);}

      auto getNbOps () const {return ops.size();}

      auto begin () const {return ops.begin();}
      auto end () const {return ops.end();}
	  
	  
      /** 
       * Write a textual representation to \p os
       *
       * @param os The stream to output the representation to
       *
       * @return Reference to \p os, to allowing chaining outputs.
       */
      std::ostream& output (std::ostream& os) const;
    
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
      auto& getLeftOp () const {return inst.getOp(1);}
      auto& getRightOp () const {return inst.getOp(2);}
      
    private:
      const Instruction& inst;
    };

    template<InstructionCode i>
    class InstBuilder<i,typename std::enable_if<InstructionData<i>::isTAC>::type> {
    public:
      auto& setRes (const Value_ptr& ptr) {res = ptr; return *this;}
      auto& setLeft (const Value_ptr& ptr) {left = ptr; return *this;}
      auto& setRight (const Value_ptr& ptr) {right = ptr;return *this;}
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
    class InstHelper<i,typename std::enable_if<InstructionData<i>::isPredicate>::type> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getLeftOp () const {return inst.getOp(0);}
      auto& getRightOp () const {return inst.getOp(1);}
      
    private:
      const Instruction& inst;
    };

    template<InstructionCode i>
    class InstBuilder<i,typename std::enable_if<InstructionData<i>::isPredicate>::type> {
    public:
      auto& setLeft (const Value_ptr& ptr) {left = ptr; return *this;}
      auto& setRight (const Value_ptr& ptr) {right = ptr;return *this;}
      Instruction BuildInstruction () {
		assert(left);
		assert(right);
		return Instruction (i,{left,right});
      }
    private:
      Value_ptr left;
      Value_ptr right;
    };

    template<InstructionCode i> 
    struct Formatter<i,typename std::enable_if<InstructionData<i>::isPredicate>::type> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
		InstHelper<i> h (inst);
		return os <<  i << " " << *h.getLeftOp () << " " << *h.getRightOp ();
      } 
    };

    template<InstructionCode i>
    class InstHelper<i,typename std::enable_if<InstructionData<i>::isUnary>::type> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
      auto& getOp () const {return inst.getOp(1);}
      
    private:
      const Instruction& inst;
    };

    template<InstructionCode i>
    class InstBuilder<i,typename std::enable_if<InstructionData<i>::isUnary>::type> {
    public:
      auto& setRes (const Value_ptr& ptr) {res = ptr; return *this;}
      auto& setOP (const Value_ptr& ptr) {left = ptr; return *this;}
      Instruction BuildInstruction () {
		assert(res);
		assert(left);
		return Instruction (i,{res,left});
      }
    private:
      Value_ptr res;
      Value_ptr left;
    };

    template<InstructionCode i> 
    struct Formatter<i,typename std::enable_if<InstructionData<i>::isUnary>::type> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
		InstHelper<i> h (inst);
		return os << *h.getResult () << " = " << i << " " << *h.getOp ();
      } 
    };
    
    template<InstructionCode i>
    class InstBuilder<i,typename std::enable_if<InstructionData<i>::isComparison>::type> {
    public:
      auto& setRes (const Value_ptr& ptr) {res = ptr; return *this;}
      auto& setLeft (const Value_ptr& ptr) {left = ptr; return *this;}
      auto& setRight (const Value_ptr& ptr) {right = ptr;return *this;}
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
      auto& getLeftOp () const {return inst.getOp(1);}
      auto& getRightOp () const {return inst.getOp(2);}
      
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
      auto& setRes (const Value_ptr& ptr) {res = ptr; return *this;}
      auto& setCastee (const Value_ptr& ptr) {castee = ptr; return *this;}
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
      auto& setRes (const Value_ptr& ptr) {res = ptr; return *this;}
      auto& setSize (const Value_ptr& ptr) {size = ptr; return *this;}
      
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
    class InstHelper<InstructionCode::ExtendObj,void> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
      auto& getPointer () const {return inst.getOp(1);}
      auto& getSize () const {return inst.getOp(2);}
      
    private:
      const Instruction& inst;
    };


    template<>
    class InstBuilder<InstructionCode::ExtendObj> {
    public:
      auto& setRes (const Value_ptr& ptr) {res = ptr; return *this;}
      auto& setPointer (const Value_ptr& ptr) {pointer = ptr; return *this;}
      auto& setSize (const Value_ptr& ptr) {size = ptr; return *this;}
      
      Instruction BuildInstruction () {
		assert(res);
		assert(size);
		assert(pointer);
		return Instruction (InstructionCode::ExtendObj,{res,pointer,size});
	  }
    private:
      Value_ptr res;
      Value_ptr pointer;
	  Value_ptr size;
	};
	
	
    template<>
    class InstHelper<InstructionCode::Malloc,void> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getPointer () const {return inst.getOp(0);}
      auto& getSize () const {return inst.getOp(1);}
      
    private:
      const Instruction& inst;
    };


    template<>
    class InstBuilder<InstructionCode::Malloc,void> {
    public:
      auto& setPointer (const Value_ptr& ptr) {pointer = ptr; return *this;}
      auto& setSize (const Value_ptr& ptr) {size = ptr; return *this;}
      
      Instruction BuildInstruction () {
		assert(pointer);
		assert(size);
		return Instruction (InstructionCode::Malloc,{pointer,size});
      }
    private:
      Value_ptr pointer;
      Value_ptr size;
    };


    template<>
    class InstHelper<InstructionCode::Free,void> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getPointer () const {return inst.getOp(0);}	  
    private:
      const Instruction& inst;
    };


    template<>
    class InstBuilder<InstructionCode::Free,void> {
    public:
      auto& setPointer (const Value_ptr& ptr) {pointer = ptr; return *this;}
      
      Instruction BuildInstruction () {
		assert(pointer);
		return Instruction (InstructionCode::Free,{pointer});
      }
    private:
      Value_ptr pointer;
    };
    
    template<>
    class InstHelper<InstructionCode::FindSpace,void> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
      auto& getSize () const {return inst.getOp(1);}
      
    private:
      const Instruction& inst;
    };


    template<>
    class InstBuilder<InstructionCode::FindSpace,void> {
    public:
      auto& setResult (const Value_ptr& ptr) {res = ptr; return *this;}
      auto& setSize (const Value_ptr& ptr) {size = ptr; return *this;}
      Instruction BuildInstruction () {
		assert(res);
		assert(size);
		return Instruction (InstructionCode::FindSpace,{res,size});
      }
    private:
      Value_ptr res;
      Value_ptr size;
    };
    
    template<> 
    struct Formatter<InstructionCode::Alloca,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
		InstHelper<InstructionCode::Alloca> h (inst);
		return os << InstructionCode::Alloca << " (" << *h.getResult () << ", "  << *h.getSize () <<" )";
      } 
    };

  template<> 
    struct Formatter<InstructionCode::ExtendObj,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
		InstHelper<InstructionCode::ExtendObj> h (inst);
		return os << InstructionCode::ExtendObj << " (" << *h.getResult () << ", " << *h.getPointer() << " by "  << *h.getSize () <<" )";
      } 
    };
  
    template<> 
    struct Formatter<InstructionCode::Malloc,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
		InstHelper<InstructionCode::Malloc> h (inst);
		return os << InstructionCode::Malloc << " (" << *h.getPointer () << ", "  << *h.getSize () <<" )";
      } 
    };
    
    template<> 
    struct Formatter<InstructionCode::Free,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
		InstHelper<InstructionCode::Free> h (inst);
		return os << InstructionCode::Free << " (" << *h.getPointer () <<" )";
      } 
    };
    
    template<> 
    struct Formatter<InstructionCode::FindSpace,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
		InstHelper<InstructionCode::Alloca> h (inst);
		return os << *h.getResult () << " = " << InstructionCode::FindSpace << *h.getSize ();
      } 
    };
    
    template<>
    class InstHelper<InstructionCode::Skip,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
    private:
      const Instruction& inst;
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
     
      auto& setResult (const Value_ptr& p) {res = p; return *this;}
      
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
     
      auto& setResult (const Value_ptr& p) {res = p; return *this;}
      
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
      
      auto& setValue (const Value_ptr& p) {res = p; return *this;}
      
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
    class InstHelper<InstructionCode::MemCpy,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getSource () const {return inst.getOp(0);}
      auto& getTarget () const {return inst.getOp(1);}
      auto& getSize () const {return inst.getOp(2);}
    private:
      const Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::MemCpy,void> {
    public:
    
      auto& setSource (const Value_ptr& p) {src = p; return *this;}
      auto& setTarget (const Value_ptr& p) {target = p; return *this;}
      auto& setSize (const Value_ptr& p) {size = p; return *this;}
	  
	  
      Instruction BuildInstruction () {
		return Instruction (InstructionCode::MemCpy,{src,target,size});
      }
    private:
      Value_ptr src = nullptr;
      Value_ptr target = nullptr;
      Value_ptr size = nullptr;
	  
    };

    template<> 
    struct Formatter<InstructionCode::MemCpy,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
		InstHelper<InstructionCode::MemCpy> h (inst);
		return os <<  InstructionCode::MemCpy;;
      } 
    };
	
	
    //
    template<InstructionCode i>
    class InstHelper<i,std::enable_if_t<i == InstructionCode::Assert ||
										i == InstructionCode::Assume ||
										i == InstructionCode::NegAssume
										>> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getAssert () const {return inst.getOp(0);}
    private:
      const Instruction& inst;
    };

    template<InstructionCode i>
    class InstBuilder<i,std::enable_if_t<i == InstructionCode::Assert ||
										 i == InstructionCode::Assume ||
										 i == InstructionCode::NegAssume
										 >> {
    public:
      Instruction BuildInstruction () {
		return Instruction (i,{ass});
      }
     
      auto& setAssert (const Value_ptr& p) {ass = p; return *this;}
      
    private:
      Value_ptr ass = nullptr;
    
    };

    template<InstructionCode i> 
    struct Formatter<i,std::enable_if_t<i == InstructionCode::Assert ||
										i == InstructionCode::Assume ||
										i == InstructionCode::NegAssume
										>> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
		InstHelper<InstructionCode::Assert> h (inst);
		return os << i << "(" << *h.getAssert() << ")"  ;
      } 
    };
    //
    
    template<>
    class InstHelper<InstructionCode::Call,void> {
    public:
      
      InstHelper (const Instruction& inst) : inst(inst) {
		nbparams = std::static_pointer_cast<MiniMC::Model::IntegerConstant<MiniMC::uint64_t>> (inst.getOp (1))->getValue();
	
      }
      
      auto nbParams ()  const {return nbparams;}
      auto getFunctionPtr () {return inst.getOp (0);}
      auto getParam (std::size_t p) {return inst.getOp (3+p);}
      auto getResult () {
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
	  
      auto& setFunctionPtr (const Value_ptr& func) {
		this->func = func;
		return *this;
      }
	  
      auto& setRes (const Value_ptr& res) {
		this->res = res;
		return *this;
      }
	  
      auto& setNbParamters (const Value_ptr& p) {
		assert(p->isConstant ());
		this->nbParameters = p;
		return *this;
      }

      auto& addParam (const Value_ptr& p) {
		params.push_back (p);
		return *this;
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
      auto& setSkipSize (const Value_ptr& ptr) {skipSize = ptr; return *this;}
      auto& setValue (const Value_ptr& ptr) {value = ptr; return *this;}
      auto& setAddress (const Value_ptr& ptr) {address = ptr; return *this;}
      auto& setResult (const Value_ptr& ptr) {result = ptr; return *this;}
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
      auto& setAggregate (const Value_ptr& aggr) {aggregate = aggr; return *this;}
      auto& setOffset (const Value_ptr& ptr) {offset = ptr; return *this;}
      auto& setResult (const Value_ptr& ptr) {res = ptr; return *this;}
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
      auto& setValue (const Value_ptr& aggr) {value = aggr; return *this;}
      auto& setResult (const Value_ptr& ptr) {res = ptr; return *this;}
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
      auto& setAggregate (const Value_ptr& aggr) {aggregate = aggr; return *this;}
      auto& setOffset (const Value_ptr& ptr) {offset = ptr;return *this;}
      auto& setResult (const Value_ptr& ptr) {res = res; return *this;}
      auto& setInsertee (const Value_ptr& ptr)  {insertee = ptr; return *this;}
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


    //
    template<>
    class InstHelper<InstructionCode::Uniform,void> {
    public:
      InstHelper (const Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
      auto& getMin () const {return inst.getOp(1);}
      auto& getMax () const {return inst.getOp(2);}
      
    private:
      const Instruction& inst;
    };

    template<>
    class InstBuilder<InstructionCode::Uniform,void> {
    public:
      auto& setResult (const Value_ptr& ptr) {res = ptr; return *this;}
      auto& setMin (const Value_ptr& ptr) {min = ptr; return *this;}
      auto& setMax (const Value_ptr& ptr) {max = ptr; return *this;}
      Instruction BuildInstruction () {
		assert(min);
		assert(max);
		assert(res);
		return Instruction (InstructionCode::Uniform,{res,min,max});
      }
    private:
      Value_ptr res;
      Value_ptr min;
      Value_ptr max;
    };

    template<>
    struct Formatter<InstructionCode::Uniform,void> {
      static std::ostream& output (std::ostream& os, const Instruction& inst) {
		InstHelper<InstructionCode::Uniform> h (inst);
		return os  << *h.getResult () << " = " << InstructionCode::Uniform << " ( " << *h.getMin() << ", " << *h.getMax() << ")"; 
	
      } 
    };

    //
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
      auto& setValue (const Value_ptr& ptr) {value = ptr; return *this;}
      auto& setAddress (const Value_ptr& ptr) {address = ptr; return *this;}
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
      auto& setRes (const Value_ptr& ptr) {res = ptr; return *this;}
      auto& setAddress (const Value_ptr& ptr) {address = ptr; return *this;}
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
      for (auto& i : str) {
		os << i << std::endl;
      }
	  return os;
    }

    inline std::ostream& Instruction::output (std::ostream& os) const {
      switch (getOpcode () ) {
#define X(OP)														\
		case InstructionCode::OP:									\
		  return Formatter<InstructionCode::OP>::output (os,*this);	\
		  break;							
		TACOPS
		  COMPARISONS
		  CASTOPS
		  MEMORY
		  INTERNAL
		  POINTEROPS
		  AGGREGATEOPS
		  UNARYOPS
		  PREDICATES
#undef X
		  }
      return os << "??";
    }
	
	
    template<InstructionCode op, InstructionCode... tail>
    inline static bool isOneOf (MiniMC::Model::Instruction& instr) {
      if constexpr (sizeof...(tail) == 0) 
					 return instr.getOpcode() ==op;
      else {
		if (instr.getOpcode () == op)
		  return true;
		else return isOneOf<tail...> (instr);
      }
    }
   		   
	
  }
}

#endif
