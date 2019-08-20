from circuit import *

def parse_arith_circuit(path):
    with open(path, "rt") as file:
        lines = file.readlines()

    c = ArithCircuit()

    for line in lines:
        line = line.strip()
        comment = ""
        if "#" in line:
            comment = line[line.find("#")+1:].strip()
            line = line[:line.find("#")].strip()
        if line.startswith("total "): pass
        elif line.startswith("input "):
            c.inputs.append(int(line.split()[1]))
            c.input_comments.append(comment)
        elif line.startswith("output "): c.outputs.append(int(line.split()[1]))
        else:
            g = Gate()
            p = line.find(">") + 1
            g.name = line.split()[0]
            g.inputs = [int(x) for x in line[line.find("<")+1:p-1].split()]
            g.outputs = [int(x) for x in line[line.find("<", p)+1:line.find(">", p)].split()]
            g.comment = comment
            c.gates.append(g)

    return c
