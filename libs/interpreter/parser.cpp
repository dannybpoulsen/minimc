#include "interpreter/parser.hpp"
#include "interpreter/lexer.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"

namespace MiniMC {
namespace Interpreter {

void Parser::command() {
  switch (lexer->token()) {
  case Token::PRINT:
    print();
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

  if (auto noinsedge = haveNoInstructionEdge((*statemap)["current"])) {
    if (transfer.Transfer((*statemap)["current"], noinsedge, proc, newstate)) {
      statemap->set("current",newstate);
    };
  }

  if (auto edge = promptForEdge((*statemap)["current"])) {
    if (transfer.Transfer((*statemap)["current"], edge, proc, newstate)) {
      statemap->set("current",newstate);
    };
  } else {
    std::cout << "Current location have no outgoing edges" << std::endl;
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

Model::Edge* Parser::haveNoInstructionEdge(CPA::AnalysisState state) {
  Algorithms::EdgeEnumerator enumerator{state};
  Algorithms::EnumResult res;

  while (enumerator.getNext(res)) {
    if(!res.edge->getInstructions()) return res.edge;
  }
  return nullptr;
}

Model::Edge *Parser::promptForEdge(CPA::AnalysisState state) {
  int index = -1;
  int n = 0;

  std::vector<Model::Edge *> edges;

  Algorithms::EdgeEnumerator enumerator{state};
  Algorithms::EnumResult res;

  // Print outgoing edges
  if (enumerator.getNext(res)) {
    std::cout << "Following edges can be picked" << std::endl;
    edges.push_back(res.edge);
    n++;
    std::cout << n << ". " << std::endl;
    std::cout << *res.edge;
    while (enumerator.getNext(res)) {
      edges.push_back(res.edge);
      n++;
      std::cout << n << ". " << std::endl;
      std::cout << *res.edge;
    }
  }

  if (edges.size() == 0) {
    return nullptr;
  }

  // Choose edge by index
  while (index <= 0) {
    std::cin >> index;
    if (edges[index - 1]) {
      return edges[index - 1];
    }
    std::cout
        << "The chosen index is not possible, please choose a index under "
        << edges.size();
  }
  return nullptr;
}

}// namespace Interpreter
} // namespace MiniMC