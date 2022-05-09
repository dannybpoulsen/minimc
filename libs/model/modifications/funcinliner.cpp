
#include "model/cfg.hpp"
#include "model/modifications/func_inliner.hpp"
#include "model/modifications/helpers.hpp"
#include "support/exceptions.hpp"
#include "support/pointer.hpp"
#include "support/sequencer.hpp"
#include "support/workinglist.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {

      void inlineCallEdgeToFunction(const MiniMC::Model::Program& prgm, const MiniMC::Model::Function_ptr& func, const MiniMC::Model::Edge_ptr& edge, MiniMC::Model::LocationInfoCreator& locinfoc, size_t depth = 10) {
        if (!depth)
          throw MiniMC::Support::Exception("Inlining Depth exceeded");
        auto from_loc = edge->getFrom();
        auto to_loc = edge->getTo();
        auto& instrs = edge->getAttribute<MiniMC::Model::AttributeType::Instructions>();
        assert(instrs.last().getOpcode() == MiniMC::Model::InstructionCode::Call);
	auto call_content = instrs.last ().getOps<MiniMC::Model::InstructionCode::Call> ();
	auto constant = std::static_pointer_cast<MiniMC::Model::Pointer>(call_content.function);
        MiniMC::pointer_t loadPtr =  constant->getValue (); 
	auto cfunc = prgm.getFunction(MiniMC::Support::getFunctionId(loadPtr));
        MiniMC::Model::Modifications::ReplaceMap<MiniMC::Model::Value> valmap;
        auto copyVar = [&](MiniMC::Model::RegisterDescr& stack) {
          for (auto& v : stack.getRegisters()) {
            valmap.insert(std::make_pair(v.get(), func->getRegisterStackDescr().addRegister(v->getName(), v->getType())));
          }
        };

        copyVar(cfunc->getRegisterStackDescr());

        ReplaceMap<MiniMC::Model::Location> locmap;
        std::vector<Location_ptr> nlocs;
        MiniMC::Support::WorkingList<Edge_ptr> wlist;

        copyCFG(cfunc->getCFA(), valmap, func->getCFA(), cfunc->getName(), locmap, std::back_inserter(nlocs), wlist.inserter(), locinfoc);

        for (auto& ne : wlist) {
          if (ne->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
            auto& ninstr = ne->getAttribute<MiniMC::Model::AttributeType::Instructions>();
            if (ninstr.last().getOpcode() == MiniMC::Model::InstructionCode::Call) {
              inlineCallEdgeToFunction(prgm,func, ne, locinfoc, depth - 1);
            }

            else if (ninstr.last().getOpcode() == MiniMC::Model::InstructionCode::RetVoid) {
              ne->setTo(edge->getTo());
              ninstr.last().replace(createInstruction<InstructionCode::Skip> (0));
            }

            else if (ninstr.last().getOpcode() == MiniMC::Model::InstructionCode::Ret) {
	      auto& content = ninstr.last().getOps<MiniMC::Model::InstructionCode::Ret> ();
          
              ne->setTo(edge->getTo());
              ninstr.last().replace(createInstruction<InstructionCode::Assign> ( {
		    .res = call_content.res,
		    .op1 = content.value 
		  })
		);
            }
          }
        }

        edge->setTo(locmap.at(cfunc->getCFA().getInitialLocation().get()));

        auto& parameters = cfunc->getParameters();
        auto it = parameters.begin();
        MiniMC::Model::InstructionStream str;
        for (auto it = instrs.begin(); it != instrs.end() - 1; ++it) {
          str.addInstruction(*it);
        }
        for (size_t i = 0; i < call_content.params.size (); i++, it++) {
          
          str.addInstruction<InstructionCode::Assign> (
					 {.res = valmap.at(it->get()),
					  .op1 = call_content.params.at(i)});  
        }
        edge->delAttribute<MiniMC::Model::AttributeType::Instructions>();
        if (str.begin() != str.end())
          edge->setAttribute<MiniMC::Model::AttributeType::Instructions>(str);
        from_loc->getInfo().template unset<MiniMC::Model::Attributes::CallPlace>();
      }

      bool InlineFunctions::runFunction(const MiniMC::Model::Function_ptr& F,std::size_t depth) {
        MiniMC::Model::LocationInfoCreator linfoc(F->getName());
        MiniMC::Support::WorkingList<Edge_ptr> wlist;
        auto inserter = wlist.inserter();
        auto& cfg = F->getCFA();
        std::for_each(cfg.getEdges().begin(),
                      cfg.getEdges().end(),
                      [&](const MiniMC::Model::Edge_ptr& e) { inserter = e; });
        for (auto& e : wlist) {
          if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions>() &&
              e->getAttribute<MiniMC::Model::AttributeType::Instructions>().last().getOpcode() ==
                  MiniMC::Model::InstructionCode::Call) {
            inlineCallEdgeToFunction(prgm,F, e, linfoc, depth);
          }
        }

        return true;
      }


    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC
