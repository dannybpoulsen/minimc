#include <list>

#include "parser.hpp"
#include "model/cfg.hpp"
#include "model/builder.hpp"
#include "parserexception.hpp"
#include "model/instructions.hpp"
#include "support/feedback.hpp"


namespace MiniMC{
namespace Loaders{

void Parser::ignore_eol() {
  while(lexer->token() == Token::EOL_TOKEN) lexer->advance();
}

void Parser::run() {
  lexer = new Lexer{*in};
  try {
    functions();
    entrypoints();
    heap();
    initialiser();
  } catch (MMCParserException const& e) {
    Support::Messager messager;

        messager.message<MiniMC::Support::Severity::Error>(
            Support::Localiser("\n%4% \n"
                               "Pos: %1% \n"
                               "Line: %2% \n"
                               "With Token Value: Token::%5% : '%3%' \n")
            .format(e.getPos(), e.getLine(), e.getValue(), e.getMesg(), lexer->token()));
  }
  delete lexer;
}

void Parser::functions() {
  if (lexer->token() == Token::HASH_SIGN) {
    lexer->advance();
    if (lexer->token() == Token::FUNCTIONS) {
      lexer->advance();
      ignore_eol();
      while (lexer->token() != Token::HASH_SIGN) {
        function();
      }
      return;
    }
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected a functions-title on the from '# Functions'");
}

void Parser::entrypoints() {
  if (lexer->token() == Token::HASH_SIGN) {
    lexer->advance();
    if (lexer->token() == Token::ENTRYPOINTS) {
      lexer->advance();
      ignore_eol();
      while (lexer->token() != Token::HASH_SIGN) {
        prgm->addEntryPoint(identifier().getFullName());
        lexer->advance();
        ignore_eol();
      }
      return;
    }
  }
  throw MMCParserException(
      lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected a entrypoints-title on the from '# Entrypoints'");
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
              lexer->advance();
              ignore_eol();
              return;
            }
          }
        }
      }
    }
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected a heap-title on the form '# Heap'");
}

void Parser::initialiser() {
  Model::InstructionStream instructionStream;
  std::vector<MiniMC::Model::Register_ptr> variables;

  if (lexer->token() == Token::HASH_SIGN) {
    lexer->advance();
    if (lexer->token() == Token::INITIALISER) {
      lexer->advance();
      ignore_eol();
      while (lexer->token() != Token::EOF_TOKEN) {
        instruction(&instructionStream, variables);
        lexer->advance();
        ignore_eol();
      }
      prgm->setInitialiser(instructionStream);
      return;
    }
  }
  throw MMCParserException(
      lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected a initialiser-segment on the form '# Initialiser'");
}
void Parser::function() {
  std::vector<MiniMC::Model::Register_ptr> params;

  if (lexer->token() == Token::HASHHASH_SIGN) {
    lexer->advance();
    Model::Symbol name = identifier();
    lexer->advance();
    ignore_eol();
    auto registerDescr = registers(name);

    parameters(&params, registerDescr.get());
    auto ret = returns();
    auto cfg = cfa(name, registerDescr.get());
    prgm->addFunction(name.getFullName(), params, ret,
                      std::move(registerDescr), std::move(cfg));
    return;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected a function-declaration beginning with '## {function-name}'");
}

Model::RegisterDescr_uptr Parser::registers(Model::Symbol name){
  auto registerDescr = std::make_unique<MiniMC::Model::RegisterDescr> (name);

  if(lexer->token() == Token::REGISTERS){
    lexer->advance();
    ignore_eol();
    while(lexer->token() != Token::PARAMETERS){
      auto v = variable();
      registerDescr->addRegister(Model::Symbol(v.getSymbol().getName()), v.getType());
      lexer->advance();
      ignore_eol();
    }
    return registerDescr;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected a registers-declaration on the form: '.registers'");
}

void Parser::parameters(std::vector<MiniMC::Model::Register_ptr>* params, const MiniMC::Model::RegisterDescr* regs) {
  std::vector<Model::Register_ptr> variables = regs->getRegisters();

  if(lexer->token() == Token::PARAMETERS){
    lexer->advance();
    ignore_eol();
    while(lexer->token() != Token::RETURNS){
      std::for_each(variables.begin(),variables.end(), [params,regs,this](Model::Register_ptr reg){
        if(reg->getSymbol().getFullName() == identifier().getFullName()){
          params->push_back(reg);
        }
      });
      lexer->advance();
      ignore_eol();
    }
    return;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected a parameteres-declaration on the form: '.parameters'");
}

Model::Type_ptr Parser::returns(){
  if(lexer->token() == Token::RETURNS){
    lexer->advance();
    ignore_eol();
    Model::Type_ptr t = type();
    lexer->advance();
    ignore_eol();
    return t;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected a returns-declaration on the form: '.returns'");
}

Model::CFA Parser::cfa(Model::Symbol name, const MiniMC::Model::RegisterDescr* regs) {
  Model::CFA cfg;
  std::unordered_map<std::string, MiniMC::Model::Location_ptr> locmap;

  if (lexer->token() == Token::CFA) {
    lexer->advance();
    ignore_eol();
    while (lexer->token() != Token::HASHHASH_SIGN && lexer->token() != Token::HASH_SIGN) {
      edge(name, regs, &cfg, &locmap);
    }
    auto edges = cfg.getEdges();
    std::for_each(edges.begin(),edges.end(),[locmap,this](auto e){
      auto location = e->getTo();
      auto to = locmap.at(location->getInfo().name.getName());
      e->setTo(to);
    });
    cfg.setInitial(locmap.at("Initial"));
    return cfg;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected a cfa-declaration on the form: '.cfa'");
}

void Parser::edge(Model::Symbol name, const MiniMC::Model::RegisterDescr* regs, Model::CFA* cfg, std::unordered_map<std::string, MiniMC::Model::Location_ptr>* locmap) {
  Model::InstructionStream instructionStream;
  MiniMC::Model::LocationInfoCreator locinfoc(name, regs);
  Model::Location_ptr to;
  auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();

  Model::Location_ptr from = location(cfg,locmap,source_loc,locinfoc);
  lexer->advance();
  ignore_eol();
  while(lexer->token() == Token::AT_SIGN){
    lexer->advance();
    switch(lexer->token()){
    case Token::AssertViolated:
      from->getInfo().set<Model::Attributes::AssertViolated>();
      break;
    default:
      throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                                     "Does not recognise the annotation.");
    }
    lexer->advance();
    ignore_eol();
  }
  if (lexer->token() == Token::L_BRACKET) {
    lexer->advance();
    ignore_eol();
    while (lexer->token() != Token::R_BRACKET) {
      if (lexer->token() == Token::R_ARROW) {
        lexer->advance();
        Model::Symbol to_str = identifier();
        if (locmap->contains (to_str.getFullName())) {
          to = locmap->at(to_str.getFullName());
        }
        else {
          auto location = cfg->makeLocation(locinfoc.make(to_str.getFullName(), 0, *source_loc));
          (*locmap)[to_str.getFullName()] = location;
          to = location;
        }
        auto edge = cfg->makeEdge(from,to);
        edge->getInstructions() = instructionStream;
        instructionStream.clear();
      } else {
        instruction(&instructionStream, regs->getRegisters());
      }
      lexer->advance();
      ignore_eol();
    }
    lexer->advance();
    ignore_eol();
    return;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected an edge on the form: \n"
                           "[\n"
                           "{INSTRUCTION1}\n"
                           "{INSTRUCTION2}\n"
                           "{...}\n"
                           "]'");
}

Model::Location_ptr Parser::location(Model::CFA* cfg,std::unordered_map<std::string, MiniMC::Model::Location_ptr>* locmap, std::shared_ptr<MiniMC::Model::SourceInfo> source_loc,MiniMC::Model::LocationInfoCreator locinfoc) {
  try {
    Model::Symbol index = identifier();
    lexer->advance();
    Model::Symbol  name = identifier();

    if (locmap->contains(index.getFullName())) {
      return locmap->at(index.getFullName());
    } else {
      auto location = cfg->makeLocation(locinfoc.make(name.getFullName(), 0, *source_loc));
      if (locmap->size() == 0) {
        (*locmap)["Initial"] = location;
      }
      (*locmap)[index.getFullName()] = location;
      return location;
    }
  } catch (MMCParserException const& e){
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a location, which is "
        "on the form: {INDEX_NAME} {LOCATION_NAME}");
  }
}

void Parser::instruction(Model::InstructionStream* instructionStream, std::vector<MiniMC::Model::Register_ptr> variables) {
  try {
    switch (lexer->token()) {
    case Token::Skip:
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Skip>({}));
      return;
    case Token::Uniform:
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Uniform>({}));
      return;
    case Token::RetVoid:
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::RetVoid>({}));
      return;
    case Token::Ret:
      lexer->advance();
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Ret>(
              {.value = value(variables)}));
      return;
#define X(OP) \
  case Token::OP: \
    lexer->advance(); \
    instructionStream->addInstruction( \
        Model::createInstruction<Model::InstructionCode::OP>( \
            {.expr = value(variables)})); \
    return;
      ASSUMEASSERTS
#undef X
    case Token::Store: {
      lexer->advance();
      Model::Value_ptr addr = value(variables);
      lexer->advance();
      Model::Value_ptr storee = value(variables);
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Store>(
              {.addr = addr, .storee = storee}));
      return;
    }
    case Token::Call: {
      lexer->advance();
      Model::Value_ptr function = value(variables);
      lexer->advance();
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Call>(
              {.res = nullptr,
               .function = function,
               .params = value_list(variables)}));
      return;
    }
#define X(OP) \
    case Token::OP: \
      PREDICATES
#undef X
      instructionStream->addInstruction(predicates(variables));
      return;
    case Token::LESS_THAN:
      instructionStream->addInstruction(instruction_eq(variables));
      return;
    default:
      throw MMCParserException(
          lexer->getLine(), lexer->getPos(),lexer->getValue(),
          "The first token of the instruction is not recognised");
    }
  } catch (MMCParserException const& e) {
    Support::Messager messager;
    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "The parser doesn't recognise this instruction. The instruction "
        "might not be supported. Otherwise you might want to check the "
        "structure of the instruction again");
  }
}

Model::Instruction Parser::instruction_eq(const std::vector<MiniMC::Model::Register_ptr> variables) {
 try {
   if (lexer->token() == Token::LESS_THAN) {
     Model::Value_ptr res = value(variables);
     lexer->advance();

     if (lexer->token() == Token::EQUAL_SIGN) {
       lexer->advance();
       switch (lexer->token()) {
       case Token::PtrAdd: {
         lexer->advance();
         Model::Value_ptr ptr = value(variables);
         lexer->advance();
         Model::Value_ptr skipsize = value(variables);
         lexer->advance();
         Model::Value_ptr nbSkips = value(variables);
         return Model::createInstruction<Model::InstructionCode::PtrAdd>(
             {.res = res,
              .ptr = ptr,
              .skipsize = skipsize,
              .nbSkips = nbSkips});
       }
#define X(OP) \
        case Token::OP:
    TACOPS
#undef X
         return tacops(variables, res);
#define X(OP) \
        case Token::OP:
    COMPARISONS
#undef X
         return comparison(variables, res);
       case Token::Not:
         lexer->advance();
         return Model::createInstruction<Model::InstructionCode::Not>(
             {.res = res, .op1 = value(variables)});
#define X(OP) \
        case Token::OP:
    CASTOPS
#undef X
         return castops(variables, res);
       case Token::NonDet:
         return nondet(variables, res);
       case Token::Call: {
         lexer->advance();
         Model::Value_ptr function = value(variables);
         lexer->advance();
         return Model::createInstruction<Model::InstructionCode::Call>(
             {.res = res,
              .function = function,
              .params = value_list(variables)});
       }
       case Token::ExtractValue:
         return extract(variables, res);
       case Token::InsertValue:
         return insert(variables, res);
       case Token::Load:
         return load(variables, res);
       case Token::LESS_THAN:
         return Model::createInstruction<Model::InstructionCode::Assign>(
             {.res = res, .op1 = value(variables)});
       default:
         throw MMCParserException(
             lexer->getLine(), lexer->getPos(), lexer->getValue(),
             "The token does not match any know/supported instruction.");
       }
     }
   }
 } catch (MMCParserException const& e){
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),e.getMesg());
 }
}


Model::Instruction Parser::predicates(const std::vector<MiniMC::Model::Register_ptr> variables) {
  try {
    Token token = lexer->token();
    lexer->advance();
    Model::Value_ptr op1 = value(variables);
    lexer->advance();
    Model::Value_ptr op2 = value(variables);
    switch (token) {
#define X(OP) \
    case Token::OP: \
    return Model::createInstruction<Model::InstructionCode::OP>( \
          {.op1 = op1, .op2 = op2});
    PREDICATES
#undef X
    default:
      __builtin_unreachable();
    }
  } catch (MMCParserException const& e){
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a predicate instruction on the form: "
        "{PRED_INSTRUCTION} {OP1} {OP2}");
  }
}

Model::Instruction Parser::tacops(const std::vector<MiniMC::Model::Register_ptr> variables, Model::Value_ptr res){
  try {
    Token token = lexer->token();
    lexer->advance();
    Model::Value_ptr op1 = value(variables);
    lexer->advance();
    Model::Value_ptr op2 = value(variables);

    switch (token) {
#define X(OP)               \
    case Token::OP:         \
      return Model::createInstruction<Model::InstructionCode::OP>( \
          {.res = res, .op1 = op1, .op2 = op2});
      TACOPS
#undef X
    default:
      __builtin_unreachable();
    }
  } catch (MMCParserException const& e){
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a TAC-instruction expression on the form: "
        "{RES} = {INSTRUCTION} {OP1} {OP2}");
  }
}

Model::Instruction Parser::comparison(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res) {
  try {
    Token token = lexer->token();
    lexer->advance();
    Model::Value_ptr op1 = value(variables);
    lexer->advance();
    Model::Value_ptr op2 = value(variables);
    switch (token) {
#define X(OP)               \
      case Token::OP:       \
        return Model::createInstruction<Model::InstructionCode::OP>( \
            {.res = res, .op1 = op1, .op2 = op2});
      COMPARISONS
#undef X
    default:
      __builtin_unreachable();
    }
  } catch (MMCParserException const& e){
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a comparison instruction on the form: "
        "{RES} = {INSTRUCTION} {OP1} {OP2}");
  }
}

Model::Instruction Parser::castops(const std::vector<MiniMC::Model::Register_ptr> variables, Model::Value_ptr res) {
  try {
    Token token = lexer->token();
    lexer->advance();
    Model::Type_ptr t = type();
    lexer->advance();
    Model::Value_ptr op1 = value(variables);
    switch (token) {
#define X(OP)               \
      case Token::OP: \
        return Model::createInstruction<Model::InstructionCode::OP>( \
          {.res = res, .op1 = op1});
      CASTOPS
#undef X
    default:
      __builtin_unreachable();
    }
  } catch (MMCParserException const& e) {
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(lexer->getLine(), lexer->getPos(),
                             lexer->getValue(),
                             "Expected a comparison instruction on the form: "
                             "{RES} = {INSTRUCTION} {TYPE} {OP1}");
  }
}

Model::Instruction Parser::nondet(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res){
  try {
    if (lexer->token() == Token::NonDet) {
      lexer->advance();
      Model::Type_ptr t = type();
      lexer->advance();
      Model::Value_ptr min = value(variables);
      lexer->advance();
      Model::Value_ptr max = value(variables);
      return Model::createInstruction<Model::InstructionCode::NonDet>(
          {.res = res, .min = min, .max = max});
    }
    __builtin_unreachable();
  } catch(MMCParserException const& e) {
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(lexer->getLine(), lexer->getPos(),
                             lexer->getValue(),
                             "Expected a NonDet instruction on the form: "
                             "{RES} = {INSTRUCTION} {TYPE} {MIN} {MAX}");
  }
}

Model::Instruction Parser::extract(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res){
  try {
    if (lexer->token() == Token::ExtractValue) {
      lexer->advance();
      Model::Type_ptr t = type();
      lexer->advance();
      Model::Value_ptr aggregate = value(variables);
      lexer->advance();
      Model::Value_ptr offset = value(variables);
      return Model::createInstruction<Model::InstructionCode::ExtractValue>(
          {.res = res, .aggregate = aggregate, .offset = offset});
    }
    __builtin_unreachable();
  }  catch(MMCParserException const& e) {
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
  }
  throw MMCParserException(
      lexer->getLine(), lexer->getPos(),lexer->getValue(),
      "Expected a ExtractValue instruction on the form: "
      "{RES} = ExtractValue {TYPE} {AGGREGATE} {OFFSET}");

}

Model::Instruction Parser::insert(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res){
  try {
    if (lexer->token() == Token::InsertValue) {
      lexer->advance();
      Model::Type_ptr t = type();
      lexer->advance();
      Model::Value_ptr aggregate = value(variables);
      lexer->advance();
      Model::Value_ptr offset = value(variables);
      lexer->advance();
      Model::Value_ptr insertee = value(variables);
      return Model::createInstruction<Model::InstructionCode::InsertValue>(
          {.res = res,
           .aggregate = aggregate,
           .offset = offset,
           .insertee = insertee});
    }
    __builtin_unreachable();
  }  catch(MMCParserException const& e) {
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a InsertValue instruction on the form: "
        "{RES} = InsertValue {TYPE} {AGGREGATE} {OFFSET} {INSERTEE}");
  }
}

Model::Instruction Parser::load(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res){
  try {
    if (lexer->token() == Token::Load) {
      lexer->advance();
      Model::Type_ptr t = type();
      lexer->advance();
      Model::Value_ptr addr = value(variables);
      return Model::createInstruction<Model::InstructionCode::Load>(
          {.res = res, .addr = addr});
    }
    __builtin_unreachable();
  } catch (MMCParserException const& e){

    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a InsertValue instruction on the form: "
        "{RES} = Load {TYPE} {ADDRESS}");
  }
}


Model::Type_ptr Parser::type(){
  switch(lexer->token()){
  case Token::Void:
    return tfac.makeVoidType();
  case Token::Bool:
    return tfac.makeBoolType();
  case Token::Int8:
    return tfac.makeIntegerType(8);
  case Token::Int16:
    return tfac.makeIntegerType(16);
  case Token::Int32:
    return tfac.makeIntegerType(32);
  case Token::Int64:
    return tfac.makeIntegerType(64);
  case Token::Float:
    return tfac.makeFloatType();
  case Token::Double:
    return tfac.makeDoubleType();
  case Token::Pointer:
    return tfac.makePointerType();
  case Token::Struct:
    lexer->advance();
    return tfac.makeStructType(integer());
  case Token::Array:
    lexer->advance();
    return tfac.makeArrayType(integer());
  case Token::Aggr8:
    return tfac.makeArrayType(8);
  default:
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a type.");
  }
}

std::vector<Model::Value_ptr> Parser::value_list(std::vector<MiniMC::Model::Register_ptr> variables){
  std::vector<Model::Value_ptr> list;
  while(lexer->token() != Token::EOL_TOKEN){
    list.push_back(value(variables));
    lexer->advance();
  }
  return list;
}


Model::Value_ptr Parser::value(std::vector<MiniMC::Model::Register_ptr> variables) {
  Model::Value_ptr ret;
  unsigned long value;
  std::vector<Model::Constant_ptr> blob;
  Token token;
  bool loop_end = 1;

  if (lexer->token() == Token::LESS_THAN) {
    lexer->advance();
    switch (lexer->token()) {
    case Token::IDENTIFIER: {
      Model::Symbol var = identifier();
      lexer->advance();
      Model::Type_ptr t = type();
      lexer->advance();
      if (lexer->token() != Token::GREATER_THAN)
        throw MMCParserException(lexer->getLine(), lexer->getPos(),
                                 lexer->getValue(), "");
      std::for_each(
          variables.begin(), variables.end(),
          [&ret, &t, &var, &loop_end, this](auto rptr) {
            if (rptr->getName() == var.getFullName() &&
                t->getTypeID() == rptr->getType()->getTypeID()) {
              if (!ret) {
                ret = rptr;
                loop_end = 0;
              } else
                throw MMCParserException(
                    lexer->getLine(), lexer->getPos(), lexer->getValue(),
                    "The identifier describes a NULL-pointer");
            }
          });
      if (loop_end) throw MMCParserException(
            lexer->getLine(), lexer->getPos(), lexer->getValue(),
            "Looped through all variables with out finding a match.");
      return ret;
    }
    case Token::DOLLAR_SIGN: {
      lexer->advance();
      blob = integer_list();
      if (lexer->token() != Token::DOLLAR_SIGN)
        throw MMCParserException(lexer->getLine(), lexer->getPos(),
                                 lexer->getValue(),
                                 "Expected a dollar-sign to enclose the list.");
      break;
    }
    case Token::HEAP_Pointer:
    case Token::FUNCTION_Pointer: {
      token = lexer->token();
      lexer->advance();
      unsigned long base = integer();
      lexer->advance();
      unsigned long offset = integer();
      lexer->advance();
      if (lexer->token() != Token::R_PARA)
        throw MMCParserException(lexer->getLine(), lexer->getPos(),
                                 lexer->getValue(),
                                 "Expected a right parenthesis.");
      lexer->advance();
      type();
      lexer->advance();
      if (lexer->token() != Token::GREATER_THAN)
        throw MMCParserException(lexer->getLine(), lexer->getPos(),
                                 lexer->getValue(), "Expected a greater than.");
      if (token == Token::HEAP_Pointer)
        return cfac.makeHeapPointer(base);
      if (token == Token::FUNCTION_Pointer)
        return cfac.makeFunctionPointer(base);
      break;
    }
    case Token::DIGIT:
    case Token::HEX:
      value = integer();
      break;
    default:
      throw MMCParserException(
          lexer->getLine(), lexer->getPos(), lexer->getValue(),
          "The token is not recognised as a part of a value. This might be a "
          "result to using a keyword as identifier.");
    }
    lexer->advance();
    Model::Type_ptr t = type();
    lexer->advance();
    if (lexer->token() != Token::GREATER_THAN)
      throw MMCParserException(
          lexer->getLine(), lexer->getPos(), lexer->getValue(),
          "Expected a greater than.");
    switch (t->getTypeID()) {
    case Model::TypeID::Bool:
    case Model::TypeID::I8:
    case Model::TypeID::I16:
    case Model::TypeID::I32:
    case Model::TypeID::I64:
      return cfac.makeIntegerConstant(value, t->getTypeID());
    case Model::TypeID::Struct:
      return cfac.makeAggregateConstant(blob, false);
    case Model::TypeID::Array:
      return cfac.makeAggregateConstant(blob, true);
    default:
      throw MMCParserException(
          lexer->getLine(), lexer->getPos(), lexer->getValue(),
          "The type is not supported in the Parser right now");
    }
  }
  throw MMCParserException(
      lexer->getLine(),lexer->getPos(), lexer->getValue(),
      "Expected a value, which always starts with a less than.");
}

Model::Register Parser::variable(){
  try {
    if (lexer->token() == Token::LESS_THAN) {
      lexer->advance();
      Model::Register var = Model::Register(identifier(), nullptr);
      lexer->advance();
      var.setType(type());
      lexer->advance();
      if (lexer->token() == Token::GREATER_THAN) {
        return var;
      }
    }
  } catch (MMCParserException const& e){

    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a variable. A variable is of the form: "
        "<{IDENTIFIER} {TYPE}>");
  }
}

Model::Symbol Parser::identifier() {

  if(lexer->token() == Token::IDENTIFIER){
    std::string val = lexer->getValue();
    std::list<std::string> tokens;
    std::string intermediate;
    std::stringstream ss(val);

    while(getline(ss, intermediate, ':'))
    {
      tokens.push_back(intermediate);
    }
    return Model::Symbol(tokens);

  }
  throw MMCParserException(
      lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected a identifier. This can be caused of using a keyword as "
      "identifier which is not allowed in MiniMC.");
}

unsigned long Parser::integer(){
  std::string value = lexer->getValue();

  if(lexer->token() == Token::DIGIT){
    return std::stoul(value);
  } else if(lexer->token() == Token::HEX){
    return std::stoul(value, 0,16);
  }
  throw MMCParserException(
      lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected either a Hex(16) or Decimal(10) value.");
}

std::vector<Model::Constant_ptr> Parser::integer_list(){
  std::vector<Model::Constant_ptr> blob;
  while(lexer->token() == Token::HEX || lexer->token() == Token::DIGIT){
    blob.push_back(std::static_pointer_cast<MiniMC::Model::Constant>(cfac.makeIntegerConstant(integer(),Model::TypeID::I8)));
    lexer->advance();
  }
  return blob;
}

} // namespace Loader
} // namespace MiniMC