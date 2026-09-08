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
  
  
  
  MiniMC::Model::CFA cfa{};
  auto frame = prgm.getRootFrame ().create ("KK");
  auto type = MiniMC::Model::I8Type::get();;
  auto res = descr.addRegister (frame.makeFresh (),type);
  
  auto init = cfa.makeLocation (prgm.getRootFrame().makeFresh(),MiniMC::Model::LocationInfo{{},frame});
  auto end = cfa.makeLocation (prgm.getRootFrame().makeFresh(),MiniMC::Model::LocationInfo{{},frame});
  cfa.setInitial (init);
  {  
    MiniMC::Model::EdgeBuilder builder{cfa,init,end,frame};
    builder.addInstr<MiniMC::Model::InstructionCode::Add> (res,MiniMC::Model::I8Integer::make (1),MiniMC::Model::I8Integer::make (2));
  }

  auto symb = prgm.getRootFrame().makeFresh ();
  auto func = prgm.addFunction (symb,
				{},
				MiniMC::Model::VoidType::get(),
				std::move(cfa),
				false,
				frame);

  MiniMC::CPA::Concrete::CPA cpa{MiniMC::Model::NonDetGenerator{}};
  auto init_state = cpa.makeInitialState ({{symb},{},prgm}) ;
  auto transfer = cpa.makeTransfer (prgm);

  MiniMC::CPA::State_ptr res_state = nullptr;
  for (auto s : transfer->doTransfer (*init_state,{*func->getCFA().getInitialLocation()->ebegin (),0})) {
    res_state = s;
    break;
  }
  
  REQUIRE(res_state != nullptr);
  
  auto val = res_state->getBuilder ().buildValue (0,*res);
  auto result = res_state->getConcretizer ()->evaluate (*val);

  auto res_val = MiniMC::Model::visitValue (MiniMC::Support::Overload {
      [](const MiniMC::Model::I8Integer& i8) {return i8.getValue ();},
      MiniMC::Support::Error<MiniMC::BV8>{}
    },
    
    *result
    );

  CHECK(res_val == 3);
  
 }
