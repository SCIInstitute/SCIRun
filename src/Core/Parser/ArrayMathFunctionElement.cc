//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2015 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <Core/Parser/ArrayMathInterpreter.h>
#include <Core/Parser/ArrayMathFunctionCatalog.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;

namespace ArrayMathFunctions {

//--------------------------------------------------------------------------
// Source functions

// Field data to buffer
bool center_fn(SCIRun::ArrayMathProgramCode& pc)
{ 
  // Get where the data needs to be stored
  double* data0 = pc.get_variable(0); 
  // Get the field where the data is coming from
  SCIRun::VMesh* data1 = pc.get_vmesh(1);

  double* data0_end = data0 + pc.get_size();
  SCIRun::VMesh::Node::index_type idx = 3*pc.get_index();

  // Safety check
  if (idx+pc.get_size() > data1->num_nodes()) return (false);

  while(data0 != data0_end) 
  {
    Point p;
    data1->get_center(p,idx);
    ++idx;
    *data0 = p.x(); data0++;
    *data0 = p.y(); data0++;
    *data0 = p.z(); data0++;
  }
  
  return (true);
}

bool center_fe(SCIRun::ArrayMathProgramCode& pc)
{ 
  // Get where the data needs to be stored
  double* data0 = pc.get_variable(0); 
  // Get the field where the data is coming from
  SCIRun::VMesh* data1 = pc.get_vmesh(1);

  double* data0_end = data0 + pc.get_size();
  SCIRun::VMesh::Elem::index_type idx = 3*pc.get_index();

  // Safety check
  if (idx+pc.get_size() > data1->num_elems()) return (false);

  while(data0 != data0_end) 
  {
    Point p;
    data1->get_center(p,idx);
    ++idx;
    *data0 = p.x(); data0++;
    *data0 = p.y(); data0++;
    *data0 = p.z(); data0++;
  }
  
  return (true);
}

bool size_fn(SCIRun::ArrayMathProgramCode& pc)
{ 
  // Get where the data needs to be stored
  double* data0 = pc.get_variable(0); 
  // Get the field where the data is coming from
  SCIRun::VMesh* data1 = pc.get_vmesh(1);

  double* data0_end = data0 + pc.get_size();
  SCIRun::VMesh::Node::index_type idx = pc.get_index();

  // Safety check
  if (idx+pc.get_size() > data1->num_nodes()) return (false);

  while(data0 != data0_end) 
  {
    *data0 = data1->get_size(idx);
    ++idx; data0++;
  }
  
  return (true);
}

bool size_fe(SCIRun::ArrayMathProgramCode& pc)
{ 
  // Get where the data needs to be stored
  double* data0 = pc.get_variable(0); 
  // Get the field where the data is coming from
  SCIRun::VMesh* data1 = pc.get_vmesh(1);

  double* data0_end = data0 + pc.get_size();
  SCIRun::VMesh::Elem::index_type idx = pc.get_index();

  // Safety check
  if (idx+pc.get_size() > data1->num_elems()) return (false);

  while(data0 != data0_end) 
  {
    *data0 = data1->get_size(idx);
    ++idx; data0++;
  }
  
  return (true);
}


bool normal_fn(SCIRun::ArrayMathProgramCode& pc)
{ 
  // Get where the data needs to be stored
  double* data0 = pc.get_variable(0); 
  // Get the field where the data is coming from
  SCIRun::VMesh* data1 = pc.get_vmesh(1);
  data1->synchronize(SCIRun::Mesh::NORMALS_E);

  double* data0_end = data0 + 3*pc.get_size();
  SCIRun::VMesh::Node::index_type idx = pc.get_index();

  // Safety check
  if (idx+pc.get_size() > data1->num_nodes()) return (false);

  while(data0 != data0_end) 
  {
    Vector v;
    data1->get_normal(v,idx);
    ++idx;
    *data0 = v.x(); data0++;
    *data0 = v.y(); data0++;
    *data0 = v.z(); data0++;
  }
  
  return (true);
}

bool normal_fe(SCIRun::ArrayMathProgramCode& pc)
{ 
  // Get where the data needs to be stored
  double* data0 = pc.get_variable(0); 
  // Get the field where the data is coming from
  SCIRun::VMesh* data1 = pc.get_vmesh(1);
  data1->synchronize(SCIRun::Mesh::NORMALS_E);

  double* data0_end = data0 + 3*pc.get_size();
  SCIRun::VMesh::Elem::index_type idx = pc.get_index();

  // Safety check
  if (idx+pc.get_size() > data1->num_elems()) return (false);
  SCIRun::VMesh::coords_type coords;
  data1->get_element_center(coords);

  while(data0 != data0_end) 
  {
    Vector v;
    data1->get_normal(v,coords,idx);
    ++idx;
    *data0 = v.x(); data0++;
    *data0 = v.y(); data0++;
    *data0 = v.z(); data0++;
  }
  
  return (true);
}


bool dimension_fn(SCIRun::ArrayMathProgramCode& pc)
{ 
  // Get where the data needs to be stored
  double* data0 = pc.get_variable(0); 
  // Get the field where the data is coming from
  VMesh* data1 = pc.get_vmesh(1);

  double* data0_end = data0 + pc.get_size();
  SCIRun::VMesh::Node::index_type idx = pc.get_index();

  // Safety check
  if (idx+pc.get_size() > data1->num_nodes()) return (false);

  while(data0 != data0_end) 
  {
    *data0 = 0.0; data0++;
  }
  
  return (true);
}

bool dimension_fe(SCIRun::ArrayMathProgramCode& pc)
{ 
  // Get where the data needs to be stored
  double* data0 = pc.get_variable(0); 
  // Get the field where the data is coming from
  auto data1 = pc.get_vmesh(1);

  double* data0_end = data0 + pc.get_size();
  
  double val = static_cast<double>(data1->dimensionality());
  while(data0 != data0_end) 
  {
    *data0 = val; data0++;
  }
  
  return (true);
}

} //end namespace

namespace SCIRun {

void
InsertElementArrayMathFunctionCatalog(ArrayMathFunctionCatalogHandle& catalog)
{
  // Source functions
  catalog->add_function(ArrayMathFunctions::center_fn,"center$FN","V");
  catalog->add_function(ArrayMathFunctions::center_fe,"center$FE","V");

  catalog->add_function(ArrayMathFunctions::size_fn,"size$FN","S");
  catalog->add_function(ArrayMathFunctions::size_fe,"size$FE","S");
  catalog->add_function(ArrayMathFunctions::size_fn,"length$FN","S");
  catalog->add_function(ArrayMathFunctions::size_fe,"length$FE","S");
  catalog->add_function(ArrayMathFunctions::size_fn,"area$FN","S");
  catalog->add_function(ArrayMathFunctions::size_fe,"area$FE","S");
  catalog->add_function(ArrayMathFunctions::size_fn,"volume$FN","S");
  catalog->add_function(ArrayMathFunctions::size_fe,"volume$FE","S");

  catalog->add_function(ArrayMathFunctions::normal_fn,"normal$FN","V");
  catalog->add_function(ArrayMathFunctions::normal_fe,"normal$FE","V");

  catalog->add_function(ArrayMathFunctions::dimension_fn,"dimension$FN","S");
  catalog->add_function(ArrayMathFunctions::dimension_fe,"dimension$FE","S");

}

} // end namespace
