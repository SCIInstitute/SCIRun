import unittest
import subprocess
import os

def getSourceRoot():
    return os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))))
    
def getNetworkFolder():
    return os.path.join(getSourceRoot(), "ExampleNets", "regression")

def script_folder():
    return os.path.join(getSourceRoot(), "Testing", "Python", "Unit")


class TestSCIRunAddModule(unittest.TestCase):
    executable = "SCIRun"
    def test_scirun_module_ids(self):
        subprocess.check_output([self.executable, "-s", os.path.join(script_folder(), "add_module.py"), "--headless"])
       
if __name__.startswith("Unit"):
  TestSCIRunAddModule.executable = os.environ["SCIRUN_EXECUTABLE_PATH_FOR_PYTHON_TESTS"]