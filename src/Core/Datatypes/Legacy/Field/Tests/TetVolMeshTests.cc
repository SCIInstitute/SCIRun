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
   
   author: Moritz Dannhauer
   latest change: 04/30/2014
*/
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <gtest/gtest.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;


FieldHandle TetMesh1()
{
 FieldInformation fi("TetVolMesh", 1, "double");    
 FieldHandle singleTetField_ = CreateField(fi);
 VMesh *vmesh = singleTetField_->vmesh();
 VMesh::Node::array_type vdata;
 vdata.resize(4);
      
 vmesh->node_reserve(8);
 vmesh->elem_reserve(1);
 vmesh->add_point( Point(0.0, 0.0, 0.0) );
 vmesh->add_point( Point(1.0, 0.0, 0.0) );
 vmesh->add_point( Point(1.0, 1.0, 0.0) );
 vmesh->add_point( Point(0.0, 1.0, 0.0) ); 
 vmesh->add_point( Point(0.0, 0.0, 1.0) );
 vmesh->add_point( Point(1.0, 0.0, 1.0) );
 vmesh->add_point( Point(1.0, 1.0, 1.0) );
 vmesh->add_point( Point(0.0, 1.0, 1.0) );
 
 vdata[0]=5; vdata[1]=6;  vdata[2]=0; vdata[3]=4;
 vmesh->add_elem(vdata);
 vdata[0]=0; vdata[1]=7;  vdata[2]=2; vdata[3]=3;
 vmesh->add_elem(vdata);
 vdata[0]=2; vdata[1]=6;  vdata[2]=0; vdata[3]=1;
 vmesh->add_elem(vdata);
 vdata[0]=0; vdata[1]=6;  vdata[2]=5; vdata[3]=1;
 vmesh->add_elem(vdata);
 vdata[0]=0; vdata[1]=6;  vdata[2]=2; vdata[3]=7;
 vmesh->add_elem(vdata);
 vdata[0]=6; vdata[1]=7;  vdata[2]=0; vdata[3]=4;
 vmesh->add_elem(vdata);
 
 return singleTetField_;
}


TEST(TetVolMeshTest, CheckMeshIteratorTetVolMesh)
{
  FieldHandle output;
  FieldHandle tetmesh=TetMesh1();
  
  VMesh* mesh = tetmesh->vmesh();
  
  VMesh::Elem::array_type elems;
  VMesh::Elem::iterator it, eit;
  VMesh::Elem::size_type sz;
 
  VMesh::Node::array_type nodearray;

  mesh->begin(it);
  mesh->end(eit);
  mesh->size(sz);
  index_type c = 0;
  
  while (it != eit)
  {
    mesh->get_nodes(nodearray, *it);
    ++it;
    c++;
  }  
  
  ASSERT_EQ(c, 6);
  
}


