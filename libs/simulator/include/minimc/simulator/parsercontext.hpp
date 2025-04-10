#ifndef _PARSER_CONTEXT__
#define _PARSER_CONTEXT__

#include "minimc/model/symbol.hpp"

#include <functional>

    namespace MiniMC {
      namespace Simulator {
	class Scanner;
	class CommandBuilder;
      }
    }


namespace MiniMC {
  namespace Simulator {

    using FrameGetter = std::function<MiniMC::Model::Frame(std::size_t)>;
    
    struct  ParserContext {
      ParserContext (MiniMC::Simulator::CommandBuilder& b,
		     MiniMC::Model::ExpressionBuilder& eb,
		     MiniMC::IO::ostream& os,
		     FrameGetter getter
		     ) : builder(b),ebuilder(eb),os(os),getter(getter) {}
      MiniMC::Simulator::CommandBuilder& builder;
      MiniMC::Model::ExpressionBuilder& ebuilder;
      MiniMC::IO::ostream& os;
      int proc{0};
      FrameGetter getter;
    };
  }
}


#endif
