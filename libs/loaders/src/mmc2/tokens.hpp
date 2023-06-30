#include "model/instructions.hpp"
#include <cstdint>

namespace MiniMC {
  namespace Loaders {
    namespace MMC {
      enum TokenType {
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
	NUMBER,
	QUALIFIEDNAME,
	END
      };

      struct Token {
	Token (TokenType type = END, std::string s =  "") : text(std::move(s)),type(type) {}
	std::string text;
	TokenType type;
      };
      
    }
  }
}
