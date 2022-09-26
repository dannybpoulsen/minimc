#include "interpreter/parser.hpp"
#include "interpreter/lexer.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"
#include <stdlib.h>

namespace MiniMC {
namespace Interpreter {

void Parser::command() {
  switch (lexer->token()) {
  case Token::PRINT:
    print();
    break;
  case Token::EDGES:
    edges();
    break;
  case Token::JUMP:
    jump();
    break;
  case Token::STEP:
    step();
    break;
  case Token::BOOKMARK:
    bookmark();
    break;
  case Token::HELP:
    help();
    break;
  default:
    nonrecognizable();
  }
}

void Parser::print() {
  std::string value = get_id();

  if(value == ""){
    value = "current";
  }
  std::cout << (*statemap)[value];
}

void Parser::edges(){
  MiniMC::CPA::AnalysisState state = statemap->get("current");
  int n = 0;
  Algorithms::EdgeEnumerator enumerator{state};
  Algorithms::EnumResult res;

  // Print outgoing edges
  if (enumerator.getNext(res)) {
    n++;
    std::cout << n << ". " << std::endl;
    std::cout << *res.edge;
    while (enumerator.getNext(res)) {
      n++;
      std::cout << n << ". " << std::endl;
      std::cout << *res.edge;
    }
  }
}

void Parser::jump() {
  std::string value = get_id();

  if(value == ""){
    value = "bookmark";
  }
  if (statemap->contains(value)) {
    statemap->set("current", value);
  }
}


void Parser::step() {
  CPA::AnalysisState newstate;
  MiniMC::proc_t proc{0};
  statemap->set("prev","current");

  if (auto edge = get_edge(get_nr())) {
    if (transfer.Transfer(statemap->get("current"), edge, proc, newstate)) {
      statemap->set("current",newstate);
    };
    if (auto noinsedge = haveNoInstructionEdge(statemap->get("current"))) {
      if (transfer.Transfer(statemap->get("current"), noinsedge, proc, newstate)) {
        statemap->set("current",newstate);
      };
    }
  } else {
    std::cout << "Either does the current location have no outgoing edges "
                 << "or the chosen index is invalid. Show valid edges by"
                 << "entering: 'edges'" << std::endl;
  }
}

void Parser::bookmark() {
  std::string value = get_id();

  if(value == ""){
    value = "current";
  }
  statemap->set("bookmark",value);
}

void Parser::help(){
  std::for_each(lexer->commandMap.begin(), lexer->commandMap.end(), [](auto i){std::cout << i.first << std::endl;});
}

void Parser::nonrecognizable() {
  std::cout << "The command is not recognised. Try typing 'help', inorder to "
            << "get a list of recognised commands" << std::endl;
}

std::string Parser::get_id() {
  lexer->advance();
  switch (lexer->token()) {
  case Token::ID:
    return lexer->getTokenAsText();
  default:
    return "";
  }
}

int Parser::get_nr(){
  lexer->advance();
  switch (lexer->token()) {
  case Token::NUMBER:
    return stoi(lexer->getTokenAsText());
  default:
    return -1;
  }
}

Model::Edge* Parser::haveNoInstructionEdge(CPA::AnalysisState state) {
  Algorithms::EdgeEnumerator enumerator{state};
  Algorithms::EnumResult res;

  while (enumerator.getNext(res)) {
    if(!res.edge->getInstructions()) return res.edge;
  }
  return nullptr;
}

Model::Edge *Parser::get_edge(int i) {
  if (i == -1){
    return nullptr;
  }

  std::vector<Model::Edge *> edges;
  CPA::AnalysisState state = statemap->get("current");

  Algorithms::EdgeEnumerator enumerator{state};
  Algorithms::EnumResult res;

  // Print outgoing edges
  while (enumerator.getNext(res)) {
    edges.push_back(res.edge);
  }

  if (edges.size() == 0) {
    return nullptr;
  }

 if (edges[i - 1]) {
   return edges[i - 1];
 }
  return nullptr;
}

}// namespace Interpreter
} // namespace MiniMC