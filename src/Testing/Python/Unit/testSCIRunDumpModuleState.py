"""
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
"""


def scirun_test_code():
  # CreateStandardColorMap defaults cover four of the variant's types at once.
  colorMap = scirun_add_module("CreateStandardColorMap")
  state = scirun_dump_module_state(colorMap)

  # A failed scirun_assert does not stop the script, so evaluate everything up
  # front rather than risk a later check raising on a bad value.
  isDict = isinstance(state, dict)
  keys = {"ColorMapName", "ColorMapResolution", "ColorMapInvert", "ColorMapShift"}
  hasKeys = isDict and keys <= set(state)
  types = hasKeys and (
    isinstance(state["ColorMapName"], str)
    and type(state["ColorMapResolution"]) is int   # bool subclasses int
    and type(state["ColorMapInvert"]) is bool
    and isinstance(state["ColorMapShift"], float))

  scirun_set_module_state(colorMap, "ColorMapResolution", 64)
  updated = scirun_dump_module_state(colorMap)
  reflectsLiveState = isinstance(updated, dict) and updated.get("ColorMapResolution") == 64

  missingModuleIsNone = scirun_dump_module_state("NotAModule:0") is None

  scirun_assert(lambda: isDict)
  scirun_assert(lambda: hasKeys)
  scirun_assert(lambda: types)
  scirun_assert(lambda: reflectsLiveState)
  scirun_assert(lambda: missingModuleIsNone, exit=True)

if __name__ == "__main__":
  scirun_test_code()
else:
  import unittest
  import Unit

  class TestSCIRunDumpModuleState(unittest.TestCase):
    def test_scirun_dump_module_state(self):
      Unit.check_scirun_python_script_input(__file__)
