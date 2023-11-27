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
	METAREGISTERS,
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
	AGGRCONSTANT,
	NEWLINE,
	
      };

      

      inline std::ostream& operator<< (std::ostream& os, const TokenType& lit) {
	switch (lit) {
	case END:
	  return os << "END";
	case LBRACK:
	  return os << "[";
	case RBRACK:
	  return os << "]";
	case LANGLE:
	  return os << "<";
	case RANGLE:
	  return os << ">";
	case LBRACE:
	  return os << "{";
	case RBRACE:
          return os << "}";
	case AT:
          return os << "@";
	case DOUBLEHASH:
          return os << "##";
	case INT8:
	  return os << "Int8";
	case INT16:
	  return os << "Int16";
	case INT32:
	  return os << "Int32";
	case INT64:
	  return os << "Int64";
	case VOID:
	  return os << "Void";
	case AGGR:
	  return os << "Aggr[0-9]+";
	case POINTER:
	  return os << "Pointer";
	case REGISTERS:
	  return os << ".registers";
	case PARAMETERS:
	  return os << ".parameters";
	case RETURNS:
	  return os << ".returns";
	case CFA:
	  return os << ".cfa";
	case GLOBALS:
	  return os << "# Globals";
	case FUNCTIONS:
	  return os << "# Functions";
	case ENTRYPOINTS:
	  return os << "# Entrypoints";
	case HEAP:
	  return os << "# Heap";
	case INITIALISER:
	  return os << "# Initialiser";
	case IDENTIFIER:
	  return os << "[a-zA-Z\\_][a-zA-Z0-9\\_\\.-]*";
	case HEXANUMBER:
	  return os << "0x[0-9a-f]+";
	case POINTERLITERAL:
	  return os << "[F|H]\\({dig}+\\+{dig}+\\)";
	case NUMBER:
	  return os << "[0-9][0-9]*";
	case QUALIFIEDNAME:
	  return os << "{name}([:]{name})+";
	case ASSIGN:
	  return os << "=";
	case ARROW:
	  return os << "->";
	case COLON:
	  return os << ":";
	case NEWLINE:
	  return os << "[\\n]";
	case AGGRCONSTANT:
	  return os << "AggrConstant";
	default:
	  std::unreachable ();
	}
      }
      
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

      struct AggrType {
	AggrType (std::int64_t size) : size(size) {}
	std::int64_t size;
      };

      inline std::ostream& operator<< (std::ostream& os, const PointerLiteral& lit) {
	return os << ((lit.segment == PointerSegment::Function) ? 'F' : 'H')  <<":" << lit.base << "+" <<lit.offset;
      }

      inline std::ostream& operator<< (std::ostream& os, const AggrType& lit) {
	return os << "Aggr" << lit.size;
      }

      struct Cursor {
	Cursor (std::size_t line = 1, std::size_t col = 1) : line(line),col(col) {}
	Cursor (const Cursor&) = default;

	Cursor& operator= (const Cursor& c) = default;
	auto& operator+= (std::size_t c) {col += c;return *this;}
	auto& newline () {col = 1; line++; return*this;}
	
	
	std::size_t line;
	std::size_t col;
      };

      class  Location {
      public:
	Location (Cursor start = Cursor{}, Cursor end = Cursor{}) : start(start),end(end) {}

	const auto& getStart () const {return start;}
	const auto& getEnd () const {return end;}
	
	
      private:
	Cursor start;
	Cursor end;
      };
      
      inline std::ostream& operator<< (std::ostream& os, const Cursor& c) {
	return os << "["<<c.line << ":" << c.col << "]";
      }

      
      struct Token {
	Token (TokenType type = END, std::string s =  "", Location loc = Location{}) : content(std::move(s)),type(type),location(loc) {}
	Token (TokenType type, std::int64_t s, Location loc = Location{}) : content(s),type(type),location(loc) {}
	Token (TokenType type, PointerLiteral s, Location loc = Location{} ) : content(s),type(type),location(loc) {}
	Token (TokenType type, AggrType s, Location loc = Location{} ) : content(s),type(type),location(loc) {}
	
	
	template<class T>
	auto get () const {
	  return std::get<T> (content);
	}
	
	operator bool () const {return type != END;}
	std::variant<std::string,std::int64_t,PointerLiteral,AggrType> content;
	TokenType type;
	Location location{};
      };

      inline std::ostream& operator<< (std::ostream& os, const Token& tok) {
	return std::visit ([&os](auto& r)->std::ostream& {return os << r;},tok.content);
      }
      
    }
  }
}
