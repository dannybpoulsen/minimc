#include "Scanner.hpp"
#include "model/cfg.hpp"
#include <istream>
#include <unordered_set>


namespace MiniMC {
  namespace  Loaders {
    namespace MMC {
      class Parser {
      public:
	Parser (std::istream& is) : scanner(is) {
	  advance ();
	}
	
	MiniMC::Model::Program parse (MiniMC::Model::TypeFactory_ptr &tfac, MiniMC::Model::ConstantFactory_ptr &cfac);
	
	
      private:
	void advance () {
	  if (!scanner.yylex (&token))
	    token.type = END;
	}

	Token& get  () {
	  return token;
	}

	bool match (TokenType type,Token* tok = nullptr) {
	  if (token.type == type) {
	    if (tok) {
	      *tok = token;
	    }
	    advance ();
	    return true;
	  }
	  else
	    return false;
	}

	void expect (TokenType type,Token* tok = nullptr) {
	  if (token.type == type) {
	    if (tok) {
	      *tok = token;
	    }
	    advance ();
	  }
	  else
	    throw MiniMC::Support::Exception (MiniMC::Support::Localiser ("Unexpected  '%1%' at %3% Expected: '%2%'").format(token,type,token.location.getStart ()));
	  
	}

		
	bool parseGlobalDeclaration ();
	bool parseFunctionDeclarations ();
	bool parseFunction ();
	bool parseEntryPoints ();
	std::vector<MiniMC::Model::Register_ptr> parseParameters ();
	
	
	void parseHeapSetup ();
	bool parseInitialiserDeclarations ();
	
	bool parseRegisterDeclarations (MiniMC::Model::RegisterDescr& );
	MiniMC::Model::CFA parseCFA (MiniMC::Model::RegisterDescr&);
	MiniMC::Model::Symbol parseNewSymbol (bool fresh = false);
	MiniMC::Model::Symbol parseSymbol ();
	MiniMC::Model::Type_ptr parseReturns ();
	MiniMC::Model::Type_ptr parseType ();
	MiniMC::Model::LocFlags parseLocationFlags ();
	MiniMC::Model::Instruction parseInstruction ();
	MiniMC::Model::Value_ptr parseValue ();
	MiniMC::Model::InstructionCode parseOpcode ();
	
	void skipBlanks () {
	  while (match (TokenType::NEWLINE)) ;
	}
	
	Scanner scanner;
	MiniMC::Loaders::MMC::Token token;
	MiniMC::Model::Program* prgm;
	MiniMC::Model::Frame curFrame;
	MiniMC::Model::SymbolTable<MiniMC::Model::Register_ptr> variableMap;
	std::unordered_set<MiniMC::Model::Symbol> symbolsUsedBeforeDef;
	
      };
    }
  }
}
