ORDER = int("1000000000000000000000000000000014DEF9DEA2F79CD65812631A5CF5D3ED", 16)

class element:
    def __init__(self, s, v):
        self.scalar = s
        self.variable = v

    def mul(self, n):
        return element((self.scalar * n) % ORDER, self.variable)

    def add(self, n):
        return element((self.scalar + n) % ORDER, self.variable)

class ext_sum:
    def __init__(self, e=None):
        if e == None: self.elements = list()
        else: self.elements = [e]

    def mul(self, n):
        res = ext_sum()
        res.elements = [x.mul(n) for x in self.elements]
        res.elements = [x for x in res.elements if x.scalar != 0]
        return res

    def add(self, e):
        res = list(self.elements)
        for y in e.elements:
            found = False
            for i,x in enumerate(res):
                if x.variable == y.variable:
                    res[i] = x.add(y.scalar)
                    found = True
            if not found: res.append(y)

        n = ext_sum()
        n.elements = [x for x in res if x.scalar != 0]
        return n

    def __str__(side):
        s = None
        side.elements = sorted(side.elements, key=lambda x: x.variable)
        for x in side.elements:
            if s == None:
                if x.scalar < 0: s = "-"
                else: s = ""
            else:
                if x.scalar < 0: s += " - "
                else: s += " + "
            val = abs(x.scalar)
            if val != 1 or x.variable == None: s += str(val)
            if val != 1 and x.variable != None: s += "*"
            if x.variable != None: s += str(x.variable)
        if s == None: s = "0"
        return s

class expr:
    def __init__(self, a, b):
        self.lhs = a
        self.rhs = b

    def __str__(self):
        lhs_str = str(self.lhs)
        rhs_str = str(self.rhs)
        if lhs_str == "0" or rhs_str == "0": return "0"
        s = None
        if len(self.lhs.elements) > 1:
            s = "("+ lhs_str+ ")"
        else:
            s = lhs_str
        s += " * "
        if len(self.rhs.elements) > 1:
            s += "("+ rhs_str + ")"
        else:
            s += rhs_str
        return s
