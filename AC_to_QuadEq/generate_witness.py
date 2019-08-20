#!/usr/bin/python3
import sys
import os
import random
from arith import ORDER
from circuit import *
from parser import parse_arith_circuit
from sympy import *
from sympy.parsing.sympy_parser import parse_expr

def to_hex(s):
    return format(s, 'x')

def simulate(gate, witness):
    if not gate.name in ["quad_gate", "split", "zerop", "xor", "pack"]:
        print("unknown gate type: "+str(gate))
        sys.exit(1)

    if gate.name == "quad_gate":
        f = gate.comment
        for i in gate.inputs: f = f.replace("_"+str(i)+"_", str(witness[i]))
        x = parse_expr(f)
        witness[gate.outputs[0]] = int(x) % ORDER

    elif gate.name == "split":
        val = witness[gate.inputs[0]]
        for o in gate.outputs:
            witness[o] = val % 2
            val //= 2

    elif gate.name == "pack":
        val = 0
        for i,x in enumerate(gate.inputs):
            val += witness[x] * (2**i)
        witness[gate.outputs[0]] = val % ORDER

    elif gate.name == "zerop":
        # output[0] = M, output[1] = Y
        if witness[gate.inputs[0]] == 0:
            witness[gate.outputs[1]] = 0
        else:
            witness[gate.outputs[1]] = 1
        witness[gate.outputs[0]] = pow(witness[gate.inputs[0]], ORDER-2, ORDER)

    elif gate.name == "xor":
        witness[gate.outputs[0]] = witness[gate.inputs[0]]^witness[gate.inputs[1]]

def process(circuit, inputs = None):
    if inputs == None:
        inputs = [random.randint(0, 256) for i in range(len(circuit.inputs))]
        inputs[0] = 1

    witness = [None]*circuit.num_variables()
    for i,x in enumerate(inputs):
        witness[circuit.inputs[i]] = x

    for g in circuit.gates:
        simulate(g, witness)

    return witness

if __name__ == '__main__':
    if os.path.isdir(sys.argv[1]):
        print("input has to be a single file")
        sys.exit(1)

    inputs = [int(x) for x in sys.argv[2:]]

    circuit = parse_arith_circuit(sys.argv[1])

    if len(circuit.inputs) != len(inputs):
        print("you have to supply "+str(len(circuit.inputs))+" input values ("+str(len(circuit.inputs)-len(inputs))+" are missing).")
        sys.exit(1)

    if inputs[0] != 1:
        print("the first input value has to be a 1.")
        sys.exit(1)

    witness = process(circuit, inputs)

    with open(sys.argv[1]+"_wit", "wt") as file:
        file.write(",".join([to_hex(x) for x in witness])+"\n")
