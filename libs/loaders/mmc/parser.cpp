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
  functions();
  entrypoints();
  heap();
  initialiser();
  delete lexer;
}

void Parser::functions(){
  if(lexer->token() == Token::HASH_SIGN){
    lexer->advance();
    if(lexer->token()== Token::FUNCTIONS){
      lexer->advance(); ignore_eol();
      while(lexer->token() != Token::HASH_SIGN){
        function();
        lexer->advance(); ignore_eol();
      }
    }
  }
  throw;
}

void Parser::entrypoints(){
  if(lexer->token() == Token::HASH_SIGN){
    lexer->advance();
    if(lexer->token() == Token::ENTRYPOINTS){
      lexer->advance(); ignore_eol();
      while(lexer->token() != Token::HASH_SIGN){
        prgm->addEntryPoint(identifier());
        lexer->advance(); ignore_eol();
      }
      return;
    }
  }
  throw;
}

void Parser::heap() {
  if (lexer->token() == Token::HASH_SIGN) {
    lexer->advance();
    if (lexer->token() == Token::HEAP) {
      lexer->advance();
      ignore_eol();
      while (lexer->token() != Token::HASH_SIGN) {
        if (lexer->token() == Token::DIGIT) {
          lexer->advance();
          if (lexer->token() == Token::COLON) {
            lexer->advance();
            if (lexer->token() == Token::DIGIT) {
              prgm->getHeapLayout().addBlock(stoi(lexer->getValue()));
              return;
            }
          }
        }
      }
    }
  }
  throw;
}

void Parser::initialiser(){
  Model::InstructionStream instructionStream;
  std::vector<MiniMC::Model::Register_ptr> params;

  if (lexer->token() == Token::HASH_SIGN) {
    lexer->advance();
    if (lexer->token() == Token::INITIALISER) {
      lexer->advance();
      ignore_eol();
      while(lexer->token() != Token::EOF_TOKEN){
        instruction(&instructionStream, &params);
        lexer->advance();
        ignore_eol();
      }
      prgm->setInitialiser(instructionStream);
      return;
    }
  }
  throw;
}
void Parser::function() {
  std::vector<MiniMC::Model::Register_ptr> params;

  if (lexer->token() == Token::HASHHASH_SIGN) {
    lexer->advance();
    std::string name = identifier();
    lexer->advance();
    ignore_eol();
    auto registerDescr = registers(name);

    parameters(&params, registerDescr.get());
    auto ret = returns();
    auto cfg = cfa(name, &params, registerDescr.get());
    prgm->addFunction(name, params, ret,
                      std::move(registerDescr), std::move(cfg));
  }
  throw;
}

Model::RegisterDescr_uptr Parser::registers(std::string name){
  auto registerDescr = std::make_unique<MiniMC::Model::RegisterDescr> (name);

  if(lexer->token() == Token::REGISTERS){
    lexer->advance();
    ignore_eol();
    while(lexer->token() != Token::PARAMETERS){
      auto v = variable();
      registerDescr->addRegister(v.getName(), v.getType());
      lexer->advance();
      ignore_eol();
    }
    return std::move(registerDescr);
  }
  throw;
}

void Parser::parameters(std::vector<MiniMC::Model::Register_ptr>* params, const MiniMC::Model::RegisterDescr* regs) {
  std::vector<Model::Register_ptr> variables = regs->getRegisters();

  if(lexer->token() == Token::PARAMETERS){
    lexer->advance();
    ignore_eol();
    while(lexer->token() != Token::RETURNS){
      std::for_each(variables.begin(),variables.end(), [params,this](Model::Register_ptr reg){
        if(reg->getName() == identifier()){
          params->push_back(reg);
        }
      });
      lexer->advance();
      ignore_eol();
    }
    return;
  }
  throw;
}

Model::Type_ptr Parser::returns(){
  if(lexer->token() == Token::RETURNS){
    lexer->advance();
    Model::Type_ptr type = type();
    lexer->advance();
    ignore_eol();
    return type;
  }
  throw;
}

Model::CFA Parser::cfa(std::string name, std::vector<MiniMC::Model::Register_ptr>* params, const MiniMC::Model::RegisterDescr* regs) {
  Model::CFA cfg;
  std::unordered_map<std::string, MiniMC::Model::Location_ptr> locmap;

  if (lexer->token() == Token::CFA) {
    lexer->advance();
    ignore_eol();
    while (lexer->token() != Token::HASHHASH_SIGN) {
      edge(name, params, regs, &cfg, &locmap);
    }
    auto edges = cfg.getEdges();
    std::for_each(edges.begin(),edges.end(),[locmap,this](auto e){
      auto location = e->getTo();
      auto to = locmap.at(location->getInfo().getName());
      e->setTo(to);
    });
    return cfg;
  }
  throw;
}

void Parser::edge(std::string name, std::vector<MiniMC::Model::Register_ptr>* params, const MiniMC::Model::RegisterDescr* regs, Model::CFA* cfg, std::unordered_map<std::string, MiniMC::Model::Location_ptr>* locmap) {
  Model::InstructionStream instructionStream;
  MiniMC::Model::LocationInfoCreator locinfoc(name, regs);
  Model::Location_ptr to;
  auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();

  Model::Location_ptr from = location(cfg,locmap,source_loc,locinfoc);
  lexer->advance();
  ignore_eol();
  if (lexer->token() == Token::L_BRACKET) {
    while (lexer->token() != Token::R_BRACKET) {
      instruction(&instructionStream, params);
      if (lexer->token() == Token::R_ARROW) {
        if (locmap->contains (lexer->getValue())) {
          to = locmap->at(lexer->getValue());
        }
        else {
          auto location = cfg->makeLocation(locinfoc.make(lexer->getValue(), 0, *source_loc));
          locmap->at(lexer->getValue()) = location;
          to = location;
        }
        auto edge = cfg->makeEdge(from,to);
        edge->getInstructions() = instructionStream;
        instructionStream.clear();
      }
      lexer->advance();
      ignore_eol();
    }
  }

  throw;
}

Model::Location_ptr Parser::location(Model::CFA* cfg,std::unordered_map<std::string, MiniMC::Model::Location_ptr>* locmap, std::shared_ptr<MiniMC::Model::SourceInfo> source_loc,MiniMC::Model::LocationInfoCreator locinfoc){
  std::string index = identifier();
  lexer->advance();
  std::string name = identifier();
  if (locmap->contains (index)) {
    return locmap->at(index);
  }
  else {
    auto location = cfg->makeLocation (locinfoc.make(name, 0, *source_loc));
    locmap->at(index) = location;
    return location;
  }
  throw;
}

void Parser::instruction(Model::InstructionStream* instructionStream, std::vector<MiniMC::Model::Register_ptr>* params) {
  switch (lexer->token()) {
  case Token::INSTR_Skip:
    instructionStream->addInstruction(
        Model::createInstruction<Model::InstructionCode::Skip>({}));
    return;
  case Token::INSTR_Uniform:
    instructionStream->addInstruction(
        Model::createInstruction<Model::InstructionCode::Uniform>({}));
    return;
  case Token::INSTR_RetVoid:
    instructionStream->addInstruction(
        Model::createInstruction<Model::InstructionCode::RetVoid>({}));
    return;
  case Token::INSTR_Ret:
    lexer->advance();
    instructionStream->addInstruction(
        Model::createInstruction<Model::InstructionCode::Ret>(
            {.value = value()}));
  case Token::INSTR_Add:
  case Token::INSTR_Sub:
  case Token::INSTR_Mul:
  case Token::INSTR_UDiv:
  case Token::INSTR_SDiv:
  case Token::INSTR_Shl:
  case Token::INSTR_LShr:
  case Token::INSTR_AShr:
  case Token::INSTR_And:
  case Token::INSTR_Or:
  case Token::INSTR_Xor:
    instructionStream->addInstruction(predicates());
    return;
  case Token::INSTR_Not:
    lexer->advance();
    Value_ptr res = value();
    lexer->advance();
    Value_ptr op1 = value();
    instructionStream->addInstruction(Model::createInstruction<Model::InstructionCode::Not>({.res=res,.op1=op1}));
    return;
  case Token::INSTR_ICMP_SGT:
  case Token::INSTR_ICMP_UGT:
  case Token::INSTR_ICMP_SGE:
  case Token::INSTR_ICMP_UGE:
  case Token::INSTR_ICMP_SLT:
  case Token::INSTR_ICMP_ULT:
  case Token::INSTR_ICMP_SLE:
  case Token::INSTR_ICMP_ULE:
  case Token::INSTR_ICMP_EQ:
  case Token::INSTR_ICMP_NEQ:
    instructionStream->addInstruction(comperisons());
    return;
  case Token::INSTR_PtrAdd:
  case Token::INSTR_PtrEq:
    instructionStream->addInstruction(pointerops());
    return;
  case Token::INSTR_ExtractValue:
  case Token::INSTR_InsertValue:
    instructionStream->addInstruction(aggregateops());
    return;
  case Token::INSTR_Trunc:
  case Token::INSTR_ZExt:
  case Token::INSTR_SExt:
  case Token::INSTR_PtrToInt:
  case Token::INSTR_IntToPtr:
  case Token::INSTR_BitCast:
  case Token::INSTR_BoolZExt:
  case Token::INSTR_BoolSExt:
  case Token::INSTR_IntToBool:
    instructionStream->addInstruction(castops());
    return;
  case Token::INSTR_Assert:
  case Token::INSTR_Assume:
  case Token::INSTR_NegAssume:
    instructionStream->addInstruction(assumeasserts());
    return;
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
    instructionStream->addInstruction(predicates());
    return;
  default:
  }
  throw;
}

Model::Value_ptr Parser::value(){

}

Model::Register Parser::variable(){
  if(lexer->token() == Token::LESS_THAN){
    lexer->advance();
    Model::Register var =  Model::Register(identifier(), nullptr);
    lexer->advance();
    var.setType(type());
    return var;
  }
  throw;
}

std::string Parser::identifier() {
  if(lexer->token() == Token::IDENTIFIER){
    return lexer->getValue();
  }
  throw;
}

} // namespace Loader
} // namespace MiniMC