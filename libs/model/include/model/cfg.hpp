#ifndef _CFG__
#define _CFG__

#include <memory>
#include <vector>
#include <gsl/pointers>

#include "model/instructions.hpp"

namespace MiniMC {
  namespace Model {
    class Edge;    
    class Location {
    public:
      Location (const std::string& n) : name(n) {}
      auto& getEdges () const {return edges;}
      void addEdge (gsl::not_null<Edge*> e) {edges.push_back(e);}
      
    private:
      std::vector<gsl::not_null<Edge*>> edges;
      std::string name;
    };
    
    class Instruction;
    class Edge {
    public:
      Edge (gsl::not_null<Location*> from, gsl::not_null<Location*> to, const std::vector<Instruction>& inst) : instructions(inst),from(from),to(to) {}
      auto& getInstructions () const {return instructions;}
      auto getFrom () const {return from;}
      auto getTo () const {return to;}
    private:
      std::vector<Instruction> instructions;
      gsl::not_null<Location*> from;
      gsl::not_null<Location*> to;
    };

    class CFG {
    public:
      gsl::not_null<Location*> makeLocation (const std::string& name) {
	locations.emplace_back (new Location (name));
	return locations.back().get();
      }

      gsl::not_null<Edge*> makeEdge (gsl::not_null<Location*> from, gsl::not_null<Location*> to, const std::vector<Instruction>& inst) {
	edges.emplace_back (new Edge (from,to,inst));
	return edges.back().get();
      }
      
      
    private:
      std::vector<std::unique_ptr<Location> > locations;
      std::vector<std::unique_ptr<Edge> > edges;
    };
    
  }
}

#endif

