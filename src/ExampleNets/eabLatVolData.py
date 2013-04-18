latvolMod = addModule("CreateLatVol")
size = 10
latvolMod.XSize = size
latvolMod.YSize = size
latvolMod.ZSize = size

data = {}
addDataMod = addModule("CreateScalarFieldDataBasic")
#eval.Operator = 2
#eval.Scalar = i


show = addModule("ShowField")
#write.Filename = os.path.join(dir, "ident" + str(i) + ".txt")

view = addModule("ViewScene")

latvolMod.output[0] >> addDataMod.input[0]
#addDataMod.output[0] >> show.input.LegacyField
show.output[0] >> view.input[0]