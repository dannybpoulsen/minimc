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

    struct ExtendObjContent {
      Value_ptr res;
      Value_ptr object;
      Value_ptr size;
    };
    
    
    struct MallocContent {
      Value_ptr object;
      Value_ptr size;
    };
    
    struct FreeContent {
      Value_ptr object;
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
					     ExtendObjContent,
					     MallocContent,
					     FreeContent,
					     NonDetContent,
					     AssertAssumeContent,
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
								       content(content) {}

      
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


    template<InstructionCode i>
    Instruction createInstruction (const typename InstructionData<i>::Content  content) {
      return Instruction (i,content);
    }
    
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

    std::ostream& operator<<(std::ostream& os, const std::vector<Instruction>& str);
    
    
    /**
	 * Structure representing InstructionStream on  edges.
	 * An InstructionStream is a sequence of instruction to be performed uninterrupted.
	 * If the isPhi is true, then the InstructionStream must be performed without the instructions affecting eachother (this is to 
	 * to reflect that some operations are atomic even within a single process - such as phi-nodes in SSA form).
	 */
    struct InstructionStream {
      using iterator = std::vector<Instruction>::iterator;
      InstructionStream() : InstructionStream(false) {}
      InstructionStream(bool isPhi) : phi(isPhi) {}
      
      InstructionStream(const std::vector<Instruction>& i, bool isPhi = false) : instr(i),
                                                                                 phi(isPhi) {
      }
      InstructionStream(const InstructionStream& str) : instr(str.instr), phi(str.phi) {}

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

      template<InstructionCode c>
      InstructionStream& addInstruction (const typename InstructionData<c>::Content  content) {
	instr.emplace_back (createInstruction<c> (content));
	return *this;
      }

      InstructionStream& addInstruction (const Instruction& i) {
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

      bool isPhi () const {
	return phi;
      }

	
      
    private:
      std::vector<Instruction> instr;
      bool phi = false;
    };
    
    std::ostream& operator<<(std::ostream& os, const InstructionStream& str);
    
    
    
  } // namespace Model
} // namespace MiniMC

#endif
