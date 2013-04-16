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

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Matrix.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
//#include <Core/Util/StringUtil.h>

using namespace SCIRun::Dataflow::Networks;


namespace SCIRun {
  namespace Modules {
    namespace Fields {

      class CreateLatVol : public Module,
        Has2InputPorts<FieldPortTag, MatrixPortTag>,
        Has1OutputPort<FieldPortTag>
      {
      public:
        CreateLatVol();

        virtual void execute();

        INPUT_PORT(0, InputField, LegacyField);
        INPUT_PORT(1, LatVolSize, DenseMatrix);
        OUTPUT_PORT(0, OutputField, LegacyField);

      private:
//         GuiInt size_x_;
//         GuiInt size_y_;
//         GuiInt size_z_;
//         GuiDouble padpercent_;
//         GuiString data_at_;
//         GuiString element_size_;
// 
        enum DataTypeEnum { SCALAR, VECTOR, TENSOR };
      };

    }
  }
}

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;

CreateLatVol::CreateLatVol()
  : Module(ModuleLookupInfo("CreateLatVol", "NewField", "SCIRun"))
  /*,
  size_x_(get_ctx()->subVar("sizex"), 16),
  size_y_(get_ctx()->subVar("sizey"), 16),
  size_z_(get_ctx()->subVar("sizez"), 16),
  padpercent_(get_ctx()->subVar("padpercent"), 0.0),
  data_at_(get_ctx()->subVar("data-at"), "Nodes"),
  element_size_(get_ctx()->subVar("element-size"),"Mesh")*/
{
}


void
CreateLatVol::execute()
{
  auto ifieldhandleOption = getOptionalInput(InputField);
  auto sizeOption = getOptionalInput(LatVolSize);
	
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (inputs_changed_ || size_x_.changed() || size_y_.changed() ||
      size_z_.changed() || padpercent_.changed() || data_at_.changed() ||
      element_size_.changed() || !oport_cached("Output Sample Field") )
#endif
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    update_state(Executing);
#endif
    
    
    if (sizeOption)
    {
      auto size = *sizeOption;
      if (size->rows() == 1 && size->cols() == 1)
      {
        unsigned int size = static_cast<unsigned int>((*size)(0,0));
        size_x_.set(size);
        size_y_.set(size);
        size_z_.set(size);
        get_ctx()->reset();
      }
      else if (size->rows() == 3 && size->cols() == 1)
      {
        unsigned int size1 = static_cast<unsigned int>((*size)(0,0));
        unsigned int size2 = static_cast<unsigned int>((*size)(0,1));
        unsigned int size3 = static_cast<unsigned int>((*size)(0,2));
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

    Point minb, maxb;
    DataTypeEnum datatype;
		
    // Create blank mesh.
    VField::size_type sizex = std::max(2, size_x_.get());
    VField::size_type sizey = std::max(2, size_y_.get());
    VField::size_type sizez = std::max(2, size_z_.get());		
		
    if (!ifieldhandleOption)
    {
      datatype = SCALAR;
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
    }
    else
    {
      datatype = SCALAR;
      FieldInformation fi(*ifieldhandleOption);
      if (fi.is_vector())
      {
        datatype = VECTOR;
      }
      else if (fi.is_tensor())
      {
        datatype = TENSOR;
      }
      BBox bbox = (*ifieldhandleOption)->vmesh()->get_bounding_box();
      minb = bbox.min();
      maxb = bbox.max();
    }

    Vector diag((maxb - minb) * (padpercent_.get()/100.0));
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

    FieldInformation lfi("LatVolMesh",basis_order,"double");
    if (datatype == VECTOR) lfi.make_vector();
    else if (datatype == TENSOR) lfi.make_tensor();

    // Create Image Field.
    MeshHandle mesh = CreateMesh(lfi,sizex, sizey, sizez, minb, maxb);
    FieldHandle ofh = CreateField(lfi,mesh);
    ofh->vfield()->clear_all_values();

    sendOutput(OutputField, ofh);
  }
}
