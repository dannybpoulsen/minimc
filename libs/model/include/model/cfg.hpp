#ifndef _CFG__
#define _CFG__

#include <memory>
#include <vector>

namespace MiniMC {
  namespace Model {
	class Edge;
	class Location {
	public:
	  Location (const std::string& n) : name(n) {}
	  auto& getEdges () const {return edges;}
	  void addEdge (Edge* e) {edges.push_back(e);}
	  
	private:
	  std::vector<Edge*> edges;
	  std::string name;
	};

	class Instruction;
	class Edge {
	public:
	  Edge (Location* from,Location* to) :from(from),to(to) {}
	  auto& getInstructions () const {return instructions;}
	  auto getFrom () const {return from;}
	  auto getTo () const {return to;}
	private:
	  std::vector<Instruction*> instructions;
	  Location* from;
	  Location* to;
	};
	
  }
}

#endif

