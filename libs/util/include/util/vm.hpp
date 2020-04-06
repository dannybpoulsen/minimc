#ifndef _VM__
#define _VM__

#include "model/instructions.hpp"

namespace MiniMC {
  namespace Util {
		  template<class Iterator,class Data,class Exec>
	  void runVM (Iterator it, Iterator end,
									Data& data) {
#define X(OP)									\
		&&OP,
		
		static void* arr[] = {
					   OPERATIONS
		};
#undef X
		
#define DISPATCH(INST,END)											\
		if (INST == END){												\
		  data.finalise();												\
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
#endif

