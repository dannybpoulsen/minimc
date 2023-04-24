#ifndef _CPA_COMMON__
#define _CPA_COMMON__

#include "vm/vmt.hpp"

namespace MiniMC {
  namespace CPA {
  namespace Common {
    template<class Value>
    struct ActivationRecord {
      ActivationRecord(MiniMC::Model::VariableMap<Value>&& values, const MiniMC::Model::Value_ptr& ret) : values(std::move(values)), ret(ret) {}
      ActivationRecord(const ActivationRecord&) = default;
      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	hash << values << ret.get();
	return hash;
      }
      
      MiniMC::Model::VariableMap<Value> values;
      MiniMC::Model::Value_ptr ret{nullptr};
    };
    
    template <class Value>
    struct ActivationStack {
      using ActRecord = ActivationRecord<Value>;
      ActivationStack(MiniMC::Model::VariableMap<Value>&& cpuregs, ActRecord&& sf) : cpuregs(std::move(cpuregs)) {
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
      auto& cpus () {return cpuregs;}
      
      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	hash << cpuregs;
	for (auto& vl : frames) {
	  hash << vl;
	}
	return hash;
      }

      
      
      MiniMC::Model::VariableMap<Value> cpuregs;
      std::vector<ActRecord> frames;
    };


    template<class T>
    struct BaseValueLookup : MiniMC::VMT::ValueLookup<T> {
    public:
      BaseValueLookup (ActivationStack<T>& values) : values(values) {}
      BaseValueLookup (const BaseValueLookup&) = delete;
      virtual  ~BaseValueLookup () {}
      virtual T lookupValue (const MiniMC::Model::Value& v) const override = 0;
      void saveValue(const MiniMC::Model::Register& v, T&& value) override {
	if (v.getRegType () == MiniMC::Model::RegType::Local) {
	  values.back().values.set (v,std::move(value));
	}
	else {
	  values.cpus ().set (v,std::move(value));
	}
      }
      virtual T unboundValue(const MiniMC::Model::Type&) const override = 0;
      virtual T defaultValue(const MiniMC::Model::Type&) const override = 0;
      
      MiniMC::Hash::hash_t hash() const { return values.hash(); }
      using Value = T;
    protected:
      T lookupRegister (const MiniMC::Model::Register& reg) const  {
	return (reg.getRegType () == MiniMC::Model::RegType::Local) ? values.back().values[reg]
	                                                            : values.cpus()[reg];
      }
    private:
      ActivationStack<T>& values;
    };
    
    

    
  }
  }
}

#endif
