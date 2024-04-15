import sys
import isa

import jinja2

env = jinja2.Environment (loader = jinja2.PackageLoader("isa2cpp"))

    

def writeInstructionConstructors (ISA,output):
    template = env.get_template ("instructions_constructors_inc.jinja")
    text = template.render (ISA = ISA )
    output.write (text)
    #output.write ("Instruction makeInstruction (InstructionCode code, std::vector<Value_ptr> values) {\n  switch(code) {")
    
    #for i in ISA.getInstructions  ():
    #    output.write (f"case InstructionCode::{i.getName ()}: {{")
    #    if i.isAssignConvertible ():
    #        output.write ("auto res = *values.begin ();")
    #        params = []
    #        for j,o in enumerate(i.getOperands ()):
    #            if o.getName () != "res":
    #                params.append (f"values.at ({j})")
    #        
    #        output.write (f"auto assigned = std::make_shared<MiniMC::Model::{i.getName()}Expr> ({','.join (params)});")
    #        output.write ("return Instruction::make<InstructionCode::Assign> (res,assigned);}")
    #    else:
    #        params = []
    #        for j,o in enumerate(i.getOperands ()):
    #            if not o.isMultiParam ():
    #                params.append (f"values.at({j})")
    #            else:
    #                params.append (f"std::vector<Value_ptr> {{values.begin()+{j},values.end ()}}")
    #        output.write (f"return Instruction::make<InstructionCode::{i.getName ()}> ({','.join (params)});\n}}")
    #output.write ("default: std::unreachable ();");
    #output.write ("}\n}")
    



def writeExpressions(ISA,output):
    
    bin_arit_ops_names = list ([i.getName () for g,i in ISA.getInstructionsWithGroupName () if g =="TAC" and i.isAssignConvertible()])
    bin_cmp_ops_names = list ([i.getName () for g,i in ISA.getInstructionsWithGroupName () if g =="Comparison" and i.isAssignConvertible()])
    all_bin_ops_names = bin_arit_ops_names+bin_cmp_ops_names
    unary_ops_names = list ([i.getName () for g,i in ISA.getInstructionsWithGroupName () if g =="Unary" and i.isAssignConvertible()])
    
    
    template = env.get_template ("expr_inc.jinja")
    output.write(template.render (all_bin_ops = all_bin_ops_names,arit_ops = bin_arit_ops_names,cmp_ops = bin_cmp_ops_names,all_unary_ops = unary_ops_names )) 
    
    
inp = sys.argv[1]
ISA = isa.readISA (inp)
with open(sys.argv[2],'w') as output:
    template = env.get_template ("instructions_inc.jinja")
    text = template.render (ISA = ISA )
    output.write (text)

    
with open(sys.argv[3],'w') as output:
    writeInstructionConstructors (ISA,output)

with open(sys.argv[4],'w') as output:    
    writeExpressions(ISA,output)
