/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
/// @todo Documentation Modules/Fields/CreateScalarFieldDataBasic.cc

#include <Modules/Fields/CreateScalarFieldDataBasic.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;

AlgorithmParameterName CreateScalarFieldDataBasic::ValueFunc("ValueFunc");
AlgorithmParameterName CreateScalarFieldDataBasic::ValueFuncParam1("ValueFuncParam1");

CreateScalarFieldDataBasic::CreateScalarFieldDataBasic()
  : Module(ModuleLookupInfo("CreateScalarFieldDataBasic", "Testing", "SCIRun"), false)
{
}

void CreateScalarFieldDataBasic::setStateDefaults()
{
  auto state = get_state();
  state->setValue(ValueFunc, std::string());
}

/// @todo: expand this module and then move it to test module category. then it can be used with python to regression test many combinations of latvol visualization!
void CreateScalarFieldDataBasic::execute()
{
  auto field = getRequiredInput(InputField);
  
  VField* vfield = field->vfield();
  VMesh* vmesh = field->vmesh();

  if (vfield && vmesh)
  {
    {
      if (vmesh->is_latvolmesh())
      {
        std::vector<index_type> dims;
        vmesh->get_dimensions(dims);
        auto nodesPerPlane = dims[0] * dims[1];

        VMesh::Node::iterator meshNodeIter;
        VMesh::Node::iterator meshNodeEnd;

        vmesh->begin(meshNodeIter);
        vmesh->end(meshNodeEnd);

        double value = 0;
        auto numNodes = vmesh->num_nodes();
        static int mult = 1;
        for (; meshNodeIter != meshNodeEnd; ++meshNodeIter)
        {
          // get edges and point from mesh node

          VMesh::Node::index_type nodeID = *meshNodeIter;
          vfield->set_value(value, nodeID);

          // by node id
          auto valueFuncName = get_state()->getValue(ValueFunc).toString();
          if (valueFuncName == "byPlane")
          {
            if ((nodeID + 1) % nodesPerPlane == 0)
            {
              value += 1;
            }
          }
          else if (valueFuncName == "random")
          {
            value = rand() / 1000;
          }
          else if (valueFuncName == "x+y+z")
          {
            Point p;
            vmesh->get_point(p, nodeID);
            value = p.x() + p.y() + p.z();
          }
          else if (valueFuncName == "distance")
          {
            Point p;
            vmesh->get_point(p, nodeID);
            value = Dot(p, p);
          }
          else if (valueFuncName == "sine")
          {
            Point p;
            vmesh->get_point(p, nodeID);
            value = sin(Dot(p,p)*mult);
          }
          else
          {
            Point p;
            vmesh->get_point(p, nodeID);
            value = sin(Dot(p,p)*10);
          }


        }
        mult += 2;
      }
    }
  }
  else
    error("VField object is null");
  


  sendOutput(OutputFieldWithData, field);
}


