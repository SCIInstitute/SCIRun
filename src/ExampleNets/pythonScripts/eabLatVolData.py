latvolMod = addModule("CreateLatVol")
size = 10
latvolMod.XSize = size
latvolMod.YSize = size
latvolMod.ZSize = size
latvolMod.DataAtLocation = "Nodes"

report1 = addModule("ReportFieldInfo")
latvolMod.output[0] >> report1.input[0]

data = {}
addDataMod = addModule("CreateScalarFieldDataBasic")
#eval.Operator = 2
#eval.Scalar = i

report2 = addModule("ReportFieldInfo")
addDataMod.output[0] >> report2.input[0]

show = addModule("ShowField")


latvolMod.output[0] >> addDataMod.input[0]
addDataMod.output[0] >> show.input.Field

view = addModule("ViewScene")
show.output[0] >> view.input[0]
view.showUI()

executeAll()

removeModule(view.id)

view = addModule("ViewScene")
show.output[0] >> view.input[0]
view.showUI()

executeAll()

#executeAll()
#executeAll()