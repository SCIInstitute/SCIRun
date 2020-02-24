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


#include <Modules/Legacy/Fields/CreateImage.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Math/MiscMath.h> // for M_PI

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;

const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::Width("Width");
const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::Height("Height");
const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::Depth("Depth");
const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::PadPercent("PadPercent");

const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::Mode("Mode");
const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::Axis("Axis");

const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::CenterX("CenterX");
const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::CenterY("CenterY");
const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::CenterZ("CenterZ");

const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::NormalX("NormalX");
const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::NormalY("NormalY");
const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::NormalZ("NormalZ");

const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::Position("Position");
const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::Index("Index");

const SCIRun::Core::Algorithms::AlgorithmParameterName CreateImage::DataLocation("DataLocation");

MODULE_INFO_DEF(CreateImage, NewField, SCIRun)

CreateImage::CreateImage() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(SizeMatrix);
  INITIALIZE_PORT(OVMatrix);
  INITIALIZE_PORT(OutputField);
}

void CreateImage::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Width, 20);
  state->setValue(Height, 20);
  state->setValue(Depth, 2);
  state->setValue(PadPercent, 0.0);

  state->setValue(Mode, std::string("Manual"));
  state->setValue(Axis, std::string("X"));

  state->setValue(CenterX, 0);
  state->setValue(CenterY, 0);
  state->setValue(CenterZ, 0);

  state->setValue(NormalX, 1);
  state->setValue(NormalY, 1);
  state->setValue(NormalZ, 1);

  state->setValue(Position, 0);
  state->setValue(Index, 0);

  state->setValue(DataLocation, std::string("Nodes(linear basis)"));
}

void CreateImage::execute()
{
  auto inputField = getOptionalInput(InputField);
  auto sizeOption = getOptionalInput(SizeMatrix);
  auto oVMatrixInput = getOptionalInput(OVMatrix);

  FieldHandle output;

  if (needToExecute())
  {
    Point customCenter;
    Vector customNormal;
    std::string axisInput = get_state()->getValue(Axis).toString();

    int axisTemp;
    if (axisInput == "X")
      axisTemp = 0;
    else if (axisInput == "Y")
      axisTemp = 1;
    else if (axisInput == "Z")
      axisTemp = 2;
    else if (axisInput == "Custom")
      axisTemp = 3;

    auto axis = std::min(3, std::max(0, axisTemp));

    Transform trans;
    trans.load_identity();

    if (sizeOption)
    {
      auto sizeMatrix = *sizeOption;
      // checking for input matrices
      if (sizeMatrix)
      {
        if (sizeMatrix->nrows() == 1 && sizeMatrix->ncols() == 1)
        {
          //double* data=sizeMatrix->getDatapointer();
          const int size1 = static_cast<int>((*sizeMatrix)(0, 0));
          const int size2 = static_cast<int>((*sizeMatrix)(0, 0));
          get_state()->setValue(Width, size1);
          get_state()->setValue(Height, size2);
        }
        else if (sizeMatrix->nrows() == 2 && sizeMatrix->ncols() == 1)
        {
          //double* data=sizeMatrix->get_data_pointer();
          int size1 = static_cast<int>((*sizeMatrix)(0, 0));
          int size2 = static_cast<int>((*sizeMatrix)(0, 1));
          get_state()->setValue(Width, size1);
          get_state()->setValue(Height, size2);
        }
        else
        {
          error("Image Size matrix must have only 1 or 2 elements");
        }
      }
    }

    if (oVMatrixInput)
    {
      auto oVMatrix = *oVMatrixInput;
      if (oVMatrix)
      {
        if (oVMatrix->nrows() != 2 || oVMatrix->ncols() != 3)
        {
          error("Custom Center and Nomal matrix must be of size 2x3. The Center is the first row and the normal is the second");
        }
        customCenter = Point((*oVMatrix)(0, 0), (*oVMatrix)(0, 1), (*oVMatrix)(0, 2));
        customNormal = Vector((*oVMatrix)(1, 0), (*oVMatrix)(1, 1), (*oVMatrix)(1, 2));
        customNormal.safe_normalize();
      }
    }

    double angle = 0;
    Vector axisVector(0.0, 0.0, 1.0);
    switch (axis)
    {
      case 0:
        angle = M_PI * -0.5;
        axisVector = Vector(0.0, 1.0, 0.0);
        break;

      case 1:
        angle = M_PI * 0.5;
        axisVector = Vector(1.0, 0.0, 0.0);
        break;

      case 2:
        angle = 0.0;
        axisVector = Vector(0.0, 0.0, 1.0);
        break;

      default:
        break;
    }

    trans.pre_rotate(angle, axisVector);

    if (axis == 3)
    {
      customNormal = Vector(get_state()->getValue(NormalX).toDouble(), get_state()->getValue(NormalY).toDouble(), get_state()->getValue(NormalZ).toDouble());
      Vector tempNormal(-customNormal);
      Vector fakey(Cross(Vector(0.0, 0.0, 1.0), tempNormal));

      if (fakey.length2() < 1.0e-6)
        fakey = Cross(Vector(1.0, 0.0, 0.0), tempNormal);
      Vector fakex(Cross(tempNormal, fakey));
      tempNormal.safe_normalize();
      fakex.safe_normalize();
      fakey.safe_normalize();
      double dg = 1.0;

      if (inputField)
      {
        BBox box = (*inputField)->vmesh()->get_bounding_box();
        Vector diag(box.diagonal());
        dg = diag.maxComponent();
        trans.pre_scale(Vector(dg, dg, dg));
      }
      Transform trans2;
      trans2.load_identity();
      trans2.load_basis(Point(0, 0, 0), fakex, fakey, tempNormal);
      trans2.invert();
      trans.change_basis(trans2);
      customCenter = Point(get_state()->getValue(CenterX).toDouble(), get_state()->getValue(CenterY).toDouble(), get_state()->getValue(CenterZ).toDouble());
      trans.pre_translate(Vector(customCenter));
    }

    DataTypeEnum datatype;
    int width, height, depth;

    if (!inputField)
    {
      datatype = SCALAR;
      // Create blank mesh.
      width = std::max(2, get_state()->getValue(Width).toInt());
      height = std::max(2, get_state()->getValue(Height).toInt());
    }
    else
    {
      datatype = SCALAR;
      FieldInformation fi(*inputField);
      if (fi.is_tensor())
      {
        datatype = TENSOR;
      }
      else if (fi.is_vector())
      {
        datatype = VECTOR;
      }
      int basis_order = 1;
      if (get_state()->getValue(Mode).toString() == "Auto")
      {
        // Guess at the size of the sample plane.
        // Currently we have only a simple algorithm for LatVolFields.

        if (fi.is_latvolmesh())
        {
          VMesh *lvm = (*inputField)->vmesh();
          basis_order = (*inputField)->vfield()->basis_order();

          switch (axis)
          {
            case 0:
              width = std::max(2, (int)lvm->get_nj());
              get_state()->setValue(Width, width);
              height = std::max(2, (int)lvm->get_nk());
              get_state()->setValue(Height, height);
              depth = std::max(2, (int)lvm->get_ni());
              if (basis_order == 0)
              {
                get_state()->setValue(Depth, depth - 1);
              }
              else
              {
                get_state()->setValue(Depth, depth);
              }
              //TCLInterface::execute(get_id()+" edit_scale");
              break;
            case 1:
              width = std::max(2, (int)lvm->get_ni());
              get_state()->setValue(Width, width);
              height = std::max(2, (int)lvm->get_nk());
              get_state()->setValue(Height, height);
              depth = std::max(2, (int)lvm->get_nj());
              if (basis_order == 0)
              {
                get_state()->setValue(Depth, depth - 1);
              }
              else
              {
                get_state()->setValue(Depth, depth);
              }
              //TCLInterface::execute(get_id()+" edit_scale");
              break;
            case 2:
              width = std::max(2, (int)lvm->get_ni());
              get_state()->setValue(Width, width);
              height = std::max(2, (int)lvm->get_nj());
              get_state()->setValue(Height, height);
              depth = std::max(2, (int)lvm->get_nk());
              if (basis_order == 0)
              {
                get_state()->setValue(Depth, depth - 1);
              }
              else
              {
                get_state()->setValue(Depth, depth);
              }
              //TCLInterface::execute(get_id()+" edit_scale");
              break;
            default:
              warning("Custom axis, resize manually.");
              width = std::max(2, get_state()->getValue(Width).toInt());
              height = std::max(2, get_state()->getValue(Height).toInt());
              break;
          }
        }
        else
        {
          warning("No autosize algorithm for this field type, resize manually.");
          width = std::max(2, get_state()->getValue(Width).toInt());
          height = std::max(2, get_state()->getValue(Height).toInt());
          get_state()->setValue(Mode, std::string("Manual"));
          //TCLInterface::execute(get_id()+" edit_scale");
        }
      }
      else
      {
        // Create blank mesh.
        width = std::max(2, get_state()->getValue(Width).toInt());
        height = std::max(2, get_state()->getValue(Height).toInt());
      }

      if (axis != 3)
      {
        BBox box = (*inputField)->vmesh()->get_bounding_box();
        Vector diag(box.diagonal());
        trans.pre_scale(diag);

        Point loc(box.center());
        double dist;
        if (get_state()->getValue(Mode).toString() == "Manual")
        {
          dist = get_state()->getValue(Position).toDouble() / 2.0;
        }
        else
        {
          if (basis_order == 0)
          {
            dist = double(get_state()->getValue(Index).toInt()) / get_state()->getValue(Depth).toDouble() + 0.5 / get_state()->getValue(Depth).toDouble();
            get_state()->setValue(Position, (dist)* 2.0);
          }
          else
          {
            dist = double(get_state()->getValue(Index).toInt()) / get_state()->getValue(Depth).toDouble();
            get_state()->setValue(Position, (dist)* 2.0);
          }
        }
        switch (axis)
        {
          case 0:
            loc.x(loc.x() + diag.x() * dist);
            break;

          case 1:
            loc.y(loc.y() + diag.y() * dist);
            break;

          case 2:
            loc.z(loc.z() + diag.z() * dist);
            break;

          default:
            break;
        }
        trans.pre_translate(Vector(loc));
      }
    }

    Point minb(-0.5, -0.5, 0.0);
    Point maxb(0.5, 0.5, 0.0);
    Vector diag((Vector(maxb) - Vector(minb)) * (get_state()->getValue(PadPercent).toDouble() / 100.0));
    minb -= diag;
    maxb += diag;

    int basis_order = 1;
    if (get_state()->getValue(DataLocation).toString() == "Nodes(linear basis)")
      basis_order = 1;
    else if (get_state()->getValue(DataLocation).toString() == "Faces(constant basis)")
      basis_order = 0;
    else if (get_state()->getValue(DataLocation).toString() == "None")
      basis_order = -1;
    /*else
     {
     error("Unsupported data_at location " + getOption(Parameters::DataLocation) + ".");
     AlgorithmOutput result;
     return result;
     }*/

    FieldInformation ifi("ImageMesh", basis_order, "double");

    if (datatype == VECTOR)
      ifi.make_vector();
    else if (datatype == TENSOR)
      ifi.make_tensor();

    MeshHandle imagemesh = CreateMesh(ifi, width, height, minb, maxb);
    output = CreateField(ifi, imagemesh);

    output->vmesh()->transform(trans);

    sendOutput(OutputField, output);
  }
}
