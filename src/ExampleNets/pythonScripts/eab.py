#$ ./SCIRun.exe --script="E:\git\SCIRunGUIPrototype\src\ExampleNets\eab.py" -E --headless

import glob
import os
dir = r"E:\git\SCIRunGUIPrototype\src\Samples\matrices"
os.chdir(dir)
file = os.path.join(dir, "identity.txt")

n = 5
for i in range(n):
	read = addModule("ReadMatrix")
	read.Filename = file
	eval = addModule("EvaluateLinearAlgebraUnary")
	eval.Operator = 2
	eval.Scalar = i
	read.output.Matrix >> eval.input.InputMatrix
	write = addModule("WriteMatrix")
	write.Filename = os.path.join(dir, "ident" + str(i) + ".txt")
	eval.output.Result >> write.input[0]
