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

#include <Modules/Fields/CreateLatVolBasic.h>

#include <Core/GeometryPrimitives/Point.h>
//#include <Core/Datatypes/Mesh/Field.h>
//#include <Core/Datatypes/Mesh/FieldInformation.h>
//#include <Core/Util/StringUtil.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Geometry;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
DECLARE_MAKER(CreateLatVol)
#endif

CreateLatVolBasic::CreateLatVolBasic()
  : Module(ModuleLookupInfo("CreateLatVolBasic", "NewField", "SCIRun"))
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

void CreateLatVolBasic::execute()
{
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  FieldHandle   ifieldhandle;
  MatrixHandle  Size;
	
  get_input_handle("Input Field", ifieldhandle, false);
  get_input_handle("LatVol Size",Size,false);
#endif

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (inputs_changed_ || size_x_.changed() || size_y_.changed() ||
      size_z_.changed() || padpercent_.changed() || data_at_.changed() ||
      element_size_.changed() || !oport_cached("Output Sample Field") )
#endif
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    update_state(Executing);
#endif

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
		
#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
    // Create blank mesh.
    auto sizex = std::max(2, size_x_.get());
    auto sizey = std::max(2, size_y_.get());
    auto sizez = std::max(2, size_z_.get());		
		Point minb, maxb;
#endif

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    DataTypeEnum datatype;
#endif

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (!ifieldhandle)
#endif
    {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      datatype = SCALAR;
#endif
#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
      if (element_size_.get() == "Mesh")
      {
        minb = Point(-1.0, -1.0, -1.0);
        maxb = Point(1.0, 1.0, 1.0);
      }
      else
      {
        minb = Point(0.0,0.0,0.0);
        maxb = Point(static_cast<double>(sizex-1),
                     static_cast<double>(sizey-1),
                     static_cast<double>(sizez-1));
      }
#endif
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

#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
    Vector diag((maxb.asVector() - minb.asVector()) * (padpercent_.get()/100.0));
    minb -= diag;
    maxb += diag;

    int basis_order;
    if (data_at_.get() == "Nodes") basis_order = 1;
    else if (data_at_.get() == "Cells") basis_order = 0;
    else if (data_at_.get() == "None") basis_order = -1;
    else 
    {
      error("Unsupported data_at location " + data_at_.get() + ".");
      return;
    }

    FieldInformation lfi("LatVolMesh", basis_order, "double");
#endif
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (datatype == VECTOR) 
      lfi.make_vector();
    else if (datatype == TENSOR) 
      lfi.make_tensor();
#endif

#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
    // Create Image Field.
    MeshHandle mesh = CreateMesh(lfi,sizex, sizey, sizez, minb, maxb);
    FieldHandle ofh = CreateField(lfi,mesh);
    ofh->vfield()->clear_all_values();

    send_output_handle("Output Sample Field", ofh);
#endif
  }
}
