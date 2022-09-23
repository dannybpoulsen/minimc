#include "interpreter/parser.hpp"
#include "interpreter/lexer.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"

namespace MiniMC {
namespace Interpreter {

void Parser::print() {
  std::string value = "current";
  print(value);
}
void Parser::print(std::string value) { std::cout << (*statemap)[value]; }

void Parser::jump(std::string value) {
  if (statemap->contains(value)) {
    (*statemap)["current"] = (*statemap)[value];
  }
}

void Parser::jump() {
  std::string value = "bookmark";
  jump(value);
}
void Parser::step() {
  lexer->advance();
  if (lexer->token() != Token::EOL) {
    return;
  }
  CPA::AnalysisState newstate;
  MiniMC::proc_t proc{0};
  (*statemap)["prev"] = (*statemap)["current"];

  if (auto noinsedge = haveNoInstructionEdge((*statemap)["current"])) {
    if (transfer.Transfer((*statemap)["current"], noinsedge, proc, newstate)) {
      (*statemap)["current"] = newstate;
    };
  }

  if (auto edge = promptForEdge((*statemap)["current"])) {
    if (transfer.Transfer((*statemap)["current"], edge, proc, newstate)) {
      (*statemap)["current"] = newstate;
    };
  } else {
    std::cout << "Current location have no outgoing edges" << std::endl;
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
void Parser::primary() {
  switch (lexer->token()) {
  case Token::PRINT:
    print(get_id());
    break;
  case Token::JUMP:
    jump(get_id());
    break;
  case Token::STEP:
    step();
    break;
  case Token::BOOKMARK:
    bookmark(get_id());
  default:
    return;
  }
}

void Parser::bookmark(std::string value) {
  (*statemap)["bookmark"] = (*statemap)[value];
}

void Parser::bookmark() {
  std::string value = "current";
  bookmark(value);
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

}// namespace Interpreter
} // namespace MiniMC