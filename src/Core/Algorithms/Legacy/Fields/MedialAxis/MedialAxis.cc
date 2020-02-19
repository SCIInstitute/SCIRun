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


#include <Core/Algorithms/Fields/MedialAxis/MedialAxis.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/VField.h>
#include <Core/Datatypes/TriSurfMesh.h>

namespace SCIRunAlgo {

typedef LockingHandle<SearchGridT<SCIRun::index_type> > SearchGridHandle;

// This function will find the second closest element to the point 'p'
// -- the first closest is 'repelpos', located in element 'repelface', with
// normal 'repelnormal'.  The second closest point must be within
// "diffdist" of the distance of the first point ('repeldist') and the
// angle between PQ (grid point and first closest) and PR (grid point and
// second closest) must be greater than 88 degrees.
bool find_second_closest_elem(VMesh *tsm, SearchGridHandle elem_grid,
			      double& rdist, Point &rloc,
			      VMesh::coords_type &rcoords,
			      VMesh::Elem::index_type &relem,
			      const Point &p,
			      VMesh::Elem::index_type &qelem,
			      const Point &qloc,
			      const Vector &pqdir,
			      double pqdist,
			      double diffdist) {
  VMesh::Elem::size_type sz = tsm->num_elems();

  /// If there are no elements we cannot find the closest one
  if (sz < 1) return (false);

  // get grid sizes
  const size_type ni = elem_grid->get_ni()-1;
  const size_type nj = elem_grid->get_nj()-1;
  const size_type nk = elem_grid->get_nk()-1;

    // Convert to grid coordinates.
  VMesh::index_type bi, ei, bj, ej, bk, ek;
  elem_grid->unsafe_locate(bi, bj, bk, p);

  // Clamp to closest point on the grid.
  if (bi > ni) bi = ni; if (bi < 0) bi = 0;
  if (bj > nj) bj = nj; if (bj < 0) bj = 0;
  if (bk > nk) bk = nk; if (bk < 0) bk = 0;

  ei = bi; ej = bj; ek = bk;
  Point *points = tsm->get_points_pointer();
  VMesh::index_type *faces = tsm->get_elems_pointer();

  double mindist2=(diffdist+pqdist)*(diffdist+pqdist);
  bool found = true;
  bool found_one = false;
  do
    {
      found = true;
      /// We need to do a full shell without any elements that are closer
      /// to make sure there no closer elements in neighboring searchgrid cells
      for (SCIRun::size_type i = bi; i <= ei; i++)
      {
        if (i < 0 || i > ni) continue;
        for (SCIRun::size_type j = bj; j <= ej; j++)
        {
        if (j < 0 || j > nj) continue;
	for (SCIRun::size_type k = bk; k <= ek; k++)
          {
            if (k < 0 || k > nk) continue;
            if (i == bi || i == ei || j == bj || j == ej || k == bk || k == ek)
            {
              if (elem_grid->min_distance_squared(p, i, j, k) < mindist2)
              {
                found = false;
                SearchGridT<SCIRun::index_type>::iterator it, eit;
                elem_grid->lookup_ijk(it,eit, i, j, k);

                while (it != eit)
                {
		  if (*it == qelem) { ++it; continue; }
		  Point p1(points[faces[(*it)*3]]);
		  Point p2(points[faces[(*it)*3+1]]);
		  Point p3(points[faces[(*it)*3+2]]);
		  Point R; // candidate for second-closest-point
                  closest_point_on_tri(R, p, p1, p2, p3);
		  Vector pRdir(p-R);
		  double pRdist2 = pRdir.length2();
		  if (pRdist2 < mindist2) {
		    pRdir.normalize();
		    double dot=Dot(pRdir, pqdir);
		    if (dot>-0.0006) { ++it; continue; } // 88 degrees
		    found_one = true;
		    rloc = R;
		    relem = (*it);
		    mindist2 = pRdist2;
                  }
                  ++it;
                }
              }
            }
          }
        }
      }
      bi--;ei++;
      bj--;ej++;
      bk--;ek++;
    }
    while (!found) ;

  if (!found_one) return (false);
  tsm->get_coords(rcoords, rloc, relem);
  rdist = sqrt(mindist2);
  return (true);
}

void
MedialAxisAlgo::search_cell_for_medial_pts(VMesh *tsm, SearchGridHandle elem_grid, const Point &p, const Vector &di, const Vector &dj, const Vector &dk, VMesh::Elem::index_type qelem, int level, int search_depth, double mindist, VMesh *medialPts) {
  double qdist, rdist;
  Point qloc, rloc;
  VMesh::coords_type qcoords, rcoords;
  Vector qnormal, rnormal;
  Vector pqdir, prdir;
  std::vector<VMesh::Face::index_type> qnbrs, qnodes_nbrs;
  VMesh::Node::array_type qnodes;
  VMesh::Elem::index_type relem;

  // find closest point on surface to this grid point
  if (tsm->find_closest_elem(qdist, qloc, qcoords, qelem, p)) {
    tsm->get_normal(qnormal, qcoords, qelem);
    pqdir=(p-qloc);
    if (level >= search_depth &&
	Dot(qnormal, pqdir)>0) return; // ignore outside points
    double pqdist=pqdir.length();
    if (level >= search_depth &&
	pqdist<mindist) return; // ignore points that are too close to surface
    pqdir.safe_normalize();
    // find another node that's close to this same distance
    //   the maximum acceptable distance depends on the dot product
    //   of PQ and PR
    if (find_second_closest_elem(tsm, elem_grid, rdist, rloc, rcoords, relem, p, qelem, qloc, pqdir, qdist, mindist)) {
      prdir=(p-rloc);
      double prdist=(prdir).length();
      if (prdist<mindist) return;
      prdir.safe_normalize();
      if (level >= search_depth)
	medialPts->add_point(p);
      else
	// recursively check eight children
	for (int i=0; i<2; i++)
	  for (int j=0; j<2; j++)
	    for (int k=0; k<2; k++)
	      search_cell_for_medial_pts(tsm, elem_grid, p+(i-.5)*di+(j-.5)*dj+(k-.5)*dk, di/2, dj/2, dk/2, qelem, level+1, search_depth, mindist, medialPts);
    }
  }
}

bool
MedialAxisAlgo::run(FieldHandle surfH, FieldHandle maskLatVolH, FieldHandle& medialPtsH)
{
  algo_start("medialaxis");

  if (surfH.get_rep() == 0)
  {
    error("No input surf");
    algo_end(); return (false);
  }
  FieldInformation surfFI(surfH);
  if (!surfFI.is_surface())
  {
    error("This algorithm only works on a surface mesh");
    algo_end(); return (false);
  }
  VMesh *tsm=surfH->vmesh();
  if (!tsm->is_surface()) {
    error("Medial Axis input wasn't a surface!\n");
    algo_end(); return (false);
  }
  if (!tsm->is_trisurfmesh()) {
    error("Medial Axis input wasn't a TriSurfMesh!\n");
    algo_end(); return (false);
  }

  BBox bbox = tsm->get_bounding_box();

  FieldInformation tmpFI(maskLatVolH);
  if (maskLatVolH.get_rep() == 0 || !tmpFI.is_constantdata())
  {
    BBox bbox = tsm->get_bounding_box();
    FieldInformation fi("LatVolMesh", 1, 0, "unsigned_char");
    MeshHandle lvm = CreateMesh(fi, 64, 64, 64, bbox.min(), bbox.max());
    maskLatVolH = CreateField(fi, lvm);
    maskLatVolH->vfield()->set_all_values(1);
  }
  FieldInformation maskLatVolFI(maskLatVolH);
  if (!maskLatVolFI.is_latvolmesh())
  {
    error("This algorithm only works on a lat vol");
    algo_end(); return (false);
  }
  if (!maskLatVolFI.is_scalar())
  {
    error("This algorithm needs scalar values on the lat vol");
    algo_end(); return (false);
  }
  FieldInformation fi("PointCloudMesh", "NoData", "double");
  fi.make_nodata();
  fi.make_constantmesh();
  fi.make_pointcloudmesh();
  MeshHandle medialPtsMeshH = CreateMesh(fi);
  VMesh *medialPtsMesh = medialPtsMeshH->vmesh();

  int refinement_levels = get_int("refinement_levels");

  VMesh *latVolMesh = maskLatVolH->vmesh();
  VField *latVolField = maskLatVolH->vfield();
  Transform t(latVolMesh->get_transform());
  VMesh::dimension_type dims;
  latVolMesh->get_dimensions(dims);
  Point origin(t.project(Point(0,0,0)));
  Point last(t.project(Point(dims[0], dims[1], dims[2])));
  Vector di((t.project(Point(1,0,0))-origin)/2);
  Vector dj((t.project(Point(0,1,0))-origin)/2);
  Vector dk((t.project(Point(0,0,1))-origin)/2);

  VMesh::Cell::iterator it, end;
  latVolMesh->begin(it);
  latVolMesh->end(end);
  double val;

  // count up the total number of mask cells that are non-zero
  unsigned int mask_sum=0;
  while(it != end) {
    latVolField->get_value(val, *it);
    if (val) mask_sum++;
    ++it;
  }
  latVolMesh->begin(it);

  // locate uses a grid acceleration structure
  tsm->synchronize(Mesh::NODE_NEIGHBORS_E);
  tsm->synchronize(Mesh::ELEM_LOCATE_E);
  tsm->synchronize(Mesh::EDGES_E);

  VMesh::Edge::iterator edgeiter, edgeend;
  tsm->begin(edgeiter);
  tsm->end(edgeend);
  int edgecount=0;
  double edgesum=0.0;
  while (edgeiter != edgeend) {
    edgecount++;
    edgesum += tsm->get_size(*edgeiter);
    ++edgeiter;
  }
  double minDistToBdry=edgesum/edgecount;

  // need the surface normals so we can make sure medial axis points are
  //   inside the surface by checking the dot product of the surface normal
  //   and the vector connection the medial axis point to the surface
  tsm->synchronize(Mesh::NORMALS_E);
  VMesh::Elem::index_type elem;
  unsigned int mask_count=0;
  VMesh::Node::array_type elem_nodes;
  SearchGridHandle elem_grid = tsm->get_elem_search_grid();

  while(it != end) {
//    if (mask_count%100 == 0) cerr << "here :"<<mask_count<<" of "<<mask_sum<<"\n";
    double val;
    latVolField->get_value(val, *it);
    if (val) {
      Point center_p;
      latVolMesh->get_center(center_p, *it);
      // check multiple points within this latvol cell, based on spacing_ratio
      VMesh::Elem::index_type qelem;
      int level=0;
      search_cell_for_medial_pts(tsm, elem_grid, center_p, di, dj, dk, qelem, level, refinement_levels, minDistToBdry, medialPtsMesh);
      mask_count++;
      if ((mask_count % 100) == 0) update_progress(mask_count*1.0/mask_sum);
    }
    ++it;
  }
  medialPtsH = CreateField(fi, medialPtsMeshH);
  algo_end();
  return (true);
}

} // namespace SCIRunAlgo
