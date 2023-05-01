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
#include "model/variables.hpp"
#include <cassert>
#include <ostream>
#include <variant>

namespace MiniMC {
  namespace Model {
#define TACOPS \
  X(Add)       \
  X(Sub)       \
  X(Mul)       \
  X(UDiv)      \
  X(SDiv)      \
  X(Shl)       \
  X(LShr)      \
  X(AShr)      \
  X(And)       \
  X(Or)        \
  X(Xor)

#define UNARYOPS \
  X(Not)

#define COMPARISONS \
  X(ICMP_SGT)       \
  X(ICMP_UGT)       \
  X(ICMP_SGE)       \
  X(ICMP_UGE)       \
  X(ICMP_SLT)       \
  X(ICMP_ULT)       \
  X(ICMP_SLE)       \
  X(ICMP_ULE)       \
  X(ICMP_EQ)        \
  X(ICMP_NEQ)

#define POINTEROPS \
  X(PtrAdd)        \
  X(PtrEq)	   
    
#define AGGREGATEOPS \
  X(ExtractValue)    \
  X(InsertValue)     \
 
#define CASTOPS \
  X(Trunc)	\
  X(ZExt)       \
  X(SExt)       \
  X(PtrToInt)   \
  X(IntToPtr)   \
  X(BitCast)    \
  X(BoolZExt)   \
  X(BoolSExt)   \
  X(IntToBool)

#define MEMORY \
  X(Store)     \
  X(Load)      \
  

    
#define INTERNAL  \
  X(Skip)         \
  X(Call)         \
  X(Assign)       \
  X(Ret)          \
  X(RetVoid)      \
  X(NonDet)       \
  X(Uniform)

#define ASSUMEASSERTS				\
    X(Assert)					\
    X(Assume)					\
    X(NegAssume)				\
    
    
#define PREDICATES \
  X(PRED_ICMP_SGT) \
  X(PRED_ICMP_UGT) \
  X(PRED_ICMP_SGE) \
  X(PRED_ICMP_UGE) \
  X(PRED_ICMP_SLT) \
  X(PRED_ICMP_ULT) \
  X(PRED_ICMP_SLE) \
  X(PRED_ICMP_ULE) \
  X(PRED_ICMP_EQ)  \
  X(PRED_ICMP_NEQ)

#define OPERATIONS \
  TACOPS           \
  UNARYOPS         \
  COMPARISONS      \
  CASTOPS          \
  MEMORY           \
  INTERNAL         \
  POINTEROPS       \
  AGGREGATEOPS     \
  ASSUMEASSERTS		   \
    PREDICATES

    enum class InstructionCode {
#define X(OP) \
  OP,
      OPERATIONS
#undef X
    };
    
    inline std::ostream& operator<<(std::ostream& os, const InstructionCode& c) {
      switch (c) {
#define X(OP)               \
  case InstructionCode::OP: \
    return os << #OP;
        OPERATIONS
#undef X
        default:
          return os << "Unknown";
      }
    }

    
    
    template <InstructionCode i>
    struct InstructionData {
      static const bool isTAC = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const bool isAssumeAssert = false;
      static const std::size_t operands = 0;
      static const bool hasResVar = false;
    };

    struct TACContent {
      TACContent (Value_ptr res, Value_ptr op1, Value_ptr op2) : res(std::move(res)),
								       op1(std::move(op1)),
								       op2(std::move(op2)) {}
      Value_ptr res;
      Value_ptr op1;
      Value_ptr op2;
    };
    
#define X(OP)                                   \
  template <>                                   \
  struct InstructionData<InstructionCode::OP> { \
    static const bool isTAC = true;             \
    static const bool isMemory = false;         \
    static const bool isUnary = false;          \
    static const bool isComparison = false;     \
    static const bool isCast = false;           \
    static const bool isPointer = false;        \
    static const bool isPredicate = false;      \
    static const bool isAggregate = false;      \
    static const std::size_t operands = 2;      \
    static const bool hasResVar = true;         \
    using Content = TACContent;			\
  };
    TACOPS
#undef X

    struct UnaryContent {
      UnaryContent (Value_ptr res, Value_ptr op1) : res(std::move(res)), op1(std::move(op1)) {}
      Value_ptr res;
      Value_ptr op1;
    };
    
#define X(OP)                                   \
  template <>                                   \
  struct InstructionData<InstructionCode::OP> { \
    static const bool isTAC = false;            \
    static const bool isUnary = true;           \
    static const bool isMemory = false;         \
    static const bool isComparison = false;     \
    static const bool isCast = false;           \
    static const bool isPointer = false;        \
    static const bool isAggregate = false;      \
    static const bool isPredicate = false;      \
    static const std::size_t operands = 1;      \
    static const bool hasResVar = true;         \
    using Content = UnaryContent;		\
  };
    UNARYOPS
#undef X

    struct BinaryContent {
      BinaryContent (Value_ptr op1, Value_ptr op2) : op1(std::move(op1)),op2(std::move(op2)) {}
      Value_ptr op1;
      Value_ptr op2;
    };
    
#define X(OP)                                   \
  template <>                                   \
  struct InstructionData<InstructionCode::OP> { \
    static const bool isTAC = false;            \
    static const bool isUnary = false;          \
    static const bool isMemory = false;         \
    static const bool isComparison = false;     \
    static const bool isCast = false;           \
    static const bool isPointer = false;        \
    static const bool isAggregate = false;      \
    static const bool isPredicate = true;       \
    static const std::size_t operands = 2;      \
    static const bool hasResVar = false;        \
    using Content = BinaryContent;		\
  };
    PREDICATES
#undef X

#define X(OP)                                   \
  template <>                                   \
  struct InstructionData<InstructionCode::OP> { \
    static const bool isTAC = false;            \
    static const bool isUnary = false;          \
    static const bool isMemory = false;         \
    static const bool isComparison = true;      \
    static const bool isCast = false;           \
    static const bool isPointer = false;        \
    static const bool isAggregate = false;      \
    static const bool isPredicate = false;      \
    static const std::size_t operands = 2;      \
    static const bool hasResVar = true;         \
    using Content = TACContent;			\
  };
    COMPARISONS
#undef X

#define X(OP)                                   \
  template <>                                   \
  struct InstructionData<InstructionCode::OP> { \
    static const bool isTAC = false;            \
    static const bool isUnary = false;          \
    static const bool isComparison = false;     \
    static const bool isMemory = false;         \
    static const bool isCast = true;            \
    static const bool isPointer = false;        \
    static const bool isAggregate = false;      \
    static const bool isPredicate = false;      \
    static const std::size_t operands = 1;      \
    static const bool hasResVar = true;         \
    using Content = UnaryContent;			\
  };
    CASTOPS
#undef X

    struct ExtractContent {
      ExtractContent (Value_ptr res, Value_ptr aggregate, Value_ptr offset) : res(std::move(res)),
									      aggregate(std::move(aggregate)),
									      offset(std::move(offset)) {}
      Value_ptr res;
      Value_ptr aggregate;
      Value_ptr offset;
    };
    
    template <>
    struct InstructionData<InstructionCode::ExtractValue> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = true;
      static const bool isPredicate = false;
      static const std::size_t operands = 3;
      static const bool hasResVar = true;
      using Content = ExtractContent;
    };

    struct InsertContent {
      InsertContent (Value_ptr res,
		     Value_ptr aggregate,
		     Value_ptr offset,
		     Value_ptr insertee) : res(std::move(res)),
					   aggregate(std::move(aggregate)),
					   offset(std::move(offset)),
					   insertee(std::move(insertee)) {}
      Value_ptr res;
      Value_ptr aggregate;
      Value_ptr offset;
      Value_ptr insertee;
    };
    
    template <>
    struct InstructionData<InstructionCode::InsertValue> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = true;
      static const bool isPredicate = false;
      static const std::size_t operands = 4;
      static const bool hasResVar = true;
      using Content = InsertContent;
    };

    struct PtrAddContent{
      PtrAddContent (Value_ptr res,
		     Value_ptr ptr,
		     Value_ptr skip,
		     Value_ptr nbSkips) : res(std::move(res)),
					  ptr(std::move(ptr)),
					  skipsize(std::move(skip)),
					  nbSkips(std::move(nbSkips)) {}
      Value_ptr res;
      Value_ptr ptr;
      Value_ptr skipsize;
      Value_ptr nbSkips;
    };
    
    template <>
    struct InstructionData<InstructionCode::PtrAdd> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = true;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 4;
      static const bool hasResVar = true;
      using Content = PtrAddContent;
    };

    template <>
    struct InstructionData<InstructionCode::PtrEq> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = true;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 2;
      static const bool hasResVar = true;
      using Content = TACContent;
    };

    
    
    template <>
    struct InstructionData<InstructionCode::Skip> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const bool isInternal = true;
      static const std::size_t operands = 0;
      static const bool hasResVar = false;
      using Content = int;
    };

    struct NonDetContent {
      Value_ptr res;
      Value_ptr min;
      Value_ptr max;
    };
    
    template <>
    struct InstructionData<InstructionCode::NonDet> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const bool isInternal = true;
      static const std::size_t operands = 2;
      static const bool hasResVar = true;
      using Content = NonDetContent;
    };

    struct AssertAssumeContent {
      AssertAssumeContent (Value_ptr res) : expr(std::move(res)) {}
      Value_ptr expr;
    };

#define X(OP)					\
    template <>						\
    struct InstructionData<InstructionCode::OP> {	\
    static const bool isTAC = false;			\
    static const bool isUnary = false;			\
    static const bool isComparison = false;		\
    static const bool isMemory = false;			\
    static const bool isCast = false;			\
    static const bool isPointer = false;		\
    static const bool isAggregate = false;		\
    static const bool isPredicate = false;		\
    static const bool isAssertAssume = true;		\
    static const std::size_t operands = 1;		\
    static const bool hasResVar = false;		\
    using Content = AssertAssumeContent;		\
  };							\

ASSUMEASSERTS
    
#undef X

    struct CallContent {
  CallContent (Value_ptr res, Value_ptr function, std::vector<Value_ptr> params) :
    res(std::move(res)),function(std::move(function)),params(std::move(params)) {}
      Value_ptr res;
      Value_ptr function;
      std::vector<Value_ptr> params;
    };
    
    template <>
    struct InstructionData<InstructionCode::Call> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const bool isInternal = true;
      
      static const std::size_t operands = 1;
      static const bool hasResVar = false;
      using Content = CallContent;
    };

    struct StackSaveContent {
      Value_ptr res;
    };
    
    
    
    
    struct LoadContent {
      LoadContent (Value_ptr res, Value_ptr add) : res(std::move(res)),addr(std::move(add)) {}
      Value_ptr res;
      Value_ptr addr;
    };
    
    template <>
    struct InstructionData<InstructionCode::Load> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = true;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 2;
      static const bool hasResVar = true;
      using Content = LoadContent;
    };
    
    struct StoreContent {
      StoreContent (Value_ptr addr,Value_ptr storee) : addr(std::move(addr)),storee(std::move(storee)) {}
      Value_ptr addr;
      Value_ptr storee;
    };
    
    template <>
    struct InstructionData<InstructionCode::Store> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = true;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 2;
      static const bool hasResVar = false;
      using Content = StoreContent;
    };

        
    template <>
    struct InstructionData<InstructionCode::Uniform> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = true;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 2;
      static const bool hasResVar = true;
      using Content = NonDetContent;
    };

    template <>
    struct InstructionData<InstructionCode::Assign> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const bool isInternal = true;
      
      static const std::size_t operands = 1;
      static const bool hasResVar = true;
      using Content = UnaryContent;
    };

    struct RetContent {
      RetContent (Value_ptr v) : value(std::move(v)) {}
      Value_ptr value;
    };
    
    template <>
    struct InstructionData<InstructionCode::Ret> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const bool isInternal = true;
      
      static const std::size_t operands = 1;
      static const bool hasResVar = false;
      using Content = RetContent;
    };

    template <>
    struct InstructionData<InstructionCode::RetVoid> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const bool isInternal = true;
      
      static const std::size_t operands = 1;
      static const bool hasResVar = false;
      using Content = int;
    };
    
    using Instruction_content = std::variant<TACContent,
					     UnaryContent,
					     BinaryContent,
					     ExtractContent,
					     InsertContent,
					     PtrAddContent,
					     NonDetContent,
					     AssertAssumeContent,
					     LoadContent,
					     StoreContent,
					     RetContent,
					     CallContent,
					     int
					     >;

    using ReplaceFunction = std::function<Value_ptr(const Value_ptr&)>;
    
    struct Instruction {
    public:
      Instruction (InstructionCode code, Instruction_content content) : opcode(code),
								       content(content) {}

      //Copy Consructor with Replacement :-)
      Instruction (const Instruction&, ReplaceFunction);
      
      template<InstructionCode op,class... Args>
      static Instruction make (Args... args) {
	return Instruction(op,typename InstructionData<op>::Content (std::forward<Args> (args)...));
      }
      
      template<InstructionCode op> 
      static Instruction make (InstructionData<op>::Content d) {
	return Instruction(op,d);
      }
      
      
      
      /**
       * Replace this instruction with contents of \p i.
       */
      void replace(const Instruction& i) {
        opcode = i.opcode;
        content = i.content;
      }

      /**
       * \returns InstructionCode of this Instruction
       */
      auto getOpcode() const { return opcode; }

      template<InstructionCode c>
      auto& getOps () const {
	return std::get<typename InstructionData<c>::Content> (content);
      }

      
      /** 
       * Write a textual representation to \p os
       *
       * @param os The stream to output the representation to
       *
       * @return Reference to \p os, to allowing chaining outputs.
       */
      std::ostream& output(std::ostream& os) const;

    private:
      InstructionCode opcode;
      Instruction_content content;
    };
    

    inline std::ostream& operator<<(std::ostream& os, const Instruction& inst) {
      return inst.output(os);
    }


    template <InstructionCode op, InstructionCode... tail>
    inline static bool isOneOf(const MiniMC::Model::Instruction& instr) {
      if constexpr (sizeof...(tail) == 0)
        return instr.getOpcode() == op;
      else {
        if (instr.getOpcode() == op)
          return true;
        else
          return isOneOf<tail...>(instr);
      }
    }


    
    std::ostream& operator<<(std::ostream& os, const std::vector<Instruction>& str);
    
    
    /**
	 * Structure representing InstructionStream on  edges.
	 * An InstructionStream is a sequence of instruction to be performed uninterrupted.
	 * If the isPhi is true, then the InstructionStream must be performed without the instructions affecting eachother (this is to 
	 * to reflect that some operations are atomic even within a single process - such as phi-nodes in SSA form).
	 */
    struct InstructionStream {
      using iterator = std::vector<Instruction>::iterator;
      InstructionStream()  {}
      
      InstructionStream(const std::vector<Instruction>& i) : instr(i) {}
      InstructionStream(const InstructionStream& str) = default;
      InstructionStream(InstructionStream&& str) = default;
      
      InstructionStream(const std::initializer_list<Instruction> i) : instr(i) {}
      
      
      InstructionStream& operator= (const InstructionStream&) = default;
      
      auto begin() const { return instr.begin(); }
      auto end() const { return instr.end(); }
      auto begin() { return instr.begin(); }
      auto end() { return instr.end(); }

      auto rbegin() const { return instr.rbegin(); }
      auto rend() const { return instr.rend(); }
      auto rbegin() { return instr.rbegin(); }
      auto rend() { return instr.rend(); }

      operator bool () const {return instr.size ();}

      template<InstructionCode c,class... Args>
      InstructionStream& add (Args... args) {
	instr.emplace_back (Instruction::make<c> (std::forward<Args> (args)...));
	return *this;
      }

      InstructionStream& add (const Instruction& i, ReplaceFunction r) {
	instr.emplace_back (Instruction(i,r));
	return *this;
      }
      
      InstructionStream& add (const Instruction& i) {
	instr.emplace_back (i);
	return *this;
      }

      void clear () {
	instr.clear ();
      }

      bool empty () {
	return instr.empty ();
      }
      
      auto& last() const {
        assert(instr.size());
        return instr.back();
      }

            
      auto& last()  {
        assert(instr.size());
        return instr.back();
      }
      
      template <class Iterator>
      auto erase(Iterator iter) {
        auto res = instr.erase(iter);
        return res;
      }

      template <class Iterator>
      auto replaceInstructionBySeq(iterator repl, Iterator beg, Iterator end) {
        return instr.insert(erase(repl), beg, end);
      }

      std::ostream& output (std::ostream& os) const  {
	return os << instr;
      }
      
      
	
      
    private:
      std::vector<Instruction> instr;
    };
    
    std::ostream& operator<<(std::ostream& os, const InstructionStream& str);
    
    
    
  } // namespace Model
} // namespace MiniMC

#endif
