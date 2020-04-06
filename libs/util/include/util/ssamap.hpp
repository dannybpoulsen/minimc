#ifndef _SSAMAP__
#define _SSAMAP__

#include <ostream>
#include <functional>
#include <unordered_map>
#include <gsl/pointers>

#include "support/localisation.hpp"
#include "model/variables.hpp"
#include "smt/term.hpp"

namespace MiniMC {
  namespace Util {
	
	template<class From,class To,typename Index =std::size_t, Index defaultindex = 1>
	class SSAMapImpl {
	public:
	  struct Entry {
		To to;
		Index index;
		bool operator== (const Entry& oth) {
		  return to == oth.to &&
			index == oth.index;
		}
		
	  };

	  SSAMapImpl () {}
	  SSAMapImpl (const SSAMapImpl& ) = default;

	  
	  void initialiseValue (gsl::not_null<From> from, gsl::not_null<To> to)  {
		assert(!entries.count(from.get()));
		entries.insert (std::make_pair (from.get(),Entry {.to = to.get(), .index = defaultindex}));
	  }

	  void updateValue (gsl::not_null<From> from, gsl::not_null<To> to) {
		assert(entries.count (from.get()));
		auto& entr = entries.at(from.get());
		entr.index++;
		entr.to = to.get();
	  }

	  To lookup (gsl::not_null<From> from ) const {
		assert(entries.count (from.get()));
		return entries.at (from.get()).to; 
	  }

	  Index getIndex (gsl::not_null<From> from ) const {
		assert(entries.count (from.get()));
		return entries.at (from.get()).index; 
	  }

	  //Merge two ssamaps.
	  //If they match on all places, just return a copy of l
	  using ConflictBreaker = std::function<To(const To&,const To&)>;
	  static SSAMapImpl merge (const SSAMapImpl& l, const SSAMapImpl& r, ConflictBreaker breakConflict) {
		assert(l.size() == r.size());
		
		SSAMapImpl nmap;
		for (auto& cur_l : l.entries) {
		  auto it = r.entries.find (cur_l.first);
		  assert (it != r.entries.end ());
		  auto& cur_r = *it;
		  if (cur_l.second == cur_r.second) {
			nmap.insert (std::make_pair (cur_l.first,cur_l.second));
		  }
		  else {
			Index nindex  = std::max (cur_l.second.index,cur_r.second.index)+1;
			To nval = breakConflict (cur_l.second.to,cur_r.second.to);
			nmap.entries.insert (std::make_pair (cur_l.first,Entry {.index = nindex, .to = nval}));
		  }
			
		}
		return nmap;
	  }

	  std::ostream& output (std::ostream& os)  const {
		MiniMC::Support::Localiser format ("%1%_%2% : %3%\n");  
		for (auto& e : entries) {
		  os << format.format (static_cast<std::string>(*e.first),e.second.index,static_cast<std::string> (*e.second.to));
		}
		return os;
	  }
	  
	private:
	  std::unordered_map<From,Entry> entries;
	};

	template<class From,class To,typename Index, Index defaultindex>
	inline std::ostream& operator<< (std::ostream& os, const SSAMapImpl<From,To,Index,defaultindex>& map) {
	  return map.output (os);
	}
	
	using SSAMap =SSAMapImpl<MiniMC::Model::Value*,SMTLib::Term_ptr>;
	
  }
}

				   

#endif
