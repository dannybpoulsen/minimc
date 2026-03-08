import sys
import isa

import jinja2

env = jinja2.Environment (loader = jinja2.PackageLoader("isa2cpp"))

    

def writeInstructionConstructors (ISA,output):
    template = env.get_template ("instructions_constructors_inc.jinja")
    text = template.render (ISA = ISA )
    output.write (text)
    



def writeExpressions(ISA,output):
    
    bin_arit_ops_names = list ([i for g,i in ISA.getInstructionsWithGroupName () if g =="TAC" and i.isAssignConvertible()])
    bin_cmp_ops_names = list ([i for g,i in ISA.getInstructionsWithGroupName () if g =="Comparison" and i.isAssignConvertible()])
    all_bin_ops_names = bin_arit_ops_names+bin_cmp_ops_names
    unary_ops_names = list ([i for g,i in ISA.getInstructionsWithGroupName () if g =="Unary" and i.isAssignConvertible()])
    cast_ops_names = list ([i for g,i in ISA.getInstructionsWithGroupName () if g =="Cast" and i.isAssignConvertible()])
    pointer_ops_names = list ([i for g,i in ISA.getInstructionsWithGroupName () if g =="Pointer" and i.isAssignConvertible()])
    aggr_ops_names = list ([i for g,i in ISA.getInstructionsWithGroupName () if g =="Aggregate" and i.isAssignConvertible()])
    
    
    template = env.get_template ("expr_inc.jinja")
    plain_non_generated_names = [
        "I8Integer",
        "I16Integer",
        "I32Integer",
        "I64Integer",
        "Bool",
        "Pointer",
        "Pointer32",
        "AggregateConstant",
        "Register",
        "Undef",
        "SymbolicConstant",
        "LoadExpr",
        "StoreExpr",
        "Ptr32ToPtrExpr",
        "PtrToPtr32Expr"
    ]
    output.write(template.render (ISA=ISA,plain_non_generated_names=plain_non_generated_names)) 
    
    
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
