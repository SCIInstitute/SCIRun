def scirun_assert(func):
  if not func():
    scirun_add_module("ReadMatrix") # guarantee errored network
    scirun_quit_after_execute()
    scirun_execute_all()
  else:
    scirun_force_quit() # SCIRun returns 0 to console

m1 = scirun_add_module("ReadMatrix")
m2 = scirun_add_module("ReportMatrixInfo")
scirun_assert(lambda: len(scirun_module_ids()) == 2)

