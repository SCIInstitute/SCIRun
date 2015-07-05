size = 50
latvolMod = addModule("CreateLatVol")
latvolMod.XSize = size
latvolMod.YSize = size
latvolMod.ZSize = size
latvolMod.DataAtLocation = "Nodes"
latvolMod.ElementSizeNormalized = True

report1 = addModule("ReportFieldInfo")
latvolMod.output[0] >> report1.input[0]

dataMod = addModule("CreateScalarFieldDataBasic")
dataMod.ValueFunc = "sine"

report2 = addModule("ReportFieldInfo")
dataMod.output[0] >> report2.input[0]

show = addModule("ShowField")
show.ShowEdges = False
show.EdgeTransparency = False
show.FaceTransparency = True

latvolMod.output[0] >> dataMod.input[0]
dataMod.output[0] >> show.input.Field

view = addModule("ViewScene")
show.output[0] >> view.input[0]
view.showUI()
executeAll()
