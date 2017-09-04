"""
SCDtoObliv.py - Parses an SCD file for various purposes.
    SCD circuit files are obtained from TinyGarble at
    https://github.com/esonghori/TinyGarble.
    After parsing, this tool can either:
      * Evaluate the circuit.
      * Produce Python code for evaluating the circuit
         (e.g. pyOblivTest.py).
      * Produce Obliv-C code for the circuit.

    Instructions:
    1) Specify a SCD circuit file as an argument to
        the program
    2) Redirect the output of the program to the
        desired output destination

    Also possible: evaluate the circuit with this
    program (for rapid testing purposes).

Author : Darion Cassel

"""

import sys

class GarbledGate:
    def __init__(self):
        self.output = None
        self.input0 = None
        self.input1 = None
        self.gateType = None

    def __repr__(self):
        return "[In0:" + str(self.input0) \
            + ", In1:" + str(self.input1) \
            + ", Typ:" + str(self.gateType) + "]"


class GarbledCircuit:
    def __init__(self):
        self.p_init_size = 0
        self.g_init_size = 0
        self.e_init_size = 0
        self.p_input_size = 0
        self.g_input_size = 0
        self.e_input_size = 0
        self.dff_size = 0
        self.output_size = 0
        self.terminate_id = 0
        self.gate_size = 0
        self.garbledGates = []
        self.outputs = []
        self.D = []
        self.I = []

    def addGates(self):
        for i in range(self.gate_size):
            self.garbledGates.append(GarbledGate())
        for j in range(self.output_size):
            self.outputs.append(None)
        if self.dff_size > 0:
            self.D = [0] * self.dff_size
            self.I = [0] * self.dff_size
        else:
            self.D = None
            self.I = None

    def get_init_size(self):
        return self.p_init_size + self.g_init_size + self.e_init_size

    def get_input_size(self):
        return self.p_input_size + self.g_input_size + self.e_input_size

    def get_wire_size(self):
        return self.get_init_size() + \
            self.get_input_size() + \
            self.dff_size + self.gate_size

    def get_secret_init_size(self):
        return self.g_init_size + self.e_init_size

    def get_secret_input_size(self):
        return self.g_input_size + self.e_input_size

    def get_init_lo_index(self):
        return 0

    def get_p_init_lo_index(self):
        return self.get_init_lo_index()

    def get_p_init_hi_index(self):
        return self.get_p_init_lo_index() + self.p_init_size

    def get_g_init_lo_index(self):
        return self.get_p_init_hi_index()

    def get_g_init_hi_index(self):
        return self.get_g_init_lo_index() + self.g_init_size

    def get_e_init_lo_index(self):
        return self.get_g_init_hi_index()

    def get_e_init_hi_index(self):
        return self.get_e_init_lo_index() + self.e_init_size

    def get_init_hi_index(self):
        return self.get_e_init_hi_index()

    def get_input_lo_index(self):
        return self.get_init_hi_index()

    def get_p_input_lo_index(self):
        return self.get_input_lo_index()

    def get_p_input_hi_index(self):
        return self.get_p_input_lo_index() + self.p_input_size

    def get_g_input_lo_index(self):
        return self.get_p_input_hi_index()

    def get_g_input_hi_index(self):
        return self.get_g_input_lo_index() + self.g_input_size

    def get_e_input_lo_index(self):
        return self.get_g_input_hi_index()

    def get_e_input_hi_index(self):
        return self.get_e_input_lo_index() + self.e_input_size

    def get_input_hi_index(self):
        return self.get_e_input_hi_index()

    def get_dff_lo_index(self):
        return self.get_input_hi_index()

    def get_dff_hi_index(self):
        return self.get_dff_lo_index() + self.dff_size

    def get_gate_lo_index(self):
        return self.get_dff_hi_index()

    def get_gate_hi_index(self):
        return self.get_gate_lo_index() + self.gate_size

    def get_wire_lo_index(self):
        return 0

    def get_wire_hi_index(self):
        return self.get_gate_hi_index()

    def __repr__(self):
        return "[Pinit:" + str(self.p_init_size) + ", Ginit:" \
            + str(self.g_init_size) + ", Einit:" \
            + str(self.e_init_size) + ", Pinput:" \
            + str(self.p_input_size) + ", Ginput:" \
            + str(self.g_input_size) + ", Einput:" \
            + str(self.e_input_size) + ", DffSz:"\
            + str(self.dff_size) + ", OutSz:" \
            + str(self.output_size) + ", TermId:" \
            + str(self.terminate_id) + ", GateSz:" \
            + str(self.gate_size) + ", Gates:" \
            + str(self.garbledGates) + ", Outputs:" \
            + str(self.outputs) + "]"


class util_cppReadOp:
    def __init__(self, openFile):
        self.f = openFile
        self.chars = []
        self.cursor = 0
        self.prepFile()

    def prepFile(self):
        fileLines = [x.rstrip() for x in self.f.readlines()]
        for line in fileLines:
            self.chars.extend([int(x.strip())
                               for x in line.split(' ') if x != ''])

    def readToBreak(self):
        if self.cursor < len(self.chars):
            char = self.chars[self.cursor]
            self.cursor += 1
            return char
        else:
            return -1

    def read(self):
        return self.readToBreak()

    def debugRead(self):
        print(self.chars)


class ReadSCD:
    def __init__(self, filename, garbledCircuit):
        self.filename = filename
        self.gc = garbledCircuit
        self.lines = []

    def readInits(self, line):
        line = [int(x.strip()) for x in line.split(' ')]
        self.gc.p_init_size = line[0]
        self.gc.g_init_size = line[1]
        self.gc.e_init_size = line[2]
        self.gc.p_input_size = line[3]
        self.gc.g_input_size = line[4]
        self.gc.e_input_size = line[5]
        self.gc.dff_size = line[6]
        self.gc.output_size = line[7]
        self.gc.terminate_id = line[8]
        self.gc.gate_size = line[9]
        self.gc.addGates()

    def genOutputs(self):
        for i in range(self.gc.gate_size):
            self.gc.garbledGates[i].output = self.gc.get_gate_lo_index() + i

    def readIn0s(self, line):
        line = [int(x.strip()) for x in line.split(' ')]
        for i in range(self.gc.gate_size):
            self.gc.garbledGates[i].input0 = line[i]

    def readIn1s(self, line):
        line = [int(x.strip()) for x in line.split(' ')]
        for i in range(self.gc.gate_size):
            self.gc.garbledGates[i].input1 = line[i]

    def readTypes(self, line):
        line = [int(x.strip()) for x in line.split(' ')]
        for i in range(self.gc.gate_size):
            self.gc.garbledGates[i].gateType = line[i]

    def readOutputs(self, line):
        line = [int(x.strip()) for x in line.split(' ')]
        for i in range(self.gc.output_size):
            self.gc.outputs[i] = line[i]

    def read(self):
        with open(self.filename, "r") as f:
            self.lines = [x.rstrip() for x in f.readlines() if x is not ""]
        self.readInits(self.lines[0])
        self.genOutputs()
        self.readIn0s(self.lines[1])
        self.readIn1s(self.lines[2])
        self.readTypes(self.lines[3])
        self.readOutputs(self.lines[4])


class EvaluateSCD:
    def __init__(self, garbledCircuit):
        self.gc = garbledCircuit
        self.wires = []
        for i in range(self.gc.gate_size +
                       self.gc.output_size + self.gc.get_input_size()):
            self.wires.append(None)

    def toBit(self, inpt):
        inpt = int(inpt)
        if inpt == 0:
            return False
        elif inpt == 1:
            return True
        else:
            raise Exception("Invalid bit parsed!: " + str(inpt))

    def toInt(self, inpt):
        if inpt:
            return 1
        else:
            return 0

    def parseInput(self, in0, in1, in2):
        p_input = [self.toBit(x) for x in list(in0)]
        g_input = [self.toBit(x) for x in list(in1)]
        e_input = [self.toBit(x) for x in list(in2)]
        input_bias = self.gc.get_input_lo_index()
        for i in range(self.gc.get_input_size()):
            # Input belongs to p
            if (i + input_bias) < self.gc.get_p_input_hi_index():
                bit_bias = i + input_bias - self.gc.get_p_input_lo_index()
                self.wires[input_bias + i] = p_input[bit_bias]
            # Input belongs to g
            elif (i + input_bias) < self.gc.get_g_input_hi_index():
                bit_bias = i + input_bias - self.gc.get_g_input_lo_index()
                self.wires[input_bias + i] = g_input[bit_bias]
            # Input belongs to e
            else:
                bit_bias = i + input_bias - self.gc.get_e_input_lo_index()
                self.wires[input_bias + i] = e_input[bit_bias]

    def gateOp(self, gt, in0, in1):
        # NOT gate
        if gt == 12:
            return not in0
        # AND gate
        elif gt == 8:
            return in0 and in1
        # OR gate
        elif gt == 14:
            return in0 or in1
        # XOR gate
        elif gt == 6:
            return in0 is not in1
        # NAND gate
        elif gt == 7:
            return not (in0 and in1)
        # NANDN gate
        elif gt == 11:
            return not (in0 and (not in1))
        # ANDN gate
        elif gt == 4:
            return in0 and (not in1)
        # XNOR gate
        elif gt == 9:
            return not (in0 is not in1)
        # NOR gate
        elif gt == 1:
            return not (in0 or in1)
        else:
            raise Exception("Unhandled Gate! Number: " + str(gt))

    def constructOutput(self):
        res = ""
        for wireNum in self.gc.outputs:
            res += str(self.toInt(self.wires[wireNum]))
        return res

    def evaluate(self, in0, in1, in2):
        p_init = [0] * 32
        g_init = [0] * 32
        e_init = [0] * 32
        dff_bias = self.gc.get_dff_lo_index()
        for i in range(self.gc.dff_size):
            wire_index = self.gc.I[i]
            b = None
            if wire_index == -2:
                b = False
            elif wire_index == -3:
                b = True
            elif wire_index >= -1 and wire_index < self.gc.get_init_size():
                if wire_index < self.gc.get_p_init_hi_index():
                    b = p_init[wire_index - self.gc.get_p_init_lo_index()]
                elif wire_index < self.gc.get_g_init_hi_index():
                    b = g_init[wire_index - self.gc.get_g_init_lo_index()]
                else:
                    b = e_init[wire_index - self.gc.get_e_init_lo_index()]
            else:
                raise Exception("Invalid I index: " + str(wire_index))
            self.wires[dff_bias + i] = b
        self.parseInput(in0, in1, in2)
        for gate in self.gc.garbledGates:
            if gate.input0 == -2:
                inp0 = False
            elif gate.input0 == -3:
                inp0 = True
            elif gate.input0 >= -1 and gate.input0 < self.gc.get_wire_size():
                inp0 = self.wires[gate.input0]
            else:
                raise Exception("Invalid input0 index: " + str(gate.input0))
            if gate.input1 == -2:
                inp1 = False
            elif gate.input1 == -3:
                inp1 = True
            elif gate.input1 >= -1 and gate.input1 < self.gc.get_wire_size():
                inp1 = self.wires[gate.input1]
            else:
                raise Exception("Invalid input1 index: " + str(gate.input1))
            self.wires[gate.output] = self.gateOp(gate.gateType, inp0, inp1)
        print(self.constructOutput())


class pyWriteOblivCircuit:
    def __init__(self, garbledCircuit):
        self.gc = garbledCircuit
        self.wires = []
        self.statements = []
        for i in range(self.gc.gate_size +
                       self.gc.output_size + self.gc.get_input_size()):
            self.wires.append("__obliv_" + str(i) + " = None")

    def toBit(self, inpt):
        inpt = int(inpt)
        if inpt == 0:
            return False
        elif inpt == 1:
            return True
        else:
            raise Exception("Invalid bit parsed!: " + str(inpt))

    def toInt(self, inpt):
        if inpt:
            return 1
        else:
            return 0

    def parseInput(self):
        input_bias = self.gc.get_input_lo_index()
        for i in range(self.gc.get_input_size()):
            # Input belongs to p
            if (i + input_bias) < self.gc.get_p_input_hi_index():
                bit_bias = i + input_bias - self.gc.get_p_input_lo_index()
                self.statements.append("__obliv_c__copyBit(__obliv_" +
                                       str(input_bias+i) + ", p_input[" +
                                       str(bit_bias) + "])")
                raise Exception("P input not handled!")
            # Input belongs to g
            elif (i + input_bias) < self.gc.get_g_input_hi_index():
                bit_bias = i + input_bias - self.gc.get_g_input_lo_index()
                self.statements.append("__obliv_" +
                                       str(input_bias+i) + " = __obliv_c__copyBit(__obliv_" +
                                       str(input_bias+i) + ", g_input[" +
                                       str(bit_bias) + "])")
            # Input belongs to e
            else:
                bit_bias = i + input_bias - self.gc.get_e_input_lo_index()
                self.statements.append("__obliv_" +
                                       str(input_bias+i) + " = __obliv_c__copyBit(__obliv_" +
                                       str(input_bias+i) + ", e_input[" +
                                       str(bit_bias) + "])")

    def gateTypeToObliv(self, typeStr):
        typeMap = {
            "AND": "__obliv_c__setBitAnd",
            "XOR": "__obliv_c__setBitXor",
            "OR": "__obliv_c__setBitOr",
            "NOT": "__obliv_c__setBitNot",
            "NAND": "__obliv_c__setBitNand",
            "ANDN": "__obliv_c__setBitAndn",
            "NANDN": "__obliv_c__setBitNandn",
            "XNOR": "__obliv_c__setBitXnor",
            "NOR": "__obliv_c__setBitNor",
        }
        return typeMap[typeStr]

    def genGateInputStr(self, out, in0, in1):
        res = "("
        res += "__obliv_" + str(out)
        res += ", __obliv_" + str(in0)
        if in1 is not None:
            res += ", __obliv_" + str(in1)
        res += ")"
        return res

    def processGate_(self, out, in0, in1, typ):
        if typ == "NOT":
            return self.gateTypeToObliv("NOT") + \
            self.genGateInputStr(out, in0, None)
        else:
            return self.gateTypeToObliv(typ) + \
            self.genGateInputStr(out, in0, in1)

    def constructOutput(self):
        for i, wireNum in enumerate(self.gc.outputs):
            self.statements.append("dest[" +
                                   str(i) + "] = __obliv_c__copyBit(dest[" +
                                   str(i) + "], __obliv_" +
                                   str(wireNum) + ")")

    def header(self, output):
        return "__obliv_" + str(output) + " = "

    def processGate(self, output, input0, input1, gateType):
        st = []
        gt = gateType
        # NOT gate
        if gt == 12:
            st.append(self.header(output) + self.processGate_(output, input0, None, "NOT"))
        # AND gate
        elif gt == 8:
            st.append(self.header(output) + self.processGate_(output, input0, input1, "AND"))
        # OR gate
        elif gt == 14:
            st.append(self.header(output) + self.processGate_(output, input0, input1, "OR"))
        # XOR gate
        elif gt == 6:
            st.append(self.header(output) + self.processGate_(output, input0, input1, "XOR"))
        # NAND gate
        elif gt == 7:
            st.append(self.header(output) + self.processGate_(output, input0, input1, "AND"))
            st.append(self.header(output) + self.processGate_(output, output, None, "NOT"))
        # NANDN gate
        elif gt == 11:
            tmpBit = str(input1) + "b" + str(output)
            self.wires.append("__obliv_" + tmpBit + " = None")
            st.append(self.header(tmpBit) + self.processGate_(tmpBit, input1, None, "NOT"))
            st.append(self.header(output) + self.processGate_(output, input0, tmpBit, "AND"))
            st.append(self.header(output) + self.processGate_(output, output, None, "NOT"))
        # ANDN gate
        elif gt == 4:
            tmpBit = str(input1) + "a" + str(output)
            self.wires.append("__obliv_" + tmpBit + " = None")
            st.append(self.header(tmpBit) + self.processGate_(tmpBit, input1, None, "NOT"))
            st.append(self.header(output) + self.processGate_(output, input0, tmpBit, "AND"))
        # XNOR gate
        elif gt == 9:
            st.append(self.header(output) + self.processGate_(output, input0, input1, "XOR"))
            st.append(self.header(output) + self.processGate_(output, output, None, "NOT"))
        # NOR gate
        elif gt == 1:
            st.append(self.header(output) + self.processGate_(output, input0, input1, "OR"))
            st.append(self.header(output) + self.processGate_(output, output, None, "NOT"))
        else:
            raise Exception("Unhandled Gate! Number: " + str(gt))
        self.statements.extend(st)

    def processSCD(self):
        p_init = [0] * 32
        g_init = [0] * 32
        e_init = [0] * 32
        dff_bias = self.gc.get_dff_lo_index()
        for i in range(self.gc.dff_size):
            wire_index = self.gc.I[i]
            b = None
            if wire_index == -2:
                b = False
            elif wire_index == -3:
                b = True
            elif wire_index >= -1 and wire_index < self.gc.get_init_size():
                if wire_index < self.gc.get_p_init_hi_index():
                    b = p_init[wire_index - self.gc.get_p_init_lo_index()]
                elif wire_index < self.gc.get_g_init_hi_index():
                    b = g_init[wire_index - self.gc.get_g_init_lo_index()]
                else:
                    b = e_init[wire_index - self.gc.get_e_init_lo_index()]
            else:
                raise Exception("Invalid I index: " + str(wire_index))
            self.statements.append("__obliv_c__copyBit(dest" +
                                   str(dff_bias + i) + ", " + str(b) + ")")
        self.parseInput()
        for gate in self.gc.garbledGates:
            if gate.input0 == -2:
                inp0 = False
            elif gate.input0 == -3:
                inp0 = True
            elif gate.input0 >= -1 and gate.input0 < self.gc.get_wire_size():
                inp0 = gate.input0
            else:
                raise Exception("Invalid input0 index: " + str(gate.input0))
            if gate.input1 == -2:
                inp1 = False
            elif gate.input1 == -3:
                inp1 = True
            elif gate.input1 >= -1 and gate.input1 < self.gc.get_wire_size():
                inp1 = gate.input1
            else:
                raise Exception("Invalid input1 index: " + str(gate.input1))
            self.processGate(gate.output, inp0, inp1, gate.gateType)
        print(self.constructOutput())

    def __repr__(self):
        output = ""
        for wire in self.wires:
            output += wire + "\n"
        for statement in self.statements:
            output += statement + "\n"
        return output


class WriteOblivCircuit:
    def __init__(self, garbledCircuit):
        self.gc = garbledCircuit
        self.wires = []
        self.statements = []
        for i in range(self.gc.gate_size +
                       self.gc.output_size + self.gc.get_input_size()):
            self.wires.append("OblivBit __obliv_" + str(i) + ";")

    def toBit(self, inpt):
        inpt = int(inpt)
        if inpt == 0:
            return False
        elif inpt == 1:
            return True
        else:
            raise Exception("Invalid bit parsed!: " + str(inpt))

    def toInt(self, inpt):
        if inpt:
            return 1
        else:
            return 0

    def parseInput(self):
        input_bias = self.gc.get_input_lo_index()
        for i in range(self.gc.get_input_size()):
            # Input belongs to p
            if (i + input_bias) < self.gc.get_p_input_hi_index():
                bit_bias = i + input_bias - self.gc.get_p_input_lo_index()
                raise Exception("P input not handled!")
            # Input belongs to g
            elif (i + input_bias) < self.gc.get_g_input_hi_index():
                bit_bias = i + input_bias - self.gc.get_g_input_lo_index()
                self.statements.append("__obliv_c__copyBit(&__obliv_" +
                                       str(input_bias + i) + ", op1+" +
                                       str(bit_bias) + ");")
            # Input belongs to e
            else:
                bit_bias = i + input_bias - self.gc.get_e_input_lo_index()
                self.statements.append("__obliv_c__copyBit(&__obliv_" +
                                       str(input_bias + i) + ", op2+" +
                                       str(bit_bias) + ");")

    def gateTypeToObliv(self, typeStr):
        typeMap = {
            "AND": "__obliv_c__setBitAnd",
            "XOR": "__obliv_c__setBitXor",
            "OR": "__obliv_c__setBitOr",
            "NOT": "__obliv_c__setBitNot",
        }
        return typeMap[typeStr]

    def genGateInputStr(self, out, in0, in1):
        res = "("
        res += "&__obliv_" + str(out)
        res += ", &__obliv_" + str(in0)
        if in1 is not None:
            res += ", &__obliv_" + str(in1)
        res += ");"
        return res

    def processGate_(self, out, in0, in1, typ):
        if typ == "NOT":
            return self.gateTypeToObliv("NOT") + \
            self.genGateInputStr(out, in0, None)
        else:
            return self.gateTypeToObliv(typ) + \
            self.genGateInputStr(out, in0, in1)

    def constructOutput(self):
        for i, wireNum in enumerate(self.gc.outputs):
            self.statements.append("__obliv_c__copyBit(dest+" +
                                   str(i) + ", &__obliv_" +
                                   str(wireNum) + ");")

    def processGate(self, output, input0, input1, gateType):
        st = []
        gt = gateType
        # NOT gate
        if gt == 12:
            st.append(self.processGate_(output, input0, None, "NOT"))
        # AND gate
        elif gt == 8:
            st.append(self.processGate_(output, input0, input1, "AND"))
        # OR gate
        elif gt == 14:
            st.append(self.processGate_(output, input0, input1, "OR"))
        # XOR gate
        elif gt == 6:
            st.append(self.processGate_(output, input0, input1, "XOR"))
        # NAND gate
        elif gt == 7:
            st.append(self.processGate_(output, input0, input1, "AND"))
            st.append(self.processGate_(output, output, None, "NOT"))
        # NANDN gate
        elif gt == 11:
            tmpBit = str(input1) + "b" + str(output)
            self.wires.append("OblivBit __obliv_" + tmpBit + ";")
            st.append(self.processGate_(tmpBit, input1, None, "NOT"))
            st.append(self.processGate_(output, input0, tmpBit, "AND"))
            st.append(self.processGate_(output, output, None, "NOT"))
        # ANDN gate
        elif gt == 4:
            tmpBit = str(input1) + "a" + str(output)
            self.wires.append("OblivBit __obliv_" + tmpBit + ";")
            st.append(self.processGate_(tmpBit, input1, None, "NOT"))
            st.append(self.processGate_(output, input0, tmpBit, "AND"))
        # XNOR gate
        elif gt == 9:
            st.append(self.processGate_(output, input0, input1, "XOR"))
            st.append(self.processGate_(output, output, None, "NOT"))
        # NOR gate
        elif gt == 1:
            st.append(self.processGate_(output, input0, input1, "OR"))
            st.append(self.processGate_(output, output, None, "NOT"))
        else:
            raise Exception("Unhandled Gate! Number: " + str(gt))
        self.statements.extend(st)

    def processSCD(self):
        p_init = [0] * 32
        g_init = [0] * 32
        e_init = [0] * 32
        dff_bias = self.gc.get_dff_lo_index()
        for i in range(self.gc.dff_size):
            wire_index = self.gc.I[i]
            b = None
            if wire_index == -2:
                b = False
            elif wire_index == -3:
                b = True
            elif wire_index >= -1 and wire_index < self.gc.get_init_size():
                if wire_index < self.gc.get_p_init_hi_index():
                    b = p_init[wire_index - self.gc.get_p_init_lo_index()]
                elif wire_index < self.gc.get_g_init_hi_index():
                    b = g_init[wire_index - self.gc.get_g_init_lo_index()]
                else:
                    b = e_init[wire_index - self.gc.get_e_init_lo_index()]
            else:
                raise Exception("Invalid I index: " + str(wire_index))
            self.statements.append("__obliv_c__copyBit(dest" +
                                   str(dff_bias + i) + ", " + str(b) + ");")
        self.parseInput()
        for gate in self.gc.garbledGates:
            if gate.input0 == -2:
                inp0 = False
            elif gate.input0 == -3:
                inp0 = True
            elif gate.input0 >= -1 and gate.input0 < self.gc.get_wire_size():
                inp0 = gate.input0
            else:
                raise Exception("Invalid input0 index: " + str(gate.input0))
            if gate.input1 == -2:
                inp1 = False
            elif gate.input1 == -3:
                inp1 = True
            elif gate.input1 >= -1 and gate.input1 < self.gc.get_wire_size():
                inp1 = gate.input1
            else:
                raise Exception("Invalid input1 index: " + str(gate.input1))
            self.processGate(gate.output, inp0, inp1, gate.gateType)
        print(self.constructOutput())

    def __repr__(self):
        output = ""
        for wire in self.wires:
            output += wire + "\n"
        for statement in self.statements:
            output += statement + "\n"
        return output


def main():
    if len(sys.argv) > 2:
        raise Exception("Too many args: only one arg, 'filename', expected.")
    else:
        filename = sys.argv[1]
    gc = GarbledCircuit()
    reader = ReadSCD(filename, gc)
    reader.read()
    # Select pyWriteOblivCircuit for pyOblivTest program
    writer = WriteOblivCircuit(gc)
    writer.processSCD()
    print(writer)
    evaluator = EvaluateSCD(gc)
    # A circuit can be evaluated with this program
    """evaluator.evaluate("00000000000000000000000000000000",
                       "00000000000000000000000111111100",
                       "00000000000000000000000000000010")"""

main()
