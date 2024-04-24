import sys
import isa

import jinja2

env = jinja2.Environment (loader = jinja2.PackageLoader("typecheckgen"))

def write_header(ISA,output):
    template = env.get_template ("typechecker_hpp.jinja")
    output.write(template.render (ISA=ISA)) 

def write_cpp(ISA,output):
    template = env.get_template ("typechecker_cpp.jinja")
    output.write(template.render (ISA=ISA)) 

    
inp = sys.argv[1]
ISA = isa.readISA (inp)

with open(sys.argv[2],'w') as output:
    write_header (ISA,output)

with open(sys.argv[3],'w') as output:
    write_cpp (ISA,output)
