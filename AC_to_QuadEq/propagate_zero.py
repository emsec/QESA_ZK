from circuit import *
from sympy import *
from sympy.parsing.sympy_parser import parse_expr

def process(circuit):
    while True:
        quad_zero_gates = [x for x in circuit.gates if x.name == "quad_gate" and x.comment == "0"]
        if len(quad_zero_gates) == 0: break
        recalculate = set()
        for ctr, qg in enumerate(quad_zero_gates):
            print("\r    remove 0 gates "+str(ctr)+"/"+str(len(quad_zero_gates)), end ="")
            for g in [x for x in circuit.gates if qg.outputs[0] in x.inputs]:
                if g.name == "quad_gate":
                    g.inputs.remove(qg.outputs[0])
                    g.comment = g.comment.replace("_"+str(qg.outputs[0])+"_", "0")
                    recalculate.add(g)
                else:
                    print("ERROR")
        for g in recalculate:
            g.comment = str(simplify(parse_expr(g.comment)))
        circuit.gates = [x for x in circuit.gates if x not in quad_zero_gates]

    print("\r"+" "*80+"\r", end ="")


