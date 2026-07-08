#include "parser.hpp"
#include "minimc/model/types.hpp"
#include "minimc/model/variables.hpp"

namespace MiniMC {
  namespace  Loaders {
    namespace MMC {
      
      MiniMC::Model::Symbol Parser::parseNewSymbol (bool fresh) {
	Token tok;
	if (!match (IDENTIFIER,&tok))
	  expect (QUALIFIEDNAME,&tok);
	if (auto symb_ = curFrame.resolveQualified (tok.get<std::string> ())) {
	  auto symbol = symb_.value();
          if (symbolsUsedBeforeDef.count (symbol)) {
	    symbolsUsedBeforeDef.erase (symbol);
	    return symbol;
	  }
	  else {
	    if (fresh)
	      return curFrame.makeFresh (symbol.getName ());
	    else 
	      throw MiniMC::Support::Exception (MiniMC::Support::Localiser{"Symbol '%1%' already exists"}.format (symbol));
	
	  }
	  
	}
	else {
	  auto symb = MiniMC::Model::Symbol::from_string (tok.get<std::string> ());
	  return (fresh) ? curFrame.makeFresh (symb.getName ()) : curFrame.makeSymbol (symb.getName ());	
	}
      }

      MiniMC::Model::Symbol Parser::parseSymbol () {
	Token tok;
	
	if (!match (IDENTIFIER,&tok) ) {
	  expect (QUALIFIEDNAME,&tok);
	}
	
        auto res = curFrame.resolveQualified (tok.get<std::string> ());
	if (res)
	  return res.value();
	else {
	  auto symb = MiniMC::Model::Symbol::from_string (tok.get<std::string> ());
	  auto res =  prgm->getRootFrame().makeSymbol (symb.getName ());
	  symbolsUsedBeforeDef.insert (res);
	  return res;
	}
      }
      
      MiniMC::Model::Type_ptr Parser::parseType () {
	Token tok;
	if (match (INT8))
	  return MiniMC::Model::I8Type::get();
	else if (match (INT16))
	  return MiniMC::Model::I16Type::get();
	else if (match (INT32))
	  return MiniMC::Model::I32Type::get();
	else if (match (INT64))
	  return MiniMC::Model::I64Type::get();//return tfactory->makeIntegerType (64);
	else if (match (BOOL))
	  return MiniMC::Model::BoolType::get();//return tfactory->makeBoolType ();
	else if (match (POINTER))
	  return MiniMC::Model::PointerType::get();//return tfactory->makePointerType ();
	else if (match (AGGR,&tok)) {
	  return MiniMC::Model::AggregateType::get(tok.get<AggrType> ().size);//return tfactory->makeAggregateType (tok.get<AggrType> ().size);
	}
	else {
	  expect (VOID);
	  return MiniMC::Model::VoidType::get();//return tfactory->makeVoidType ();
	}
	
      }
      
      bool Parser::parseRegisterDeclarations (MiniMC::Model::RegisterDescr& descr) {
	while (match (LANGLE)) {
	  auto symb = parseNewSymbol ();
	  auto type = parseType ();
	  expect (RANGLE);
	  expect (NEWLINE);
	  variableMap.emplace (symb,descr.addRegister (std::move(symb),type));
	}
	return true;
      }
	
      MiniMC::Model::Type_ptr Parser::parseReturns () {
	expect(RETURNS);
	expect(NEWLINE);
	auto res = parseType ();
	expect(NEWLINE);
	return res;
      }
	
      
      bool Parser::parseGlobalDeclaration  () {
	if (match (GLOBALS)) {
	  expect(NEWLINE);
	  expect(REGISTERS);
	  skipBlanks ();
	  curFrame = prgm->getRootFrame ();
	  parseRegisterDeclarations (prgm->getCPURegs ());
	  skipBlanks ();
	  if (match(METAREGISTERS)) {
	    skipBlanks ();
	    curFrame = prgm->getRootFrame ();
	    parseRegisterDeclarations (prgm->getMetaRegs ());
	    skipBlanks ();
	  }
	  
	  return true;
	}
	return false;
      }

      std::vector<MiniMC::Model::Symbol> Parser::parseParameters () {
	std::vector<MiniMC::Model::Symbol> res;
	Token tok;
	while (get().type != RETURNS) {
	  auto symb = parseSymbol ();
	  expect (NEWLINE);
	  res.emplace_back (variableMap.at (symb)->getSymbol());
	}
	return res;
      }
      
      
      bool Parser::parseFunction () {
	auto oldVarmap = variableMap;
	Token tok;
	//expect (IDENTIFIER,&tok);
	auto symbol = parseNewSymbol ();
	expect (NEWLINE);
	MiniMC::Model::RegisterDescr rdescr;
	std::vector<MiniMC::Model::Symbol> params;
	
	curFrame = curFrame.create (symbol.getName ());
	if (match (REGISTERS)) {
	  expect (NEWLINE);
	  parseRegisterDeclarations(rdescr);
	}
	if (match (PARAMETERS)) {
	  expect (NEWLINE);
	  params = parseParameters ();
	}
	MiniMC::Model::Type_ptr resType = parseReturns ();
	MiniMC::Model::CFA cfa = parseCFA (rdescr);
	
	prgm->addFunction (symbol,std::move(params),resType,std::move(rdescr),std::move(cfa),false,curFrame);
	
	variableMap = oldVarmap;	   
	curFrame = curFrame.close ();

	
	return true;
      }

      bool Parser::parseEntryPoints () {
	expect (ENTRYPOINTS);
	expect (NEWLINE);
	while (get().type == IDENTIFIER) {
	  Token tok;
	  expect (IDENTIFIER,&tok);
	  expect (NEWLINE);
	  if (auto symb = curFrame.resolve (tok.get<std::string> ()))
	    prgm->addEntryPoint (symb.value());
	  else {
	    throw MiniMC::Support::Exception (MiniMC::Support::Localiser ("Cannot resolve name '%1%'").format (tok.get<std::string> ()));
	  }
	    
	}
	return true;
      }
      
      bool Parser::parseFunctionDeclarations () {
	if (match (FUNCTIONS)) {
	  skipBlanks ();
	  while (match (DOUBLEHASH)) {
	    parseFunction ();
	    
	  }
	  return true;
	}
	return false;
      }

      MiniMC::Model::LocFlags Parser::parseLocationFlags () {
	MiniMC::Model::LocFlags flags;
	if (match (AT)) {
	  Token tok;
	  expect(IDENTIFIER,&tok);
	  if (tok.get<std::string> () == "AssertViolated") {
	    flags |= MiniMC::Model::Attributes::AssertViolated;
	  }
	  else if (tok.get<std::string> () == "UnrollFailed") {
	    flags |= MiniMC::Model::Attributes::UnrollFailed;
	  }
	  expect(NEWLINE);
	}
	
	return flags;
      }

      MiniMC::Model::Value_ptr Parser::parseValue () {
	if (get ().type != LANGLE)
	  return nullptr;
	else {
	  Token tok;
	  expect (LANGLE);
	  if (match (NUMBER,&tok) || match (NUMBER,&tok)) {
	    auto type = parseType ();
	    expect (RANGLE);
	    switch (type->getTypeID ()) {
	    case MiniMC::Model::TypeID::I8:
	      return MiniMC::Model::I8Integer::make(tok.get<int64_t>());
	    case MiniMC::Model::TypeID::I16:
	      return MiniMC::Model::I16Integer::make(tok.get<int64_t>());
	    case MiniMC::Model::TypeID::I32:
	      return MiniMC::Model::I32Integer::make(tok.get<int64_t>());
	    case MiniMC::Model::TypeID::I64:
	      return MiniMC::Model::I64Integer::make(tok.get<int64_t>());
	    default:
	      throw MiniMC::Support::Exception ("Not an integer type");
	    }
	  }
	  else if (match (POINTERLITERAL,&tok)) {
	    auto type = parseType ();
	    expect (RANGLE);
	    auto literal = tok.get<PointerLiteral> ();
	    if (literal.segment == PointerSegment::Function) {
	      return MiniMC::Model::Pointer::make(MiniMC::Model::pointer64_t::makeFunctionPointer(literal.base));
	    }
	    else 
	      return MiniMC::Model::Pointer::make(MiniMC::Model::pointer64_t::makeHeapPointer(literal.base,literal.offset));
	  }
	  else if (get().type == IDENTIFIER ||
		   get().type == QUALIFIEDNAME) {
	    auto symbol = parseSymbol ();
	    auto type = parseType ();
	    expect (RANGLE);
	    if (variableMap.count (symbol)) {
	      auto res = variableMap.at (symbol);
	      res->setType (type);
	      return res;
	    }

	    else {
	      auto res = MiniMC::Model::SymbolicConstant::make (symbol);
	      res->setType (type);
	      return res;
	    }
	    
	  }
	  else if (match (AGGRCONSTANT,&tok)) {
	    //std::vector<MiniMC::Model::Constant_ptr> inputs;
	    MiniMC::Model::AggregateConstantBuilder builder;
	    auto aggr_str_encoded = tok.get<std::string> ();
	    MiniMC::Support::STDEncode encode;
	    auto decoded= encode.decode (aggr_str_encoded);
	    for (char c : decoded) {
	      builder << std::static_pointer_cast<MiniMC::Model::Constant> (MiniMC::Model::I8Integer::make (c));
	    }
	    auto type = parseType ();
	    expect (RANGLE);
	    auto res = builder.build();
	    res->setType (type);
	    return res;
	  }
	  throw MiniMC::Support::Exception ("Don't know how to create type");
	  
	}
      }

      MiniMC::Model::InstructionCode Parser::parseOpcode () {
	if (get().type  != IDENTIFIER) {
	  std::cerr << get().type << std::endl; 
	  return MiniMC::Model::InstructionCode::Assign;
	  
	}
	Token tok;
	expect (IDENTIFIER,&tok);
	
	return MiniMC::Model::str2opcode.at (tok.get<std::string> ());
      }
      
      MiniMC::Model::Instruction Parser::parseInstruction () {
	
	
	std::vector<MiniMC::Model::Value_ptr> params;
	MiniMC::Model::Value_ptr res = parseValue ();
	
	if (res ) {
	  expect (ASSIGN);
	  
	}
	MiniMC::Model::InstructionCode opcode = parseOpcode();
	
	if (res  || opcode == MiniMC::Model::InstructionCode::Call)
	  params.push_back(res);
	if (MiniMC::Model::isCast(opcode) ||
	    opcode == MiniMC::Model::InstructionCode::Load ||
	    opcode == MiniMC::Model::InstructionCode::InsertValue ||
	    opcode == MiniMC::Model::InstructionCode::ExtractValue
	    )
	  parseType ();
	
	MiniMC::Model::Value_ptr val;
	do {
	  val = parseValue ();
	  if (val)
	    params.push_back (val);
	  
	}while (val!=nullptr);
	if (opcode == MiniMC::Model::InstructionCode::Load) {
	  params = std::vector<MiniMC::Model::Value_ptr> {params.at(0),heap_var,params.at(1)};
	}
	if (opcode == MiniMC::Model::InstructionCode::Store) {
	  params = std::vector<MiniMC::Model::Value_ptr> {heap_var,heap_var,params.at(0),params.at(1)};
	}
	if (opcode == MiniMC::Model::InstructionCode::PtrAdd ||
	    opcode == MiniMC::Model::InstructionCode::PtrSub) {
	  params = std::vector<MiniMC::Model::Value_ptr> {params.at(0),params.at(1),std::make_shared<MiniMC::Model::MulExpr>(params.at(2),params.at(3))};
	}
	expect (NEWLINE);
	return MiniMC::Model::makeInstruction (opcode,params);
      }
      
      void Parser::parseHeapSetup () {
	expect (HEAP);
	expect (NEWLINE);
	
	auto oldStyle = [this](auto& p) -> bool {
	  Token num_tok;
	  if  (match (NUMBER,&num_tok)) {
	    p = MiniMC::Model::pointer_t::makeHeapPointer(num_tok.get<int64_t> (),0);
	    return true;
	  }
	  return false;
	};

	auto newStyle = [this](auto& p) -> bool {
	  Token ptr_tok;
	  if  (match (POINTERLITERAL,&ptr_tok)) {
	    auto literal = ptr_tok.get<PointerLiteral> ();
	    p = MiniMC::Model::pointer_t::makeHeapPointer(literal.base,0);
	    return true;
	  }
	  return false;
	};
	
	Token size_tok;
	MiniMC::Model::pointer_t pointer;
	
	while (oldStyle (pointer) || newStyle(pointer)) {
	  
	  
	  expect (COLON);
	  expect (NUMBER,&size_tok);
	  auto value = parseValue();
	  expect (NEWLINE);
	  prgm->getHeapLayout ().addBlock (prgm->getRootFrame().makeFresh ("heap"),pointer,size_tok.get<int64_t> (),heap_var,value);
	    
	  
	}
	
      }
      
      MiniMC::Model::CFA Parser::parseCFA (MiniMC::Model::RegisterDescr& rdescr) {
	expect (CFA);
	expect (NEWLINE);
	MiniMC::Model::CFA cfa;
	std::unordered_map<std::string,MiniMC::Model::Location_ptr> locations;
	auto newLocation = [this,&rdescr,&cfa,&locations](const std::string& s, MiniMC::Model::LocFlags flags, MiniMC::Model::Symbol symbol) {
	  if (!locations.count(s)){
	    MiniMC::Model::LocationInfo info {flags,curFrame};
	    auto loc = cfa.makeLocation (symbol,info);
	    if (!locations.size ())
	      cfa.setInitial (loc);
	    locations.emplace(s,loc);
	  }
	  else {
	    locations.at (s)->getInfo().getFlags () = flags;
	    locations.at (s)->rename (symbol);
	    locations.at (s)->setSymbol (symbol);
	    symbol.setUserData (locations.at (s));
	  }
	  return locations.at (s);
	};

	auto lookupLocation = [this,&rdescr,&cfa,&locations](const std::string& s) {
	  if (!locations.count(s)){
	    MiniMC::Model::LocFlags flags {};
	    MiniMC::Model::LocationInfo info {flags,curFrame};
	    auto loc = cfa.makeLocation (MiniMC::Model::Symbol::from_string (s),info);
	    locations.emplace(s,loc);
	  }
	  return locations.at (s);
	};
	
	while (get().type == IDENTIFIER) {
	  
	  Token tok;
	  expect (IDENTIFIER,&tok);
	  expect (LBRACE);
	  auto symbol = parseNewSymbol (true);
	  expect (RBRACE);
	  expect (NEWLINE);
	  auto flags = parseLocationFlags ();
	  expect (LBRACK);
	  expect (NEWLINE);
	  auto from = newLocation (tok.get<std::string> (),flags,symbol);
	  while (get().type != RBRACK) {
	    MiniMC::Model::InstructionStream instructions;
	    while (get().type != ARROW) {
	      instructions.add (parseInstruction ());
	    }
	    expect (ARROW);
	    expect (IDENTIFIER,&tok);
	    auto to = lookupLocation (tok.get<std::string> ());
	    expect (NEWLINE);
	    cfa.makeEdge (from,to,std::move(instructions));
	  }
	  expect(RBRACK);
	  expect(NEWLINE);
	    
	}
	
	return cfa;
      }


      
      MiniMC::Model::Program Parser::parse () {
	MiniMC::Model::Program program;
	prgm = &program;
	MiniMC::Loaders::MMC::Token tt;

	heap_var = program.getPersistentRegs().addRegister (program.getRootFrame().makeSymbol ("heap"),MiniMC::Model::MemoryType::get());
	
	parseGlobalDeclaration ();
	parseFunctionDeclarations ();
	parseEntryPoints ();
	parseHeapSetup ();
	if (symbolsUsedBeforeDef.size ()) {
	  std::stringstream str;
	  for (auto& s : symbolsUsedBeforeDef) {
	    str << s << "," ;
	  }
	  throw MiniMC::Support::Exception (MiniMC::Support::Localiser {"Symbols '%1%' used by not defined"}.format (str.str()));
	}
	return program;
      }
      
    }
  }
} // namespace MiniMC
