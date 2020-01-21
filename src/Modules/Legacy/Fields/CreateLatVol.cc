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


///
///@file  CreateLatVol.cc
///@brief Make an LatVolField that fits the source field.
///
///@author
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  March 2001
///
/// @class CreateLatVol
/// @image html CreateLatVol.png

#include <Modules/Legacy/Fields/CreateLatVol.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;

const AlgorithmParameterName CreateLatVol::XSize("XSize");
const AlgorithmParameterName CreateLatVol::YSize("YSize");
const AlgorithmParameterName CreateLatVol::ZSize("ZSize");
const AlgorithmParameterName CreateLatVol::PadPercent("PadPercent");
const AlgorithmParameterName CreateLatVol::DataAtLocation("DataAtLocation");
const AlgorithmParameterName CreateLatVol::ElementSizeNormalized("ElementSizeNormalized");

MODULE_INFO_DEF(CreateLatVol, NewField, SCIRun)

CreateLatVol::CreateLatVol()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(LatVolSize);
  INITIALIZE_PORT(OutputField);
}

void CreateLatVol::setStateDefaults()
{
  auto state = get_state();
  state->setValue(XSize, 16);
  state->setValue(YSize, 16);
  state->setValue(ZSize, 16);
  state->setValue(PadPercent, 0.0);
  state->setValue(DataAtLocation, static_cast<int>(NODES));
  state->setValue(ElementSizeNormalized, static_cast<int>(ELEMENTS_NORMALIZED));
}

/// @todo: extract algorithm class!!!!!!!!!!!!!!!!!!!!

void
CreateLatVol::execute()
{
  auto ifieldhandleOption = getOptionalInput(InputField);
  auto sizeOption = getOptionalInput(LatVolSize);

  if (needToExecute())
  {
    if (sizeOption)
    {
      auto sizeMatrix = *sizeOption;
      if (sizeMatrix->rows() == 1 && sizeMatrix->cols() == 1)
      {
        int size = static_cast<int>((*sizeMatrix)(0,0));
        get_state()->setValue(XSize, size);
        get_state()->setValue(YSize, size);
        get_state()->setValue(ZSize, size);
      }
      else if (sizeMatrix->rows() == 3 && sizeMatrix->cols() == 1)
      {
        int size1 = static_cast<int>((*sizeMatrix)(0,0));
        int size2 = static_cast<int>((*sizeMatrix)(0,1));
        int size3 = static_cast<int>((*sizeMatrix)(0,2));
        get_state()->setValue(XSize, size1);
        get_state()->setValue(YSize, size2);
        get_state()->setValue(ZSize, size3);
      }
      else
      {
        error("LatVol size matrix needs to have either one element or three elements");
        return;
      }
    }

    Point minb, maxb;
    DataTypeEnum datatype;

    // Create blank mesh.
    VField::size_type sizex = std::max(2, get_state()->getValue(XSize).toInt());
    VField::size_type sizey = std::max(2, get_state()->getValue(YSize).toInt());
    VField::size_type sizez = std::max(2, get_state()->getValue(ZSize).toInt());

    if (!ifieldhandleOption)
    {
      datatype = SCALAR;
      if (get_state()->getValue(ElementSizeNormalized).toInt() == ELEMENTS_NORMALIZED)
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
      minb = bbox.get_min();
      maxb = bbox.get_max();
    }

    double padPercent = get_state()->getValue(PadPercent).toDouble();
    Vector diag((maxb - minb) * (padPercent/100.0));
    minb -= diag;
    maxb += diag;

    int basis_order;
    auto dataAtLocation = get_state()->getValue(DataAtLocation).toInt();
    if (dataAtLocation == NODES) basis_order = 1;
    else if (dataAtLocation == CELLS) basis_order = 0;
    else if (dataAtLocation == NONE) basis_order = -1;
    else
    {
      error("Unsupported data_at location " + boost::lexical_cast<std::string>(dataAtLocation) + ".");
      return;
    }

    FieldInformation lfi("LatVolMesh",basis_order,"double");
    if (datatype == VECTOR) lfi.make_vector();
    else if (datatype == TENSOR) lfi.make_tensor();

    MeshHandle mesh = CreateMesh(lfi,sizex, sizey, sizez, minb, maxb);
    FieldHandle ofh = CreateField(lfi,mesh);
    ofh->vfield()->clear_all_values();

    sendOutput(OutputField, ofh);
  }
}
