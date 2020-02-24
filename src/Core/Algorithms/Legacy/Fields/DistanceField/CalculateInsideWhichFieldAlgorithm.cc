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


#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateInsideWhichFieldAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/Vector.h>

#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoNodes.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/ConvertFieldBasisType.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateIsInsideField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;


ALGORITHM_PARAMETER_DEF(Fields, ChangeOutsideValue);
ALGORITHM_PARAMETER_DEF(Fields, StartValue);
ALGORITHM_PARAMETER_DEF(Fields, DataLocation);

CalculateInsideWhichFieldAlgorithm::CalculateInsideWhichFieldAlgorithm()
{
  // set parameters defaults UI
  addOption(Parameters::Method,"one","one|most|all");
  addOption(Parameters::SamplingScheme,"regular1","regular1|regular2|regular3|regular4|regular5");
  addOption(Parameters::ChangeOutsideValue,"true","true|false");
  addParameter(Parameters::OutsideValue, 0.0);
  addParameter(Parameters::StartValue, 1.0);
  addOption(Parameters::OutputType, "same as input", "same as input|char|short|unsigned short|unsigned int| int|float|double");
  addOption(Parameters::DataLocation, "element", "element|node");
}


FieldHandle CalculateInsideWhichFieldAlgorithm::run(FieldHandle input,const std::vector<FieldHandle>& objField) const
{
  FieldHandle output;
  std::vector<FieldHandle> inputs;
  std::copy_if(objField.begin(), objField.end(), std::back_inserter(inputs), [](FieldHandle f){return f;});

  //pull parameter from UI
  std::string method=getOption(Parameters::Method);

  if(!input)
  {
    THROW_ALGORITHM_INPUT_ERROR("No input fields given");
    return FieldHandle();
  }

  for(size_t p=0;p<objField.size();p++)
  {
    if(!objField[p])
    {
      error("No object field");
      return FieldHandle();
    }
  }

  // no precompiled version available so compile one
  FieldInformation fi(input);
  FieldInformation fo(input);

  if(fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    return FieldHandle();
  }

  std::string outputType=getOption(Parameters::OutputType);

  if(outputType!="same as input")
  {
    fo.set_data_type(outputType);
  }

  if(fo.is_vector()) fo.make_double();
  if(fo.is_tensor()) fo.make_double();

  fo.make_constantdata();

  if(checkOption(Parameters::DataLocation,"node"))
    fo.make_lineardata();

  output=CreateField(fo,input->mesh());

  if(!output)
  {
    error("Could not create output field");
    return FieldHandle();
  }

  // For the moment we calculate everything in doubles

  VField* ifield=input->vfield();

  VMesh* omesh=output->vmesh();
  VField* ofield=output->vfield();

  double outsideValue=get(Parameters::OutsideValue).toDouble();
  double startValue=get(Parameters::StartValue).toDouble();
  std::string changeOutsideValue=getOption(Parameters::ChangeOutsideValue);

  if(changeOutsideValue=="true")
    ofield->set_all_values(outsideValue);
  else
    ofield->copy_values(ifield);

  std::vector<VMesh*> objmesh(objField.size(),0);

  if(ofield->basis_order()==0)
  {
    for(size_t p=0;p<objField.size();p++)
    {
      objmesh[p]=objField[p]->vmesh();
      objmesh[p]->synchronize(Mesh::ELEM_LOCATE_E);
    }

    VMesh::size_type numElems=omesh->num_elems();

    VMesh::Node::array_type nodes;
    VMesh::Elem::index_type cidx;

    std::vector<Point> points1;
    std::vector<Point> points2;

    std::vector<VMesh::coords_type> coords;
    std::vector<double> weights;

    std::string samplingScheme=getOption(Parameters::SamplingScheme);
    if(samplingScheme=="regular1") omesh->get_regular_scheme(coords,weights,1);
    if(samplingScheme=="regular2") omesh->get_regular_scheme(coords,weights,2);
    if(samplingScheme=="regular3") omesh->get_regular_scheme(coords,weights,3);
    if(samplingScheme=="regular4") omesh->get_regular_scheme(coords,weights,4);
    if(samplingScheme=="regular5") omesh->get_regular_scheme(coords,weights,5);

    std::string method=getOption(Parameters::Method);
    int cnt=0;

    if(method=="one")
    {
      for(VMesh::Elem::index_type idx=0;idx<numElems;idx++)
      {
        omesh->minterpolate(points2,coords,idx);
        for(size_t p=0;p<objmesh.size();p++)
        {
          bool is_inside=false;
          for(size_t r=0;r<points2.size();r++)
          {
            if(objmesh[p]->locate(cidx,points2[r]))
            {
              is_inside=true;
              break;
            }
          }

          if(is_inside) ofield->set_value(startValue+p,idx);
        }
        cnt++;
        //Progress Reporting
       /* if(cnt==100)
        {
          update_progress(idx/numElems);
          cnt=0;
        }*/
      }
    }
    else if(method=="all")
    {
      for(VMesh::Elem::index_type idx=0;idx<numElems;idx++)
      {
        omesh->minterpolate(points2,coords,idx);
        for(size_t p=0;p<objmesh.size();p++)
        {
          bool is_inside=true;
          for(size_t r=0;r<points2.size();r++)
          {
            if(!objmesh[p]->locate(cidx,points2[r]))
            {
              is_inside=false;
              break;
            }
          }

          if(is_inside) ofield->set_value(startValue+p,idx);
        }
        cnt++;
        //Progress Reporting
        /*if(cnt==100)
        {
          update_progress(idx/numElems);
          cnt=0;
        }*/
      }
    }
    else
    {
      for(VMesh::Elem::index_type idx=0;idx<numElems;idx++)
      {
        omesh->minterpolate(points2,coords,idx);
        for(size_t p=0;p<objmesh.size();p++)
        {
          int outside=0;
          int inside=0;
          for(size_t r=0;r<points2.size();r++)
          {
            if(objmesh[p]->locate(cidx,points2[r]))
              inside++;
            else
              outside++;
          }

          if(inside>=outside) ofield->set_value(startValue+p,idx);
        }
        cnt++;
        //Progress Reporting
        /*if(cnt==100)
        {
          update_progress(idx/numElems);
          cnt=0;
        }*/
      }
    }
  }
  else
  {
    for(size_t p=0;p<objField.size();p++)
    {
      objmesh[p] = objField[p]->vmesh();
      objmesh[p]->synchronize(Mesh::ELEM_LOCATE_E);
    }

    VMesh::size_type numNodes = omesh->num_nodes();

    int cnt = 0;

    for(VMesh::Node::index_type idx=0; idx<numNodes;idx++)
    {
      Point point;
      VMesh::Elem::index_type cidx;
      omesh->get_center(point,idx);

      for (size_t p=0; p<objmesh.size(); p++)
      {
        if (objmesh[p]->locate(cidx,point))
        {
          ofield->set_value(startValue+p,idx);
        }
      }

      // Progress Reporting
      cnt++;
     /* if (cnt == 100)
      {
        update_progress(idx/numNodes);
        cnt = 0;
      }*/
    }
  }
    return output;
}


AlgorithmOutput CalculateInsideWhichFieldAlgorithm::run(const AlgorithmInput& input) const
{
  auto inputField=input.get<Field>(Variables::InputField);
  auto inputFields=input.getList<Field>(Variables::InputFields);

  FieldHandle outputField=run(inputField,inputFields);

  if(!outputField)
    THROW_ALGORITHM_PROCESSING_ERROR("null returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;
  return output;
}
