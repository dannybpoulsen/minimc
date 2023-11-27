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
        params = []
        for j,o in enumerate(i.getOperands ()):
            if not o. isMultiParam ():
                params.append (f"values.at({j})")
            else:
                params.append (f"std::vector<Value_ptr> {{values.begin()+{j},values.end ()}}")
        output.write (f"case InstructionCode::{i.getName ()}: return Instruction::make<InstructionCode::{i.getName ()}> ({','.join (params)});\n")
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
    
    
inp = sys.argv[1]
ISA = isa.readISA (inp)
with open(sys.argv[2],'w') as output:
    writeEnumDeclaration (ISA,output)
    writeInstructionData( ISA,output)
    writeFooter (ISA,output)
    
with open(sys.argv[3],'w') as output:
    writeInstructionConstructors (ISA,output)
    
