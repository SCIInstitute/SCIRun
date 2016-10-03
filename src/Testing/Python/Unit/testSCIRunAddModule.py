import unittest
import subprocess
import os
import Unit

class TestSCIRunAddModule(unittest.TestCase):
    def test_scirun_module_ids(self):
        subprocess.check_output([Unit.executable(), "-s", os.path.join(Unit.script_folder(), "add_module.py"), "--headless"])
       