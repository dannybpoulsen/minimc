#ifndef _CPA_COMMON__
#define _CPA_COMMON__

namespace MiniMC {
  namespace CPA {
  namespace Common {
    template<class Value,class Lookup>
    struct ActivationRecord {
      ActivationRecord(Lookup&& values, const MiniMC::Model::Value_ptr& ret) : values(std::move(values)), ret(ret) {}
      ActivationRecord(const ActivationRecord&) = default;

      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	hash << values << ret.get();
	return hash;
      }

      Lookup values;
      MiniMC::Model::Value_ptr ret{nullptr};
    };

    template <class ActRecord>
    struct ActivationStack {
      ActivationStack(ActRecord&& sf) {
        frames.push_back(std::move(sf));
      }
      ActivationStack(const ActivationStack&) = default;

      auto pop() {
        auto val = frames.back();
        frames.pop_back();
        return val;
      }

      void push(ActRecord&& frame) {
        frames.push_back (std::move(frame));
      }

      auto& back () {return frames.back ();}
      auto& back () const {return frames.back ();}
      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	for (auto& vl : frames) {
	  hash << vl;
	}
	return hash;
      }
      
      std::vector<ActRecord> frames;
    };
  }
  }
}

#endif
