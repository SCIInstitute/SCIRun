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


#include <Core/GeometryPrimitives/Vector.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Fields, Width);
ALGORITHM_PARAMETER_DEF(Fields, Height);
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

ALGORITHM_PARAMETER_DEF(Fields, DataLocation);

CreateImageAlgo::CreateImageAlgo()
{
  // set parameters defaults UI
  addParameter(Parameters::Width,20);
  addParameter(Parameters::Height,20);
  addParameter(Parameters::PadPercent,0.0);
  
  addParameter(Varibales::Mode, 0)
  
  addOption(Parameters::Axis,"X","X|Y|Z|Custom");
  
  addParameter(Parameters::CenterX,0);
  addParameter(Parameters::CenterY,0);
  addParameter(Parameters::CenterZ,0);
  
  addParameter(Parameters::NormalX,0);
  addParameter(Parameters::NormalX,0);
  addParameter(Parameters::NormalX,0);
  
  addParameter(Parameters::Position,0);
  
  addOption(Parameters::DataLocation,"Nodes(linear basis)","Nodes(linear basis)|Faces(constant basis)|None");

}


AlgorithmOutput CreateImageAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  auto sizeMatrix=input.get<DenseMatrix>(Varibales::SizeMatrix);
  auto oVMatrix=input.get<DenseMatrix>(Variables::OVMatrix);
  
  FieldHandle output;
  
  
  Point customNormal, customCenter;
  //pull parameter from UI
  std::string axisInput=getOption(Parameters::axis);
  int axis;
  if(axisInput=="X")
    axis=0;
  else if(axisInput=="Y")
    axis=1;
  else if(axisInput=="Z")
    axis=2;
  else if(axisInput=="Custom")
    axis=3;
  
  axis=Min(2,Max(0,axis));
  
  Transform trans;
  trans.load_identity();
  
  // checking for input matrices
  
  if(sizeMatrix)
  {
    if(sizeMatrix->get_data_size()==1)
    {
      double* data=sizeMatrix->get_data_pointer();
      unsigned int size1= static_cast<unsigned int>(data[0]);
      unsigned int size2= static_cast<unsigned int>(data[1]);
      Width.set(size1);
      Height.set(size2);
    }
    else if(sizeMatrix->get_data_size()==2)
    {
      double* data=sizeMatrix->get_data_pointer();
      unsigned int size1= static_cast<unsigned int>(data[0]);
      unsigned int size2= static_cast<usnigned int>(data[1]);
      Width.set(size1);
      Height.set(size2);
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
    
    double* data=oVMatrix->get_data_pointer();

    customCenter=Point(data[0],data[1],data[2]);
    customNormal=Point(data[3],data[4],data[5]);
    customNormal.safe_normalize();
    
  }
  
  double angle=0;
  Vector axisVector(0.0,0.0,1.0);
  switch(axis)
  {
    case 0:
      angle = M_PI * -0.5;
      axis_vector = Vector(0.0, 1.0, 0.0);
      break;
      
    case 1:
      angle = M_PI * 0.5;
      axis_vector = Vector(1.0, 0.0, 0.0);
      break;
      
    case 2:
      angle = 0.0;
      axis_vector = Vector(0.0, 0.0, 1.0);
      break;
      
    default:
      break;
  }
  
  trans.pre_rotate(angle,axisVector);
  
  if(axis==3)
  {
    customNormal=Point(get(Parameters::NormalX).toDouble(),get(Parameters::NormalY).toDouble(),get(Parameters::NormalZ).toDouble())
    Vector tmpNormal(-customNormal);
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
    const Vector &center(get(Parameters::CenterX).toDouble(),get(Parameters::CenterY).toDouble(),get(Parameters::CenterZ).toDouble());
    trans.pre_translate(center);
  }
  
  DataTypeEnum datatype;
  unsigned int width,height;
  
  if(!inputField)
  {
    datatype=SCALAR;
    // Create blank mesh.
    width=Max(2,get(Parameters::Width).toDouble());
    height=Max(2,get(Parameters::Height).toDouble());
  }
  else
  {
    datatype = SCALAR;
    FieldInformation fi(ifieldhandle);
    if (fi.is_tensor())
    {
      datatype = TENSOR;
    }
    else if (fi.is_vector())
    {
      datatype = VECTOR;
  }
    
    int basis_order=1;
    if(get(Varibales::Mode).toInt())
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
            width = Max(2, (int)lvm->get_nj());
            Width.set(width);
            height = Max(2, (int)lvm->get_nk());
            Height.set(height);
            sizez = Max(2, (int)lvm->get_ni());
            if( basis_order == 0 )
            {
              size_z_.set( sizez - 1 );
            }
            else
            {
              size_z_.set( sizez );
            }
            TCLInterface::execute(get_id()+" edit_scale");
            break;
          case 1:
            sizex =  Max(2, (int)lvm->get_ni());
            size_x_.set( sizex );
            sizey =  Max(2, (int)lvm->get_nk());
            size_y_.set( sizey );
            sizez = Max(2, (int)lvm->get_nj());
            if( basis_order == 0 )
            {
              size_z_.set( sizez - 1 );
            }
            else
            {
              size_z_.set( sizez );
            }
            TCLInterface::execute(get_id()+" edit_scale");
            break;
          case 2:
            sizex =  Max(2, (int)lvm->get_ni());
            size_x_.set( sizex );
            sizey =  Max(2, (int)lvm->get_nj());
            size_y_.set( sizey );
            sizez =  Max(2, (int)lvm->get_nk());
            if( basis_order == 0 )
            {
              size_z_.set( sizez - 1 );
            }
            else
            {
              size_z_.set( sizez );
            }
            TCLInterface::execute(get_id()+" edit_scale");
            break;
          default:
            warning("Custom axis, resize manually.");
            sizex = Max(2, size_x_.get());
            sizey = Max(2, size_y_.get());
            break;
        }
    }
  
  AlgorithmOutput result;
  result[Variables::OutputField] = output ;
  return result;
}
