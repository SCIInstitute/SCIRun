m1 = scirun_add_module("CreateLatVol")
m2 = scirun_add_module("SetFieldDataToConstantValue")
m3 = scirun_add_module("WriteField")

scirun_connect_modules(m1, 0, m2, 0)
scirun_connect_modules(m2, 0, m3, 0)

for i in range(5):
	scirun_set_module_state(m2, "Value", i)
	scirun_set_module_state(m3, "Filename", "field%d.fld" % i)
	scirun_execute_all()

#scirun_force_quit()  #crashes right now
scirun_quit()
scirun_execute_all()
