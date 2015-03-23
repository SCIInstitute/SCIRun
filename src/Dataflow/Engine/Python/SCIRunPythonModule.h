/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
/// @todo Documentation Dataflow/Engine/Python/SCIRunPythonModule.h

#ifndef ENGINE_PYTHON_SCIRUNPYTHONMODULE_H
#define ENGINE_PYTHON_SCIRUNPYTHONMODULE_H

#include <boost/python.hpp>
#include <Dataflow/Engine/Python/NetworkEditorPythonInterface.h>
#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>
#include <Dataflow/Engine/Python/share.h>

BOOST_PYTHON_MODULE(SCIRunPythonAPI)
{
  using namespace SCIRun;

  boost::python::class_<PyPort, boost::shared_ptr<PyPort>, boost::noncopyable>("SCIRun::PyPort", boost::python::no_init)
    .add_property("name", &PyPort::name)
    .add_property("type", &PyPort::type)
    .add_property("isInput", &PyPort::isInput)
    .def(boost::python::self >> boost::python::self)
    .def("connect", &PyPort::connect)
    ;

  boost::python::class_<PyPorts, boost::shared_ptr<PyPorts>, boost::noncopyable>("SCIRun::PyPorts", boost::python::no_init)
    .def("__getattr__", &PyPorts::getattr)
    .def("__getitem__", &PyPorts::getitem)
    .def("__len__", &PyPorts::size)
    ;

  boost::python::class_<PyModule, boost::shared_ptr<PyModule>, boost::noncopyable>("SCIRun::PyModule", boost::python::no_init)
    .add_property("id", &PyModule::id)
    .add_property("stateVars", &PyModule::stateVars)
    .add_property("input", &PyModule::input)
    .add_property("output", &PyModule::output)
    .def("showUI", &PyModule::showUI)
    .def("hideUI", &PyModule::hideUI)
    .def("__getattr__", &PyModule::getattr)
    .def("__setattr__", &PyModule::setattr)
    ;

  boost::python::def("addModule", &NetworkEditorPythonAPI::addModule);
  boost::python::def("removeModule", &NetworkEditorPythonAPI::removeModule);
  boost::python::def("modules", &NetworkEditorPythonAPI::modules);
  boost::python::def("executeAll", &NetworkEditorPythonAPI::executeAll);
  boost::python::def("saveNetwork", &NetworkEditorPythonAPI::saveNetwork);
  boost::python::def("loadNetwork", &NetworkEditorPythonAPI::loadNetwork);
  boost::python::def("quit", &NetworkEditorPythonAPI::quit);
}

#endif
