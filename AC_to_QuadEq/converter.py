#!/usr/bin/python3
import sys
import os
from internal.circuit import *
from internal.parser import parse_arith_circuit
from internal import move_constant_input
from internal import insert_quadratic_gates
from internal import insert_padding_gates
from internal import propagate_zero
from internal import generate_matrices
import generate_witness

def to_hex(s):
    return format(s, 'x')

def process(circuit):
    matrices = None
    witness = None

    if not "one-input" in circuit.input_comments[0]:
        print("  moving constant-1 input to index 0")
        move_constant_input.process(circuit)

    print("  removing floating gates")
    circuit.remove_floating_gates()

    print("  adding preparation gates")
    insert_padding_gates.process(circuit)

    print("  restructuring wires")
    circuit.restructure_wires()

    print("  inserting quadratic gates")
    insert_quadratic_gates.process(circuit)

    print("  propagating constant 0")
    propagate_zero.process(circuit)

    print("  removing floating gates")
    circuit.remove_floating_gates()

    print("  restructuring wires")
    circuit.restructure_wires()

    print("  sorting")
    circuit.sort()

    print("  new circuit: #gates = "+str(len(circuit.gates)))

    print("  generating matrices")
    matrices = generate_matrices.process(circuit)

    print("  generating witness")
    witness = generate_witness.process(circuit)

    return circuit, matrices, witness

files = []
if os.path.isdir(sys.argv[1]):
    for file in os.listdir(sys.argv[1]):
        if file.endswith(".arith"):
            files += [sys.argv[1]+"/"+file]
else:
    files = [sys.argv[1]]

for f in files:
    print("Processing '"+os.path.basename(f)+"'")
    circuit = parse_arith_circuit(f)
    circuit, matrices, witness = process(circuit)

    with open(f+"_out", "wt") as file:
        file.write("total "+str(circuit.num_variables())+"\n")
        for i in range(len(circuit.inputs)):
            file.write(("input "+str(circuit.inputs[i])).ljust(40)+" # "+circuit.input_comments[i]+"\n")
        for g in circuit.gates: file.write(str(g)+"\n")
        for o in circuit.outputs: file.write("output "+str(o)+"\n")

    if matrices != None:
        with open(f+"_mat", "wt") as file:
            file.write(str(circuit.num_variables()) + " # num variables/wires\n")
            for num, mat in enumerate(matrices):
                file.write("# matrix "+str(num+1)+"\n")
                for i in range(len(mat[0])):
                    s = ",".join([str(x)+":"+str(mat[0][i][x]) for x in mat[0][i]])
                    s += ";"
                    s += ",".join([str(x)+":"+str(mat[1][i][x]) for x in mat[1][i]])
                    file.write(s+"\n")

    if witness != None:
        with open(f+"_wit", "wt") as file:
            file.write(",".join([to_hex(x) for x in witness])+"\n")
