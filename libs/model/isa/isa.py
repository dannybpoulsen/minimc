import pyparsing as pp

class SizeRestriction:
    def __init__(self,left,right, _cmp="<="):
        self._left = left
        self._right = right
        self._cmp = _cmp
        
    def getLesser (self):
        return self._left

    def getGreater (self):
        return self._right

    def getCMP (self):
        return self._cmp

class TypeDescriptor:
    def __init__(self,name,type_d):
        self._name = name
        self._type = type_d
        self._operands = []
        
    def getName (self):
        return self._name

    def getTypeD (self):
        return self._type

    def addOperand(self,op):
        self._operands.append(op)
    
    def operands (self):
        return self._operands

    def NonResoperands (self):
        return list([i for i in self._operands if i.getName()!="res"])

    def Resoperands (self):
        return list([i for i in self._operands if i.getName()=="res"])
    

class Operand:
    def __init__ (self,name, ty,multi = False):
        self._name = name
        self._multi = multi
        self._type = ty 

    def getName (self):
        return self._name

    def isMultiParam (self):
        return self._multi

    def getType(self):
        return self._type
    
    def __str__ (self):
        str = ""
        if self._multi:
            str="*"
        return f"{self._name}{str}"
    
class Instruction:
    def __init__(self,name, operands = [],assign = False,tempcreate = None,vm = False,typedescriptors = {},sizes = None, tempreplace = None):
        self._name = name
        self._operands = operands
        self._assign = assign
        self._tempcreate = tempcreate
        self._vm = vm
        self._types = typedescriptors 
        self._sizes = sizes
        self._tempreplace = tempreplace
        
    def getName (self):
        return self._name

    def getOperands (self):
        return self._operands

    def getNonResOperands (self):
        return list([i for i in self._operands if i.getName() != "res"])

    def getResOperand (self):
        for i in self._operands:
            if i.getName() == "res":
                return i
        return None
        
    
    def getTypes (self):
        return self._types

    def getTypeSizeRestrictions(self):
        return self._sizes 

    def temp_replace(self):
        return self._tempreplace
    
    def isAssignConvertible (self):
        return self._assign

    def temp_creation (self):
        return self._tempcreate

    def isVM (self):
        return self._vm
    
    def __str__ (self):
        l = ",".join ([str(o) for o in self._operands])
        return f"{self._name} {l}"

class InstructionGroup:
    def __init__(self,name,instructions):
        self._name = name
        self._instructions = instructions

    def getName (self):
        return self._name

    def getInstructions (self):
        return self._instructions

    def getVMInstructions (self):
        yield from [j for j in self.getInstructions () if  j.isVM ()]
    
    def getPseudoInstructions (self):
        yield from [j for j in self.getInstructions () if not  j.isVM ()]
    
        
    
class ISA:
    def __init__ (self,instrgroups = []):
        self._groups = instrgroups

    def getGroups (self):
        return self._groups

    def getInstructions (self):
        for i in self._groups:
            yield from i.getInstructions ()

    def getInstructionsWithGroupName (self):
        for i in self._groups:
            yield from [(i.getName(),j) for j in i.getInstructions ()]

    def getNonAssignableInstructions (self):
        for i in self._groups:
            yield from [j for j in i.getInstructions () if not j.isAssignConvertible ()]
    
    def getVMInstructions (self):
        for i in self._groups:
            yield from  i.getVMInstructions () 
    
    def getPseudoInstructions (self):
        for i in self._groups:
            yield from [j for j in i.getInstructions () if not  j.isVM ()]

    def getPseudoGroupedInstructions (self):
        for i in self._groups:
            yield from [(i,j) for j in i.getInstructions () if not  j.isVM ()]
    

def parseSizeConstraints (inp,types):
    BW = (pp.Word ("BW") + pp.Literal ("(") + pp.Word(pp.alphanums) + pp.Literal (")")).set_parse_action (lambda toks: types[toks[2]])
    greater_eq  = (BW + pp.Literal (">=") + BW).set_parse_action (lambda toks:  SizeRestriction (toks[0],toks[2],toks[1]))
    lesser_eq  = (BW + pp.Literal ("<=") + BW).set_parse_action (lambda toks:  SizeRestriction (toks[0],toks[2],toks[1]))
    greater  = (BW + pp.Literal (">") + BW).set_parse_action (lambda toks:  SizeRestriction (toks[0],toks[2],toks[1]))
    lesser  = (BW + pp.Literal ("<") + BW).set_parse_action (lambda toks:  SizeRestriction (toks[0],toks[2],toks[1]))
    eq  = (BW + pp.Literal ("==") + BW).set_parse_action (lambda toks:  SizeRestriction (toks[0],toks[2],toks[1]))

    parser = greater_eq | lesser_eq | greater | lesser | eq
    
    return parser.parse_string (inp)[0]
                
            
            
def readISA (path):
    
    import yaml
    with open(path) as ff:
        groups = []
        datat = yaml.load (ff.read (), Loader = yaml.Loader)
        for gname, instructions in datat.items ():
            instr = []
            for i,data in instructions.items():
                opcode = data["opcode"]
                ops = []
                types = {}
                if "types" in data:
                    for k,t in data["types"].items():
                        types[k] = TypeDescriptor (k,t)
                for p in data["params"]:
                    name = p["name"]
                    
                    t = p["type"]
                    mname = name.replace("*","")
                    op = Operand(mname,types[t],"*" in name)
                    types[t].addOperand(op)
                    ops.append (op)
                size = None
                if "size_constraints" in data:
                    size = parseSizeConstraints (data["size_constraints"],types)
                assign_convertible = data.get("assign_convertible",False)
                temp_create = data.get("template_construction",None)
                temp_replace = data.get("replace_construction",None)
                vm = data.get("vm",False)
                instr.append (Instruction (opcode,ops,assign_convertible,temp_create,vm,types,size,temp_replace))
            groups.append (InstructionGroup (gname,instr))
        return ISA(groups)
    
