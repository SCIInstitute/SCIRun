import sys

m1 = scirun_add_module("ReadMatrix")
m2 = scirun_add_module("ReportMatrixInfo")

if len(scirun_module_ids()) != 2:
  scirun_quit_after_execute()
  scirun_execute_all()
else:
  scirun_force_quit()
