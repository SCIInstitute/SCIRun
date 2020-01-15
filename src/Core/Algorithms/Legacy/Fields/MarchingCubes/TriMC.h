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

   Author:          Michael Callahan
   Date:            September 2002
*/


#ifndef CORE_ALGORITHMS_LEGACY_FIELDS_MARCHINGCUBES_TRIMC_H
#define CORE_ALGORITHMS_LEGACY_FIELDS_MARCHINGCUBES_TRIMC_H

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/Point.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 #include <Core/Geom/GeomLine.h>
#endif

#include <Core/Algorithms/Legacy/Fields/MarchingCubes/BaseMC.h>

namespace SCIRun {

/// A Marching Cube tesselator for a triangle face

class TriMC : public BaseMC
{
  public:
    TriMC( FieldHandle field ) : field_handle_(field),
                            field_(field->vfield()),
                            mesh_(field->vmesh()),
                            //lines_(0),
                            curve_handle_(0),
                            curve_(0) {}

    virtual ~TriMC() {}

    void extract( VMesh::Elem::index_type, double );
    virtual void reset( int, bool build_field, bool build_geom, bool transparency );
    virtual FieldHandle get_field(double val);

  private:
    void extract_n( VMesh::Elem::index_type, double );
    void extract_f( VMesh::Elem::index_type, double );

    VMesh::Node::index_type find_or_add_edgepoint(index_type n0, index_type n1, double d0, const SCIRun::Core::Geometry::Point &p);

    VMesh::Node::index_type find_or_add_nodepoint(VMesh::Node::index_type &idx);

    void find_or_add_parent(index_type u0, index_type u1, double d0, index_type edge);

    FieldHandle field_handle_;
    VField* field_;
    VMesh*  mesh_;

    #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
     GeomLines   *lines_;
    #endif

    FieldHandle curve_handle_;
    VMesh*      curve_;
};

} // End namespace SCIRun

#endif // TriMC_h
