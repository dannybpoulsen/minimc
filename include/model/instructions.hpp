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
  //  X(InsertValueFromConst)

#define CASTOPS \
  X(Trunc)      \
  X(ZExt)       \
  X(SExt)       \
  X(PtrToInt)   \
  X(IntToPtr)   \
  X(BitCast)    \
  X(BoolZExt)   \
  X(BoolSExt)   \
  X(IntToBool)

#define MEMORY \
  X(Alloca)    \
  X(FindSpace) \
  X(Malloc)    \
  X(Free)      \
  X(Store)     \
  X(Load)      \
  X(ExtendObj)

#define INTERNAL  \
  X(Skip)         \
  X(Call)         \
  X(Assign)       \
  X(Ret)          \
  X(RetVoid)      \
  X(NonDet)       \
  X(Assert)       \
  X(Assume)       \
  X(NegAssume)    \
  X(StackRestore) \
  X(StackSave)    \
  X(MemCpy)       \
  X(Uniform)

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
      static const std::size_t operands = 0;
      static const bool hasResVar = false;
      using Content = int;
    };

    struct TACContent {
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

    /*template <>
    struct InstructionData<InstructionCode::InsertValueFromConst> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = true;
      static const bool isPredicate = false;
      static const std::size_t operands = 0;
      static const bool hasResVar = true;
    };
    */

    struct PtrAddContent{
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
      static const bool isComparison = true;
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
    struct InstructionData<InstructionCode::Alloca> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = true;
      static const bool isCast = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;
      static const bool hasResVar = true;
      using Content = UnaryContent;
    };

    struct ExtendObjContent {
      Value_ptr res;
      Value_ptr object;
      Value_ptr size;
    };
    
    template <>
    struct InstructionData<InstructionCode::ExtendObj> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = true;
      static const bool isCast = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 2;
      static const bool hasResVar = true;
      using Content = ExtendObjContent;
    };

    struct MallocContent {
      Value_ptr object;
      Value_ptr size;
    };
    
    template <>
    struct InstructionData<InstructionCode::Malloc> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = true;
      static const bool isCast = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 2;
      static const bool hasResVar = false;
      using Content = MallocContent;
    };

    struct FreeContent {
      Value_ptr object;
    };
    
    
    template <>
    struct InstructionData<InstructionCode::Free> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = true;
      static const bool isCast = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;
      static const bool hasResVar = false;
      using Content = FreeContent;
    };

    
    struct FindSpaceContent {
      Value_ptr res;
      Value_ptr size;
    };
    
    template <>
    struct InstructionData<InstructionCode::FindSpace> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = true;
      static const bool isCast = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;
      static const bool hasResVar = true;
      using Content = FindSpaceContent;
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
      static const std::size_t operands = 2;
      static const bool hasResVar = true;
      using Content = NonDetContent;
    };

    struct AssertAssumeContent {
      Value_ptr expr;
    };
    
    template <>
    struct InstructionData<InstructionCode::Assert> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;
      static const bool hasResVar = false;
      using Content = AssertAssumeContent;
    };

    template <>
    struct InstructionData<InstructionCode::Assume> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;
      static const bool hasResVar = false;
      using Content = AssertAssumeContent;
    };

    template <>
    struct InstructionData<InstructionCode::NegAssume> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;
      static const bool hasResVar = false;
      using Content = AssertAssumeContent;
    };

    struct CallContent {
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
      static const std::size_t operands = 1;
      static const bool hasResVar = false;
      using Content = CallContent;
    };

    struct StackSaveContent {
      Value_ptr res;
    };
    
    
    template <>
    struct InstructionData<InstructionCode::StackSave> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 0;
      static const bool hasResVar = true;
      using Content = StackSaveContent;
    };

    struct StackRestoreContent {
      Value_ptr stackobject;
    };
    
    
    template <>
    struct InstructionData<InstructionCode::StackRestore> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;
      static const bool hasResVar = false;
      using Content = StackRestoreContent ;
    };

    struct MemCpyContent {
      Value_ptr dst;
      Value_ptr src;
      Value_ptr size;
      
    };
    
    template <>
    struct InstructionData<InstructionCode::MemCpy> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = false;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 3;
      static const bool hasResVar = false;
      using Content = MemCpyContent;
    };

    struct LoadContent {
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
      static const bool isMemory = true;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;
      static const bool hasResVar = true;
      using Content = UnaryContent;
    };

    struct RetContent {
      Value_ptr value;
    };
    
    template <>
    struct InstructionData<InstructionCode::Ret> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = true;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
      static const std::size_t operands = 1;
      static const bool hasResVar = false;
      using Content = RetContent;
    };

    template <>
    struct InstructionData<InstructionCode::RetVoid> {
      static const bool isTAC = false;
      static const bool isUnary = false;
      static const bool isComparison = false;
      static const bool isMemory = true;
      static const bool isCast = false;
      static const bool isPointer = false;
      static const bool isAggregate = false;
      static const bool isPredicate = false;
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
					     ExtendObjContent,
					     MallocContent,
					     FreeContent,
					     FindSpaceContent,
					     NonDetContent,
					     AssertAssumeContent,
					     StackSaveContent,
					     StackRestoreContent,
					     MemCpyContent,
					     LoadContent,
					     StoreContent,
					     RetContent,
					     CallContent,
					     int
					     >;

    using ReplaceFunction = std::function<Value_ptr(const Value_ptr&)>;
    template<class T>
    T copyReplace (const T& t, ReplaceFunction replace) {
      if constexpr (std::is_same<TACContent, T> ()) {
	return {.res = replace(t.res), .op1 = replace(t.op1), .op2 = replace(t.op2)};
      }

      else if constexpr (std::is_same<UnaryContent, T> ()) {
	return {.res = replace(t.res), .op1 = replace(t.op1)};
      }

      else if constexpr (std::is_same<BinaryContent,T> ()) {
	return {.op1 = replace(t.op1), .op2 = replace(t.op2)};
      }
      else if constexpr (std::is_same<ExtractContent,T> ()) {
	return {.res = replace(t.res), .aggregate = replace(t.aggregate), .offset = replace (t.offset)};
      }

      else if constexpr (std::is_same<InsertContent,T> ()) {
	return {.res = replace(t.res), .aggregate = replace(t.aggregate), .offset = replace (t.offset), .insertee = replace (t.insertee)};
		
      }

      else  if constexpr (std::is_same<PtrAddContent,T> ()) {
	return {.res = replace(t.res),
	        .ptr = replace(t.ptr),
		.skipsize = replace(t.skipsize),
		.nbSkips = replace(t.nbSkips)
	};
      }

      else if constexpr (std::is_same<ExtendObjContent,T> ()) {
	return {.res = replace(t.res), .object = replace(t.object), .size = replace(t.size)};
      }

      else if constexpr (std::is_same<MallocContent,T> ()) {
	return { .object= replace(t.object), .size = replace(t.size)};
      }

      else if constexpr (std::is_same<FreeContent,T> ()) {
	return { .object= replace(t.object)};
      }
      
      else if constexpr (std::is_same<FindSpaceContent,T> ()) {
	return { .res = replace(t.res),
	  .size = replace(t.size)
	};
      }
      
      else if constexpr (std::is_same<NonDetContent,T> ()) {
	return { .res = replace(t.res), .min = replace(t.min), .max = replace(t.max)};
      }

      else if constexpr (std::is_same<AssertAssumeContent,T> ()) {
	return {.expr = replace(t.expr)};
      }

      else if constexpr (std::is_same<StackSaveContent,T> ()) {
	return {.res = replace(t.res)};
      }

      else if constexpr (std::is_same<StackRestoreContent,T> ()) {
	return {.stackobject = replace(t.stackobject)};
      }

      else if constexpr (std::is_same<MemCpyContent,T> ()) {
	return {.dst = replace(t.dst), .src = replace(t.src), .size = replace(t.size)};
      }

      else if constexpr (std::is_same<LoadContent,T> ()) {
	return {.res = replace(t.res), .addr = replace(t.addr)};
      }
      
      else if constexpr (std::is_same<StoreContent,T> ()) {
	return {.addr = replace(t.addr), .storee = replace(t.storee)};
      }

      else if constexpr (std::is_same<RetContent,T> ()) {
	return {.value = replace(t.value)};
      }

      else if constexpr (std::is_same<CallContent,T> ()) {
	std::vector<Value_ptr> params;
	auto inserter = std::back_inserter(params);
	std::for_each (t.params.begin(),t.params.end(),[replace,&inserter](auto& p) {inserter = replace(p);}); 
	
	return {.res = replace(t.res), .function = replace(t.function), .params = params};

      }

      else if constexpr (std::is_same<int,T> ()) {
	return t;
      }

      else {
	[]<bool flag = false>() {static_assert(flag, "Uncopiable Object");}();
      }
      
    }
    
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
      Instruction(InstructionCode code, Instruction_content content) : opcode(code),
								       content(content),
								       parent(nullptr) {}

      
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
      Function_ptr parent;
    };


    template<InstructionCode i>
    Instruction createInstruction (const typename InstructionData<i>::Content  content) {
      return Instruction (i,content);
    }
    
    inline std::ostream& operator<<(std::ostream& os, const Instruction& inst) {
      return inst.output(os);
    }

    template <InstructionCode i, class T = void>
    class InstHelper;

    template <InstructionCode i, class T = void>
    class InstBuilder;

    template <InstructionCode i, class T = void>
    struct Formatter {
      static std::ostream& output(std::ostream& os, const Instruction&) { return os << "??"; }
    };

    
    
    template <InstructionCode i>
    class InstHelper<i, typename std::enable_if<InstructionData<i>::isTAC ||
						InstructionData<i>::isComparison
						>::type>   {
    public:
      InstHelper(const Instruction& inst) :  inst(inst) {}
      auto& getResult() const { return inst.getOps<i>().res; }
      auto& getLeftOp() const { return inst.getOps<i>().op1; }
      auto& getRightOp() const { return inst.getOps<i>().op2; }

    private:
      const Instruction& inst;
    };

    template <InstructionCode i>
    class InstBuilder<i, typename std::enable_if<InstructionData<i>::isTAC>::type> {
    public:
      auto& setRes(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      auto& setLeft(const Value_ptr& ptr) {
        left = ptr;
        return *this;
      }
      auto& setRight(const Value_ptr& ptr) {
        right = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        assert(res);
        assert(left);
        assert(right);
        return createInstruction<i> ({.res = res, .op1 = left, .op2 = right});
      }

    private:
      Value_ptr res;
      Value_ptr left;
      Value_ptr right;
      };

    template <InstructionCode i>
    struct Formatter<i, typename std::enable_if<InstructionData<i>::isTAC>::type> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<i> h(inst);
        return os << *h.getResult() << " = " << i << " " << *h.getLeftOp() << " " << *h.getRightOp();
      }
    };

    template <InstructionCode i>
    class InstHelper<i, typename std::enable_if<InstructionData<i>::isPredicate>::type> {
    public:
      InstHelper(const Instruction& inst) : inst(inst)
					    
      {}
      auto& getLeftOp() const { return inst.getOps<i>().op1; }
      auto& getRightOp() const { return inst.getOps<i>().op2; }

    private:
      const Instruction& inst;
    };

    template <InstructionCode i>
    class InstBuilder<i, typename std::enable_if<InstructionData<i>::isPredicate>::type> {
    public:
      auto& setLeft(const Value_ptr& ptr) {
        left = ptr;
        return *this;
      }
      auto& setRight(const Value_ptr& ptr) {
        right = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        assert(left);
        assert(right);
        return createInstruction<i> ({.op1 = left, .op2 = right});
      }

    private:
      Value_ptr left;
      Value_ptr right;
    };

    template <InstructionCode i>
    struct Formatter<i, typename std::enable_if<InstructionData<i>::isPredicate>::type> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<i> h(inst);
        return os << i << " " << *h.getLeftOp() << " " << *h.getRightOp();
      }
    };

    template <InstructionCode i>
    class InstHelper<i, typename std::enable_if<InstructionData<i>::isUnary>::type> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getResult() const { return inst.getOps<i> ().res; }
      auto& getOp() const { return inst.getOps<i> ().op1; }

    private:
      const Instruction& inst;
    };

    template <InstructionCode i>
    class InstBuilder<i, typename std::enable_if<InstructionData<i>::isUnary>::type> {
    public:
      auto& setRes(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      auto& setOP(const Value_ptr& ptr) {
        left = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        assert(res);
        assert(left);
        return createInstruction<i> ({.res = res, .op1 = left});
      }

    private:
      Value_ptr res;
      Value_ptr left;
    };

    template <InstructionCode i>
    struct Formatter<i, typename std::enable_if<InstructionData<i>::isUnary>::type> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<i> h(inst);
        return os << *h.getResult() << " = " << i << " " << *h.getOp();
      }
    };

    template <InstructionCode i>
    class InstBuilder<i, typename std::enable_if<InstructionData<i>::isComparison>::type> {
    public:
      auto& setRes(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      auto& setLeft(const Value_ptr& ptr) {
        left = ptr;
        return *this;
      }
      auto& setRight(const Value_ptr& ptr) {
        right = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        assert(res);
        assert(left);
        assert(right);
        return createInstruction<i> ({.res = res, .op1 = left, .op2 = right});
      }

    private:
      Value_ptr res;
      Value_ptr left;
      Value_ptr right;
    };

    template <InstructionCode i>
    struct Formatter<i, typename std::enable_if<InstructionData<i>::isComparison>::type> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<i> h(inst);
        return os << *h.getResult() << " = " << i << " " << *h.getLeftOp() << " " << *h.getRightOp();
      }
    };

    template <InstructionCode i>
    class InstHelper<i, typename std::enable_if<InstructionData<i>::isCast>::type> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getResult() const { return inst.getOps<i>().res; }
      auto& getCastee() const { return inst.getOps<i>().op1; }

    private:
      const Instruction& inst;
    };

    template <InstructionCode i>
    class InstBuilder<i, typename std::enable_if<InstructionData<i>::isCast>::type> {
    public:
      auto& setRes(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      auto& setCastee(const Value_ptr& ptr) {
        castee = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        assert(res);
        assert(castee);
        return createInstruction<i> ({.res = res, .op1 = castee});
      }

    private:
      Value_ptr res;
      Value_ptr castee;
    };

    template <InstructionCode i>
    struct Formatter<i, typename std::enable_if<InstructionData<i>::isCast>::type> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<i> h(inst);
        return os << *h.getResult() << " = " << i << *h.getCastee();
      }
    };

    template <>
    class InstHelper<InstructionCode::Alloca, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getResult() const { return inst.getOps<InstructionCode::Alloca> ().res; }
      auto& getSize() const { return inst.getOps<InstructionCode::Alloca>().op1; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::Alloca, void> {
    public:
      auto& setRes(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      auto& setSize(const Value_ptr& ptr) {
        size = ptr;
        return *this;
      }

      Instruction BuildInstruction() {
        assert(res);
        assert(size);
        return createInstruction<InstructionCode::Alloca> ({.res = res, .op1 = size});
      }

    private:
      Value_ptr res;
      Value_ptr size;
    };

    template <>
    class InstHelper<InstructionCode::ExtendObj, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getResult() const { return inst.getOps<InstructionCode::ExtendObj> ().res; }
      auto& getPointer() const { return inst.getOps<InstructionCode::ExtendObj>().object; }
      auto& getSize() const { return inst.getOps<InstructionCode::ExtendObj>().size; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::ExtendObj> {
    public:
      auto& setRes(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      auto& setPointer(const Value_ptr& ptr) {
        pointer = ptr;
        return *this;
      }
      auto& setSize(const Value_ptr& ptr) {
        size = ptr;
        return *this;
      }

      Instruction BuildInstruction() {
        assert(res);
        assert(size);
        assert(pointer);
        return createInstruction<InstructionCode::ExtendObj> ({.res = res, .object = pointer, .size = size});
      }

    private:
      Value_ptr res;
      Value_ptr pointer;
      Value_ptr size;
    };

    template <>
    class InstHelper<InstructionCode::Malloc, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getPointer() const { return inst.getOps<InstructionCode::Malloc>().object; }
      auto& getSize() const { return inst.getOps<InstructionCode::Malloc>().size; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::Malloc, void> {
    public:
      auto& setPointer(const Value_ptr& ptr) {
        pointer = ptr;
        return *this;
      }
      auto& setSize(const Value_ptr& ptr) {
        size = ptr;
        return *this;
      }

      Instruction BuildInstruction() {
        assert(pointer);
        assert(size);
        return createInstruction<InstructionCode::Malloc> ({.object = pointer, .size = size});
      }

    private:
      Value_ptr pointer;
      Value_ptr size;
    };

    template <>
    class InstHelper<InstructionCode::Free, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getPointer() const { return inst.getOps<InstructionCode::Free>().object; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::Free, void> {
    public:
      auto& setPointer(const Value_ptr& ptr) {
        pointer = ptr;
        return *this;
      }

      Instruction BuildInstruction() {
        assert(pointer);
        return createInstruction<InstructionCode::Free> ({.object = pointer});
      }

    private:
      Value_ptr pointer;
    };

    template <>
    class InstHelper<InstructionCode::FindSpace, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getResult() const { return inst.getOps<InstructionCode::FindSpace> ().res; }
      auto& getSize() const { return inst.getOps<InstructionCode::FindSpace> ().size; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::FindSpace, void> {
    public:
      auto& setResult(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      auto& setSize(const Value_ptr& ptr) {
        size = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        assert(res);
        assert(size);
        return createInstruction<InstructionCode::FindSpace> ({.res = res, .size = size});
      }

    private:
      Value_ptr res;
      Value_ptr size;
    };

    template <>
    struct Formatter<InstructionCode::Alloca, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::Alloca> h(inst);
        return os << InstructionCode::Alloca << " (" << *h.getResult() << ", " << *h.getSize() << " )";
      }
    };

    template <>
    struct Formatter<InstructionCode::ExtendObj, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::ExtendObj> h(inst);
        return os << InstructionCode::ExtendObj << " (" << *h.getResult() << ", " << *h.getPointer() << " by " << *h.getSize() << " )";
      }
    };

    template <>
    struct Formatter<InstructionCode::Malloc, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::Malloc> h(inst);
        return os << InstructionCode::Malloc << " (" << *h.getPointer() << ", " << *h.getSize() << " )";
      }
    };

    template <>
    struct Formatter<InstructionCode::Free, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::Free> h(inst);
        return os << InstructionCode::Free << " (" << *h.getPointer() << " )";
      }
    };

    template <>
    struct Formatter<InstructionCode::FindSpace, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::Alloca> h(inst);
        return os << *h.getResult() << " = " << InstructionCode::FindSpace << *h.getSize();
      }
    };

    template <>
    class InstHelper<InstructionCode::Skip, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::Skip, void> {
    public:
      Instruction BuildInstruction() {
        return Instruction(InstructionCode::Skip, 0);
      }
    };

    template <>
    struct Formatter<InstructionCode::Skip, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::Alloca> h(inst);
        return os << InstructionCode::Skip;
      }
    };

    //
    template <>
    class InstHelper<InstructionCode::NonDet, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getResult() const { return inst.getOps<InstructionCode::NonDet> ().res; }
      auto& getMin() const { return inst.getOps<InstructionCode::NonDet> ().min; }
      auto& getMax() const { return inst.getOps<InstructionCode::NonDet>().max; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::NonDet, void> {
    public:
      auto& setResult(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      auto& setMin(const Value_ptr& ptr) {
        min = ptr;
        return *this;
      }
      auto& setMax(const Value_ptr& ptr) {
        max = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        assert(min);
        assert(max);
        assert(res);
        return createInstruction<InstructionCode::NonDet> ({.res = res, .min = min, .max = max});
      }

    private:
      Value_ptr res;
      Value_ptr min;
      Value_ptr max;
    };

    template <>
    struct Formatter<InstructionCode::NonDet, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::NonDet> h(inst);
        return os << *h.getResult() << "=" << InstructionCode::NonDet << "(" << *h.getMin() << ", " << *h.getMax() << ")";
      }
    };
    //

    template <>
    class InstHelper<InstructionCode::StackSave, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getResult() const { return inst.getOps<InstructionCode::StackSave> ().res; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::StackSave, void> {
    public:
      Instruction BuildInstruction() {
        return createInstruction<InstructionCode::StackSave> ({.res = res});
      }

      auto& setResult(const Value_ptr& p) {
        res = p;
        return *this;
      }

    private:
      Value_ptr res = nullptr;
    };

    template <>
    struct Formatter<InstructionCode::StackSave, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::StackSave> h(inst);
        return os << *h.getResult() << "=" << InstructionCode::StackSave;
      }
    };

    //

    template <>
    class InstHelper<InstructionCode::StackRestore, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getValue() const { return inst.getOps<InstructionCode::StackRestore> ().stackobject ; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::StackRestore, void> {
    public:
      Instruction BuildInstruction() {
        return createInstruction<InstructionCode::StackRestore> ({.stackobject = res});
      }

      auto& setValue(const Value_ptr& p) {
        res = p;
        return *this;
      }

    private:
      Value_ptr res = nullptr;
    };

    template <>
    struct Formatter<InstructionCode::StackRestore, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::StackRestore> h(inst);
        return os << InstructionCode::StackRestore << "( " << *h.getValue() << " )";
      }
    };

    //

    template <>
    class InstHelper<InstructionCode::MemCpy, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getSource() const { return inst.getOps<InstructionCode::MemCpy> ().src;; }
      auto& getTarget() const { return inst.getOps<InstructionCode::MemCpy>().dst; }
      auto& getSize() const { return inst.getOps<InstructionCode::MemCpy>().size; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::MemCpy, void> {
    public:
      auto& setSource(const Value_ptr& p) {
        src = p;
        return *this;
      }
      auto& setTarget(const Value_ptr& p) {
        target = p;
        return *this;
      }
      auto& setSize(const Value_ptr& p) {
        size = p;
        return *this;
      }

      Instruction BuildInstruction() {
        return createInstruction<InstructionCode::MemCpy> ({.dst = target, .src = src,  .size = size});
      }

    private:
      Value_ptr src = nullptr;
      Value_ptr target = nullptr;
      Value_ptr size = nullptr;
    };

    template <>
    struct Formatter<InstructionCode::MemCpy, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::MemCpy> h(inst);
        return os << InstructionCode::MemCpy;
        ;
      }
    };

    //
    template <InstructionCode i>
    class InstHelper<i, std::enable_if_t<i == InstructionCode::Assert ||
                                         i == InstructionCode::Assume ||
                                         i == InstructionCode::NegAssume>> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getAssert() const { return inst.getOps<i>().expr; }

    private:
      const Instruction& inst;
    };

    template <InstructionCode i>
    class InstBuilder<i, std::enable_if_t<i == InstructionCode::Assert ||
                                          i == InstructionCode::Assume ||
                                          i == InstructionCode::NegAssume>> {
    public:
      Instruction BuildInstruction() {
        return createInstruction<i> ({.expr = ass});
      }

      auto& setAssert(const Value_ptr& p) {
        ass = p;
        return *this;
      }

    private:
      Value_ptr ass = nullptr;
    };

    template <InstructionCode i>
    struct Formatter<i, std::enable_if_t<i == InstructionCode::Assert ||
                                         i == InstructionCode::Assume ||
                                         i == InstructionCode::NegAssume>> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::Assert> h(inst);
        return os << i << "(" << *h.getAssert() << ")";
      }
    };
    //

    template <>
    class InstHelper<InstructionCode::Call, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}

      auto nbParams() const { return inst.getOps<InstructionCode::Call> ().params.size(); }
      auto getFunctionPtr() { return inst.getOps<InstructionCode::Call> ().function; }
      auto getParam(std::size_t p) { return inst.getOps<InstructionCode::Call> ().params.at(p); }
      auto getResult() {return inst.getOps<InstructionCode::Call> ().res;}

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::Call, void> {
    public:
      Instruction BuildInstruction() {
        return createInstruction<InstructionCode::Call> ({.res = res, .function = func, .params = params});
      }

      auto& setFunctionPtr(const Value_ptr& func) {
        this->func = func;
        return *this;
      }

      auto& setRes(const Value_ptr& res) {
        this->res = res;
        return *this;
      }

      auto& setNbParamters(const Value_ptr& p) {
        assert(p->isConstant());
        this->nbParameters = p;
        return *this;
      }

      auto& addParam(const Value_ptr& p) {
        params.push_back(p);
        return *this;
      }

    private:
      Value_ptr func;
      Value_ptr nbParameters;
      Value_ptr res = nullptr;
      std::vector<Value_ptr> params;
    };

    template <>
    struct Formatter<InstructionCode::Call, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        //InstHelper<InstructionCode::Call> h (inst);
        return os << InstructionCode::Call;
      }
    };

    template <>
    class InstHelper<InstructionCode::PtrAdd, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getValue() const { return inst.getOps<InstructionCode::PtrAdd> ().nbSkips; }
      auto& getAddress() const { return inst.getOps<InstructionCode::PtrAdd> ().ptr; }
      auto& getSkipSize() const { return inst.getOps<InstructionCode::PtrAdd> ().skipsize; ; }
      auto& getResult() const { return inst.getOps<InstructionCode::PtrAdd> ().res;; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::PtrAdd, void> {
    public:
      auto& setSkipSize(const Value_ptr& ptr) {
        skipSize = ptr;
        return *this;
      }
      auto& setValue(const Value_ptr& ptr) {
        value = ptr;
        return *this;
      }
      auto& setAddress(const Value_ptr& ptr) {
        address = ptr;
        return *this;
      }
      auto& setResult(const Value_ptr& ptr) {
        result = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        return createInstruction<InstructionCode::PtrAdd>  ({.res = result, .ptr = address, .skipsize = skipSize, .nbSkips = value});
      }

    private:
      Value_ptr skipSize;
      Value_ptr value;
      Value_ptr address;
      Value_ptr result;
    };

    template <>
    struct Formatter<InstructionCode::PtrAdd, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::PtrAdd> h(inst);
        return os << *h.getResult() << " = " << *h.getAddress() << " + " << *h.getSkipSize() << "*" << *h.getValue();
      }
    };

    template <>
    class InstHelper<InstructionCode::ExtractValue, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getAggregate() const { return inst.getOps<InstructionCode::ExtractValue> ().aggregate;}
      auto& getOffset() const { return inst.getOps<InstructionCode::ExtractValue> ().offset; }
      auto& getResult() const { return inst.getOps<InstructionCode::ExtractValue> ().res; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::ExtractValue, void> {
    public:
      auto& setAggregate(const Value_ptr& aggr) {
        aggregate = aggr;
        return *this;
      }
      auto& setOffset(const Value_ptr& ptr) {
        offset = ptr;
        return *this;
      }
      auto& setResult(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        return createInstruction<InstructionCode::ExtractValue> ({.res = res, .aggregate = aggregate, .offset = offset});
      }

    private:
      Value_ptr aggregate;
      Value_ptr offset;
      Value_ptr res;
    };

    template <>
    struct Formatter<InstructionCode::ExtractValue, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::ExtractValue> h(inst);
        return os << *h.getResult() << " = " << *h.getAggregate() << " [ " << *h.getOffset() << " ] ";
      }
    };

    template <>
    class InstHelper<InstructionCode::Assign, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getValue() const { return inst.getOps<InstructionCode::Assign> ().op1; }
      auto& getResult() const { return inst.getOps<InstructionCode::Assign> ().res; }

    private:
      const Instruction& inst;
    };

    template <>
    struct Formatter<InstructionCode::Assign, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::Assign> h(inst);
        return os << *h.getResult() << " = " << *h.getValue();
      }
    };

    template <>
    class InstBuilder<InstructionCode::Assign, void> {
    public:
      auto& setValue(const Value_ptr& aggr) {
        value = aggr;
        return *this;
      }
      auto& setResult(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        return createInstruction<InstructionCode::Assign> ({.res = res, .op1 = value});
      }

    private:
      Value_ptr value;
      Value_ptr res;
    };

    template <>
    class InstHelper<InstructionCode::RetVoid, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::RetVoid, void> {
    public:
      Instruction BuildInstruction() {
        return createInstruction<InstructionCode::RetVoid> (0);
      }
    };

    template <>
    struct Formatter<InstructionCode::RetVoid, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::RetVoid> h(inst);
        return os << "RetVoid";
      }
    };

    //
    template <>
    class InstHelper<InstructionCode::Ret, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getValue() const { return inst.getOps<InstructionCode::Ret> ().value; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::Ret, void> {
    public:
      Instruction BuildInstruction() {
        return createInstruction<InstructionCode::Ret> ({.value  = value});
      }

      void setRetValue(const Value_ptr& value) {
        this->value = value;
      }

    private:
      Value_ptr value;
    };

    template <>
    struct Formatter<InstructionCode::Ret, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::Ret> h(inst);
        return os << "Ret " << *h.getValue();
      }
    };

    //
    template <>
    class InstHelper<InstructionCode::InsertValue, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getAggregate() const { return inst.getOps<InstructionCode::InsertValue> ().aggregate; }
      auto& getOffset() const { return inst.getOps<InstructionCode::InsertValue> ().offset; }
      auto& getInsertee() const { return inst.getOps<InstructionCode::InsertValue> ().insertee; }
      auto& getResult() const { return inst.getOps<InstructionCode::InsertValue> ().res; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::InsertValue, void> {
    public:
      auto& setAggregate(const Value_ptr& aggr) {
        aggregate = aggr;
        return *this;
      }
      auto& setOffset(const Value_ptr& ptr) {
        offset = ptr;
        return *this;
      }
      auto& setResult(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      auto& setInsertee(const Value_ptr& ptr) {
        insertee = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        return createInstruction<InstructionCode::InsertValue> ({.res = res, .aggregate = aggregate, .offset = offset, .insertee = insertee});
      }

    private:
      Value_ptr aggregate;
      Value_ptr offset;
      Value_ptr insertee;
      Value_ptr res;
    };

    /*   template <>
    class InstHelper<InstructionCode::InsertValueFromConst, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getAggregate() const { return inst.getOp(0); }
      size_t nbOps() const { return inst.getNbOps() - 3; }
      auto& getOp(size_t op) const { return inst.getOp(op + 3); }
      auto& getInsertee() const { return inst.getOp(2); }
      auto& getResult() const { return inst.getOp(1); }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::InsertValueFromConst, void> {
    public:
      void setAggregate(const Value_ptr& aggr) { aggregate = aggr; }
      void addOps(const Value_ptr& ptr) { consts.push_back(ptr); }
      void setResult(const Value_ptr& ptr) { res = res; }
      void setInsertee(const Value_ptr& ptr) { insertee = ptr; }
      Instruction BuildInstruction() {
        std::vector<Value_ptr> vals({aggregate, res, insertee});
        std::copy(consts.begin(), consts.end(), std::back_inserter(vals));
        return Instruction(InstructionCode::InsertValueFromConst, vals);
      }

    private:
      Value_ptr aggregate;
      Value_ptr insertee;
      Value_ptr res;
      std::vector<Value_ptr> consts;
    };
    */
    template <>
    struct Formatter<InstructionCode::InsertValue, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::InsertValue> h(inst);
        return os << h.getResult() << " = " << *h.getAggregate() << " [ " << *h.getOffset() << " ]:= " << *h.getInsertee();
      }
    };

    //
    template <>
    class InstHelper<InstructionCode::Uniform, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getResult() const { return inst.getOps<InstructionCode::Uniform> ().res; }
      auto& getMin() const { return inst.getOps<InstructionCode::Uniform> ().min; }
      auto& getMax() const { return inst.getOps<InstructionCode::Uniform> ().max; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::Uniform, void> {
    public:
      auto& setResult(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      auto& setMin(const Value_ptr& ptr) {
        min = ptr;
        return *this;
      }
      auto& setMax(const Value_ptr& ptr) {
        max = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        assert(min);
        assert(max);
        assert(res);
        return createInstruction<InstructionCode::Uniform> ({.res = res, .min = min, .max = max});
      }

    private:
      Value_ptr res;
      Value_ptr min;
      Value_ptr max;
    };

    template <>
    struct Formatter<InstructionCode::Uniform, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::Uniform> h(inst);
        return os << *h.getResult() << " = " << InstructionCode::Uniform << " ( " << *h.getMin() << ", " << *h.getMax() << ")";
      }
    };

    //
    template <>
    class InstHelper<InstructionCode::Store, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {}
      auto& getValue() const { return inst.getOps<InstructionCode::Store> ().storee; }
      auto& getAddress() const { return  inst.getOps<InstructionCode::Store> ().addr;}

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::Store, void> {
    public:
      auto& setValue(const Value_ptr& ptr) {
        value = ptr;
        return *this;
      }
      auto& setAddress(const Value_ptr& ptr) {
        address = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        assert(value);
        assert(address);
        return createInstruction<InstructionCode::Store> ({.addr = address, .storee =value });
      }

    private:
      Value_ptr value;
      Value_ptr address;
    };

    template <>
    struct Formatter<InstructionCode::Store, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::PtrAdd> h(inst);
        return os << "*" << *h.getAddress() << " = " << *h.getValue();
      }
    };

    template <>
    class InstHelper<InstructionCode::Load, void> {
    public:
      InstHelper(const Instruction& inst) : inst(inst) {
      }
      auto& getResult() const { return inst.getOps<InstructionCode::Load> ().res;}
      auto& getAddress() const { return inst.getOps<InstructionCode::Load> ().addr; }

    private:
      const Instruction& inst;
    };

    template <>
    class InstBuilder<InstructionCode::Load, void> {
    public:
      auto& setRes(const Value_ptr& ptr) {
        res = ptr;
        return *this;
      }
      auto& setAddress(const Value_ptr& ptr) {
        address = ptr;
        return *this;
      }
      Instruction BuildInstruction() {
        assert(res);
        assert(address);
        return createInstruction<InstructionCode::Load> ({.res = res, .addr = address});
      }

    private:
      Value_ptr res;
      Value_ptr address;
    };

    template <>
    struct Formatter<InstructionCode::Load, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        InstHelper<InstructionCode::Load> h(inst);
        return os << *h.getResult() << " = *" << *h.getAddress();
      }
    };

    template <InstructionCode code>
    InstHelper<code> makeHelper(Instruction& inst) {
      assert(inst.getOpcode() == code);
      return InstHelper<code, void>(inst);
    }

    using instructionstream = std::vector<Instruction>;

    inline std::ostream& operator<<(std::ostream& os, const instructionstream& str) {
      for (auto& i : str) {
        os << i << std::endl;
      }
      return os;
    }

    inline std::ostream& Instruction::output(std::ostream& os) const {
      switch (getOpcode()) {
#define X(OP)                                                 \
  case InstructionCode::OP:                                   \
    return Formatter<InstructionCode::OP>::output(os, *this); \
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

    template <InstructionCode op, InstructionCode... tail>
    inline static bool isOneOf(MiniMC::Model::Instruction& instr) {
      if constexpr (sizeof...(tail) == 0)
        return instr.getOpcode() == op;
      else {
        if (instr.getOpcode() == op)
          return true;
        else
          return isOneOf<tail...>(instr);
      }
    }

    template<InstructionCode i>
    Instruction copyInstructionWithReplaceT (const Instruction& inst, ReplaceFunction replace) {
      assert(inst.getOpcode () == i);
      return createInstruction<i> (copyReplace (inst.getOps<i> (), replace));
    }

    inline Instruction copyInstructionWithReplace (const Instruction& inst, ReplaceFunction replace) {
      switch (inst.getOpcode ()) {
#define X(OP)					\
	case InstructionCode::OP:		\
	  return copyInstructionWithReplaceT<InstructionCode::OP>  (inst,replace);
	OPERATIONS
#undef X
      }
      throw MiniMC::Support::Exception ("Uncopiable Object");
    }

    
    
    
  } // namespace Model
} // namespace MiniMC

#endif
