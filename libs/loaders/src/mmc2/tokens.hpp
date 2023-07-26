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
	//TYPES
	INT8,
	INT16,
	INT32,
	INT64,
	VOID,
	BOOL,
	AGGR,
	//SEGMENTS
	REGISTERS,
	PARAMETERS,
	RETURNS,
	CFA,
	IDENTIFIER,
	HEXANUMBER,
	HEAPPOINTERLITERAL,
	FUNCTIONPOINTERLITERAL,
	NUMBER,
	QUALIFIEDNAME,
	ASSIGN,
	ARROW,
	COLON
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
      
      struct Token {
	Token (TokenType type = END, std::string s =  "") : content(std::move(s)),type(type) {}
	Token (TokenType type, std::int64_t s ) : content(s),type(type) {}
	
	std::variant<std::string,std::int64_t> content;
	TokenType type;
      };

      inline std::ostream& operator<< (std::ostream& os, const Token& tok) {
	return std::visit ([&os](auto& r)->std::ostream& {return os << r;},tok.content);
      }
      
    }
  }
}
