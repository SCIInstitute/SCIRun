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


#include <Core/Datatypes/Legacy/Field/QuadSurfMesh.h>
#include <Core/Datatypes/Legacy/Field/VSurfMesh.h>

/// Only include this class if we included QuadSurf Support
#if (SCIRUN_QUADSURF_SUPPORT > 0)

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Basis;

namespace SCIRun {

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.
template<class MESH>
class VQuadSurfMesh : public VSurfMesh<MESH> {
public:
  bool is_quadsurfmesh() override { return (true); }

  VQuadSurfMesh(MESH* mesh) : VSurfMesh<MESH>(mesh)
  {
    DEBUG_CONSTRUCTOR("VQuadSurfMesh")
  }
  virtual ~VQuadSurfMesh()
  {
    DEBUG_DESTRUCTOR("VQuadSurfMesh")
  }
};

/// Functions for creating the virtual interface for specific mesh types
/// These are similar to compare maker and only serve to instantiate the class

/// Currently there are only 3 variations of this mesh available
/// 1) linear interpolation
/// 2) quadratic interpolation
/// 3) cubic interpolation

/// Add the LINEAR virtual interface and the meshid for creating it

/// Create virtual interface
VMesh* CreateVQuadSurfMesh(QuadSurfMesh<QuadBilinearLgn<Point> >* mesh)
{
  return new VQuadSurfMesh<QuadSurfMesh<QuadBilinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID QuadSurfMesh_MeshID1(QuadSurfMesh<QuadBilinearLgn<Point> >::type_name(-1),
                  QuadSurfMesh<QuadBilinearLgn<Point> >::mesh_maker);


/// Add the QUADRATIC virtual interface and the meshid for creating it
#if (SCIRUN_QUADRATIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVQuadSurfMesh(QuadSurfMesh<QuadBiquadraticLgn<Point> >* mesh)
{
  return new VQuadSurfMesh<QuadSurfMesh<QuadBiquadraticLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID QuadSurfMesh_MeshID2(QuadSurfMesh<QuadBiquadraticLgn<Point> >::type_name(-1),
                  QuadSurfMesh<QuadBiquadraticLgn<Point> >::mesh_maker);
#endif


/// Add the CUBIC virtual interface and the meshid for creating it
#if (SCIRUN_CUBIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVQuadSurfMesh(QuadSurfMesh<QuadBicubicHmt<Point> >* mesh)
{
  return new VQuadSurfMesh<QuadSurfMesh<QuadBicubicHmt<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID QuadSurfMesh_MeshID3(QuadSurfMesh<QuadBicubicHmt<Point> >::type_name(-1),
                  QuadSurfMesh<QuadBicubicHmt<Point> >::mesh_maker);

#endif


} // namespace SCIRun

#endif
