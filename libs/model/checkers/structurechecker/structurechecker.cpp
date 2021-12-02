/**
 * @file   structurechecker.cpp
 * @author Danny Bøgsted Poulsen <caramon@homemachine>
 * @date   Mon Apr 20 14:35:32 2020
 * 
 * @brief  
 * 
 * 
 */
#include "model/checkers/structure.hpp"

#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"
#include "support/pointer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Checkers {
      /** 
	   * Make sure that \p e does not have both Guard and InstructionStream
	   *
	   * @return true if the check passes false otherwise 
	   */
      bool checkNoGuardAndInstructionStream(const MiniMC::Model::Edge& e, MiniMC::Support::Messager& mess) {
        if (e.hasAttribute<MiniMC::Model::AttributeType::Instructions>() &&
            e.hasAttribute<MiniMC::Model::AttributeType::Guard>()) {
          mess.error("Edges must not have both guards and Instructions");
          return false;
        }
        return true;
      }

      /** 
	   * Make sure that if \p e has a guard, then it is boolean. 
	   *
	   * @return true if the check passes false otherwise 
	   */
      bool checkGuardIsBoolean(const MiniMC::Model::Edge& e, MiniMC::Support::Messager& mess) {
        if (e.hasAttribute<MiniMC::Model::AttributeType::Guard>()) {
          auto guard = e.getAttribute<MiniMC::Model::AttributeType::Guard>();
          if (guard.guard->getType()->getTypeID() != MiniMC::Model::TypeID::Bool) {
            mess.error("Edge guard must be booleans");
            return false;
          }
        }
        return true;
      }

      /** 
	   * Check that a (possible) call is last in the InstructionStream.
	   * @return true if the check passes false otherwise 
	   */
      bool checkCallIsLast(const MiniMC::Model::Edge& e, MiniMC::Support::Messager& mess) {
        if (e.hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
          auto instr = e.getAttribute<MiniMC::Model::AttributeType::Instructions>();
          auto it = instr.rbegin();
          auto end = instr.rend();
          for (++it; it != end; ++it) {
            if (it->getOpcode() == MiniMC::Model::InstructionCode::Call) {
              mess.error("Calls can only be last instruction of an instructionstream");
              return false;
            }
          }
        }
        return true;
      }

      /** 
	   * Check that phi InstructionStream consists of only direct assignments
	   * @return true if the check passes false otherwise 
	   */
      bool checkPhiIsOnlyAssign(const MiniMC::Model::Edge& e, MiniMC::Support::Messager& mess) {
        if (e.hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
          auto instr = e.getAttribute<MiniMC::Model::AttributeType::Instructions>();
          auto it = instr.rbegin();
          auto end = instr.rend();
          if (instr.isPhi () ) {
            for (++it; it != end; ++it) {
              if (it->getOpcode() != MiniMC::Model::InstructionCode::Assign) {
                mess.error("Phi edges can only have assignments");
                return false;
              }
            }
          }
        }
        return true;
      }

      /** 
	   * Make sure that  asserts are the last instruction in an instructionstream
	   * @return true if the check passes false otherwise 
	   */

      bool checkAssertIsLast(const MiniMC::Model::Edge& e, MiniMC::Support::Messager& mess) {
        if (e.hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
          auto instr = e.getAttribute<MiniMC::Model::AttributeType::Instructions>();
          auto it = instr.rbegin();
          auto end = instr.rend();
          for (++it; it != end; ++it) {
            if (it->getOpcode() != MiniMC::Model::InstructionCode::Assert) {
              mess.error("Asserts can only be last instruction of an instructionstream");
              return false;
            }
          }
        }
        return true;
      }

      bool checkEdge(const MiniMC::Model::Edge& e, MiniMC::Support::Messager& mess) {
        return checkNoGuardAndInstructionStream(e, mess) &&
               checkGuardIsBoolean(e, mess) &&
               checkCallIsLast(e, mess) &&
               checkPhiIsOnlyAssign(e, mess);
      }

      bool checkFunction(const MiniMC::Model::Function& F, MiniMC::Support::Messager& messager) {
        for (auto& E : F.getCFG()->getEdges()) {
          if (!checkEdge(*E, messager))
            return false;
        }
        return true;
      }

      bool StructureChecker::run(MiniMC::Model::Program& prgm) {
        messager.message("Initiating Structural Checks");
        bool res = true;
        for (auto& F : prgm.getFunctions()) {
          if (!checkFunction(*F, messager))
            res = false;
        }
        if (!res) {
          messager.error("Structural checks   not passing");
        }
        messager.message("Structural checks finished");
        return res;
      }
    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC
