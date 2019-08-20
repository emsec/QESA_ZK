#!/usr/bin/python3
import sys
import os
from circuit import *
from parser import parse_arith_circuit

def to_hex(s):
    return format(s, 'x')

def isInt(s):
    try:
        int(s)
        return True
    except ValueError:
        return False

def level_split(s, delim):
    res = list()
    lvl = 0
    last = 0
    for i,c in enumerate(s):
        if c == "(": lvl += 1
        if c == ")": lvl -= 1
        if lvl == 0 and c == delim:
            res.append(s[last:i].strip())
            last = i+1
    res.append(s[last:].strip())
    return res

def process_quad_equation(A, B, eq):
    # print(eq)
    addition_terms = level_split(eq.replace("-", "+-").replace(" ", "").replace("-_","-1*_"), "+")
    # print("+ "+str(addition_terms))

    for add_term in addition_terms:
        mul_terms = level_split(add_term, "*")
        for i, x in enumerate(mul_terms):
            # print("x="+str(x))
            if not " " in x and not "_" in x:
                mul_terms[i+1] = x+"*"+mul_terms[i+1]
                mul_terms[i] = ""
        mul_terms = [x for x in mul_terms if x != ""]

        # print("* "+str(mul_terms))

        a = dict()
        b = dict()

        def process_term(term, res):
            if term[0] == "(" and term[-1] == ")": term = term[1:-1]
            parts = [x for x in term.split("+")]
            # print("++ "+str(parts))
            for p in parts:
                if p =="": continue
                if not "*" in p:
                    if isInt(p):
                        res[0] = to_hex(int(p))
                    else:
                        res[int(p[1:-1])] = 1
                else:
                    lr = [x.strip() for x in p.split("*")]
                    res[int(lr[1][1:-1])] = to_hex(int(lr[0]))

        process_term(mul_terms[0], a)
        process_term(mul_terms[1], b)

        A.append(a)
        B.append(b)
    # print(" ")

def process(circuit):
    dim = circuit.num_variables()

    matrices = list()

    for g in circuit.gates:
        if not g.name in ["quad_gate", "split", "zerop"]:
            print("unknown gate type: "+str(g))
            continue


        A = list()
        B = list()

        if g.name == "quad_gate":
            # output
            A.append({0: -1})
            B.append({g.outputs[0]: 1})

            # inputs
            process_quad_equation(A, B, g.comment)

            matrices.append([A,B])

        if g.name == "split":
            # sum of bits = value
            A.append({0: 1})
            b = {g.inputs[0]: -1}
            for i,x in enumerate(g.outputs):
                b[x] = to_hex(2**i)
            B.append(b)

            matrices.append([A,B])

            # all outputs are bits
            for x in g.outputs:
                matrices.append([[{x:1}],[{x:1, 0:-1}]])

        if g.name == "zerop":
            # outputs[0]=M, outputs[1]=Y
            # X*M-Y = 0
            A.append({g.inputs[0]:1})
            B.append({g.outputs[0]:1})
            A.append({0:-1})
            B.append({g.outputs[1]:1})
            matrices.append([A,B])

            # (1-Y)*X = 0
            matrices.append([[{0:1, g.outputs[1]:-1}],[{g.inputs[0]:1}]])

    # for A, B in matrices:
    #     print("  "+"-"*20)
    #     for i in range(len(A)):
    #         print("  "+str(A[i])+"  "+str(B[i]))

    return matrices
