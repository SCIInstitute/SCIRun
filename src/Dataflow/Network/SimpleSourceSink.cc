/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Dataflow/Network/SimpleSourceSink.h>

using namespace SCIRun::Dataflow::Networks;

SimpleSink::SimpleSink() : hasData_(false)
{
}

void SimpleSink::waitForData()
{
  //do nothing
}

SCIRun::Core::Datatypes::DatatypeHandleOption SimpleSink::receive()
{
  return data_;
}

void SimpleSink::setHasData(bool dataPresent) 
{ 
  hasData_ = dataPresent; 
  if (!hasData_)
    data_.reset();
}

void SimpleSink::setData(SCIRun::Core::Datatypes::DatatypeHandle data)
{
  data_ = data;
  setHasData(true);
}

DatatypeSinkInterface* SimpleSink::clone() const
{
  return new SimpleSink;
}

void SimpleSource::send(DatatypeSinkInterfaceHandle receiver, SCIRun::Core::Datatypes::DatatypeHandle data)
{
  SimpleSink* sink = dynamic_cast<SimpleSink*>(receiver.get());
  if (!sink)
    THROW_INVALID_ARGUMENT("SimpleSource can only send to SimpleSinks");
  sink->setData(data);
}
