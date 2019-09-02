class Gate:
    def __init__(self):
        self.name = ""
        self.inputs = list()
        self.outputs = list()
        self.comment = ""

    def __str__(self):
        s = self.name +" in " +str(len(self.inputs))+" <" + " ".join([str(x) for x in self.inputs])+"> "
        s += "out " +str(len(self.outputs))+" <" + " ".join([str(x) for x in self.outputs])+">"
        if self.comment != "":
            s += " " * (40-len(s))+" # "+self.comment
        return s

    def description(self):
        s = self.name +" in <" + " ".join([str(x) for x in self.inputs])+"> "
        s += "out <" + " ".join([str(x) for x in self.outputs])+">"
        return s

class ArithCircuit:
    def __init__(self):
        self.gates = list()
        self.inputs = list()
        self.input_comments = list()
        self.outputs = list()

    def num_variables(self):
        count = set()
        for x in self.inputs+self.outputs:
            count.add(x)
        for g in self.gates:
            for x in g.inputs+g.outputs:
                count.add(x)
        return len(count)

    def remove_floating_gates(self):
        while True:
            input_wires = list(self.outputs)
            for g in self.gates: input_wires += g.inputs
            input_wires = set(input_wires)
            candidates = [g for g in self.gates if not any(x in input_wires for x in g.outputs)]
            if len(candidates) == 0: break
            candidates = set(candidates)
            self.gates = [x for x in self.gates if x not in candidates]

    def restructure_wires(self):
        wire_target = dict()
        for g in self.gates:
            for i in g.inputs:
                if not i in wire_target: wire_target[i] = list()
                wire_target[i].append(g)

        current_wire = 0
        new_wires = dict()

        for wire in self.inputs:
            new_wires[wire] = current_wire
            current_wire += 1

        queue = list()
        for i in self.inputs:
            if i in wire_target: queue += wire_target[i]

        seen = set()
        while len(queue) > 0:
            g = queue.pop(0)
            if g in seen: continue
            seen.add(g)
            for x in g.outputs:
                if not x in self.outputs:
                    new_wires[x] = current_wire
                    current_wire += 1
                    if x in wire_target: queue += wire_target[x]

        for x in self.outputs:
            new_wires[x] = current_wire
            current_wire += 1

        for i in range(len(self.inputs)):
            self.inputs[i] = new_wires[self.inputs[i]]
        for i in range(len(self.outputs)):
            self.outputs[i] = new_wires[self.outputs[i]]

        for gate_index, g in enumerate(self.gates):
            print("\r    processing gate "+str(gate_index)+"/"+str(len(self.gates)), end ="")

            for i in range(len(g.inputs)):
                g.inputs[i] = new_wires[g.inputs[i]]
            for i in range(len(g.outputs)):
                g.outputs[i] = new_wires[g.outputs[i]]

            if g.name == "quad_gate":
                new_comment = ""
                old_comment = g.comment
                while "_" in old_comment:
                    new_comment += old_comment[:old_comment.find("_")+1]
                    old_comment = old_comment[old_comment.find("_")+1:]
                    x = int(old_comment[:old_comment.find("_")])
                    x = new_wires[x]
                    new_comment += str(x)+"_"
                    old_comment = old_comment[old_comment.find("_")+1:]
                new_comment += old_comment
                g.comment = new_comment
        print("\r"+" "*80+"\r", end ="")


    def sort(self):
        #self.gates = sorted(self.gates, key=lambda g: g.outputs[0])
        #self.gates = sorted(self.gates, key=lambda g: max(g.inputs))
        seen = set(self.inputs)
        todo = list(self.gates)
        ordered = list()
        while len(todo) > 0:
            subset = [x for x in todo if all(i in seen for i in x.inputs)]
            ordered += subset
            for g in subset:
                seen.update(g.outputs)
            todo = [x for x in todo if x not in subset]
        self.gates = ordered

        self.outputs = sorted(self.outputs)
