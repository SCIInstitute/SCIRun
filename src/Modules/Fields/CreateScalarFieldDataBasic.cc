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

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

/*
 *  CreateLatVol.cc:  Make an ImageField that fits the source field.
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   March 2001
 *
 */

#include <Modules/Fields/CreateLatVolMesh.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Mesh/MeshFactory.h>
#include <Core/Datatypes/Mesh/Mesh.h>
//#include <Core/Datatypes/Mesh/Field.h>
#include <Core/Datatypes/Mesh/FieldInformation.h>
//#include <Core/Util/StringUtil.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

AlgorithmParameterName CreateLatVolMesh::XSize("X size");
AlgorithmParameterName CreateLatVolMesh::YSize("Y size");
AlgorithmParameterName CreateLatVolMesh::ZSize("Z size");
AlgorithmParameterName CreateLatVolMesh::PadPercent("Pad Percentage");
AlgorithmParameterName CreateLatVolMesh::ElementSizeNormalized("ElementSizeNormalized");

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
DECLARE_MAKER(CreateLatVol)
#endif

CreateLatVolMesh::CreateLatVolMesh()
  : Module(ModuleLookupInfo("CreateLatVolMesh", "NewField", "SCIRun"))
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  , size_x_(get_ctx()->subVar("sizex"), 16),
    size_y_(get_ctx()->subVar("sizey"), 16),
    size_z_(get_ctx()->subVar("sizez"), 16),
    padpercent_(get_ctx()->subVar("padpercent"), 0.0),
    data_at_(get_ctx()->subVar("data-at"), "Nodes"),
    element_size_(get_ctx()->subVar("element-size"),"Mesh")
#endif
{
}

void CreateLatVolMesh::execute()
{
