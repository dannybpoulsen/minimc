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

TEST_CASE("Registers are variables, not values") {
  static_assert(!std::is_base_of_v<MiniMC::Model::Value, MiniMC::Model::Register>);

  MiniMC::Model::Program prgm;
  auto frame = prgm.getRootFrame().create("Function");
  MiniMC::Model::RegisterFactory descr{frame};
  
  auto symbol = frame.makeFresh("reg");
  auto reg = descr.makeRegister(symbol, MiniMC::Model::I8Type::get());

  auto value = std::dynamic_pointer_cast<MiniMC::Model::SymbolicConstant>(
      MiniMC::Model::makeRegisterValue(*reg));
  REQUIRE(value != nullptr);
  CHECK(value->getValue().getFullName() == symbol.getFullName());
  const MiniMC::Model::Value& borrowed_value = *value;
  const auto owner_count = reg.use_count();
  CHECK(&MiniMC::Model::getRegister(borrowed_value) == reg.get());
  CHECK(reg.use_count() == owner_count);
  CHECK(MiniMC::Model::getRegisterPtr(*value) == reg);
}

TEST_CASE("Add") {
  MiniMC::Model::Program prgm;
  auto frame = prgm.getRootFrame ().create ("KK");
  
  MiniMC::Model::RegisterFactory descr{frame};
  
  
  
  MiniMC::Model::CFA cfa{};
  auto type = MiniMC::Model::I8Type::get();;
  auto res = descr.makeRegister (frame.makeFresh (),type);
  
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
  
  auto val = res_state->getBuilder ().buildValue (0,*MiniMC::Model::makeRegisterValue(*res));
  auto result = res_state->getConcretizer ()->evaluate (*val);

  auto res_val = MiniMC::Model::visitValue (MiniMC::Support::Overload {
      [](const MiniMC::Model::I8Integer& i8) {return i8.getValue ();},
      MiniMC::Support::Error<MiniMC::BV8>{}
    },
    
    *result
    );

  CHECK(res_val == 3);
  
 }
