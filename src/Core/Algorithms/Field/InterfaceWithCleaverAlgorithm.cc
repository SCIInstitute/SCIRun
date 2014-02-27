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
    Last modification : February 25 2014
*/
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

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;


const AlgorithmInputName InterfaceWithCleaverAlgorithm::InputField1("InputField1");
const AlgorithmInputName InterfaceWithCleaverAlgorithm::InputField2("InputField2");
const AlgorithmOutputName InterfaceWithCleaverAlgorithm::OutputField("OutputField");

FieldHandle InterfaceWithCleaverAlgorithm::run(FieldHandle field1, FieldHandle field2/*, FieldHandle& output*/) const
{
   FieldHandle output;
   VMesh*  imesh1   = field1->vmesh();
   VMesh*  imesh2   = field2->vmesh();
   bool verbose=true;
   
   
   if( !imesh1->is_structuredmesh() ||  !imesh2->is_structuredmesh() )
   {
      std::cout << "needs to be structured mesh!" << std::endl;
      return FieldHandle();
   } else
   {
     VField* vfield1 = field1->vfield();
     VField* vfield2 = field2->vfield();
     
     if (!vfield1->is_scalar() || !vfield2->is_scalar())
     {
      std::cout << "values at the node needs to be scalar!" << std::endl;
      return FieldHandle();
     }
     
     VMesh::dimension_type dims;
     imesh1->get_dimensions( dims ); //TODO: check for dimensions of all inputs
     
     if (dims.size()!=3)
     {
      std::cout << "need a three dimensional indicator function" << std::endl;
      return FieldHandle();
     }    
     
     std::vector<Cleaver::ScalarField*> fields;

     //need a for loop for dynamic inport ports here, unrolled loop
     if (vfield1->is_float())
     {
       float* ptr = static_cast<float*>(vfield1->fdata_pointer());
       if (ptr)
       {
         Cleaver::FloatField *tmp1 = new Cleaver::FloatField(dims[0], dims[1], dims[2], ptr);
         fields.push_back(tmp1); 
       } else
       {
        std::cout << " float field is NULL pointer" << std::endl;
	return FieldHandle();
       }
     }
     
     if (vfield2->is_float())
     {
       float* ptr = static_cast<float*>(vfield2->fdata_pointer());
       if (ptr)
       {
         Cleaver::FloatField *tmp2 = new Cleaver::FloatField(dims[0], dims[1], dims[2], ptr);
         fields.push_back(tmp2); 
       } else
       {
        std::cout << " float field is NULL pointer" << std::endl;
	return FieldHandle();
       }
     }
     
     std::cout << ": " << fields.size() << std::endl;
     
     if (fields.size()<2)
     {
       std::cout << " At least 2 indicator functions stored as float values are needed to run cleaver! " << std::endl;
       return FieldHandle();
     }
     
     
     Cleaver::AbstractVolume *volume = new Cleaver::Volume(fields);
     
     //abs res  
     ((Cleaver::Volume*)volume)->setSize(dims[0],dims[1],dims[2]);
     
     //scaled res ((Cleaver::Volume*)volume)->setSize(sx*volume->size().x, sy*volume->size().y, sz*volume->size().z);
     volume = new Cleaver::PaddedVolume(volume);
   
     std::cout << "Creating Mesh with Volume Size " << volume->size().toString() << std::endl;
     
     Cleaver::TetMesh *mesh = Cleaver::createMeshFromVolume(volume, verbose);
     
     FieldInformation fi("TetVolMesh",0,"double");   //create output field
     //FieldInformation fi(field1);
     output = CreateField(fi);
     auto omesh = output->vmesh();
     auto ofield = output->vfield();

     auto nr_of_tets  = mesh->tets.size();
     auto nr_of_verts = mesh->verts.size();
     
     //std::cout << "Tets " << nr_of_tets << std::endl;
     //std::cout << "Verts " << nr_of_verts << std::endl;
     
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
     //std::cout << "1: " <<  ofield->vmesh()->num_elems() << std::endl;
     //std::cout << "2: " <<  ofield->vmesh()->num_nodes() << std::endl;
     //std::cout << "3: " <<  ofield->vfield()->num_values() << std::endl;
     // mesh.verts[i].pos().x ; mesh.verts[i].pos().y ; mesh.verts[i].pos().z
     // mesh.tets[i].verts[0].tm_vindex
     // mesh.tets[i].mat_label

     // mesh->computeAngles();
     // if(verbose){
     //   std::cout.precision(12);
     //   std::cout << "Worst Angles:" << std::endl;
     //   std::cout << "min: " << mesh->min_angle << std::endl;
     //   std::cout << "max: " << mesh->max_angle << std::endl;
      
   }
  
  return output;
}

AlgorithmOutput InterfaceWithCleaverAlgorithm::run_generic(const AlgorithmInput& input) const
{
   
  auto field1 = input.get<Field>(InputField1);
  ENSURE_ALGORITHM_INPUT_NOT_NULL(field1, "field1 is not ready");
  auto field2 = input.get<Field>(InputField2);
  ENSURE_ALGORITHM_INPUT_NOT_NULL(field2, "field2 is not ready");
  
  //auto field = input.get<Field>(Variables::InputField);
  FieldHandle output_fld;
  output_fld=run(field1,field2); 
  if ( output_fld.get() == nullptr) THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");
 // auto field = input.get<Field>(Variables::InputField);

 // auto info = run(field);
  AlgorithmOutput output;
  //output.setTransient(info);
  output[OutputField] = output_fld;
  return output;
}
