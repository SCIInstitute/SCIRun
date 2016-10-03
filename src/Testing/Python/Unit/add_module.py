m1 = scirun_add_module("ReadMatrix")
m2 = scirun_add_module("ReportMatrixInfo")
scirun_assert(lambda: len(scirun_module_ids()) == 2)

