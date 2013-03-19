/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#ifndef PYTHONIC_API_DIAGRAMVIEW_H
#define PYTHONIC_API_DIAGRAMVIEW_H

#include <vector>
#include <Interface/PythonTestGui/API/Share.h>
#include <boost/python.hpp>

class SCISHARE DiagramViewInterface
{
public:
  virtual ~DiagramViewInterface() {}
  virtual int numEdges() const = 0;
  virtual int numNodes() const = 0;
  virtual void addNode() = 0;
  virtual std::string firstNodeName() const = 0;
  virtual std::string removeNode(const std::string& name) = 0;
  virtual std::vector<std::string> listNodeNames() const = 0;
};

class SCISHARE DiagramView
{
public:
  static int numEdges();
  static int numNodes();
  static void addNode();
  static std::string firstNodeName();
  static std::vector<std::string> listNodeNames();
  static std::string removeNode(const std::string& name);
  static void setImpl(boost::shared_ptr<DiagramViewInterface> impl) { impl_ = impl; }
private:
  DiagramView();
  static boost::shared_ptr<DiagramViewInterface> impl_;
};

BOOST_PYTHON_MODULE(PythonAPI)
{
  //boost::python::class_<DiagramView>("DiagramView")
    boost::python::def("numEdges", &DiagramView::numEdges);
    boost::python::def("numNodes", &DiagramView::numNodes);
    boost::python::def("addNode", &DiagramView::addNode);
    boost::python::def("listNodeNames", &DiagramView::listNodeNames);
    boost::python::def("firstNodeName", &DiagramView::firstNodeName);
    boost::python::def("removeNode", &DiagramView::removeNode);
}


#endif