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

#include <Modules/Visualization/ShowMesh.h>
#include <Core/Datatypes/Mesh/Mesh.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/Datatypes/Geometry.h>
#include <boost/foreach.hpp>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

ShowMeshModule::ShowMeshModule() : Module(ModuleLookupInfo("ShowMesh", "Visualization", "SCIRun")) {}

void ShowMeshModule::execute()
{
  auto mesh = getRequiredInput(Mesh);
  
  {  //TODO: temporary code to sanity-check CreateLatVolMesh.
    auto facade = mesh->getFacade();

    if (facade)
    {
      std::cout << "Hey, I got a mesh facade!" << std::endl;
      std::cout << "Here are the nodes:" << std::endl;

      BOOST_FOREACH(const NodeInfo& node, facade->nodes())
      {
        std::cout << "Node " << node.index() << " point=" << node.point().get_string() << std::endl;
      }
    }
  }

  GeometryHandle geom(new GeometryObject(mesh));
  sendOutput(SceneGraph, geom);
}