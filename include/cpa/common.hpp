#ifndef _CPA_COMMON__
#define _CPA_COMMON__

namespace MiniMC {
  namespace CPA {
  namespace Common {
    template<class Value,class Lookup>
    struct ActivationRecord {
      ActivationRecord(Lookup&& values, const MiniMC::Model::Value_ptr& ret, typename Value::Pointer top) : values(std::move(values)), ret(ret), next_stack_alloc(top) {}
      ActivationRecord(const ActivationRecord&) = default;

      MiniMC::Hash::hash_t hash() const {
        MiniMC::Hash::seed_t seed{0};
        MiniMC::Hash::hash_combine(seed, values);
        MiniMC::Hash::hash_combine(seed, ret.get());
        return seed;
      }

      Lookup values;
      MiniMC::Model::Value_ptr ret{nullptr};
      typename Value::Pointer next_stack_alloc;
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
      
      MiniMC::Hash::hash_t hash(MiniMC::Hash::seed_t seed = 0) const {
	for (auto& vl : frames) {
	  MiniMC::Hash::hash_combine(seed, vl);
	}
	return seed;
      }
      
      std::vector<ActRecord> frames;
    };
  }
  }
}

#endif
