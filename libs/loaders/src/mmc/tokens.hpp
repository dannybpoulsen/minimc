#ifndef MINIMC_TOKENS_HPP
#define MINIMC_TOKENS_HPP


#include <unordered_map>
#include "model/instructions.hpp"

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
  X(PLUS_SIGN)  \
  X(LINE_COMMENT) \
  X(COMMENT_START)\
  X(COMMENT_END)\
  X(AT_SIGN)              \
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
  X(Pointer)  \
  X(Aggr8)           \
  X(HEAP_Pointer) \
  X(FUNCTION_Pointer)

#define ATTRIBUTES \
  X(AssertViolated)

#define TOKENS \
 OPERATIONS    \
  KEYWORDS     \
  NUMBERS      \
  ID           \
  TYPES        \
  SYMBOLS      \
  ATTRIBUTES

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
 OPERATIONS \
 ATTRIBUTES
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
    {'\n',Token::EOL_TOKEN},
    {'@', Token::AT_SIGN}
};

inline static std::unordered_map<std::string,Token> twosignsymbolMap = {
    {"->", Token::R_ARROW},
    {"##", Token::HASHHASH_SIGN},
    {"F(", Token::FUNCTION_Pointer},
    {"H(", Token::HEAP_Pointer},
    {"0x", Token::HEX},
    {"//", Token::LINE_COMMENT},
    {"/*", Token::COMMENT_START},
    {"*/", Token::COMMENT_END}
};



} // namespace Loaders
} // namespace MiniMC


#endif // MINIMC_TOKENS_HPP
