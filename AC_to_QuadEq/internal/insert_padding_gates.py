from internal.circuit import *

def process(circuit):
    to_add = list()
    to_remove = list()

    next_wire = 100000000

    candidates = [g for g in circuit.gates if g.name == "split" or g.name == "zerop"]
    for i,g in enumerate(candidates):
        print("\r    split/zerop "+str(i)+"/"+str(len(candidates)), end ="")
        in_wire = g.inputs[0]
        if in_wire in circuit.inputs:
            continue

        new_gate = Gate()
        new_gate.name = "mul"
        new_gate.inputs = [in_wire, circuit.inputs[0]]
        new_gate.outputs = [next_wire]
        new_gate.comment = "QuadGate before "+g.name
        to_add.append(new_gate)

        g.inputs[0] = next_wire

        next_wire+=1

    print("\r"+" "*80+"\r", end ="")

    candidates = [g for g in circuit.gates if g.name == "pack"]
    for i,g in enumerate(candidates):
        print("\r    pack "+str(i)+"/"+str(len(candidates)), end ="")

        to_remove.append(g)

        last_output = g.inputs[-1]
        for bit in range(len(g.inputs)-2, -1, -1):
            x = g.inputs[bit]

            new_gate = Gate()
            new_gate.name = "const-mul-2"
            new_gate.inputs = [last_output]
            new_gate.outputs = [next_wire]
            new_gate.comment = "QuadGate for pack"
            to_add.append(new_gate)
            last_output = next_wire
            next_wire += 1

            new_gate = Gate()
            new_gate.name = "add"
            new_gate.inputs = [x, last_output]
            new_gate.comment = "QuadGate for pack"
            if bit > 0:
                new_gate.outputs = [next_wire]
                last_output = next_wire
                next_wire += 1
            else:
                new_gate.outputs = g.outputs

            to_add.append(new_gate)

    print("\r"+" "*80+"\r", end ="")

    circuit.gates += to_add
    to_add.clear()

    candidates = [g for g in circuit.gates if g.outputs[0] in circuit.outputs and g.name.startswith(("const-mul", "add", "sub"))]
    for i,g in enumerate(candidates):
        print("\r    outputs "+str(i)+"/"+str(len(candidates)), end ="")
        new_gate = Gate()
        new_gate.name = "mul"
        new_gate.inputs = [next_wire, circuit.inputs[0]]
        new_gate.outputs = [g.outputs[0]]
        new_gate.comment = "QuadGate before output"
        to_add.append(new_gate)

        g.outputs[0] = next_wire

        next_wire += 1

    print("\r"+" "*80+"\r", end ="")

    circuit.gates += to_add

    to_remove = set(to_remove)
    circuit.gates = [x for x in circuit.gates if x not in to_remove]
