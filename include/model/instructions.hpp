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
      using Content = UnaryContent;
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
    struct Formatter {
      static std::ostream& output(std::ostream& os, const Instruction&) { return os << "??"; }
    };

    
    template <InstructionCode i>
    struct Formatter<i, typename std::enable_if<InstructionData<i>::isTAC || InstructionData<i>::isComparison>::type> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        auto& content = inst.getOps<i> ();
        return os << *content.res << " = " << i << " " << *content.op1 << " " << *content.op1;
      }
    };

    
    template <InstructionCode i>
    struct Formatter<i, typename std::enable_if<InstructionData<i>::isPredicate>::type> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
	auto& content = inst.getOps<i> (); 
	return os << i << " " << *content.op1 << " " << *content.op2;
      }
    };
    
    
    template <InstructionCode i>
    struct Formatter<i, typename std::enable_if<InstructionData<i>::isUnary>::type> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        auto& content = inst.getOps<i> (); 
	return os << *content.res << " = " << i << " " << *content.op1;
      }
    };

   
    template <InstructionCode i>
    struct Formatter<i, typename std::enable_if<InstructionData<i>::isCast>::type> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
	auto& content = inst.getOps<i>();
	return os << *content.res << " = " << i << *content.op1;
      }
    };
    
    template <>
    struct Formatter<InstructionCode::Alloca, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
	auto& content = inst.getOps<InstructionCode::Alloca> ();
        return os << InstructionCode::Alloca << " (" << *content.res << ", " << *content.op1 << " )";
      }
    };
    
    template <>
    struct Formatter<InstructionCode::ExtendObj, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        auto& content = inst.getOps<InstructionCode::ExtendObj> ();
        return os << InstructionCode::ExtendObj << " (" << *content.res << ", " << *content.object << " by " << *content.size << " )";
      }
    };

    template <>
    struct Formatter<InstructionCode::Malloc, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        auto& content = inst.getOps<InstructionCode::Malloc> ();
	return os << InstructionCode::Malloc << " (" << *content.object << ", " << *content.size << " )";
      }
    };

    template <>
    struct Formatter<InstructionCode::Free, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
	auto& content = inst.getOps<InstructionCode::Free> ();
	return os << InstructionCode::Free << " (" << *content.object << " )";
      }
    };

    template <>
    struct Formatter<InstructionCode::FindSpace, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
	auto& content = inst.getOps<InstructionCode::FindSpace> ();
	return os << *content.res << " = " << InstructionCode::FindSpace << *content.op1;
      }
    };

    
    template <>
    struct Formatter<InstructionCode::Skip, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        return os << InstructionCode::Skip;
      }
    };

    template <>
    struct Formatter<InstructionCode::NonDet, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
	auto& content = inst.getOps<InstructionCode::NonDet> ();
	return os << *content.res << "=" << InstructionCode::NonDet << "(" << *content.min << ", " << *content.max << ")";
      }
    };
    //


    

    template <>
    struct Formatter<InstructionCode::StackSave, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
	auto& content = inst.getOps<InstructionCode::StackSave> ();
	return os << *content.res << "=" << InstructionCode::StackSave;
      }
    };

    //

    
    template <>
    struct Formatter<InstructionCode::StackRestore, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
	auto& content =  inst.getOps<InstructionCode::StackRestore> ();
	return os << InstructionCode::StackRestore << "( " << *content.stackobject << " )";
      }
    };

    //

    template <>
    struct Formatter<InstructionCode::MemCpy, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
         return os << InstructionCode::MemCpy;
        ;
      }
    };

    //
    
    
    template <InstructionCode i>
    struct Formatter<i, std::enable_if_t<i == InstructionCode::Assert ||
                                         i == InstructionCode::Assume ||
                                         i == InstructionCode::NegAssume>> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        auto& content = inst.getOps<InstructionCode::Assert> ();
	return os << i << "(" << *content.expr << ")";
      }
    };
    //


    template <>
    struct Formatter<InstructionCode::Call, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        //InstHelper<InstructionCode::Call> h (inst);
        return os << InstructionCode::Call;
      }
    };

    
    template <>
    struct Formatter<InstructionCode::PtrAdd, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        //InstHelper<InstructionCode::PtrAdd> h(inst);
        //return os << *h.getResult() << " = " << *h.getAddress() << " + " << *h.getSkipSize() << "*" << *h.getValue();
	return os << InstructionCode::PtrAdd;
      }
    };

    
    template <>
    struct Formatter<InstructionCode::ExtractValue, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        //InstHelper<InstructionCode::ExtractValue> h(inst);
        //return os << *h.getResult() << " = " << *h.getAggregate() << " [ " << *h.getOffset() << " ] ";
	return os << InstructionCode::ExtractValue;
      }
    };


    template <>
    struct Formatter<InstructionCode::Assign, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
	auto& content = inst.getOps<InstructionCode::Assign> (); 
	return os << *content.res << " = " << *content.op1;
      }
    };
    
    
    template <>
    struct Formatter<InstructionCode::RetVoid, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        return os << "RetVoid";
      }
    };

    //
    
    template <>
    struct Formatter<InstructionCode::Ret, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        return os << "Ret " << *(inst.getOps<InstructionCode::Ret> ().value);
      }
    };

    //
    /*template <>
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
    */
    
    template <>
    struct Formatter<InstructionCode::InsertValue, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
	//  InstHelper<InstructionCode::InsertValue> h(inst);
        //return os << h.getResult() << " = " << *h.getAggregate() << " [ " << *h.getOffset() << " ]:= " << *h.getInsertee();
	return os << InstructionCode::InsertValue;
      }
    };

    //
    template <>
    struct Formatter<InstructionCode::Uniform, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        //InstHelper<InstructionCode::Uniform> h(inst);
        //return os << *h.getResult() << " = " << InstructionCode::Uniform << " ( " << *h.getMin() << ", " << *h.getMax() << ")";
	return os << InstructionCode::Uniform;
      }
    };
    
    
    template <>
    struct Formatter<InstructionCode::Store, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
        //return os << "*" << *h.getAddress() << " = " << *h.getValue();
	return os << InstructionCode::Store << std::endl;
      }
    };

    
    template <>
    struct Formatter<InstructionCode::Load, void> {
      static std::ostream& output(std::ostream& os, const Instruction& inst) {
	return os << InstructionCode::Load;
      }
    };

    
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
