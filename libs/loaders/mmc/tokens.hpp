#ifndef MINIMC_TOKENS_HPP
#define MINIMC_TOKENS_HPP


#include <unordered_map>

namespace MiniMC {
namespace Loaders {

#define SYMBOLS \
  X(EOF_TOKEN) \
  X(COLON) \
  X(EQUAL_SIGN) \
  X(L_BRACKET) \
  X(R_BRACKET) \
  X(L_PARA) \
  X(R_PARA) \
  X(R_ARROW) \
  X(GREATER_THAN) \
  X(LESS_THAN) \
  X(HASH_SIGN) \
  X(HASHHASH_SIGN) \
  X(DOLLAR_SIGN) \
  X(PLUS_SIGN) \
  X(EOL_TOKEN)

#define KEYWORDS \
  X(FUNCTIONS) \
  X(REGISTERS) \
  X(PARAMETERS) \
  X(RETURNS) \
  X(CFA) \
  X(ENTRYPOINTS) \
  X(HEAP) \
  X(INITIALISER)

#define NUMBERS \
  X(HEX) \
  X(DIGIT)

#define ID \
  X(IDENTIFIER)

#define TYPES \
  X(Void) \
  X(Bool) \
  X(Int8) \
  X(Int16) \
  X(Int32) \
  X(Int64) \
  X(Float) \
  X(Double) \
  X(Struct) \
  X(Array) \
  X(Pointer) \
  X(HEAP_Pointer) \
  X(FUNCTION_Pointer)

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

#define UNARYOPS X(Not)

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
  X(InsertValue)

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
  X(Load)

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
    X(NegAssume)

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
  ASSUMEASSERTS    \
    PREDICATES

#define TOKENS \
 OPERATIONS    \
  KEYWORDS     \
  NUMBERS      \
  ID           \
  TYPES        \
  SYMBOLS

enum class Token {
#define X(OP) OP,
TOKENS
#undef X
};

inline std::ostream& operator<<(std::ostream& os, const Token& c) {
  switch (c) {
#define X(OP)               \
  case Token::OP: \
    return os << #OP;
    TOKENS
#undef X
  default:
    return os << "Unknown";
  }
}

inline static std::unordered_map<std::string,Token> keywordsMap = {
    {"Entrypoints", Token::ENTRYPOINTS},
    {"Functions", Token::FUNCTIONS},
    {"Initialiser", Token::INITIALISER},
    {".registers", Token::REGISTERS},
    {".parameters", Token::PARAMETERS},
    {".returns", Token::RETURNS},
    {".cfa", Token::CFA},
    {"Heap", Token::HEAP},
#define X(OP) { #OP , Token::OP },
 TYPES \
 OPERATIONS
#undef X
};

inline static std::unordered_map<char,Token> symbolsMap = {
    {EOF ,Token::EOF_TOKEN},
    {':',Token::COLON},
    {'=',Token::EQUAL_SIGN},
    {'[',Token::L_BRACKET},
    {']',Token::R_BRACKET},
    {'(',Token::L_PARA},
    {')',Token::R_PARA},
    {'>',Token::GREATER_THAN},
    {'<',Token::LESS_THAN},
    {'#',Token::HASH_SIGN},
    {'$',Token::DOLLAR_SIGN},
    {'+',Token::PLUS_SIGN},
    {'\n',Token::EOL_TOKEN}
};





} // namespace Loaders
} // namespace MiniMC


#endif // MINIMC_TOKENS_HPP
