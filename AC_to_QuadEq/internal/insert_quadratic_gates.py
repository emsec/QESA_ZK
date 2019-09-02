from internal.circuit import *
from internal.arith import *

def var(x):
    return "_"+str(x)+"_"

def rec_gen_equation(equations, to_remove, wire_src, g):
    #print(g)
    if g in equations:
        return equations[g]
    to_remove.add(g)

    if g.inputs[0] in wire_src:
        lhs = rec_gen_equation(equations, to_remove, wire_src, wire_src[g.inputs[0]])
    else:
        lhs = ext_sum(element(1, var(g.inputs[0])))

    if g.name == "add":
        if g.inputs[1] in wire_src:
            rhs = rec_gen_equation(equations, to_remove, wire_src, wire_src[g.inputs[1]])
        else:
            rhs = ext_sum(element(1, var(g.inputs[1])))

        equations[g] = lhs.add(rhs)

    elif g.name == "mul":
        if g.inputs[1] in wire_src:
            rhs = rec_gen_equation(equations, to_remove, wire_src, wire_src[g.inputs[1]])
        else:
            rhs = ext_sum(element(1, var(g.inputs[1])))

        equations[g] = expr(lhs, rhs)


    elif g.name.startswith("const-mul"):
        rhs = int(g.name[g.name.rfind("-")+1:],16)
        if g.name.startswith("const-mul-neg"): rhs = -rhs

        equations[g] = lhs.mul(rhs)


    else:
        print("unknown gate: ")
        print(g)
        return None

    return equations[g]

def process(circuit):
    quad_gates = list()
    to_remove = set()

    xor_gates = [g for g in circuit.gates if g.name == "xor"]
    for ctr, xor_gate in enumerate(xor_gates):
        print("\r    xor gates "+str(ctr)+"/"+str(len(xor_gates)), end ="")
        to_remove.add(xor_gate)

        quad_gate = Gate()
        quad_gate.name = "quad_gate"
        quad_gate.comment = "(1-_"+str(xor_gate.inputs[0])+"_)*_"+str(xor_gate.inputs[1])+"_+(1-_"+str(xor_gate.inputs[1])+"_)*_"+str(xor_gate.inputs[0])+"_"
        quad_gate.outputs = xor_gate.outputs
        quad_gate.inputs = xor_gate.inputs
        quad_gates.append(quad_gate)
    print("\r"+" "*80+"\r", end ="")

    circuit.gates = [x for x in circuit.gates if x not in to_remove]
    to_remove.clear()

    candidates = [g for g in circuit.gates if g.name.startswith(("add","sub","const-mul"))]
    wire_src = dict()
    for g in candidates:
        for i in g.outputs:
            wire_src[i] = g

    # for x in wire_src:
    #     print(x)
    #     for g in wire_src[x]:
    #         print("  "+str(g))

    equations = dict()
    variables = dict()
    for i in circuit.inputs:
        variables[i] = "_"+str(i)+"_" #Symbol("_"+str(i)+"_")

    mul_gates = [g for g in circuit.gates if g.name == "mul"]
    for ctr, mul_gate in enumerate(mul_gates):
        print("\r    mul gates "+str(ctr)+"/"+str(len(mul_gates)), end ="")

        quad_gate = Gate()
        quad_gate.name = "quad_gate"
        quad_gate.comment = str(rec_gen_equation(equations, to_remove, wire_src, mul_gate))
        quad_gate.outputs = list(mul_gate.outputs)
        func = quad_gate.comment
        while "_" in func:
            func = func[func.find("_")+1:]
            quad_gate.inputs.append(int(func[:func.find("_")]))
            func = func[func.find("_")+1:]
        quad_gate.inputs = sorted(list(set(quad_gate.inputs)))

        quad_gates.append(quad_gate)

    print("\r"+" "*80+"\r", end ="")

    circuit.gates = [x for x in circuit.gates if x not in to_remove] + quad_gates
