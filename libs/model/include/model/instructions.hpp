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
    X(Xor)								\
    X(ICMP_SGT)								\
    X(ICMP_UGT)								\
    X(ICMP_SGE)								\
    X(ICMP_UGE)								\
    X(ICMP_SLT)								\
    X(ICMP_ULT)								\
    X(ICMP_SLE)								\
    X(ICMP_ULE)								\
    X(PtrAdd)								\
    
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
    
    enum class InstructionCode {
#define X(OP)					\
				OP,
				TACOPS
				CASTOPS
				MEMORY
#undef X
    };

    inline std::ostream& operator<< (std::ostream& os, const InstructionCode& c) {
      switch (c) {
#define X(OP)					\
	case InstructionCode::OP:		\
	  return os << #OP;			
	TACOPS
	  MEMORY
	  CASTOPS
#undef X
      default:
	return os << "Unknown";
	
      }	
    }
    
    template<InstructionCode>
    struct InstructionData{
      static const bool isTAC = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const std::size_t operands = 0;
      static const bool hasResVar = false;
    };

#define X(OP)					\
    template<>						\
    struct InstructionData<InstructionCode::OP>{	\
      static const bool isTAC = true;			\
      static const bool isMemory = false;			\
      static const bool isCast = false;		\
      static const std::size_t operands = 2;			\
      static const bool hasResVar = true;		\
    };
    TACOPS
#undef X

#define X(OP)						\
    template<>						\
    struct InstructionData<InstructionCode::OP>{	\
      static const bool isTAC = false;			\
      static const bool isMemory = false;			\
      static const bool isCast = true;			\
      static const std::size_t operands = 1;			\
      static const bool hasResVar = true;		\
    };
    CASTOPS
#undef X
    
    
    template<>						
    struct InstructionData<InstructionCode::Alloca> {		
      static const bool isTAC = false;			
      static const bool isMemory = true;			
      static const bool isCast = false;			
      static const std::size_t operands = 1;			
      static const bool hasResVar = true;			
    };
    
    
    template<>						
    struct InstructionData<InstructionCode::Load>{		
      static const bool isTAC = false;			
      static const bool isMemory = true;			
      static const bool isCast = false;			
      static const std::size_t operands = 1;			
      static const bool hasResVar = true;			
    };
    
    template<>						
    struct InstructionData<InstructionCode::Store>{		
      static const bool isTAC = false;			
      static const bool isMemory = true;			
      static const bool isCast = false;			
      static const std::size_t operands = 2;			
      static const bool hasResVar = false;			
    };
    
    
    struct Instruction {
    public:
      Instruction (InstructionCode code, std::initializer_list<Value_ptr> ops) : opcode(code),
										    ops(ops) {}
      auto getOpcode () const {return opcode;}
      auto& getOps () const {return ops;}
      auto& getOp (std::size_t i) const {return ops.at(i);}
      auto getNbOps () const {return ops.size();}
      std::ostream& output (std::ostream& os) const {
	os << "(" << opcode;
	for (auto& op : ops) {
	  os << " " << *op; 
	}
	return os << ")";
      }
    private:
      InstructionCode opcode;
      std::vector<Value_ptr> ops;
    };

    inline std::ostream& operator<< (std::ostream& os, const Instruction& inst) {
      return inst.output(os);
    }
    
    template<InstructionCode i,class T = void>
    class InstHelper;

    template<InstructionCode i,class T = void>
    class InstBuilder;
    
    template<InstructionCode i>
    class InstHelper<i,typename std::enable_if<InstructionData<i>::isTAC>::type> {
    public:
      InstHelper (Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
      auto getLeftOp () const {return inst.getOp(1);}
      auto getRightOp () const {return inst.getOp(2);}
      
      private:
      Instruction& inst;
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
    class InstHelper<i,typename std::enable_if<InstructionData<i>::isCast>::type> {
    public:
      InstHelper (Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
      auto& getCastee () const {return inst.getOp(1);}
      
      private:
      Instruction& inst;
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

      

    template<>
    class InstHelper<InstructionCode::Alloca,void> {
    public:
      InstHelper (Instruction& inst) : inst(inst) {}
      auto& getResult () const {return inst.getOp(0);}
      auto& getSize () const {return inst.getOp(1);}
      
      private:
      Instruction& inst;
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
    class InstHelper<InstructionCode::Store,void> {
    public:
      InstHelper (Instruction& inst) : inst(inst) {}
      auto& getValue () const {return inst.getOp(0);}
      auto& getAddress () const {return inst.getOp(1);}
      
      private:
      Instruction& inst;
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
    class InstHelper<InstructionCode::Load,void> {
    public:
      InstHelper (Instruction& inst) : inst(inst) {
      }
      auto& getResult () const {return inst.getOp(0);}
      auto& getAddress () const {return inst.getOp(1);}
      
      private:
      Instruction& inst;
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
    
    template<InstructionCode code>
    InstHelper<code> makeHelper (Instruction& inst) {
      assert(inst.getOpcode() == code);
      return InstHelper<code,void> (inst);
    }
    
    
  }
}

#endif
