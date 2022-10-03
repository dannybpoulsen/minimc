#include "parser.hpp"
#include "lexer.hpp"
#include "model/cfg.hpp"
#include "model/builder.hpp"

namespace MiniMC{
namespace Loader{

void Parser::ignore_eol() {
  while(lexer->token() == Token::EOL_TOKEN) lexer->advance();
}

void Parser::run() {
  lexer = new Lexer{*in};
  hash_sign();
  delete lexer;
}

void Parser::hash_sign() {
  ignore_eol();

  switch(lexer->token()){
  case Token::HASH_SIGN:
    switch(lexer->token()) {
    case Token::FUNCTIONS:
      functions();
      break;
    case Token::ENTRYPOINTS:
      entrypoints();
      break;
    case Token::HEAP:
      heap();
      break;
    case Token::INITIALISER:
      intialiser();
      break;
    case Token::EOF_TOKEN:
      return;
    default:
      // todo errors
      throw;
    }
    break;
  default:
    // todo errors
    throw;
  }
}

void Parser::functions() {
  lexer->advance();
  ignore_eol();

  switch(lexer->token()){
  case Token::HASHHASH_SIGN:
    hashhash_sign();
    break;
  case Token::HASH_SIGN:
    hash_sign();
    break;
  default:
    // todo errors
    throw;
  }
}
void Parser::hashhash_sign() {
  lexer->advance();
  ignore_eol();

  switch (lexer->token()) {
  case Token::IDENTIFIER:
    function();
    break;
  default:
    //todo errors
    throw;
  }
}
void Parser::function() {
  std::string fname = identifier();

  auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();
  Model::CFA cfg;
  std::vector<MiniMC::Model::Register_ptr> params;
  Model::RegisterDescr_uptr registerdescr;
  MiniMC::Model::LocationInfoCreator locinfoc(fname,registerdescr.get());

  Model::Type_ptr retType;

  lexer->advance();
  ignore_eol();

  if (lexer->token() == Token::REGISTERS) {
    registerdescr = registers(fname);
  }
  if (lexer->token() == Token::PARAMETERS) {
    parameters(registerdescr->getRegisters(),&params);
  }
  if(lexer->token() == Token::RETURNS) {
    returns(retType);
  }
  if(lexer->token() == Token::CFA) {
    cfa(&cfg,registerdescr.get());
  }
  if(&registerdescr && &params && &retType && &cfg) {
    prgm->addFunction(fname, params, retType, std::move(registerdescr),
                      std::move(cfg));
  } else {
    //todo errors
    throw;
  }

  lexer->advance();
  ignore_eol();
  switch(lexer->token()){
  case Token::HASHHASH_SIGN:
    hashhash_sign();
    break;
  case Token::HASH_SIGN:
    hash_sign();
    break;
  default:
    //todo errors
    throw;
  }
}

Model::RegisterDescr_uptr Parser::registers(std::string name) {

  Model::RegisterDescr_uptr registerDescr = std::make_unique<MiniMC::Model::RegisterDescr>(name);
  std::string id;
  Model::Type_ptr t;

  while(true){
    lexer->advance();
    ignore_eol();

    switch(lexer->token()){
    case Token::LESS_THAN: {
      lexer->advance();
      auto v = variable();
      registerDescr->addRegister(v.value, v.type);
      break;
    }
    default:
      return registerDescr;
    }
  }
  // Todo Errors
  throw;
}

struct Variable Parser::variable(){
  struct Variable v;

  if(lexer->token() == Token::IDENTIFIER) {
    v.value = identifier();
  } else if(lexer->token() == Token::HEX) {
    v.value = hex();
  } else {
    //todo errors
    throw;
  }
  lexer->advance();
  v.type = type();
  lexer->advance();
  if(lexer->token() != Token::GREATER_THAN){
    //todo errors
    throw;
  }
  return v;
}

std::string Parser::identifier() {
  if(lexer->token()==Token::IDENTIFIER){
    return lexer->getValue();
  }else{
    //todo errors
    throw;
  }
}

std::string Parser::hex(){
  if(lexer->token() == Token::HEX){
    return lexer->getValue();
  } else {
    //todo errors
    throw;
  }
}

Model::Type_ptr Parser::type() {
  switch(lexer->token()){
  case Token::TYPE_Void:
    return tfac.makeVoidType();
  case Token::TYPE_Bool:
    return tfac.makeBoolType();
  case Token::TYPE_I8:
    return tfac.makeIntegerType(8);
  case Token::TYPE_I16:
    return tfac.makeIntegerType(16);
  case Token::TYPE_I32:
    return tfac.makeIntegerType(32);
  case Token::TYPE_I64:
    return tfac.makeIntegerType(64);
  case Token::TYPE_Float:
    return tfac.makeFloatType();
  case Token::TYPE_Double:
    return tfac.makeDoubleType();
  case Token::TYPE_Pointer:
    return tfac.makePointerType();
  case Token::TYPE_Struct:
    return tfac.makeStructType(1);
  case Token::TYPE_Array:
    return tfac.makeArrayType(1);
  default:
    //todo errors
    throw;
  }
}
void Parser::parameters(std::vector<Model::Register_ptr> registers, std::vector<Model::Register_ptr> *params) {
  while(true) {
    lexer->advance();
    ignore_eol();
    switch (lexer->token()) {
    case Token::IDENTIFIER:
      std::for_each(registers.begin(), registers.end(), [params,this](auto reg){
        if(reg->getName() == this->identifier()) params->push_back(reg);
              });
      break;
    case Token::EOL_TOKEN:
      break;
    default:
      return;
    }
  }
}
void Parser::returns(Model::Type_ptr retType) {
  ignore_eol();
  retType = type();
}
void Parser::cfa(Model::CFA * cfg,Model::RegisterDescr* regs) {
  std::unordered_map<std::string, Model::Location_ptr> locmap;

next_edge:
  while(lexer->token() != Token::HASH_SIGN || lexer->token() != Token::HASHHASH_SIGN) {
    auto source_loc = std::make_shared<Model::SourceInfo>();
    std::vector<Model::Instruction> instructions;

    lexer->advance();
    ignore_eol();

    std::string from;
    std::string name;
    if (lexer->token() == Token::IDENTIFIER) {
      from = identifier();
      lexer->advance();
      name = identifier();
      lexer->advance();
    } else {
      // todo errors
      throw;
    }

    Model::LocationInfoCreator locinfoc(name, regs);
    if(!locmap.contains(from)){
      auto location = cfg->makeLocation (locinfoc.make(name, 0, *source_loc));
      locmap.insert (std::make_pair(from,location));
    }

    ignore_eol();
    if (lexer->token() == Token::L_BRACKET) {
      while (true) {
        switch (lexer->token()) {
        case Token::R_BRACKET:
          goto next_edge;
        case Token::R_ARROW:
          std::string to = lexer->getValue();
          if (!locmap.contains(to)) {
            auto location =
                cfg->makeLocation(locinfoc.make(name, 0, *source_loc));
            locmap.insert(std::make_pair(to, location));
          }
          auto edge = cfg->makeEdge(locmap[from], locmap[to]);
          std::for_each(instructions.begin(), instructions.end(),
                        [edge](auto ins) {
                          edge->getInstructions().addInstruction(ins);
                        });
          break;
        default:
          lexer->advance();
          ignore_eol();
          instructions.push_back(instruction());
        }
      }
    }
  }
}

void Parser::entrypoints() {
}
void Parser::heap() {
}
void Parser::intialiser() {
}

Model::Instruction Parser::instruction() {
  switch(lexer->token()){
  case Token::INSTR_Skip:
    return MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Skip>();
  case Token::INSTR_Uniform:
    return MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Uniform>();
  case Token::INSTR_PRED_ICMP_SGT:
  case Token::INSTR_PRED_ICMP_UGT:
  case Token::INSTR_PRED_ICMP_SGE:
  case Token::INSTR_PRED_ICMP_UGE:
  case Token::INSTR_PRED_ICMP_SLT:
  case Token::INSTR_PRED_ICMP_ULT:
  case Token::INSTR_PRED_ICMP_SLE:
  case Token::INSTR_PRED_ICMP_ULE:
  case Token::INSTR_PRED_ICMP_EQ:
  case Token::INSTR_PRED_ICMP_NEQ:
    return predicate();
  case Token::INSTR_Assert:
  case Token::INSTR_Assume:
  case Token::INSTR_NegAssume:
    return assumeasserts();
  case Token::LESS_THAN:

  }

}

Model::Instruction Parser::predicate(){
  Model::InstructionData<InstructionCode::PREDICATES> ins;
  switch(lexer->token()){
  case Token::INSTR_PRED_ICMP_SGT:
    ins = Model::createInstruction<Model::InstructionsCode::PRED_ICMP_SGT>();
  case Token::INSTR_PRED_ICMP_UGT:
    ins = Model::createInstruction<Model::InstructionsCode::PRED_ICMP_UGT>();
  case Token::INSTR_PRED_ICMP_SGE:
    ins = Model::createInstruction<Model::InstructionsCode::PRED_ICMP_SGE>();
  case Token::INSTR_PRED_ICMP_UGE:
    ins = Model::createInstruction<Model::InstructionsCode::PRED_ICMP_UGE>();
  case Token::INSTR_PRED_ICMP_SLT:
    ins = Model::createInstruction<Model::InstructionsCode::PRED_ICMP_SLT>();
  case Token::INSTR_PRED_ICMP_ULT:
    ins = Model::createInstruction<Model::InstructionsCode::PRED_ICMP_ULT>();
  case Token::INSTR_PRED_ICMP_SLE:
    ins = Model::createInstruction<Model::InstructionsCode::PRED_ICMP_SLE>();
  case Token::INSTR_PRED_ICMP_ULE:
    ins = Model::createInstruction<Model::InstructionsCode::PRED_ICMP_ULE>();
  case Token::INSTR_PRED_ICMP_EQ:
    ins = Model::createInstruction<Model::InstructionsCode::PRED_ICMP_EQ>();
  case Token::INSTR_PRED_ICMP_NEQ:
    ins = Model::createInstruction<Model::InstructionsCode::PRED_ICMP_NEQ>();
  default:
    //todo error
    throw;
  }
  lexer->advance();
  lexer->advance();
  ins.getOps().op1 = value();
  lexer->advance();
  ins.getOps().op2 = value();
}

Value_ptr Parser::value(){
  struct Variable value;

  if(lexer->token() == Token::LESS_THAN){
    lexer->advance();
    value = variable();
  }
  switch(value.type->geTypeID()){
  case Model::TypeID::Void:
    cfact->
    break;
  case Model::TypeID::Bool:
    cfact->
    break;
  case Model::TypeID::I8:
    cfact->makeIntegerConstant
    break;
  case Model::TypeID::I16:
    cfact->makeIntegerConstant
    break;
  case Model::TypeID::I32:
    cfact->makeIntegerConstant
    break;
  case Model::TypeID::I64:
    cfact->makeIntegerConstant
    break;
  case Model::TypeID::Float:
    cfact->
    break;
  case Model::TypeID::Double:
    cfact->
    break;
  case Model::TypeID::Pointer:
    cfact->
    break;
  case Model::TypeID::Pointer32:
    cfact->
    break;
  case Model::TypeID::Struct:
    cfact->
    break;
  case  cfact-> Model::TypeID::
      Arrabreak;y:

  }
  //todo errors
  throw;
}

Model::Instruction Parser::assumeasserts(){

}



} // namespace Loader
} // namespace MiniMC