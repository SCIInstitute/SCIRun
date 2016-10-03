__author__ = 'dwhite'
__date__ = '9/23/16'

import os
import subprocess

def getSourceRoot():
  return os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))))
    
def getNetworkFolder():
  return os.path.join(getSourceRoot(), "ExampleNets", "regression")

def script_folder():
  return os.path.join(getSourceRoot(), "Testing", "Python", "Unit")

def executable():
  return os.environ["SCIRUN_EXECUTABLE_PATH_FOR_PYTHON_TESTS"]

def check_scirun_python_script_input(file):
  subprocess.check_output([executable(), "-s", os.path.join(script_folder(), os.path.basename(file)), "--headless"])