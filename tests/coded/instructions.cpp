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
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64>();
  MiniMC::Model::ConstantFactory_ptr cfac = std::make_shared<MiniMC::Model::ConstantFactory64>(tfac);
  MiniMC::Model::Program prgm {tfac,cfac};

  MiniMC::Model::RegisterDescr descr;
  
  
  auto type = tfac->makeIntegerType (8);
  auto res = descr.addRegister (prgm.getRootFrame ().makeFresh (),type);

  MiniMC::Model::CFA cfa{};
  auto frame = prgm.getRootFrame ().create ("KK");
  auto init = cfa.makeLocation (prgm.getRootFrame().makeFresh(),MiniMC::Model::LocationInfo{{},descr});
  auto end = cfa.makeLocation (prgm.getRootFrame().makeFresh(),MiniMC::Model::LocationInfo{{},descr});
  cfa.setInitial (init);
  {  
    MiniMC::Model::EdgeBuilder builder{cfa,init,end,frame};
    builder.addInstr<MiniMC::Model::InstructionCode::Add> (res,cfac->makeIntegerConstant (1,MiniMC::Model::TypeID::I8),cfac->makeIntegerConstant (2,MiniMC::Model::TypeID::I8));
  }

  auto func = prgm.addFunction (prgm.getRootFrame().makeFresh (),
		    {},
		    tfac->makeVoidType (),
		    std::move(descr),
		    std::move(cfa),
		    false,
		    frame);

  MiniMC::CPA::Concrete::CPA cpa;
  auto init_state = cpa.makeInitialState ({{func},{},prgm});
  auto transfer = cpa.makeTransfer (prgm);

  auto res_state = transfer->doTransfer (*init_state,{*func->getCFA().getInitialLocation()->ebegin (),0});

  auto val = res_state->getBuilder ().buildValue (0,res);
  auto result = res_state->getConcretizer ()->evaluate (*val);

  auto res_val = MiniMC::Model::visitValue<MiniMC::BV8> (MiniMC::Support::Overload {
      [](const MiniMC::Model::I8Integer& i8) {return i8.getValue ();},
      MiniMC::Support::Error<MiniMC::BV8>{}
    },
    
    *result
    );

  CHECK(res_val == 3);
  
 }
