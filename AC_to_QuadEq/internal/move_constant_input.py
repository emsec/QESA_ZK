from internal.circuit import *

def process(circuit):
    last_input = circuit.inputs[-1]

    circuit.input_comments.insert(0, circuit.input_comments[-1])
    del circuit.input_comments[-1]

    for g in circuit.gates:
        for i,x in enumerate(g.inputs):
            if x in circuit.inputs:
                if x == last_input:
                    g.inputs[i] = 0
                else:
                    g.inputs[i] += 1
