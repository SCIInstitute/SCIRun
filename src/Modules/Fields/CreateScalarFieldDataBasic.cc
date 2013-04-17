/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Modules/Fields/CreateScalarFieldDataBasic.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

AlgorithmParameterName CreateScalarFieldDataBasic::DataMap("DataMap");

CreateScalarFieldDataBasic::CreateScalarFieldDataBasic()
  : Module(ModuleLookupInfo("CreateScalarFieldDataBasic", "NewField", "SCIRun"), false)
{
}

void CreateScalarFieldDataBasic::execute()
{
  auto field = getRequiredInput(InputField);
  
  VField* vfield = field->vfield();
  VMesh* vmesh = field->vmesh();

  if (vfield && vmesh)
  {
    std::cout << "Assuming values on nodes." << std::endl;
    {
      VMesh::Node::iterator meshNodeIter;
      VMesh::Node::iterator meshNodeEnd;

      vmesh->begin(meshNodeIter);
      vmesh->end(meshNodeEnd);
      std::ostringstream ostr;

      double value = 0.1;
      for (; meshNodeIter != meshNodeEnd; ++meshNodeIter)
      {
        // get edges and point from mesh node

        VMesh::Node::index_type nodeID = *meshNodeIter;
        value += 0.123;
        vfield->set_value(value, nodeID);
        std::cout << "Set value " << value << " at node " << nodeID << std::endl;
      }
    }
  }
  else
    error("VField object is null");

  sendOutput(OutputFieldWithData, field);
}
