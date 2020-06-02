#ifndef _VM__
#define _VM__

#define DEBUGVM
#include "model/instructions.hpp"

namespace MiniMC {
  namespace Util {
	
	template <typename T, typename S = decltype(std::declval<T>().finalise(std::declval<void>))> static constexpr bool hasFinalise(void) {return true;}
	template <typename T> static constexpr bool hasFinalise(...) {return false;}
	
	template<class Iterator,class Data,class Exec>
	  void runVM (Iterator it, Iterator end,
				Data& data) {
#define X(OP)									\
		&&OP,
		
		static void* arr[] = {
					   OPERATIONS
		};
#undef X
		
#define DISPATCH(INST,END)												\
		if (INST == END){												\
		  if constexpr (hasFinalise<Exec> ()) {							\
			  data.finalise();											\
			}															\
		  return;														\
		}																\
		else															\
		  goto *arr[static_cast<std::size_t> (INST->getOpcode ())];		\
		
		
			
		DISPATCH(it,end);
		
			
#define X(OP)															\
		OP:																\
		  Exec::template execute<MiniMC::Model::InstructionCode::OP> (data,*it); \
		++it;															\
		DISPATCH(it,end);
		
		OPERATIONS
#undef X
		  
		  
		  }
	


  }
}
#undef DEBUGVM
#endif

