class Operand:
    def __init__ (self,name, multi = False):
        self._name = name
        self._multi = multi

    def getName (self):
        return self._name

    def isMultiParam (self):
        return self._multi

    def __str__ (self):
        str = ""
        if self._multi:
            str="*"
        return f"{self._name}{str}"
    
class Instruction:
    def __init__(self,name, operands = [],assign = False):
        self._name = name
        self._operands = operands
        self._assign = assign
        
    def getName (self):
        return self._name

    def getOperands (self):
        return self._operands

    def isAssignConvertible (self):
        return self._assign
    
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
                for p in data["params"]:
                    name = p["name"]
                    mname = name.replace("*","")
                    ops.append (Operand(mname,"*" in name))
                assign_convertible = data.get("assign_convertible",False)
                instr.append (Instruction (opcode,ops,assign_convertible))
            groups.append (InstructionGroup (gname,instr))
        return ISA(groups)
    
