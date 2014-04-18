/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#ifndef CORE_ALGORITHMS_VISUALIZATION_UHEXMC_H
#define CORE_ALGORITHMS_VISUALIZATION_UHEXMC_H 1

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>

#include <Core/Geom/GeomTriangles.h>

#include <Core/Algorithms/Fields/MarchingCubes/mcube2.h>
#include <Core/Algorithms/Fields/MarchingCubes/BaseMC.h>

namespace SCIRun {

/// A Marching Cube teselator for an Unstructured Hexagon cell     

class UHexMC : public BaseMC
{
  public:
  
    UHexMC( Field *field ) : field_handle_(field), 
                            field_(field->vfield()),
                            mesh_(field->vmesh()),
                            triangles_(0), 
                            trisurf_(0), 
                            trisurf_handle_(0),
                            quadsurf_(0),
                            quadsurf_handle_(0) { }
                            
  virtual ~UHexMC() {}
	
  void extract( VMesh::Elem::index_type , double);
  virtual void reset( int, bool build_field, bool build_geom, bool transparency );
  virtual FieldHandle get_field(double val);

  private:
    void extract_c( VMesh::Elem::index_type, double);
    void extract_n( VMesh::Elem::index_type, double);

    VMesh::Node::index_type find_or_add_edgepoint(index_type n0,
                                                  index_type n1,
                                                  double d0,
                                                  const Point &p);

    VMesh::Node::index_type find_or_add_nodepoint(VMesh::Node::index_type& );
    
    void find_or_add_parent(index_type u0, index_type u1,
          double d0, index_type face);

    FieldHandle field_handle_;
    VField*     field_;
    VMesh*      mesh_;

    GeomFastTriangles *triangles_;
  
    VMesh*      trisurf_;
    FieldHandle trisurf_handle_;
    VMesh*      quadsurf_;
    FieldHandle quadsurf_handle_;
};
  
} // End namespace SCIRun

#endif
