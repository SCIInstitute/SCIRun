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

   Author:              Moritz Dannhauer
   Last Modification:   March 16 2014
   ToDo: Padding is always enabled because of exit() in cleaver lib
*/


///TODO: fix include path to remove Externals/ part

#include <Externals/cleaver/lib/FloatField.h>
#include <Externals/cleaver/lib/vec3.h>
#include <Externals/cleaver/lib/BoundingBox.h>
#include <Externals/cleaver/lib/Cleaver.h>
#include <Externals/cleaver/lib/InverseField.h>
#include <Externals/cleaver/lib/PaddedVolume.h>
#include <Externals/cleaver/lib/Volume.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Field/InterfaceWithCleaverAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <iostream>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Utils/StringUtil.h>
#include <boost/scoped_ptr.hpp>
#include <Core/Logging/Log.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;

AlgorithmParameterName InterfaceWithCleaverAlgorithm::Verbose("VerboseCheckBox");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::Padding("PaddingCheckBox");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::VolumeScalingOption("VolumeScalingOption");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::VolumeScalingX("VolumeScalingSpinBox_X");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::VolumeScalingY("VolumeScalingSpinBox_Y");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::VolumeScalingZ("VolumeScalingSpinBox_Z");

InterfaceWithCleaverAlgorithm::InterfaceWithCleaverAlgorithm()
{
  addParameter(Verbose,true);
  addParameter(Padding,true);
  addOption(VolumeScalingOption, "Relative size", "Absolute size|Relative size|None");
  addParameter(VolumeScalingX,1.0);
  addParameter(VolumeScalingY,1.0);
  addParameter(VolumeScalingZ,1.0);
}

boost::shared_ptr<Cleaver::ScalarField> InterfaceWithCleaverAlgorithm::makeCleaverFieldFromLatVol(FieldHandle field )
{
  VMesh*  vmesh   = field->vmesh();
  VField* vfield = field->vfield();
  VMesh::dimension_type dims;
  vmesh->get_dimensions( dims );

  float* ptr = static_cast<float*>(vfield->fdata_pointer());

  auto cleaverField = boost::make_shared<Cleaver::FloatField>(dims[0], dims[1], dims[2], ptr);
  Cleaver::BoundingBox bb(Cleaver::vec3::zero, Cleaver::vec3(dims[0],dims[1],dims[2]));
  cleaverField->setBounds(bb);
  const Transform &transform = vmesh->get_transform();

  int x_spacing=fabs(transform.get_mat_val(0,0)), y_spacing=fabs(transform.get_mat_val(1,1)), z_spacing=fabs(transform.get_mat_val(2,2));

  if (IsNan(x_spacing) || x_spacing<=0) x_spacing=1; /// dont allow negative or zero scaling of the bounding box
  if (IsNan(y_spacing) || y_spacing<=0) y_spacing=1;
  if (IsNan(z_spacing) || z_spacing<=0) z_spacing=1;

  cleaverField->setScale(Cleaver::vec3(x_spacing,y_spacing,z_spacing));

  return cleaverField;
}

FieldHandle InterfaceWithCleaverAlgorithm::run(const std::vector<FieldHandle>& input) const
{
  FieldHandle output;
  std::vector<FieldHandle> inputs;
  std::copy_if(input.begin(), input.end(), std::back_inserter(inputs), [](FieldHandle f) { return f; });

  if (inputs.empty())
  {
    THROW_ALGORITHM_INPUT_ERROR(" No input fields given ");
    return FieldHandle();
  }
  if (inputs.size()<2)
  {
    THROW_ALGORITHM_INPUT_ERROR(" At least 2 indicator functions stored as float values are needed to run cleaver! " );
    return FieldHandle();
  }

  std::ostringstream ostr0;
  ostr0 << "Be aware that inside and outside of materials (to be meshed) need to be defined as positive and negative (e.g. surface distance) values across all module inputs. The zero crossings represents material boundaries." << std::endl;
  remark(ostr0.str());

  std::vector<boost::shared_ptr<Cleaver::ScalarField>> fields;

  VMesh::dimension_type dims; int x=0,y=0,z=0;
  for (size_t p=0; p<inputs.size(); p++)
  {
    FieldHandle input = inputs[p];
    VMesh*  imesh1   = input->vmesh();

    if( !imesh1->is_structuredmesh() )
    {
      THROW_ALGORITHM_INPUT_ERROR("needs to be structured mesh!");
    }
    else
    {
      VField* vfield1 = input->vfield();
      if (!vfield1->is_scalar())
      {
        THROW_ALGORITHM_INPUT_ERROR("values at the node needs to be scalar!");
        return FieldHandle();
      }

      imesh1->get_dimensions( dims );
      if (p==0)
      {
        x=dims[0]; y=dims[1]; z=dims[2];
        if (x<1 || y<1 || z<1)
        {
          THROW_ALGORITHM_INPUT_ERROR(" Size of input fields should be non-zero !");
        }
      }
      else
      {
        if ( dims[0]!=x || dims[1]!=y || dims[2]!=z)
        {
          THROW_ALGORITHM_INPUT_ERROR(" Size of input fields is inconsistent !");
        }
      }

      if (dims.size()!=3)
      {
        THROW_ALGORITHM_INPUT_ERROR("need a three dimensional indicator function");
        return FieldHandle();
      }

      //0 = constant, 1 = linear
       if (1 != vfield1->basis_order())
       {
        THROW_ALGORITHM_INPUT_ERROR("Input data need to be defined on input mesh nodes.");
       }

      if (vfield1->is_float())
      {
        float* ptr = static_cast<float*>(vfield1->fdata_pointer());
	if (ptr)
        {
          fields.push_back(makeCleaverFieldFromLatVol(input));
        }
        else
        {
          THROW_ALGORITHM_INPUT_ERROR(" float field is NULL pointer");
          return FieldHandle();
        }
      } else
      {
       THROW_ALGORITHM_INPUT_ERROR(" Input field needs to be a structured mesh (best would be a LatVol) with float values defnied on mesh nodes. ");
      }

    }

  }

  boost::shared_ptr<Cleaver::Volume> volume(new Cleaver::Volume(toVectorOfRawPointers(fields)));

  const double xScale = get(VolumeScalingX).toDouble();
  const double yScale = get(VolumeScalingY).toDouble();
  const double zScale = get(VolumeScalingZ).toDouble();

  if (xScale > 0 && yScale > 0 && zScale > 0)
  {
    const std::string scaling = getOption(VolumeScalingOption);
    if ("Absolute size" == scaling)
    {
      volume->setSize(xScale, yScale, zScale);
    }
    else if ("Relative size" == scaling)
    {
      double newX = xScale*volume->size().x;
      double newY = yScale*volume->size().y;
      double newZ = zScale*volume->size().z;
      volume->setSize(newX, newY, newZ);
    }
    else // None
    {
      volume->setSize(dims[0],dims[1],dims[2]);
      std::ostringstream ostr1,ostr2;
      ostr1 << "Scaling 'None' .... using " << "Scaling " << dims[0] << "x" << dims[1] << "x" << dims[2] << std::endl;
      remark(ostr1.str());
    }
  }
  else
  {
    THROW_ALGORITHM_INPUT_ERROR(" Invalid Scaling. Use Input sizes.");
  }

  /// Padding is now optional!
  boost::shared_ptr<Cleaver::AbstractVolume> paddedVolume(volume);
  const bool verbose = get(Verbose).toBool();
  const bool pad = get(Padding).toBool();

  if (pad)
  {
    paddedVolume.reset(new Cleaver::PaddedVolume(volume.get()));
  }

  if (verbose)
  {
   std::cout << "Input Dimensions: " << dims[0] << " x " << dims[1] << " x " << dims[2] << std::endl;
   if (pad)
    std::cout << "Padded Mesh with Volume Size " << paddedVolume->size().toString() << std::endl;
       else
          std::cout << "Creating Mesh with Volume Size " << volume->size().toString() << std::endl;
  }

  boost::scoped_ptr<Cleaver::TetMesh> mesh(Cleaver::createMeshFromVolume(pad ? paddedVolume.get() : volume.get(), verbose));

  auto nr_of_tets  = mesh->tets.size();
  auto nr_of_verts = mesh->verts.size();

  if (nr_of_tets==0 || nr_of_verts==0)
  {
    THROW_ALGORITHM_INPUT_ERROR(" Number of resulting tetrahedral nodes or elements is 0. If you disabled padding enable it and execute again. ");
  }

  FieldInformation fi("TetVolMesh",0,"double");   ///create output field

  output = CreateField(fi);
  auto omesh = output->vmesh();
  auto ofield = output->vfield();

  omesh->node_reserve(nr_of_verts);
  omesh->elem_reserve(nr_of_tets);

  for (auto i=0; i<nr_of_verts; i++)
  {
    omesh->add_point(Point(mesh->verts[i]->pos().x,mesh->verts[i]->pos().y,mesh->verts[i]->pos().z));
  }

  VMesh::Node::array_type vdata;
  vdata.resize(4);
  std::vector<double> values(nr_of_tets);

  for (auto i=0; i<nr_of_tets; i++)
  {
    vdata[0]=mesh->tets[i]->verts[0]->tm_v_index;
    vdata[1]=mesh->tets[i]->verts[1]->tm_v_index;
    vdata[2]=mesh->tets[i]->verts[2]->tm_v_index;
    vdata[3]=mesh->tets[i]->verts[3]->tm_v_index;
    omesh->add_elem(vdata);
    auto mat_label = mesh->tets[i]->mat_label;
    values[i]=mat_label;
  }
  ofield->resize_values();
  ofield->set_values(values);
  mesh->computeAngles();

  std::ostringstream ostr1,ostr2;
  ostr1 << "(nodes, elements, dims) - (" << nr_of_verts << " , " << nr_of_tets << " , " << volume->size().toString() << ")" << std::endl;
  ostr2 << "(min angle, max angle) - (" <<  mesh->min_angle << " , " << mesh->max_angle << ")" << std::endl;

  remark(ostr1.str());
  remark(ostr2.str());

  return output;
}

AlgorithmOutput InterfaceWithCleaverAlgorithm::run(const AlgorithmInput& input) const
{
  auto inputfields = input.getList<Field>(Variables::InputFields);

  FieldHandle output_fld = run(inputfields);
  if ( !output_fld )
    THROW_ALGORITHM_PROCESSING_ERROR("Null returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = output_fld;
  return output;
}
