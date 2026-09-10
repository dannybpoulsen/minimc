#ifndef _FUNCTION__
#define _FUNCTION__

#include "minimc/model/cfg.hpp"

namespace MiniMC {
  namespace Model {

    
     
    class Program;
    using Program_ptr = std::shared_ptr<Program>;

    
    class Function  {
    public:
      Function(MiniMC::Model::func_t id,
               const std::string& name,
               const std::vector<MiniMC::Model::Symbol>& params,
               const Type_ptr rtype,
               CFA&& cfa,
               Program& prgm,
               bool varargs,
               MiniMC::Model::Frame frame) : name(name),
					     parameters(params),
					     cfa(std::move(cfa)),
                                             id(id),
                                             prgm(prgm),
                                             retType(rtype),
                                             varargs(varargs),
                                             frame(frame)

      {
	
      }
      Function (const Function&) = delete;
      Function (Function&&) = default;
      /*auto& getSymbol() { return name; }
	auto& getSymbol() const { return name; }*/
      const auto& getName() const {return name;} 
      auto getParameters() const { return parameters ;}
      auto& getCFA() const { return cfa; }
      auto& getCFA()  { return cfa; }
      
      auto& getID() const { return id; }
      auto& getReturnType() const { return retType; }
      auto& getFrame () {return frame;}
      auto& getFrame () const {return frame;}
      
      Program& getPrgm() const { return prgm; }
      auto isVarArgs () const {return varargs;}
      auto function_ptr () const {return MiniMC::Model::Pointer(MiniMC::Model::pointer64_t::makeFunctionPointer(id));}
    private:
      std::string  name;
      std::vector<Symbol> parameters;
      CFA cfa;
      MiniMC::Model::func_t id;
      Program& prgm;
      Type_ptr retType;
      bool varargs;
      MiniMC::Model::Frame frame;
    };


    
        
  
    
  } // namespace Model
}




#endif
