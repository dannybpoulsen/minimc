/**
 * @file   instructions.hpp
 * @author Danny Bøgsted Poulsen <caramon@homemachine>
 * @date   Sun Apr 19 11:30:31 2020
 * 
 * @brief  
 * 
 * 
 */

#ifndef _INSTRUCTIONS__
#define _INSTRUCTIONS__
#include "model/variables.hpp"
#include <cassert>
#include <ostream>
#include <variant>

namespace MiniMC {
  namespace Model {

    #include "model/instructions.inc"
    
    template<class T>
    constexpr bool isInternal_v = InstructionData<T::getOpcode ()>::isInternal;

    template<class T>
    constexpr bool  isTAC_v = InstructionData<T::getOpcode ()>::isTAC;

    template<class T>
    constexpr bool isComparison_v = InstructionData<T::getOpcode ()>::isComparison;

    
    template<class T>
    constexpr bool isUnary_v = InstructionData<T::getOpcode ()>::isUnary;

    template<class T>
    constexpr bool isMemory_v = InstructionData<T::getOpcode ()>::isMemory;

    template<class T>
    constexpr  bool isCast_v = InstructionData<T::getOpcode ()>::isCast;
    
    template<class T>
    constexpr  bool isPointer_v = InstructionData<T::getOpcode ()>::isPointer;

    template<class T>
    constexpr bool isAggregate_v = InstructionData<T::getOpcode ()>::isAggregate;

    template<class T>
    constexpr bool isPredicate_v = InstructionData<T::getOpcode ()>::isPredicate;

    template<class T>
    constexpr bool isAssertAssume_v = InstructionData<T::getOpcode ()>::isAssertAssume;;
    
    
    
    
    struct Instruction {
    public:
      Instruction (Instruction_internal internal) : internal(internal) {}
      
      //Copy Consructor with Replacement :-)
      Instruction (const Instruction&, ReplaceFunction);
      
      
      template<InstructionCode op,class... Args>
      static Instruction make (Args... args) {
	if constexpr (hasOperands<op>) 
	  return make<op> (typename InstructionData<op>::Content (std::forward<Args> (args)...));
	else
	  return Instruction_internal{TInstruction<op> ()};
      }
      
      template<InstructionCode c>
      auto& getAs () const {return std::get<TInstruction<c>> (internal);}
      
      auto  visit (auto v) const {return std::visit(v,internal);}
      
      /** 
       * Write a textual representation to \p os
       *
       * @param os The stream to output the representation to
       *
       * @return Reference to \p os, to allowing chaining outputs.
       */
      std::ostream& output(std::ostream& os) const;

    private:
      
      template<InstructionCode op> 
      static Instruction make (InstructionData<op>::Content d) {
	return Instruction_internal{TInstruction<op>(d)};
      }
      
      Instruction_internal internal;
    };
    

    inline std::ostream& operator<<(std::ostream& os, const Instruction& inst) {
      return inst.output(os);
    }
    
    
    std::ostream& operator<<(std::ostream& os, const std::vector<Instruction>& str);
    
    
    /**
	 * Structure representing InstructionStream on  edges.
	 * An InstructionStream is a sequence of instruction to be performed uninterrupted.
	 * If the isPhi is true, then the InstructionStream must be performed without the instructions affecting eachother (this is to 
	 * to reflect that some operations are atomic even within a single process - such as phi-nodes in SSA form).
	 */
    struct InstructionStream {
      using iterator = std::vector<Instruction>::iterator;
      InstructionStream()  {}
      
      InstructionStream(const std::vector<Instruction>& i) : instr(i) {}
      InstructionStream(const InstructionStream& str) = default;
      InstructionStream(InstructionStream&& str) = default;
      
      InstructionStream(const std::initializer_list<Instruction> i) : instr(i) {}
      
      
      InstructionStream& operator= (const InstructionStream&) = default;
      
      auto begin() const { return instr.begin(); }
      auto end() const { return instr.end(); }
      auto begin() { return instr.begin(); }
      auto end() { return instr.end(); }

      auto rbegin() const { return instr.rbegin(); }
      auto rend() const { return instr.rend(); }
      auto rbegin() { return instr.rbegin(); }
      auto rend() { return instr.rend(); }

      operator bool () const {return instr.size ();}

      template<InstructionCode c,class... Args>
      InstructionStream& add (Args... args) {
	instr.emplace_back (Instruction::make<c> (std::forward<Args> (args)...));
	return *this;
      }

      InstructionStream& add (const Instruction& i, ReplaceFunction r) {
	instr.emplace_back (Instruction(i,r));
	return *this;
      }
      
      InstructionStream& add (const Instruction& i) {
	instr.emplace_back (i);
	return *this;
      }

      void clear () {
	instr.clear ();
      }

      bool empty () {
	return instr.empty ();
      }
      
      auto& last() const {
        assert(instr.size());
        return instr.back();
      }

            
      auto& last()  {
        assert(instr.size());
        return instr.back();
      }
      
      template <class Iterator>
      auto erase(Iterator iter) {
        auto res = instr.erase(iter);
        return res;
      }

      template <class Iterator>
      auto replaceInstructionBySeq(iterator repl, Iterator beg, Iterator end) {
        return instr.insert(erase(repl), beg, end);
      }

      std::ostream& output (std::ostream& os) const  {
	return os << instr;
      }
      
      
	
      
    private:
      std::vector<Instruction> instr;
    };
    
    std::ostream& operator<<(std::ostream& os, const InstructionStream& str);

    Instruction makeInstruction (InstructionCode code, std::vector<Value_ptr> values);
    
    
  } // namespace Model
} // namespace MiniMC

#endif
