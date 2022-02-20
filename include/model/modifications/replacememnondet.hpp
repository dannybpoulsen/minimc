/**
 * @file   replacememnondet.hpp
 * @date   Mon Apr 20 17:05:18 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _REPLACEMEM__
#define _REPLACEMEM__

#include "model/cfg.hpp"
#include "model/modifications/helpers.hpp"
#include "model/modifications/simplify_cfg.hpp"
#include "model/variables.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      /**
       * Translates all memory  \ref MiniMC::Model::InstructionCode::Load instructions to  \ref MiniMC::Model::InstructionCode::NonDet instrucitons.
       * This is useful when an analysis engine does not model, but only works on registers. 
       **/
      struct RemoveMemNondet : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        virtual bool runFunction(const MiniMC::Model::Function_ptr& F) {
          auto prgm = F->getPrgm();
          for (auto& E : F->getCFG()->getEdges()) {
            if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
              for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                if (I.getOpcode() == MiniMC::Model::InstructionCode::Load) {
                  //MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Load> load(I);
		  auto& content = I.getOps<MiniMC::Model::InstructionCode::Load> ();
		  MiniMC::Model::Value_ptr  min = nullptr, max = nullptr;;
                  switch (content.res->getType()->getSize()) {
                    case 1:
                      min = prgm->getConstantFactory()->makeIntegerConstant(std::numeric_limits<MiniMC::uint8_t>::min(), content.res->getType());
		      max = prgm->getConstantFactory()->makeIntegerConstant(std::numeric_limits<MiniMC::uint8_t>::max(), content.res->getType());
                      break;
                    case 2:
                      min = prgm->getConstantFactory()->makeIntegerConstant(std::numeric_limits<MiniMC::uint16_t>::min(), content.res->getType());
                      max = prgm->getConstantFactory()->makeIntegerConstant(std::numeric_limits<MiniMC::uint16_t>::max(), content.res->getType());
                      break;
                    case 4:
                      min = prgm->getConstantFactory()->makeIntegerConstant(std::numeric_limits<MiniMC::uint32_t>::min(), content.res->getType());
                      max = prgm->getConstantFactory()->makeIntegerConstant(std::numeric_limits<MiniMC::uint32_t>::max(), content.res->getType());
                      break;
                    case 8:
                      min = prgm->getConstantFactory()->makeIntegerConstant(std::numeric_limits<MiniMC::uint64_t>::min(), content.res->getType());
                      max = prgm->getConstantFactory()->makeIntegerConstant(std::numeric_limits<MiniMC::uint64_t>::max(), content.res->getType());
                      break;
                    default:
                      MiniMC::Support::Exception("Shouldnøt get here");
                  }
		  assert(min);
		  assert(max);
                  I.replace(createInstruction<InstructionCode::NonDet> ({.res = content.res, .min = min, .max = max}));
                }
                if (I.getOpcode() == MiniMC::Model::InstructionCode::Alloca ||
                    //I.getOpcode() == MiniMC::Model::InstructionCode::Malloc ||
                    I.getOpcode() == MiniMC::Model::InstructionCode::ExtendObj ||
                    I.getOpcode() == MiniMC::Model::InstructionCode::Store

                ) {
                  I.replace(createInstruction<InstructionCode::Skip> (0));
                }
              }
            }
          }
          return true;
        }
        virtual bool run(MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getFunctions()) {
            runFunction(F);
          }
          return true;
        }
      };

      struct ExpandUndefValues : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        virtual bool runFunction(const MiniMC::Model::Function_ptr&) {

	  /* auto& prgm = *F->getPrgm();
          auto cfg = F->getCFG().get();
          MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
          auto inserter = wlist.inserter();
          std::for_each(cfg->getEdges().begin(),
                        cfg->getEdges().end(),
                        [&](const MiniMC::Model::Edge_ptr& e) { inserter = e; });
          while (wlist.size()) {
            auto edge = wlist.pop();
            if (edge->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
              auto& origstr = edge->getAttribute<MiniMC::Model::AttributeType::Instructions>();
              bool expanded = false;
              for (auto& instr : origstr) {
                if (expanded)
                  break;
                for (auto& op : instr) {
                  if (std::dynamic_pointer_cast<MiniMC::Model::Undef>(op)) {
                    MiniMC::Model::Modifications::ReplaceMap<Value> replace;
                    MiniMC::uint64_t min = 0;
                    MiniMC::uint64_t max = 0;

                    switch (op->getType()->getSize()) {
                      case 1:
                        min = std::numeric_limits<MiniMC::uint8_t>::min();
                        max = std::numeric_limits<MiniMC::uint8_t>::max();
                        break;
                      case 2:
                        min = std::numeric_limits<MiniMC::uint16_t>::min();
                        max = std::numeric_limits<MiniMC::uint16_t>::max();
                        break;
                      case 4:
                        min = std::numeric_limits<MiniMC::uint32_t>::min();
                        max = std::numeric_limits<MiniMC::uint32_t>::max();
                        break;
                      case 8:
                        min = std::numeric_limits<MiniMC::uint64_t>::min();
                        max = std::numeric_limits<MiniMC::uint64_t>::max();
                        break;
                      default:
                        assert(false);
                    }
                    auto& fact = prgm.getConstantFactory();
                    MiniMC::uint64_t it = min;

                    while (true) {
                      auto replc = fact->makeIntegerConstant(it, op->getType());
                      MiniMC::Model::Modifications::ReplaceMap<MiniMC::Model::Value> vals;
                      MiniMC::Model::Modifications::ReplaceMap<MiniMC::Model::Location> loc;

                      vals.insert(std::make_pair(op.get(), replc));

                      copyEdgeAndReplace(edge, vals, loc, cfg, inserter);
                      if (it == max)
                        break;
                      it++;
                    }
                    cfg->deleteEdge(edge);
                    expanded = true;
                    break;
                  }
                }
              }
            }
	    }*/
          return true;
        }

        virtual bool run(MiniMC::Model::Program& prgm) {
          //First make sure we only have one NonDet on each edge
          for (auto& F : prgm.getFunctions()) {
            runFunction(F);
          }

          return true;
        }
      };
      struct ExpandNondet : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        virtual bool runFunction(const MiniMC::Model::Function_ptr& F) {
          ExpandUndefValues{}.runFunction(F);
          EnsureEdgesOnlyHasOne<MiniMC::Model::InstructionCode::NonDet>{}.runFunction(F);
          auto& prgm = *F->getPrgm();
          auto cfg = F->getCFG();
          MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
          auto inserter = wlist.inserter();
          std::for_each(cfg->getEdges().begin(),
                        cfg->getEdges().end(),
                        [&](const MiniMC::Model::Edge_ptr& e) { inserter = e; });

          for (auto& E : wlist) {
            if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
              auto& origstr = E->getAttribute<MiniMC::Model::AttributeType::Instructions>();
              auto& instr = origstr.last();

              if (instr.getOpcode() == MiniMC::Model::InstructionCode::NonDet) {
                auto& content = instr.getOps<MiniMC::Model::InstructionCode::NonDet> ();
		assert(content.res->getType()->isInteger ());
                auto type = content.res->getType();
                auto from = E->getFrom();
                auto to = E->getTo();
                MiniMC::uint64_t min = 0;
                MiniMC::uint64_t max = 0;

                switch (type->getTypeID()) {
		case MiniMC::Model::TypeID::I8:
                    min = std::static_pointer_cast<TConstant<MiniMC::uint8_t>>(content.min)->getValue();
                    max = std::static_pointer_cast<TConstant<MiniMC::uint8_t>>(content.max)->getValue();
                    break;
		case MiniMC::Model::TypeID::I16:
                    min = std::static_pointer_cast<TConstant<MiniMC::uint16_t>>(content.min)->getValue();
                    max = std::static_pointer_cast<TConstant<MiniMC::uint16_t>>(content.max)->getValue();
                    break;
		case MiniMC::Model::TypeID::I32:
                    min = std::static_pointer_cast<TConstant<MiniMC::uint32_t>>(content.min)->getValue();
                    max = std::static_pointer_cast<TConstant<MiniMC::uint32_t>>(content.max)->getValue();
                    break;
		case MiniMC::Model::TypeID::I64:
                    min = std::static_pointer_cast<TConstant<MiniMC::uint64_t>>(content.min)->getValue();
                    max = std::static_pointer_cast<TConstant<MiniMC::uint64_t>>(content.max)->getValue();
                    break;
                  default:
		    throw MiniMC::Support::Exception ("Error");
		}
                auto& fact = prgm.getConstantFactory();
                MiniMC::uint64_t it = min;

                while (true) {
                  auto val = fact->makeIntegerConstant(it, type);
                  
                  auto nedge = cfg->makeEdge(from, to);
                  nedge->setAttribute<MiniMC::Model::AttributeType::Instructions>(origstr);
                  nedge->getAttribute<MiniMC::Model::AttributeType::Instructions>().last().replace(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Assign> ({
			.res = content.res,
			.op1 = val})
		    );
		  
		    
                  it++;
                  if (it == max)
                    break;
                }
                cfg->deleteEdge(E);
              }
            }
          }
          return true;
        }

        virtual bool run(MiniMC::Model::Program& prgm) {
          //First make sure we only have one NonDet on each edge
          for (auto& F : prgm.getFunctions()) {
            runFunction(F);
          }

          return true;
        }
      };

    } // namespace Modifications
  }   // namespace Model

} // namespace MiniMC

#endif
