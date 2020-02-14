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


#ifndef INTERFACE_APPLICATION_PORTWIDGETMANAGER_H
#define INTERFACE_APPLICATION_PORTWIDGETMANAGER_H

#ifndef Q_MOC_RUN
#include <boost/range/join.hpp>

#include <Dataflow/Network/NetworkFwd.h>
#endif

namespace SCIRun {
namespace Gui {

class PortWidget;
class InputPortWidget;
class OutputPortWidget;

class PortWidgetManager
{
public:
  typedef std::deque<PortWidget*> Ports;

  auto getAllPorts() const -> decltype(boost::join(Ports(), Ports()))
  {
    return boost::join(inputPorts_, outputPorts_);
  }

  const Ports& inputs() const { return inputPorts_; }
  const Ports& outputs() const { return outputPorts_; }

  size_t numInputPorts() const { return inputPorts_.size(); }
  size_t numOutputPorts() const { return outputPorts_.size(); }

  void addPort(InputPortWidget* port);
  void insertPort(int index, InputPortWidget* port);
  void addPort(OutputPortWidget* port);
  bool removeDynamicPort(const SCIRun::Dataflow::Networks::PortId& pid, QHBoxLayout* layout);
  void addInputsToLayout(QHBoxLayout* layout);
  void addOutputsToLayout(QHBoxLayout* layout);
  void reindexInputs();
  void setHighlightPorts(bool on);
  void setSceneFunc(SceneFunc getScene);

private:
  Ports inputPorts_, outputPorts_;
  SceneFunc getScene_;
};


}
}

#endif
