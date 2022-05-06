#ifndef _SSAMAP__
#define _SSAMAP__

#include <functional>
#include <gsl/pointers>
#include <ostream>
#include <unordered_map>

#include "model/variables.hpp"
#include "smt/term.hpp"
#include "support/localisation.hpp"

/*namespace MiniMC {
  namespace Util {

    template <class From, class To, typename Index = std::size_t, Index defaultindex = 1>
    class SSAMapImpl {
    public:
      struct Entry {
        To to;
        Index index;
        bool operator==(const Entry& oth) const {
          return to == oth.to &&
                 index == oth.index;
        }
      };

      SSAMapImpl() {}
      SSAMapImpl(const SSAMapImpl&) = default;

      void initialiseValue(gsl::not_null<From> from, gsl::not_null<To> to) {
        assert(!entries.count(from.get()));
        entries.insert(std::make_pair(from.get(), Entry{.to = to.get(), .index = defaultindex}));
      }

      void updateValue(gsl::not_null<From> from, gsl::not_null<To> to) {
        assert(entries.count(from.get()));
        auto& entr = entries.at(from.get());
        entr.index++;
        entr.to = to.get();
      }

      To lookup(gsl::not_null<From> from) const {
        assert(entries.count(from.get()));
        return entries.at(from.get()).to;
      }

      Index getIndex(gsl::not_null<From> from) const {
        assert(entries.count(from.get()));
        return entries.at(from.get()).index;
      }

      //Merge two ssamaps.
      //If they match on all places, just return a copy of l
      using ConflictBreaker = std::function<To(const To&, const To&)>;
      static SSAMapImpl merge(const SSAMapImpl& l, const SSAMapImpl& r, ConflictBreaker breakConflict) {
        assert(l.size() == r.size());

        SSAMapImpl nmap;
        for (auto& cur_l : l.entries) {
          auto it = r.entries.find(cur_l.first);
          assert(it != r.entries.end());
          auto& cur_r = *it;
          if (cur_l.second == cur_r.second) {
            nmap.entries.insert(std::make_pair(cur_l.first, cur_l.second));
          } else {
            Index nindex = std::max(cur_l.second.index, cur_r.second.index) + 1;
            To nval = breakConflict(cur_l.second.to, cur_r.second.to);
            nmap.entries.insert(std::make_pair(cur_l.first, Entry{.to = nval, .index = nindex}));
          }
        }
        return nmap;
      }

      std::ostream& output(std::ostream& os) const {
        MiniMC::Support::Localiser format("%1%_%2% : %3%\n");
        for (auto& e : entries) {
          os << format.format(static_cast<std::string>(*e.first), e.second.index, static_cast<std::string>(*e.second.to));
        }
        return os;
      }

      auto size() const { return entries.size(); }

    private:
      std::unordered_map<From, Entry> entries;
    };

    template <class From, class To, typename Index, Index defaultindex>
    inline std::ostream& operator<<(std::ostream& os, const SSAMapImpl<From, To, Index, defaultindex>& map) {
      return map.output(os);
    }

    using SSAMap = SSAMapImpl<MiniMC::Model::Value*, SMTLib::Term_ptr>;

  } // namespace Util
} // namespace MiniMC
*/

#include "util/valuemap.hpp"

namespace MiniMC {
  namespace Util {
    template<class From,class To, typename Index = std::size_t,Index defaultIndex = 1> 
    class SSAMap {
    public:
      SSAMap (std::size_t vars = 0) : entries(vars) {}
      
      void set (const  From& f, To&& to) {
	const auto& cur= entries[f];
	entries.set (f,{.val = std::move(to), .ssaindex = cur.ssaindex+1});
      }

      const To& get (const From& f) const {
	return entries.at (f).val;
      }

      //If they match on all places, just return a copy of l
      using ConflictBreaker = std::function<To(const To&, const To&)>;
      static SSAMap merge(const SSAMap& l, const SSAMap& r, ConflictBreaker breakConflict) {
        SSAMap mmap{l.entries.getSize  ()};
	auto lit =  l.entries.begin();
	auto lend = l.entries.end ();
	auto rit = r.entries.begin ();
	auto rend = r.entries.end ();
	std::size_t i = 0;
	for  (; lit != lend && rit != rend; ++i,++lit,++rit) {
	  if (lit->ssaindex != rit->ssaindex || lit->val != rit->val) {
	    mmap.entries[i] = {.val = breakConflict(lit->val,rit->val),
	      .ssaindex = std::max(lit->ssaindex,rit->ssaindex)+1
	    };
	      
	  }
	  else {
	    mmap.entries[i]  = *lit;
	  }
	}
        return mmap;
      }

      
    private:
      struct Entry {
	To val;
	Index ssaindex{0};	
      };
      FixedVector<From,Entry> entries;
      
    };
    
  }
}

#endif
