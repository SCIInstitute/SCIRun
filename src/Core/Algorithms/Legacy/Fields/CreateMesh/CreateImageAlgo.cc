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

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/Fields/CreateMesh/CreateImageAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Math/MiscMath.h> // for M_PI
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Point.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
//using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;

ALGORITHM_PARAMETER_DEF(Fields, Width);
ALGORITHM_PARAMETER_DEF(Fields, Height);
ALGORITHM_PARAMETER_DEF(Fields, Depth);
ALGORITHM_PARAMETER_DEF(Fields, PadPercent);

ALGORITHM_PARAMETER_DEF(Fields, Mode);

ALGORITHM_PARAMETER_DEF(Fields, Axis);

ALGORITHM_PARAMETER_DEF(Fields, CenterX);
ALGORITHM_PARAMETER_DEF(Fields, CenterY);
ALGORITHM_PARAMETER_DEF(Fields, CenterZ);

ALGORITHM_PARAMETER_DEF(Fields, NormalX);
ALGORITHM_PARAMETER_DEF(Fields, NormalY);
ALGORITHM_PARAMETER_DEF(Fields, NormalZ);

ALGORITHM_PARAMETER_DEF(Fields, Position);
ALGORITHM_PARAMETER_DEF(Fields, Index);

ALGORITHM_PARAMETER_DEF(Fields, DataLocation);

CreateImageAlgo::CreateImageAlgo()
{
  
  // set parameters defaults UI
  addParameter(Parameters::Width,20);
  addParameter(Parameters::Height,20);
  addParameter(Parameters::Depth, 2);
  addParameter(Parameters::PadPercent,0.0);
  
  addOption(Parameters::Mode, "Manual", "Manual|Auto");
  
  addOption(Parameters::Axis,"X","X|Y|Z|Custom");
  
  addParameter(Parameters::CenterX,0);
  addParameter(Parameters::CenterY,0);
  addParameter(Parameters::CenterZ,0);
  
  addParameter(Parameters::NormalX,0);
  addParameter(Parameters::NormalX,0);
  addParameter(Parameters::NormalX,0);
  
  addParameter(Parameters::Position,0);
  addParameter(Parameters::Index,0);
  
  addOption(Parameters::DataLocation,"Nodes(linear basis)","Nodes(linear basis)|Faces(constant basis)|None");

}

const AlgorithmInputName CreateImageAlgo::OVMatrix("OVMatrix");
const AlgorithmInputName CreateImageAlgo::SizeMatrix("SizeMatrix");

AlgorithmOutput CreateImageAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  auto sizeMatrix=input.get<DenseMatrix>(SizeMatrix);
  auto oVMatrix=input.get<DenseMatrix>(OVMatrix);
  //auto state=get_state();
  FieldHandle output;
  
  
  Point customNormal, customCenter;
  //pull parameter from UI
  std::string axisInput=getOption(Parameters::Axis);
  int axisTemp;
  if(axisInput=="X")
    axisTemp=0;
  else if(axisInput=="Y")
    axisTemp=1;
  else if(axisInput=="Z")
    axisTemp=2;
  else if(axisInput=="Custom")
    axisTemp=3;
  
  auto axis=std::min(2, std::max(0,0,axisTemp));
  
  Transform trans;
  trans.load_identity();
  
  // checking for input matrices
  
  if(sizeMatrix)
  {
    if(sizeMatrix->rows()==1 && sizeMatrix->cols()==1)
    {
      //double* data=sizeMatrix->getDatapointer();
      const unsigned int size1= static_cast<unsigned int>((*sizeMatrix)(0,0));
      const unsigned int size2= static_cast<unsigned int>((*sizeMatrix)(0,0));
      set(const Parameters::Width, size1);
      set(const Parameters::Height, size2);
    }
    else if(sizeMatrix->rows()==2 && sizeMatrix->cols()==1)
    {
      //double* data=sizeMatrix->get_data_pointer();
      unsigned int size1= static_cast<unsigned int>((*sizeMatrix)(0,0));
      unsigned int size2= static_cast<unsigned int>((*sizeMatrix)(0,1));
      Parameters::Width.set(size1);
      Parameters::Height.set(size2);
    }
    else
    {
      THROW_ALGORITHM_INPUT_ERROR("Image Size matrix must have only 1 or 2 elements");
    }
  }
  
  if(oVMatrix)
  {
    if(oVMatrix->nrows()!=2 || oVMatrix->ncols()!=3)
    {
      THROW_ALGORITHM_INPUT_ERROR("Custom Center and Nomal matrix must be of size 2x3. The Center is the first row and the normal is the second");
    }
    
    //double* data=oVMatrix->get_data_pointer();

    customCenter=Point((*oVMatrix)(0,0),(*oVMatrix)(0,1),(*oVMatrix)(0,2));
    customNormal=Point((*oVMatrix)(1,0),(*oVMatrix)(1,1),(*oVMatrix)(1,2));
    customNormal.safe_normalize();
    
  }
  
  double angle=0;
  Vector axisVector(0.0,0.0,1.0);
  switch(axis)
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
  
  trans.pre_rotate(angle,axisVector);
  
  if(axis==3)
  {
    customNormal=Point(get(Parameters::NormalX).toDouble(),get(Parameters::NormalY).toDouble(),get(Parameters::NormalZ).toDouble());
    Vector tempNormal(-customNormal);
    Vector fakey(Cross(Vector(0.0,0.0,1.0),tempNormal));
    
    if(fakey.length2()<1.0e-6)
      fakey=Cross(Vector(1.0,0.0,0.0),tempNormal);
    Vector fakex(Cross(tempNormal,fakey));
    tempNormal.safe_normalize();
    fakex.safe_normalize();
    fakey.safe_normalize();
    double dg=1.0;
    
    if(inputField)
    {
      BBox box=inputField->vmesh()->get_bounding_box();
      Vector diag(box.diagonal());
      dg=diag.maxComponent();
      trans.pre_scale(Vector(dg,dg,dg));
    }
    Transform trans2;
    trans2.load_identity();
    trans2.load_basis(Point(0,0,0), fakex, fakey, tempNormal);
    trans2.invert();
    trans.change_basis(trans2);
    customCenter=Point(get(Parameters::CenterX).toDouble(),get(Parameters::CenterY).toDouble(),get(Parameters::CenterZ).toDouble());
    trans.pre_translate(customCenter);
  }
  
  DataTypeEnum datatype;
  unsigned int width,height, depth;
  
  if(!inputField)
  {
    datatype=SCALAR;
    // Create blank mesh.
    width=std::max(2,get(Parameters::Width).toInt());
    height=std::max(2,get(Parameters::Height).toInt());
  }
  else
  {
    datatype = SCALAR;
    FieldInformation fi(inputField);
    if (fi.is_tensor())
    {
      datatype = TENSOR;
    }
    else if (fi.is_vector())
    {
      datatype = VECTOR;
    }
    
    int basis_order=1;
    if(getOption(Parameters::Mode)=="Auto")
    {
      // Guess at the size of the sample plane.
      // Currently we have only a simple algorithm for LatVolFields.
      
      if (fi.is_latvolmesh())
      {
        VMesh *lvm = inputField->vmesh();
        basis_order = inputField->vfield()->basis_order();
        
        switch(axis)
        {
          case 0:
            width = std::max(2, (int)lvm->get_nj());
            Width.set(width);
            height = std::max(2, (int)lvm->get_nk());
            Height.set(height);
            depth = std::max(2, (int)lvm->get_ni());
            if( basis_order == 0 )
            {
              Depth.set( depth - 1 );
            }
            else
            {
              Depth.set( depth );
            }
            //TCLInterface::execute(get_id()+" edit_scale");
            break;
          case 1:
            width =  std::max(2, (int)lvm->get_ni());
            Width.set( width );
            height =  std::max(2, (int)lvm->get_nk());
            Height.set( height );
            depth = std::max(2, (int)lvm->get_nj());
            if( basis_order == 0 )
            {
              Depth.set( depth - 1 );
            }
            else
            {
              Depth.set( depth );
            }
            //TCLInterface::execute(get_id()+" edit_scale");
            break;
          case 2:
            width =  std::max(2, (int)lvm->get_ni());
            Width.set( width );
            height =  std::max(2, (int)lvm->get_nj());
            Height.set( height );
            depth =  std::max(2, (int)lvm->get_nk());
            if( basis_order == 0 )
            {
              Depth.set( depth - 1 );
            }
            else
            {
              Depth.set( depth );
            }
            //TCLInterface::execute(get_id()+" edit_scale");
            break;
          default:
            warning("Custom axis, resize manually.");
            sizex = std::max(2, get(Parameters::Width).toInt());
            sizey = std::max(2, get(Parameters::Height).toInt());
            break;
        }
    }
    else
    {
      warning("No autosize algorithm for this field type, resize manually.");
      sizex = std::max(2, get(Parameters::Width).toInt());
      sizey = std::max(2, get(Parameters::Height).toInt());
      Mode.set("Manual");
      //TCLInterface::execute(get_id()+" edit_scale");
    }
  }
    else
    {
      // Create blank mesh.
      width = std::max(2, get(Parameters::Width.toInt()));
      height = std::max(2, get(Parameters::Height.toInt()));
    }
    
    if(axis!=3)
    {
      BBox box = inputField->vmesh()->get_bounding_box();
      Vector diag(box.diagonal());
      trans.pre_scale(diag);
      
      Point loc(box.center());
      Position.reset();
      double dist;
      if (getOption(Parameters::Mode)=="Manual")
      {
        dist = get(Parameters::Position).toDouble()/2.0;
      }
      else
      {
        if( basis_order == 0 )
        {
          dist = double( get(Parameters::Index).toInt())/ get(Parameters::Depth).toDouble() + 0.5/get(Parameters::Depth).toDouble();
          Position.set( ( dist) * 2.0 );
        }
        else
        {
          dist = double( get(Parameters::Index)toInt() )/ get(Parameters::Depth);
          Position.set( ( dist) * 2.0 );
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
  Vector diag((maxb.asVector() - minb.asVector()) * (get(Parameters::PadPercent).toDouble()/100.0));
  minb -= diag;
  maxb += diag;
  
  int basis_order;
  if (getOption(Parameters::DataLocation) == "Nodes") basis_order = 1;
  else if (getOption(Parameters::DataLocation) == "Faces") basis_order = 0;
  else if (getOption(Parameters::DataLocation) == "None") basis_order = -1;
  else
  {
    error("Unsupported data_at location " + getOption(Parameters::DataLocation) + ".");
    return;
  }
  
  FieldInformation ifi("ImageMesh",basis_order,"double");
  
  if (datatype == VECTOR) ifi.make_vector();
  else if (datatype == TENSOR) ifi.make_tensor();
  
  MeshHandle imagemesh = CreateMesh(ifi,sizex, sizey, minb, maxb);
  output = CreateField(ifi,imagemesh);
  
  // Transform field.
  output->vmesh()->transform(trans);
  
  
  
  AlgorithmOutput result;
  result[Variables::OutputField] = output ;
  return result;
}
