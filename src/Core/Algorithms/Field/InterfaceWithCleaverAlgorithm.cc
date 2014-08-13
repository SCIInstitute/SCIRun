/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2012 Scientific Computing and Imaging Institute,
University of Utah.

License for the specific language governing rights and limitations under
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

Author            : Moritz Dannhauer
Last modification : March 16 2014
ToDo: Padding is always enabled because of exit() in cleaver lib
*/

///TODO: fix include path to remove Externals/ part
#include <Externals/cleaver/lib/FloatField.h>
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

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;

AlgorithmInputName InterfaceWithCleaverAlgorithm::InputFields("InputFields");
AlgorithmOutputName InterfaceWithCleaverAlgorithm::OutputField("OutputField");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::VerboseCheckBox("VerboseCheckBox");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::PaddingCheckBox("PaddingCheckBox");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::AbsoluteVolumeScalingRadioButton("AbsoluteVolumeScalingRadioButton");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::RelativeVolumeScalingRadioButton("RelativeVolumeScalingRadioButton");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_X("VolumeScalingSpinBox_X");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_Y("VolumeScalingSpinBox_Y");
AlgorithmParameterName InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_Z("VolumeScalingSpinBox_Z");

InterfaceWithCleaverAlgorithm::InterfaceWithCleaverAlgorithm()
{
  addParameter(VerboseCheckBox,true);
  addParameter(PaddingCheckBox,true);
  addParameter(AbsoluteVolumeScalingRadioButton,false);
  addParameter(RelativeVolumeScalingRadioButton,true);  
  addParameter(VolumeScalingSpinBox_X,1.0);
  addParameter(VolumeScalingSpinBox_Y,1.0);  
  addParameter(VolumeScalingSpinBox_Z,1.0);  
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
  
  std::vector<boost::shared_ptr<Cleaver::ScalarField>> fields;  
  VMesh::dimension_type dims; int x=0,y=0,z=0; 
  for (size_t p=1; p<inputs.size(); p++)
  {
    VMesh*  imesh1   = inputs[p]->vmesh();
    
    if( !imesh1->is_structuredmesh() )
    {
     THROW_ALGORITHM_INPUT_ERROR("needs to be structured mesh!");    
    } else
    {
     VField* vfield1 = inputs[p]->vfield();
     if (!vfield1->is_scalar())
     {
      THROW_ALGORITHM_INPUT_ERROR("values at the node needs to be scalar!");
      return FieldHandle();
     }

     imesh1->get_dimensions( dims ); 
     if (p==1)
     {
       x=dims[0]; y=dims[1]; z=dims[2];
       if (x<1 || y<1 || z<1)
       {
        THROW_ALGORITHM_INPUT_ERROR(" Size of input fields should be non-zero !");
       }
     } else
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

     if (vfield1->is_float())
     {
      float* ptr = static_cast<float*>(vfield1->fdata_pointer());
      if (ptr)
      {
        fields.push_back(boost::make_shared<Cleaver::FloatField>(dims[0], dims[1], dims[2], ptr)); 
      } else
      {
        THROW_ALGORITHM_INPUT_ERROR(" float field is NULL pointer");
        return FieldHandle();
      }
     }

    }
    
  }
  
  boost::shared_ptr<Cleaver::Volume> volume(new Cleaver::Volume(toVectorOfRawPointers(fields)));

  if ( get(VolumeScalingSpinBox_X).getDouble()>0 && get(VolumeScalingSpinBox_Y).getDouble()>0 && get(VolumeScalingSpinBox_Z).getDouble()>0 )
  {
    if (get(AbsoluteVolumeScalingRadioButton).getBool()) 
        volume->setSize(get(VolumeScalingSpinBox_X).getDouble(),get(VolumeScalingSpinBox_Y).getDouble(),get(VolumeScalingSpinBox_Z).getDouble());
      else
	if (get(RelativeVolumeScalingRadioButton).getBool())
	  volume->setSize(get(VolumeScalingSpinBox_X).getDouble()*volume->size().x, get(VolumeScalingSpinBox_Y).getDouble()*volume->size().y, get(VolumeScalingSpinBox_Z).getDouble()*volume->size().z);
        else
	 volume->setSize(dims[0],dims[1],dims[2]);
   }
    else
    {
      volume->setSize(dims[0],dims[1],dims[2]);
      THROW_ALGORITHM_INPUT_ERROR(" Invalid Scaling. Use Input sizes.");
    }
   
    /// Padding is now optional! 
    boost::scoped_ptr<Cleaver::TetMesh> mesh(Cleaver::createMeshFromVolume(get(PaddingCheckBox).getBool() ?  ((boost::shared_ptr<Cleaver::AbstractVolume>) new Cleaver::PaddedVolume(volume.get())).get() : volume.get(), get(VerboseCheckBox).getBool()));        
    
    FieldInformation fi("TetVolMesh",0,"double");   ///create output field

    output = CreateField(fi);
    auto omesh = output->vmesh();
    auto ofield = output->vfield();

    auto nr_of_tets  = mesh->tets.size();
    auto nr_of_verts = mesh->verts.size();

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
      auto mat_label = mesh->tets[i]->mat_label+1;
      values[i]=mat_label;
    }
    ofield->resize_values();
    ofield->set_values(values);
    mesh->computeAngles();
    std::ostringstream ostr1;
    ostr1 << "Number of tetrahedral elements:" << ofield->vmesh()->num_elems() <<  std::endl;
    ostr1 << "Number of tetrahedral nodes:" << ofield->vmesh()->num_nodes() << std::endl;
    ostr1 << "Number of tetrahedral nodes:" << ofield->vmesh()->num_nodes() << std::endl;
    ostr1 << "Worst Angle (min):" <<  mesh->min_angle << std::endl;
    ostr1 << "Worst Angle (max):" <<  mesh->max_angle << std::endl;
    ostr1 << "Volume:" << volume->size().toString() << std::endl;
    
    remark(ostr1.str()); 

  return output;
}

AlgorithmOutput InterfaceWithCleaverAlgorithm::run_generic(const AlgorithmInput& input) const
{ 
  auto inputfields = input.getList<Field>(InputFields);
  
  FieldHandle output_fld;
  output_fld=run(inputfields); 
  if ( !output_fld ) THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[OutputField] = output_fld;
  return output;
}
