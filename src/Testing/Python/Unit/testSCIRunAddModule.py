import unittest
import subprocess
import os
import Unit

class TestSCIRunAddModule(unittest.TestCase):
    executable = "SCIRun"
    def test_scirun_module_ids(self):
        subprocess.check_output([self.executable, "-s", os.path.join(Unit.script_folder(), "add_module.py"), "--headless"])
       
if __name__.startswith("Unit"):
  TestSCIRunAddModule.executable = os.environ["SCIRUN_EXECUTABLE_PATH_FOR_PYTHON_TESTS"]