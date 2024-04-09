import sys
import isa


def writeEnumDeclaration (ISA,output):
    output.write ("#include <utility>\n enum class InstructionCode {\n")
    output.write (",\n".join ([f"  {i.getName ()}" for i in ISA.getInstructions ()]))
    output.write ("\n};\n")
    output.write ("using ReplaceFunction = std::function<Value_ptr(const Value_ptr&)>;\n");


def writeInstructionConstructors (ISA,output):
    output.write ("Instruction makeInstruction (InstructionCode code, std::vector<Value_ptr> values) {\n  switch(code) {")
    
    for i in ISA.getInstructions ():
        output.write (f"case InstructionCode::{i.getName ()}: {{")
        if i.isAssignConvertible ():
            output.write ("auto res = *values.begin ();")
            params = []
            for j,o in enumerate(i.getOperands ()):
                if o.getName () != "res":
                    params.append (f"values.at ({j})")
            
            output.write (f"auto assigned = std::make_shared<MiniMC::Model::{i.getName()}Expr> ({','.join (params)});")
            output.write ("return Instruction::make<InstructionCode::Assign> (res,assigned);}")
        else:
            params = []
            for j,o in enumerate(i.getOperands ()):
                if not o.isMultiParam ():
                    params.append (f"values.at({j})")
                else:
                    params.append (f"std::vector<Value_ptr> {{values.begin()+{j},values.end ()}}")
            output.write (f"return Instruction::make<InstructionCode::{i.getName ()}> ({','.join (params)});\n}}")
    output.write ("default: std::unreachable ();");
    output.write ("}\n}")
    
def writeInstructionData (ISA,output):
    instrgroups = list([g.getName () for g in ISA.getGroups ()])
    output.write ("template<InstructionCode> struct InstructionData {};\n")

    for group in (ISA.getGroups ()):
        header = "\n".join (f"  static const bool is{groupn} = {'true' if groupn == group.getName () else 'false'};" for groupn in instrgroups)
        #output.write (header)

        for i in group.getInstructions ():
            output.write (f"template<> \nstruct InstructionData<InstructionCode::{i.getName ()}> {{\n")
            output.write (header)

            if len(i.getOperands ()) > 0:
                output.write ("\n  struct Content {")
                ops = []
                for o in i.getOperands ():
                    if not o.isMultiParam  ():
                        ops.append ( ("Value_ptr", o.getName ()))
                    else:
                        ops.append ( ("std::vector<Value_ptr>", o.getName ()))
                #Constructor
                paramline = ",".join ([f"{t[0]} {t[1]}" for t in ops])
                instantline = ",".join ([f"{t[1]}(std::move({t[1]}))" for t in ops])
                attribute = "\n".join ([f"    {t[0]} {t[1]};" for t in ops])
                
                output.write (f"Content ({paramline}) : {instantline} {{}} \n{attribute} \n")

                #Copy COnstructor
                instantline = ",".join ([f"{t[1]}(replacer(oth.{t[1]}))" for t in ops if not "std::vector" in t[0] ])
                output.write (f"Content (const Content& oth, ReplaceFunction replacer ) : {instantline} {{")

                for o in ops:
                    if "std::vector"  in o[0]:
                        output.write (f"std::for_each (oth.{o[1]}.begin (),oth.{o[1]}.end (),[replacer,this](auto& v) {{ {o[1]}.push_back (replacer(v));}});")
                    
                        
                output.write (f"}}")

                output.write ("template<class Adder>")
                output.write ("void getUsages (Adder add) const {")
                for o in ops:
                    if o[1]  != "res":
                        if "std::vector"  in o[0]:
                            output.write (f"std::for_each ({o[1]}.begin (),{o[1]}.end (),[&add](auto& v) {{ add = v;}});")
                        else:
                            output.write  (f"add = {o[1]};")

                
            
                output.write ("}")

                
                output.write ("};")
            output.write (f"\n}};\n\n")
                        
    #output.write (",".join (instrgroups))

def writeFooter (ISA,output):
    output.write ('''template<InstructionCode opc>
    concept hasOperands =  requires  {
      typename InstructionData<opc>::Content;
    };

    template<InstructionCode opc>
    concept hasRes =  requires (const typename InstructionData<opc>::Content& c) {
      c.res;
    };
    
    
    template<InstructionCode>
    struct TInstruction;
    
    template<InstructionCode opc> requires hasOperands<opc>
    struct TInstruction<opc> {
      TInstruction (typename InstructionData<opc>::Content content ) :  content(content) {}
      auto& getOps () const {return content;}
      static consteval auto getOpcode () {return opc;}
      Value_ptr getDefines () const requires hasRes<opc> {return content.res;} 
      template<class Adder>
      void getUsages (Adder add) const {content.getUsages (add);}
      static constexpr bool hasOperands () {return true;}
      static constexpr bool hasResult () requires hasRes<opc> {
        return true;
      }
      static constexpr bool hasResult () requires (!hasRes<opc>) {
        return false;
      }
    
    private:
      typename InstructionData<opc>::Content content;
    };
    
    template<InstructionCode opc> requires (!hasOperands<opc>)
    struct TInstruction<opc> {
      TInstruction ()  {}
      static constexpr auto getOpcode () {return opc;}
      static constexpr bool hasOperands () {return false;}
      static constexpr bool hasRes () {return false;}
    };''')
    
    instrline = ",\n".join ([f"TInstruction<InstructionCode::{i.getName ()}>" for i in ISA.getInstructions ()])
    output.write (f"using Instruction_internal = std::variant<{instrline}>;")

    retline = ";\n".join ([f' case InstructionCode::{i.getName()}: return os << "{i.getName ()}";' for i in ISA.getInstructions ()])  
    output.write (f"inline std::ostream& operator<< (std::ostream& os, InstructionCode oc) {{ switch (oc) {{ {retline} \n default: std::unreachable ();  }} }}")  
    
    map_line = ",\n".join ([f'{{"{i.getName ()}",MiniMC::Model::InstructionCode::{i.getName ()}}}' for i in ISA.getInstructions ()])
    output.write (f"const std::unordered_map<std::string,MiniMC::Model::InstructionCode> str2opcode = {{ {map_line} }};")

    retline = ";\n".join ([f' case InstructionCode::{i.getName()}: return InstructionData<InstructionCode::{i.getName()}>::isCast;' for i in ISA.getInstructions ()])  
    output.write (f"inline bool isCast (InstructionCode oc) {{ switch (oc) {{ {retline} \n default: std::unreachable ();  }} }}")  
    

def writeExpressions(ISA,output):
    bin_arit_ops_names = list ([i.getName () for g,i in ISA.getInstructionsWithGroupName () if g =="TAC" and i.isAssignConvertible()])
    bin_cmp_ops_names = list ([i.getName () for g,i in ISA.getInstructionsWithGroupName () if g =="Comparison" and i.isAssignConvertible()])
    all_bin_ops_names = bin_arit_ops_names+bin_cmp_ops_names
    output.write (f'''
    

    enum class BinOps {{
    {",".join (bin_arit_ops_names+bin_cmp_ops_names)}
    }};

    template<BinOps>
    struct BinOpsDetails {{}};

    {"".join ([f'template<> struct BinOpsDetails<BinOps::{N}> {{ static consteval auto text () {{return "{N}";}} }};' for N in all_bin_ops_names])}
    
    template<BinOps op>
    class BinaryExpression : public Value{{
    public:
      BinaryExpression (Value_ptr l, Value_ptr r);

      auto& getLeft () {{
        return *left;
      }}

      auto& getRight () {{
	return *right;
      }}
      
      const auto& getLeft () const{{
	return *left;
      }}

      const auto& getRight () const {{
	return *right;
      }}
      
      std::ostream& output(std::ostream& os) const override {{
	return os << "(" <<BinOpsDetails<op>::text()   << *left << " " << *right <<  ")";
      }}
      
    private:
      Value_ptr left;
      Value_ptr right;
    }};
    
    using Bool = TConstant<MiniMC::BV8, true>;
    using I8Integer = TConstant<MiniMC::BV8>;
    using I16Integer = TConstant<MiniMC::BV16>;
    using I32Integer = TConstant<MiniMC::BV32>;
    using I64Integer = TConstant<MiniMC::BV64>;
    using Pointer = TConstant<MiniMC::Model::pointer64_t>;
    using Pointer32 = TConstant<MiniMC::Model::pointer32_t>;
    using SymbolicConstant = TConstant<MiniMC::Model::Symbol>;


   
    {";".join ([f"using {N}Expr = BinaryExpression<BinOps::{N}>;" for N in bin_arit_ops_names+bin_cmp_ops_names])} 
    

    
    template<type_id_t i , class T, class F, class... Args>
    constexpr auto  auto_index ()  {{
      if constexpr ( std::is_same_v<T,F>) {{
	return i;
      }}
      else {{
	return auto_index<i+1,T,Args...> ();
      }}
    }}
    
    template<type_id_t i , class T, class F>
    constexpr auto  auto_index ()  {{
      if constexpr ( std::is_same_v<T,F>) {{
	return i;
      }}
      else {{
	return []<bool b = false> {{static_assert (b);}}();
      }}
    }}
    
    template <class T>
    struct ValueInfo {{
      static constexpr auto type_t () {{
	return auto_index<0,T,
			  I8Integer,
			  I16Integer,
			  I32Integer,
			  I64Integer,
			  Bool,
			  Pointer,
			  Pointer32, 
			  AggregateConstant,
			  Register,
			  Undef,
			  SymbolicConstant,
                          {",".join ([f"{N}Expr" for N in bin_arit_ops_names+bin_cmp_ops_names])}

			  > ();
      }}
    }};
    
    
    template <class T, bool is_bool>
    inline TConstant<T, is_bool>::TConstant(T val) : Constant(ValueInfo<TConstant<T, is_bool>>::type_t()),
                                                     value(val) {{
    }}

    template <BinOps b>
    inline BinaryExpression<b>::BinaryExpression(Value_ptr left, Value_ptr right) : Value(ValueInfo<BinaryExpression<b>>::type_t()),
										    left(left),right(right) {{
    }}

    template<class A>
    constexpr bool is_root = std::is_same_v<std::remove_const_t<A>,Bool>
      || std::is_same_v<std::remove_const_t<A>,I8Integer>
      || std::is_same_v<std::remove_const_t<A>,I16Integer>
      || std::is_same_v<std::remove_const_t<A>,I32Integer>
      || std::is_same_v<std::remove_const_t<A>,I64Integer>
      || std::is_same_v<std::remove_const_t<A>,Pointer>
      || std::is_same_v<std::remove_const_t<A>,Pointer32>
      || std::is_same_v<std::remove_const_t<A>,SymbolicConstant>
      || std::is_same_v<std::remove_const_t<A>,Register>
      || std::is_same_v<std::remove_const_t<A>,AggregateConstant>      
      || std::is_same_v<std::remove_const_t<A>,Undef>      
      ;

    template<class A>
    constexpr bool is_bin_arith = 
      {"||".join (["false"]+[f"std::is_same_v<std::remove_const_t<A>,{N}Expr>" for N in bin_arit_ops_names])}
    ;

    template<class A>
    constexpr bool is_bin_cmp = 
      {"||".join (["false"]+[f"std::is_same_v<std::remove_const_t<A>,{N}Expr>" for N in bin_cmp_ops_names])}
    ;
    
    template<class... Ts> struct Overload : Ts... {{
      using Ts::operator()...;

    }};

    template<class T,bool consts>
    using CType = typename std::conditional<consts,const T,T>::type; 
    
    template<class E,class Res = void,bool cconsts =true>
    concept TValueVisitator = requires(const E e,
				    CType<MiniMC::Model::I8Integer,cconsts>& i8,
				    CType<MiniMC::Model::I16Integer,cconsts> i16,
				    CType<MiniMC::Model::I32Integer,cconsts> i32,
				    CType<MiniMC::Model::I64Integer,cconsts> i64,
				    CType<MiniMC::Model::Bool,cconsts> b,
				    CType<MiniMC::Model::Pointer,cconsts> ptr,
				    CType<MiniMC::Model::Pointer32,cconsts> ptr32,
				    CType<MiniMC::Model::AggregateConstant,cconsts> aggrc,
				    CType<MiniMC::Model::Register,cconsts> reg,
				    CType<MiniMC::Model::Undef,cconsts> und,
				    CType<MiniMC::Model::SymbolicConstant,cconsts> sc,
                                   {",".join ([f"CType<MiniMC::Model::{N}Expr,cconsts> o{i}" for i,N in enumerate(all_bin_ops_names)])}
    ) {{
      {{e(i8)}}->std::convertible_to<Res>;
      {{e(i16)}}->std::convertible_to<Res>;
      {{e(i32)}}->std::convertible_to<Res>;
      {{e(i64)}}->std::convertible_to<Res>;
      {{e(b)}}->std::convertible_to<Res>;
      {{e(ptr)}}->std::convertible_to<Res>;
      {{e(ptr32)}}->std::convertible_to<Res>;
      {{e(aggrc)}}->std::convertible_to<Res>;
      {{e(reg)}}->std::convertible_to<Res>;
      {{e(und)}}->std::convertible_to<Res>;
      {{e(sc)}}->std::convertible_to<Res>;
    {"".join ([f"{{e(o{i})}}->std::convertible_to<Res>;" for i,N in enumerate(all_bin_ops_names)])}
    
    }};

    template<class E,class Res = void>
    concept ValueVisitator = TValueVisitator<E,Res,true>;

    template<class E,class Res = void>
    concept MutableValueVisitator = TValueVisitator<E,Res,false>;
    
    
    template <class Res,bool c, TValueVisitator<Res,c> F>
    auto visitValueT(F&& f,CType<MiniMC::Model::Value,c>& v)  {{
      switch (v.type_t ()) {{
      case ValueInfo<I8Integer>::type_t ():
	return f (static_cast<CType<I8Integer,c>&> (v));
	break;
      case ValueInfo<I16Integer>::type_t ():
	return f (static_cast<CType<I16Integer,c>&> (v));
	break;
      case ValueInfo<I32Integer>::type_t ():
	return f (static_cast<CType<I32Integer,c>&> (v));
	break;
      case ValueInfo<I64Integer>::type_t ():
	return f (static_cast<CType<I64Integer,c>&> (v));
	break;
      case ValueInfo<Bool>::type_t ():
	return f (static_cast<CType<Bool,c>&> (v));
	break;
      case ValueInfo<Pointer>::type_t ():
	return f (static_cast<CType<Pointer,c>&> (v));
	break;
      case ValueInfo<Pointer32>::type_t ():
	return f (static_cast<CType<Pointer32,c>&> (v));
	break;
      case ValueInfo<AggregateConstant>::type_t ():
	return f (static_cast<CType<AggregateConstant,c>&> (v));
	break;
      case ValueInfo<Register>::type_t ():
	return f (static_cast<CType<Register,c>&> (v));
	break;
      case ValueInfo<Undef>::type_t ():
	return f (static_cast<CType<Undef,c>&> (v));
	break;
      case ValueInfo<SymbolicConstant>::type_t ():
	return f (static_cast<CType<SymbolicConstant,c>&> (v));
	break;
       {"".join ([f"case ValueInfo<{N}Expr>::type_t (): return f (static_cast<CType<{N}Expr,c>&> (v)); break;" for N in all_bin_ops_names])}
    
        default:
#ifdef NDEBUG
	__builtin_unreachable();
#else
	throw MiniMC::Support::Exception ("Missing category");
#endif
        }}
      
    }}

    template <class Res,ValueVisitator<Res> F>
    auto visitValue(F&& f,const MiniMC::Model::Value& v) {{
      return visitValueT<Res,true,F>(std::forward<F>(f),v);
    }}

    template <class Res,MutableValueVisitator<Res> F>
    auto visitValue(F&& f,MiniMC::Model::Value& v) {{
      return visitValueT<Res,false,F>(std::forward<F>(f),v);
    }}

    
    
''')

    
inp = sys.argv[1]
ISA = isa.readISA (inp)
with open(sys.argv[2],'w') as output:
    writeEnumDeclaration (ISA,output)
    writeInstructionData( ISA,output)
    writeFooter (ISA,output)
    
with open(sys.argv[3],'w') as output:
    writeInstructionConstructors (ISA,output)

with open(sys.argv[4],'w') as output:    
    writeExpressions(ISA,output)
