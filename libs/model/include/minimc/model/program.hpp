#ifndef _PROGRAM__
#define _PROGRAM__

#include "minimc/model/function.hpp"

namespace MiniMC {
  namespace Model {
      class Program  {
    public:
      Program()  : cpu_regs(RegType::CPU),
		   meta_regs(RegType::Meta),
		   pers_regs(RegType::Persistent)
      {}

      Program (const Program&) = delete ;
      Program (Program&&) = default;
      Function_ptr addFunction(MiniMC::Model::Symbol symbol,
			       const std::vector<Symbol>& params,
			       const Type_ptr retType,
			       CFA&& cfg,
			       bool varargs,
			       Frame frame
		) {
	functions.push_back(std::make_shared<Function>(functions.size(), symbol.getFullName(), params, retType,  std::move(cfg), *this,varargs,frame));
	symbol.setUserData (functions.back());
	function_map.emplace(symbol, functions.back());
        return functions.back();
      }

      Program& operator= (Program&&) = default;
      
      auto& getFunctions() const { return functions; }

      void addEntryPoint(MiniMC::Model::Symbol symb) {
        entrypoints.push_back(symb);
      }
      
      Function_ptr getFunction(MiniMC::Model::func_t id) const {
        return functions.at(id);
      }
      
      Function_ptr getFunction(const MiniMC::Model::Symbol& symb) const {
	if (std::holds_alternative<Function_ptr> (symb.getUserData ())) {
	  return std::get<Function_ptr> (symb.getUserData());
	}
	return nullptr;
      }
      
      bool functionExists(MiniMC::Model::func_t id) const {
	return static_cast<std::size_t> (id) < functions.size();
      }
      
      auto& getEntryPoints() const {
	return entrypoints;
      }
      
      HeapLayout& getHeapLayout () {return heaplayout;}
      const HeapLayout& getHeapLayout () const  {return heaplayout;}
      
      auto& getCPURegs () {return cpu_regs;}
      const auto& getCPURegs () const  {return cpu_regs;}
      auto& getPersistentRegs () {return pers_regs;}
      const auto& getPersistentRegs () const  {return pers_regs;}
      auto& getMetaRegs () {return meta_regs;}
      const auto& getMetaRegs () const  {return meta_regs;}
      
      
      auto& getRootFrame () {return frame;}
      auto& getRootFrame () const {return frame;}
    
    private:
      std::vector<Function_ptr> functions;
      std::vector<Symbol> entrypoints;
      SymbolTable<Function_ptr> function_map;
      HeapLayout heaplayout;
      MiniMC::Model::RegisterDescr cpu_regs;
      MiniMC::Model::RegisterDescr meta_regs;
      MiniMC::Model::RegisterDescr pers_regs;
      MiniMC::Model::Frame frame{"prgm"};
      
    };
    


  }
}

#endif
