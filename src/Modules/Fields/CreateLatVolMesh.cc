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
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  FieldHandle   ifieldhandle;
  MatrixHandle  Size;
	
  get_input_handle("Input Field", ifieldhandle, false);
  get_input_handle("LatVol Size",Size,false);

  if (inputs_changed_ || size_x_.changed() || size_y_.changed() ||
      size_z_.changed() || padpercent_.changed() || data_at_.changed() ||
      element_size_.changed() || !oport_cached("Output Sample Field") )
#endif
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    update_state(Executing);

    if (Size.get_rep())
    {
      if (Size->get_data_size() == 1)
      {
        double* data = Size->get_data_pointer();
        unsigned int size = static_cast<unsigned int>(data[0]);
        size_x_.set(size);
        size_y_.set(size);
        size_z_.set(size);
        get_ctx()->reset();
      }
      else if (Size->get_data_size() == 3)
      {
        double* data = Size->get_data_pointer();
        unsigned int size1 = static_cast<unsigned int>(data[0]);
        unsigned int size2 = static_cast<unsigned int>(data[1]);
        unsigned int size3 = static_cast<unsigned int>(data[2]);
        size_x_.set(size1);
        size_y_.set(size2);
        size_z_.set(size3);		
        get_ctx()->reset();		
      }
      else
      {
        error("LatVol size matrix needs to have or one element or three elements");
      }	
    }	
#endif
		
    // Create blank mesh.
    auto sizex = std::max(2, get_state()->getValue(XSize).getInt());
    auto sizey = std::max(2, get_state()->getValue(YSize).getInt());
    auto sizez = std::max(2, get_state()->getValue(ZSize).getInt());

    //std::cout << "CreateLatVolMesh: dimensions are " << sizex << " x " << sizey << " x " << sizez << std::endl;

		Point minb, maxb;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    DataTypeEnum datatype;

    if (!ifieldhandle)
#endif
    {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      datatype = SCALAR;
#endif
      if (get_state()->getValue(ElementSizeNormalized).getBool())
      {
        //std::cout << "LatVol will live between 1,1,1 and -1,-1,-1" << std::endl;
        minb = Point(-1.0, -1.0, -1.0);
        maxb = Point(1.0, 1.0, 1.0);
      }
      else
      {
        //std::cout << "LatVol will live between 0,0,0 and (nx,ny,nz)" << std::endl;
        minb = Point(0.0,0.0,0.0);
        maxb = Point(static_cast<double>(sizex-1),
                     static_cast<double>(sizey-1),
                     static_cast<double>(sizez-1));
      }
    }
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    else
    {
      datatype = SCALAR;
      FieldInformation fi(ifieldhandle);
      if (fi.is_vector())
      {
        datatype = VECTOR;
      }
      else if (fi.is_tensor())
      {
        datatype = TENSOR;
      }
      BBox bbox = ifieldhandle->vmesh()->get_bounding_box();
      minb = bbox.min();
      maxb = bbox.max();
    }
#endif

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    double padScalar = get_state()->getValue(PadPercent).getDouble() / 100.0;
    Vector diag((maxb - minb) * padScalar);
    minb -= diag;
    maxb += diag;

    //TODO extract method
    int basis_order;
    auto data_at = get_state()->getValue(ElementSizeNormalized).getString();
    if (data_at == "Nodes") basis_order = 1;
    else if (data_at == "Cells") basis_order = 0;
    else if (data_at == "None") basis_order = -1;
    else 
    {
      THROW_ALGORITHM_INPUT_ERROR("Unsupported data_at location " + data_at + ".");
    }
#else
    int basis_order = 1;
#endif
    
    
    FieldInformation lfi("LatVolMesh", basis_order, "double");

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (datatype == VECTOR) 
      lfi.make_vector();
    else if (datatype == TENSOR) 
      lfi.make_tensor();
#endif


    // Create Image Field.
    MeshHandle mesh = MeshFactory::Instance().CreateMesh(lfi, MeshConstructionParameters(sizex, sizey, sizez, minb, maxb));
    sendOutput(OutputSampleField, mesh);

#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
    FieldHandle ofh = CreateField(lfi,mesh);
    ofh->vfield()->clear_all_values();

    send_output_handle("Output Sample Field", ofh);
#endif
  }
}
