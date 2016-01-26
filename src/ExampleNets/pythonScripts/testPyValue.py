m3 = scirun_add_module("CreateString")
m4 = scirun_add_module("ReportStringInfo")
scirun_connect_modules(m3, 0, m4, 0)
scirun_set_module_state(m3, "InputString", "Hello")
scirun_execute_all()

x = scirun_get_module_input_copy(m4, 0)
