import sys
import isa


def writeEnumDeclaration (ISA,output):
    output.write ("#include <utility>\n enum class InstructionCode {\n")
    output.write (",\n".join ([f"  {i.getName ()}" for i in ISA.getInstructions ()]))
    output.write ("\n};\n")
    output.write ("using ReplaceFunction = std::function<Value_ptr(const Value_ptr&)>;\n");
    
    
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
                
                output.write ("};")
            output.write (f"\n}};\n\n")
                        
    #output.write (",".join (instrgroups))

def writeFooter (ISA,output):
    output.write ('''template<InstructionCode opc>
    concept hasOperands =  requires  {
      typename InstructionData<opc>::Content;
    };
    
    template<InstructionCode>
    struct TInstruction;
    
    template<InstructionCode opc> requires hasOperands<opc>
    struct TInstruction<opc> {
      TInstruction (typename InstructionData<opc>::Content content ) :  content(content) {}
      auto& getOps () const {return content;}
      static consteval auto getOpcode () {return opc;}
    private:
      typename InstructionData<opc>::Content content;
    };
    
    template<InstructionCode opc> requires (!hasOperands<opc>)
    struct TInstruction<opc> {
      TInstruction ()  {}
      static consteval auto getOpcode () {return opc;}
    };''')

    instrline = ",\n".join ([f"TInstruction<InstructionCode::{i.getName ()}>" for i in ISA.getInstructions ()])
    output.write (f"using Instruction_internal = std::variant<{instrline}>;")

    retline = ";\n".join ([f' case InstructionCode::{i.getName()}: return os << "{i.getName ()}";' for i in ISA.getInstructions ()])  
    output.write (f"inline std::ostream& operator<< (std::ostream& os, InstructionCode oc) {{ switch (oc) {{ {retline} \n default: std::unreachable ();  }} }}")  
    
inp = sys.argv[1]

with open(sys.argv[2],'w') as output:
    ISA = isa.readISA (inp)
    writeEnumDeclaration (ISA,output)
    writeInstructionData( ISA,output)
    writeFooter (ISA,output)
