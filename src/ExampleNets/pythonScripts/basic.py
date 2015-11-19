read = addModule("ReadMatrix")
#read.Filename = file
eval = addModule("EvaluateLinearAlgebraUnary")
eval.Operator = 2
eval.ScalarValue = 2.5
read.output.Matrix >> eval.input.InputMatrix
write = addModule("WriteMatrix")
eval.output.Result >> write.input[0]