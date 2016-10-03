def scirun_test_code():
  m1 = scirun_add_module("ReadMatrix")
  m2 = scirun_add_module("ReportMatrixInfo")
  scirun_assert(lambda: len(scirun_module_ids()) == 2)
  scirun_assert(lambda: scirun_module_ids()[0] == "ReadMatrix:0")
  scirun_assert(lambda: scirun_module_ids()[1] == "ReportMatrixInfo:0", exit=True)

if __name__ == "__main__":
  scirun_test_code()
else:
  import unittest
  import Unit
  
  class TestSCIRunAddModule(unittest.TestCase):
    def test_scirun_module_ids(self):
      Unit.check_scirun_python_script_input(__file__)
       