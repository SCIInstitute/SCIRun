/*
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
*/


//#include <Python.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include <gtest/gtest.h>
// #include <Testing/ModuleTestBase/ModuleTestBase.h>
// #include <Core/Python/PythonDatatypeConverter.h>
// #include <Core/Datatypes/Legacy/Field/FieldInformation.h>
// #include <Core/Matlab/matlabconverter.h>
// #include <Core/Datatypes/Legacy/Field/VMesh.h>
// #include <Testing/Utils/MatrixTestUtilities.h>
// #include <Testing/Utils/SCIRunUnitTests.h>
// #include <Testing/Utils/SCIRunFieldSamples.h>
// #ifdef WIN32
// #ifndef DEBUG
// #include <Core/Python/PythonInterpreter.h>
// #endif
// #endif

#include <Core/Python/PythonInterpreter.h>
#include <boost/filesystem.hpp>


int add(int i, int j);
//
//PYBIND11_MODULE(example, m)
//{
//  m.doc() = "pybind11 example plugin"; // optional module docstring
//  m.def("add", &add, "A function which adds two numbers");
//}
