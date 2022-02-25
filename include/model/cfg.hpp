
/**
 * @file   cfg.hpp
 * @date   Sun Apr 19 20:16:17 2020
 * 
 * 
 */
#ifndef _CFG__
#define _CFG__

#include <algorithm>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "model/edge.hpp"
#include "model/instructions.hpp"
#include "model/location.hpp"
#include "model/variables.hpp"
#include "model/heaplayout.hpp"
#include "support/types.hpp"
#include "support/workinglist.hpp"

namespace MiniMC {
  namespace Model {

    class Function;
    using Function_ptr = std::shared_ptr<Function>;
    using Function_wptr = std::weak_ptr<Function>;

    /**
	 *
	 * Representation of an Control Flow Automaton. 
	 * The CFA is responsible for creating( and deleting) edges and
	 * locations of a function. It will also make sure that  the
	 * incoming/outgoing edges of locations are properly update when
	 * deleting edges. This 
	 *
	 */
    class CFA : public std::enable_shared_from_this<CFA> {
    protected:
      friend class Program;
      CFA(const Program_ptr& prgm) : prgm(prgm) {}
      void setFunction(const Function_ptr& func) { function = func; }

    public:
      Location_ptr makeLocation(const LocationInfo& info) {
        locations.emplace_back(new Location(info, locations.size(), this->shared_from_this()));
        return locations.back();
      }

      /** 
	   * Make a new edge 
	   *
	   * @param from source of the edge
	   * @param to target of the edge
	   *
	   * @return 
	   */
      Edge_ptr makeEdge(Location_ptr from, Location_ptr to) {
        edges.emplace_back(new Edge(from, to, prgm));
        to->addIncomingEdge(edges.back());
        from->addEdge(edges.back());
        return edges.back();
      }

      Location_ptr getInitialLocation() {
        assert(initial);
        return initial;
      }

      void setInitial(Location_ptr loc) {
        initial = loc;
      }

      /** 
	   * Delete \p edge from this CFG. Update also the
	   * incoming/outgoing edges of the target/source of \p edge. 
	   *
	   * @param edge The edge to delete
	   */
      void deleteEdge(const Edge_ptr& edge) {
        edge->getFrom()->removeEdge(edge);
        edge->getTo()->removeIncomingEdge(edge);

        auto it = std::find(edges.begin(), edges.end(), edge);
        if (it != edges.end()) {
          edges.erase(it);
        }
      }

      void deleteLocation(const Location_ptr& location) {
        MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
        auto insert = wlist.inserter();
        std::for_each(location->ebegin(), location->eend(), [&](const auto& e) { insert = e; });
        std::for_each(location->iebegin(), location->ieend(), [&](const auto& e) { insert = e; });

        std::for_each(wlist.begin(), wlist.end(), [&](const auto& e) { this->deleteEdge(e); });

        auto it = std::find(locations.begin(), locations.end(), location);
        if (it != locations.end()) {
          locations.erase(it);
        }
      }

      auto& getLocations() const { return locations; }
      auto& getLocations() { return locations; }
      auto& getEdges() { return edges; }

  
      Function_ptr getFunction() const {
        return function.lock();
      }
      
    private:
      std::vector<Location_ptr> locations;
      std::vector<Edge_ptr> edges;
      Location_ptr initial = nullptr;
      Program_wptr prgm;
      Function_wptr function;
    };

    using CFA_ptr = std::shared_ptr<CFA>;

    class Function : public std::enable_shared_from_this<Function> {
    public:
      Function(MiniMC::func_t id,
               const std::string& name,
               const std::vector<Variable_ptr>& params,
               const Type_ptr rtype,
               const VariableStackDescr_ptr& variableStackDescr,
               const CFA_ptr cfg,
               const Program_ptr& prgm) : name(name),
                                          parameters(params),
                                          variableStackDescr(variableStackDescr),
                                          cfg(cfg),
                                          id(id),
                                          prgm(prgm),
					  retType(rtype)
                                          
      {
      }

      auto& getName() const { return name; }
      auto& getParameters() const { return parameters; }
      auto& getVariableStackDescr() const { return variableStackDescr; }
      auto& getVariableStackDescr() { return variableStackDescr; }
      auto& getCFG() const { return cfg; }
      auto& getID() const { return id; }
      auto& getReturnType() { return retType; }
      Program_ptr getPrgm() const { return prgm.lock(); }

    private:
      std::string name;
      std::vector<Variable_ptr> parameters;
      VariableStackDescr_ptr variableStackDescr;
      CFA_ptr cfg;
      MiniMC::func_t id;
      Program_wptr prgm;
      Type_ptr retType;
    };

    class Program : public std::enable_shared_from_this<Program> {
    public:
      Program(const MiniMC::Model::TypeFactory_ptr& tfact,
              const MiniMC::Model::ConstantFactory_ptr& cfact) : cfact(cfact), tfact(tfact) {
      }

      Function_ptr addFunction(const std::string& name,
			       const std::vector<Variable_ptr>& params,
			       const Type_ptr retType,
			       const VariableStackDescr_ptr& variableStackDescr,
			       const CFA_ptr cfg) {
        functions.push_back(std::make_shared<Function>(functions.size(), name, params, retType, variableStackDescr, cfg, shared_from_this()));
        function_map.insert(std::make_pair(name, functions.back()));
        cfg->setFunction(functions.back());
        return functions.back();
      }
      
      CFA_ptr makeCFG() {
        return std::shared_ptr<CFA>(new CFA(this->shared_from_this()));
      }
      
      auto& getFunctions() const { return functions; }
      
      void addEntryPoint(const std::string& str) {
        auto function = getFunction(str);
        entrypoints.push_back(function);
      }

      Function_ptr getFunction(MiniMC::func_t id) const {
        return functions.at(id);
      }

      Function_ptr getFunction(const std::string& name) {
        if (function_map.count(name)) {
          return function_map.at(name);
        }

        throw MiniMC::Support::FunctionDoesNotExist(name);
      }

      bool functionExists(MiniMC::func_t id) const {
        return static_cast<std::size_t> (id) < functions.size();
      }

      auto& getEntryPoints() const { return entrypoints; }

      bool hasEntryPoints() const { return entrypoints.size(); }
      VariableStackDescr_ptr makeVariableStack(const std::string& name) {
        return std::make_shared<VariableStackDescr>(name);
      }

      auto& getConstantFactory() { return cfact; }
      auto& getTypeFactory() { return tfact; }
      
      const auto& getInitialisation() const { return initialiser; }
      void setInitialiser(const InstructionStream& instr) { initialiser = instr; }

      HeapLayout& getHeapLayout () {return heaplayout;}
      const HeapLayout& getHeapLayout () const  {return heaplayout;}
      
    private:
      std::vector<Function_ptr> functions;
      std::vector<Function_ptr> entrypoints;
      MiniMC::Model::ConstantFactory_ptr cfact;
      MiniMC::Model::TypeFactory_ptr tfact;
      InstructionStream initialiser;
      std::unordered_map<std::string, Function_ptr> function_map;
      HeapLayout heaplayout;
    };

    Function_ptr createEntryPoint(Program_ptr& program, Function_ptr function);
  } // namespace Model
} // namespace MiniMC

namespace std {
  template <>
  struct hash<MiniMC::Model::Location> {
    std::size_t operator()(const MiniMC::Model::Location& loc) { return reinterpret_cast<size_t>(&loc); }
  };
} // namespace std

#endif
