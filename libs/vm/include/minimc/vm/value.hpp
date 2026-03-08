#ifndef _VM_VALUE__
#define _VM_VALUE__

#include <string>
#include <memory>
#include <type_traits>
#include <variant>
#include "minimc/model/variables.hpp"
#include "minimc/model/array.hpp"
#include "minimc/support/exceptions.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/smt/smt.hpp"
#include <generator>

namespace MiniMC {
  namespace VMT {

    enum class TriBool {
      True,
      False,
      Unk
    };
    
    
    template<class T>
    concept BoolC = requires (const T t) {
      {t.boolState ()}->std::convertible_to<TriBool>;
      
    };
    
    template<class T, class... Ts>
    concept is_same = 
    (... && std::is_same<T, Ts>::value);

    
    template<typename Int8,
	     typename Int16,
	     typename Int32,
	     typename Int64,
	     typename PointerT,
	     typename Pointer32T,
	     BoolC BoolT,
	     typename Ag,
	     typename Mem>
    struct GenericVal {
      using I8 = Int8;
      using I16 = Int16;
      using I32 = Int32;
      using I64 = Int64;
      using Pointer = PointerT;
      using Pointer32 = Pointer32T;
      using Bool = BoolT;
      using Aggregate = Ag;
      using Memory = Mem;
      
      GenericVal () : content(BoolT{}) {} 
      GenericVal (I8 val) : content(val) {}
      GenericVal (I16 val) : content(val) {}
      GenericVal (I32 val) : content(val) {}
      GenericVal (I64 val) : content(val) {}
      GenericVal (Pointer val) : content(val) {}
      GenericVal (Pointer32 val) : content(val) {}
      GenericVal (Bool val) : content(val) {}
      GenericVal (Aggregate ag) : content(ag) {}
      GenericVal (Memory mem) : content(mem) {}

      GenericVal (const GenericVal&) = default;
      
      template<class Func, class... Values> requires (... && std::is_same_v<GenericVal,Values>)
      static auto visit (Func f,Values... values)  {
	return std::visit (f,values.content...);
      }
    
      auto hash () const {return std::hash<decltype(content)>{} (content);}
      
      auto& output (std::ostream& os) const {return std::visit([&os](const auto& x) ->std::ostream&  { return os << x; }, content);}

      template<class V>
      static constexpr std::size_t bitsize () {
	if constexpr (std::is_same_v<V,I8>) return 8;
	if constexpr (std::is_same_v<V,I16>) return 16;
	if constexpr (std::is_same_v<V,I32>) return 32;
	if constexpr (std::is_same_v<V,I64>) return 64;
	if constexpr (std::is_same_v<V,Pointer>) return 64;
	if constexpr (std::is_same_v<V,Pointer32>) return 32;
	return 0;
      }

      template<class V>
      static constexpr std::size_t bytesize () {
	return bitsize<V> () / 8;
      }
 
      
    private:
      
      std::variant<I8,I16,I32,I64,Pointer,Pointer32,Bool,Ag,Memory> content;
    };

    
    
    template<class G>
    concept Outputtable = requires (std::ostream& os, const G&g) {g.output (os);};
    
    template<Outputtable G> 
    inline std::ostream& operator<< (std::ostream&  os, const G& val) {
      return val.output (os);
    }

    
    template<class Int, class Bool,class Operation>
    concept IntOperation_ = requires (Operation op, const Int&left,const Bool& right) {
      {op.template Not<Int> (left)} -> std::convertible_to<Int>;
      {op.template Add<Int> (left,left)} -> std::convertible_to<Int>;
      {op.template Sub<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template Mul<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template UDiv<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template SDiv<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template LShl<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template LShr<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template AShr<Int> (left,left)  } -> std::convertible_to<Int>;
      {op.template And<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template Or<Int> (left,left)  } -> std::convertible_to<Int>;
      {op.template Xor<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template SGt<Int> (left,left)} -> std::convertible_to<Bool>;
      {op.template SGe<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template SLt<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template SLe<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template UGt<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template UGe<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template ULt<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template ULe<Int> (left,left)  } -> std::convertible_to<Bool>;
      {op.template Eq<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template NEq<Int> (left,left)  } -> std::convertible_to<Bool>;
      {op.template BoolAnd (right,right)  } -> std::convertible_to<Bool>;
      {op.template BoolNegate (right)} -> std::convertible_to<Bool>;
      
    };

    template<class Value,class Operation>
    concept IntOperation =  (IntOperation_<typename Value::I8,typename Value::Bool,Operation> &&
				       IntOperation_<typename Value::I16,typename Value::Bool,Operation> &&
				       IntOperation_<typename Value::I32,typename Value::Bool,Operation> &&
				       IntOperation_<typename Value::I64,typename Value::Bool,Operation>
				       );
    
    template<class Int, class Pointer,class Bool,class Operation>
    concept PointerOperation_ = requires (Operation op, const Int&left, const Pointer& ptr) {
      {op.PtrAdd (ptr,left)} -> std::convertible_to<Pointer>;
      {op.PtrSub (ptr,left)} -> std::convertible_to<Pointer>;
    };

    template<class Value, class Operation>
    concept PointerOperation = (PointerOperation_<typename Value::I8,typename Value::Pointer,typename Value::Bool,Operation>&&
					  PointerOperation_<typename Value::I16,typename Value::Pointer,typename Value::Bool,Operation>&&
					  PointerOperation_<typename Value::I32,typename Value::Pointer,typename Value::Bool,Operation>&&
					  PointerOperation_<typename Value::I64,typename Value::Pointer,typename Value::Bool,Operation>
					   );
    
    template<class Int, class Aggregate,class Operation>
    concept AggregateOperation_ = requires (Operation op, const Aggregate& aggr, MiniMC::BV64 index, const Int& insertee,size_t s) {
      {op.template ExtractBaseValue<Int> (aggr,index)} -> std::convertible_to<Int>;
      {op.ExtractAggregateValue (aggr,index,s)} -> std::convertible_to<Aggregate>;
      {op.template InsertBaseValue<Int> (aggr,index, insertee)} -> std::convertible_to<Aggregate>;
      {op.InsertAggregateValue (aggr,index,aggr)} -> std::convertible_to<Aggregate>;
    };

    template<class Value,class Operation>
    concept AggregateOperation = (AggregateOperation_<typename Value::I8,typename Value::Aggregate,Operation> &&
				   AggregateOperation_<typename Value::I16,typename Value::Aggregate,Operation> &&
				   AggregateOperation_<typename Value::I32,typename Value::Aggregate,Operation> &&
				   AggregateOperation_<typename Value::I64,typename Value::Aggregate,Operation>
				   ) ;

    template<class I8, class I16,class I32,class I64, typename Bool, typename Pointer,class Pointer32,class Aggregate,class Caster>
    concept CastOperation_ = requires (Caster op, const I8& i8,const I16& i16, const I32& i32, const I64& i64, const Bool& b,  const Pointer& p, const Pointer32& p32, const Aggregate& aggr) {
      {op.template ZExt<MiniMC::Model::TypeID::I8> (i8)} -> std::convertible_to<I8>;
      {op.template ZExt<MiniMC::Model::TypeID::I16> (i8)} -> std::convertible_to<I16>;
      {op.template ZExt<MiniMC::Model::TypeID::I32> (i8)} -> std::convertible_to<I32>;
      {op.template ZExt<MiniMC::Model::TypeID::I64> (i8)} -> std::convertible_to<I64>;
      {op.template ZExt<MiniMC::Model::TypeID::I16> (i16)} -> std::convertible_to<I16>;
      {op.template ZExt<MiniMC::Model::TypeID::I32> (i16)} -> std::convertible_to<I32>;
      {op.template ZExt<MiniMC::Model::TypeID::I64> (i16)} -> std::convertible_to<I64>;
      {op.template ZExt<MiniMC::Model::TypeID::I32> (i32)} -> std::convertible_to<I32>;
      {op.template ZExt<MiniMC::Model::TypeID::I64> (i32)} -> std::convertible_to<I64>;
      {op.template ZExt<MiniMC::Model::TypeID::I64> (i64)} -> std::convertible_to<I64>;

      {op.template SExt<MiniMC::Model::TypeID::I8> (i8)} -> std::convertible_to<I8>;
      {op.template SExt<MiniMC::Model::TypeID::I16> (i8)} -> std::convertible_to<I16>;
      {op.template SExt<MiniMC::Model::TypeID::I32> (i8)} -> std::convertible_to<I32>;
      {op.template SExt<MiniMC::Model::TypeID::I64> (i8)} -> std::convertible_to<I64>;
      {op.template SExt<MiniMC::Model::TypeID::I16> (i16)} -> std::convertible_to<I16>;
      {op.template SExt<MiniMC::Model::TypeID::I32> (i16)} -> std::convertible_to<I32>;
      {op.template SExt<MiniMC::Model::TypeID::I64> (i16)} -> std::convertible_to<I64>;
      {op.template SExt<MiniMC::Model::TypeID::I32> (i32)} -> std::convertible_to<I32>;
      {op.template SExt<MiniMC::Model::TypeID::I64> (i32)} -> std::convertible_to<I64>;
      {op.template SExt<MiniMC::Model::TypeID::I64> (i64)} -> std::convertible_to<I64>;
      
      {op.template Trunc<MiniMC::Model::TypeID::I64> (i64)} -> std::convertible_to<I64>;
      {op.template Trunc<MiniMC::Model::TypeID::I32> (i64)} -> std::convertible_to<I32>;
      {op.template Trunc<MiniMC::Model::TypeID::I16> (i64)} -> std::convertible_to<I16>;
      {op.template Trunc<MiniMC::Model::TypeID::I8> (i64)} -> std::convertible_to<I8>;
      {op.template Trunc<MiniMC::Model::TypeID::I32> (i32)} -> std::convertible_to<I32>;
      {op.template Trunc<MiniMC::Model::TypeID::I16> (i32)} -> std::convertible_to<I16>;
      {op.template Trunc<MiniMC::Model::TypeID::I8> (i32)} -> std::convertible_to<I8>;
      {op.template Trunc<MiniMC::Model::TypeID::I16> (i16)} -> std::convertible_to<I16>;
      {op.template Trunc<MiniMC::Model::TypeID::I8> (i16)} -> std::convertible_to<I8>;
      {op.template Trunc<MiniMC::Model::TypeID::I8> (i8)} -> std::convertible_to<I8>;


      {op.template ZExt<MiniMC::Model::TypeID::I8> (b)} -> std::convertible_to<I8>;
      {op.template ZExt<MiniMC::Model::TypeID::I16> (b)} -> std::convertible_to<I16>;
      {op.template ZExt<MiniMC::Model::TypeID::I32> (b)} -> std::convertible_to<I32>;
      {op.template ZExt<MiniMC::Model::TypeID::I64> (b)} -> std::convertible_to<I64>;

      {op.template SExt<MiniMC::Model::TypeID::I8> (b)} -> std::convertible_to<I8>;
      {op.template SExt<MiniMC::Model::TypeID::I16> (b)} -> std::convertible_to<I16>;
      {op.template SExt<MiniMC::Model::TypeID::I32> (b)} -> std::convertible_to<I32>;
      {op.template SExt<MiniMC::Model::TypeID::I64> (b)} -> std::convertible_to<I64>;
      {op.template BoolNegate (b)} -> std::convertible_to<Bool>;
      {op.template IntToBool<I8> (i8)} -> std::convertible_to<Bool>;
      {op.template IntToBool<I16> (i16)} -> std::convertible_to<Bool>;
      {op.template IntToBool<I32> (i32)} -> std::convertible_to<Bool>;
      {op.template IntToBool<I64> (i64)} -> std::convertible_to<Bool>;
      
      
      {op.template PtrToPtr32 (p)} -> std::convertible_to<Pointer32>;
      {op.template Ptr32ToPtr (p32)} -> std::convertible_to<Pointer>;
      
      {op.template BitCast<Aggregate> (i8)} -> std::convertible_to<Aggregate>;
      {op.template BitCast<Aggregate> (i16)} -> std::convertible_to<Aggregate>;
      {op.template BitCast<Aggregate> (i32)} -> std::convertible_to<Aggregate>;
      {op.template BitCast<Pointer32> (i32)} -> std::convertible_to<Pointer32>;
      {op.template BitCast<Aggregate> (i64)} -> std::convertible_to<Aggregate>;
      {op.template BitCast<Pointer> (i64)} -> std::convertible_to<Pointer>;
      {op.template BitCast<Aggregate> (p32)} -> std::convertible_to<Aggregate>;
      {op.template BitCast<I32> (p32)} -> std::convertible_to<I32>;
      {op.template BitCast<I64> (p)} -> std::convertible_to<I64>;
      {op.template BitCast<I8> (aggr)} -> std::convertible_to<I8>;
      {op.template BitCast<I16> (aggr)} -> std::convertible_to<I16>;
      {op.template BitCast<I32> (aggr)} -> std::convertible_to<I32>;
      {op.template BitCast<I64> (aggr)} -> std::convertible_to<I64>;
      {op.template BitCast<Pointer32> (aggr)} -> std::convertible_to<Pointer32>;
      {op.template BitCast<Pointer> (aggr)} -> std::convertible_to<Pointer>;
      
      
    };

    template<class I8, class I16,class I32,class I64, typename Bool, typename Pointer,class Pointer32,class Aggregate,class Aux>
    concept AuxOperation_ = requires (Aux op, const I8& i8,const I16& i16, const I32& i32, const I64& i64,  const Pointer& p, const Pointer32& p32, const Aggregate& aggr,std::size_t offset,std::size_t by) {
      {op.bytes (i8)}->std::convertible_to<std::generator<I8>>;
      {op.bytes (i16)}->std::convertible_to<std::generator<I8>>;
      {op.bytes (i32)}->std::convertible_to<std::generator<I8>>;
      {op.bytes (i64)}->std::convertible_to<std::generator<I8>>;
      {op.bytes (p)}->std::convertible_to<std::generator<I8>>;
      {op.bytes (p32)}->std::convertible_to<std::generator<I8>>;
      {op.bytes (aggr)}->std::convertible_to<std::generator<I8>>;
      {op.extractbytes (aggr,offset,by)}->std::convertible_to<std::generator<I8>>;
    };

    template<class T,class Operation>
    concept AuxOperation = AuxOperation_<typename T::I8,typename T::I16, typename T::I32, typename T::I64,
					   typename T::Bool,typename T::Pointer,typename T::Pointer32,typename T::Aggregate,Operation>;
    
    
    template<class T,class Operation>
    concept CastOperation = CastOperation_<typename T::I8,typename T::I16, typename T::I32, typename T::I64,
					   typename T::Bool,typename T::Pointer,typename T::Pointer32,typename T::Aggregate,Operation>;
    

    template<class Creato,class Res>
    concept Creator = requires (const Creato e,
				const MiniMC::Model::I8Integer& i8,
				const MiniMC::Model::I16Integer& i16,
				const MiniMC::Model::I32Integer& i32,
				const MiniMC::Model::I64Integer& i64,
				const MiniMC::Model::Bool& b,
				const MiniMC::Model::Pointer& ptr,
				const MiniMC::Model::Pointer32& ptr32,
				const MiniMC::Model::AggregateConstant& aggrc,
				const MiniMC::Model::Undef& und,
 				const MiniMC::Model::Type& ty) {
      {e.create(i8)}->std::convertible_to<typename Res::I8>;
      {e.create(i16)}->std::convertible_to<typename Res::I16>;
      {e.create(i32)}->std::convertible_to<typename Res::I32>;
      {e.create(i64)}->std::convertible_to<typename Res::I64>;
      {e.create(b)}->std::convertible_to<typename Res::Bool>;
      {e.create(ptr)}->std::convertible_to<typename Res::Pointer>;
      {e.create(ptr32)}->std::convertible_to<typename Res::Pointer32>;
      {e.create(aggrc)}->std::convertible_to<typename Res::Aggregate>;
      {e.create(und)}->std::convertible_to<std::generator<Res>>;
      {e.defaultValue (ty)}->std::convertible_to<Res>;
    };

    template<class Mem,class T>
    concept MemoryOperations = requires (Mem& memc,
					 const typename T::Memory& mem, 
					 const typename T::Pointer& p,
					 const typename T::I8& i8,
					 const typename T::I16& i16,
					 const typename T::I32& i32,
					 const typename T::I64& i64,
					 const typename T::Aggregate& aggr,
					 const typename T::Pointer& ptr,
					 const typename T::Pointer32& ptr32,
					 const MiniMC::Model::Type&ty,
					 const std::size_t bytes
					 ) {
					   {memc.store (mem,p,i8)}->std::convertible_to<typename T::Memory>;
					   {memc.find_space(mem,i64)}->std::convertible_to<typename T::Pointer>;
					   {memc.allocate(mem,ptr,i64)}->std::convertible_to<typename T::Memory>;
					   {memc.free (mem,p)}->std::convertible_to<typename T::Memory>;
					   {memc.loadBytes (mem,p,bytes)}->std::convertible_to<std::generator<typename T::I8>>;
    };
    
    
    template<class Operation,class Value>
    concept Ops = CastOperation<Value,Operation> &&
                  IntOperation<Value,Operation> &&
                  PointerOperation<Value,Operation> &&
                  AggregateOperation<Value,Operation> &&
                  Creator<Operation,Value> &&
                  AuxOperation<Value,Operation>
      ;

     enum class Feasibility {
       Feasible,
       Infeasible,
       Unknown
     };
    
    template<class Sol,class Value>
    concept ConstraintSolver = requires (Sol sol, const Sol csol, typename Value::Bool r, const Value& v){
      {sol.addConstraint (r)};
      {sol.push ()};
      {sol.pop ()};
      {csol.check ()}->std::convertible_to<Feasibility>;
      {csol.eval (v)}->std::convertible_to<MiniMC::Model::Constant_ptr>;
    };

    struct SolverOptions {
      SolverOptions (SMTLib::Context_ptr context):context(std::move(context)) {}
      SolverOptions (const SolverOptions&) = default;
      SMTLib::Context_ptr context;
      
    };
    
    template<class Def>
    concept ValueDefinition = requires (const Def def,SolverOptions sol) {
      {def.ops ()}->Ops<typename Def::Val>;
      {def.memops ()}->MemoryOperations<typename Def::Val>;
      {def.solver ()}->ConstraintSolver<typename Def::Val>;
      {def.solver (sol)}->ConstraintSolver<typename Def::Val>;
      
    };
    
  }
  
} // namespace MiniMC

#endif
