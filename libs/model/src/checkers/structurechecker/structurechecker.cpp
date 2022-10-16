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

namespace MiniMC {
  namespace Model {
    namespace Checkers {
      
      /** 
	   * Check that a (possible) call is last in the InstructionStream.
	   * @return true if the check passes false otherwise 
	   */
      bool checkCallIsLast(const MiniMC::Model::Edge& e, MiniMC::Support::Messager& mess) {
        if (e.getInstructions ()) {
          auto& instr = e.getInstructions ();
          auto it = instr.rbegin();
          auto end = instr.rend();
          for (++it; it != end; ++it) {
            if (it->getOpcode() == MiniMC::Model::InstructionCode::Call) {
              mess.message<MiniMC::Support::Severity::Error>("Calls can only be last instruction of an instructionstream");
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
        if (e.getInstructions ()) {
          auto& instr = e.getInstructions ();
          auto it = instr.rbegin();
          auto end = instr.rend();
          if (instr.isPhi () ) {
            for (++it; it != end; ++it) {
              if (it->getOpcode() != MiniMC::Model::InstructionCode::Assign) {
                mess.message<MiniMC::Support::Severity::Error>("Phi edges can only have assignments");
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
        if (e.getInstructions ()) {
          auto& instr = e.getInstructions ();
          auto it = instr.rbegin();
          auto end = instr.rend();
          for (++it; it != end; ++it) {
            if (it->getOpcode() != MiniMC::Model::InstructionCode::Assert) {
              mess.message<MiniMC::Support::Severity::Error>("Asserts can only be last instruction of an instructionstream");
              return false;
            }
          }
        }
        return true;
      }

      bool checkEdge(const MiniMC::Model::Edge& e, MiniMC::Support::Messager& mess) {
        return 
               checkCallIsLast(e, mess) &&
               checkPhiIsOnlyAssign(e, mess);
      }

      bool checkFunction(const MiniMC::Model::Function& F, MiniMC::Support::Messager& messager) {
        for (auto& E : F.getCFA().getEdges()) {
          if (!checkEdge(*E, messager))
            return false;
        }
        return true;
      }

      bool StructureChecker::run(MiniMC::Model::Program& prgm) {
	MiniMC::Support::Messager messager;
        bool res = true;
        for (auto& F : prgm.getFunctions()) {
          if (!checkFunction(*F, messager))
            res = false;
        }
        return res;
      }
    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC
