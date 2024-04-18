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
    def __init__(self,name, operands = [],assign = False,tempcreate = None,vm = False ):
        self._name = name
        self._operands = operands
        self._assign = assign
        self._tempcreate = tempcreate
        self._vm = vm
        
    def getName (self):
        return self._name

    def getOperands (self):
        return self._operands

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
                temp_create = data.get("template_construction",None)
                vm = data.get("vm",False)
                instr.append (Instruction (opcode,ops,assign_convertible,temp_create,vm))
            groups.append (InstructionGroup (gname,instr))
        return ISA(groups)
    
