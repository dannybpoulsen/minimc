#include "model/instructions.hpp"
#include <cstdint>
#include <variant>
#include <ostream>

namespace MiniMC {
  namespace Loaders {
    namespace MMC {
      enum TokenType {
	END,
	LBRACK,
	RBRACK,
	LANGLE,
	RANGLE,
	LBRACE,
	RBRACE,
	AT,
	HASH,
	DOUBLEHASH,
	//TYPES
	INT8,
	INT16,
	INT32,
	INT64,
	VOID,
	BOOL,
	AGGR,
	POINTER,
	//SEGMENTS
	REGISTERS,
	PARAMETERS,
	RETURNS,
	CFA,
	GLOBALS,
	FUNCTIONS,
	ENTRYPOINTS,
	HEAP,
	INITIALISER,
	IDENTIFIER,
	HEXANUMBER,
	POINTERLITERAL,
	NUMBER,
	QUALIFIEDNAME,
	ASSIGN,
	ARROW,
	COLON,
	NEWLINE
      };

      enum class PointerSegment {
	Function,
	Heap
      };
      
      struct PointerLiteral {
	PointerLiteral (PointerSegment seg, std::int64_t b, std::int64_t offset) : segment(seg),base(b),offset(offset)  {}
	PointerSegment segment;
	std::int64_t base;
	std::int64_t offset;
      };

      inline std::ostream& operator<< (std::ostream& os, const PointerLiteral& lit) {
	return os << ((lit.segment == PointerSegment::Function) ? 'F' : 'H')  <<":" << lit.base << "+" <<lit.offset;
      }
      
      struct Token {
	Token (TokenType type = END, std::string s =  "") : content(std::move(s)),type(type) {}
	Token (TokenType type, std::int64_t s ) : content(s),type(type) {}
	Token (TokenType type, PointerLiteral s ) : content(s),type(type) {}
	
	std::variant<std::string,std::int64_t,PointerLiteral> content;
	TokenType type;

	template<class T>
	auto get () const {
	  return std::get<T> (content);
	}
	
	operator bool () const {return type != END;}
      };

      inline std::ostream& operator<< (std::ostream& os, const Token& tok) {
	return std::visit ([&os](auto& r)->std::ostream& {return os << r;},tok.content);
      }
      
    }
  }
}
