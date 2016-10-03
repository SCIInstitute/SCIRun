__author__ = 'dwhite'
__date__ = '9/23/16'

import os

def getSourceRoot():
  return os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))))
    
def getNetworkFolder():
  return os.path.join(getSourceRoot(), "ExampleNets", "regression")

def script_folder():
  return os.path.join(getSourceRoot(), "Testing", "Python", "Unit")

def executable():
  return os.environ["SCIRUN_EXECUTABLE_PATH_FOR_PYTHON_TESTS"]
