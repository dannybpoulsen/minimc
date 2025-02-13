#include <type_traits>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"


#include "minimc/model/cfg.hpp"
#include "minimc/model/builder.hpp"
#include "minimc/model/types.hpp"
#include "minimc/model/valuevisitor.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/concrete.hpp"
#include "minimc/model/modifications/modifications.hpp"
#include "minimc/algorithms/reachability.hpp"
#include "minimc/loaders/loader.hpp"
#include "minimc/support/overload.hpp"

TEST_CASE("Add") {
  MiniMC::Model::Program prgm;
  
  MiniMC::Model::RegisterDescr descr;
  
  
  auto type = MiniMC::Model::I8Type::get();//tfac->makeIntegerType (8);
  auto symb = prgm.getRootFrame ().makeFresh ();
  auto symbol_value = MiniMC::Model::SymbolicConstant::make(symb);
  
  auto res = descr.addRegister (std::move(symb),type);

  
  MiniMC::Model::CFA cfa{};
  auto frame = prgm.getRootFrame ().create ("KK");
  auto init = cfa.makeLocation (prgm.getRootFrame().makeFresh(),MiniMC::Model::LocationInfo{{},descr,frame});
  auto end = cfa.makeLocation (prgm.getRootFrame().makeFresh(),MiniMC::Model::LocationInfo{{},descr,frame});
  cfa.setInitial (init);
  {  
    MiniMC::Model::EdgeBuilder builder{cfa,init,end,frame};
    builder.addInstr<MiniMC::Model::InstructionCode::Add> (res,MiniMC::Model::I8Integer::make(1),MiniMC::Model::I8Integer::make(2));
  }

  auto func = prgm.addFunction (prgm.getRootFrame().makeFresh (),
				{},
				MiniMC::Model::VoidType::get(),
				std::move(descr),
				std::move(cfa),
				false,
				frame);

  MiniMC::CPA::Concrete::CPA cpa;
  auto init_state = cpa.makeInitialState ({{func},{},prgm});
  auto transfer = cpa.makeTransfer (prgm);

  auto res_state = transfer->doTransfer (*init_state,{*func->getCFA().getInitialLocation()->ebegin (),0});

  auto val = res_state->getBuilder ().buildValue (0,*symbol_value);
  auto result = res_state->getConcretizer ()->evaluate (*val);

  auto res_val = MiniMC::Model::visitValue<MiniMC::BV8> (MiniMC::Support::Overload {
      [](const MiniMC::Model::I8Integer& i8) {return i8.getValue ();},
      MiniMC::Support::Error<MiniMC::BV8>{}
    },
    
    *result
    );

  CHECK(res_val == 3);
  
 }
