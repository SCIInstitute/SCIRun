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


#include <Core/Algorithms/Fields/MedialAxis/MedialAxis2.h>
#include <Core/Algorithms/Fields/MergeFields/JoinFields.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/VField.h>

#include <Core/Thread/Barrier.h>
#include <Core/Thread/Thread.h>

#include <Core/Geometry/CompGeom.h>

#include <float.h>
#include <math.h>
#include <vector>

namespace SCIRunAlgo {

using namespace SCIRun;

class MedialAxisP {
  public:
   MedialAxisP(FieldHandle surface,
               FieldHandle pointcloud,
               double max_distance_diff,
               double max_normal_dot_product,
               double max_axis_dot_product,
			   double forward_mult,
			   double backward_mult,
               int    max_level,
               int    num_bins,
               ProgressReporter* pr) :
       surface_field_(surface),
       pointcloud_field_(pointcloud),
       max_distance_diff_(max_distance_diff),
       max_normal_dot_product_(max_normal_dot_product),
       max_axis_dot_product_(max_axis_dot_product),
	   forward_mult_(forward_mult),
	   backward_mult_(backward_mult),
       max_level_(max_level),
       num_bins_(num_bins),
       barrier_("MedialAxis barrier"),
       pr_(pr)
    {
      surface->vmesh()->synchronize(Mesh::NODE_NEIGHBORS_E);
      surface->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      surface->vmesh()->synchronize(Mesh::EDGES_E);
    }

    void parallel(int proc, int nproc);

    bool find_closest_two_distances(VMesh* surfmesh,
                                    Point p,
                                    double level_distance_diff,
                                    double max_normal_dot_product,
                                    double max_axis_dot_product,
                                    double max_distance_diff,
									double forward_mult,
									double backward_mult,
                                    double& dist1,
                                    double& dist2,
                                    bool& inside);
  private:

    FieldHandle surface_field_;
    FieldHandle pointcloud_field_;
    VMesh*      pointcloud_mesh_;

    double max_distance_diff_;
    double max_normal_dot_product_;
    double max_axis_dot_product_;
	double forward_mult_;
	double backward_mult_;

    int    max_level_;
    int    num_bins_;

    double level_distance_diff_;
    double cell_length_;
    std::vector<Point>  points_;
    std::vector<char>   values_;
    std::vector<char>   inside_;
    std::vector<double> diff_;

    Barrier barrier_;
    ProgressReporter* pr_;
};



void
MedialAxisP::parallel(int proc, int nproc)
{
  // Build initial grid
  if (proc==0)
  {
    BBox bbox = surface_field_->vmesh()->get_bounding_box(); // bounding box of the mesh

    Point min = bbox.min();  // min and max points of cartisian system will define box
    Point max = bbox.max();
    double tlen = Abs(max.x()-min.x()) + Abs(max.y()-min.y()) + Abs(max.z()-min.z());

    cell_length_ = tlen/static_cast<double>(num_bins_); // first cells built

	// start filling in seed points
    for (double x = min.x()+(cell_length_*0.5); x<(max.x()+(0.51*cell_length_)); x+=cell_length_)
    {
      for (double y = min.y()+(cell_length_*0.5); y<(max.y()+(0.51*cell_length_)); y+=cell_length_)
      {
        for (double z = min.z()+(cell_length_*0.5); z<(max.z()+(0.51*cell_length_)); z+=cell_length_)
        {
				points_.push_back(Point(x,y,z));
        }
      }
    }

	// arrays used for storing info
    values_.resize(points_.size());	//should we use it or not
    inside_.resize(points_.size());
    diff_.resize(points_.size());

    // length of diagonal
    level_distance_diff_ = sqrt(3.0)*cell_length_;

    pointcloud_mesh_ = pointcloud_field_->vmesh(); // point cloud mesh available to all the threads

    // Setup defaults
    if (max_level_ < 2) max_level_ = 2;


    ///////////////////

    if ((max_distance_diff_ < 0.0) ||
        (max_distance_diff_ > 0.5*cell_length_/static_cast<double>(Pow(2.0,max_level_))))
    {
      max_distance_diff_ = 0.5*cell_length_/static_cast<double>(Pow(2.0,max_level_));
    }
    if (max_normal_dot_product_ > cos(25.0/180.0*M_PI))
    {
        max_normal_dot_product_ = cos(25.0/180.0*M_PI);
    }

    if (max_axis_dot_product_ > cos(25.0/180.0*M_PI))
    {
	    max_axis_dot_product_ = cos(25.0/180.0*M_PI);
    }
  }

	// parallel algo begins, this waits till all processes are ready
  barrier_.wait(nproc);

  for (int level=0; level <= max_level_; level++)
  {
    if (proc == 0) std::cout << "*** Medial Axis: processing level "<<level<<std::endl;

    VMesh* surfmesh = surface_field_->vmesh();
	// since processor number is different for all the threads they will all have a diff beginning and ending
    size_t num_nodes = points_.size();
    size_t num_nodes_per_processor = num_nodes/nproc;
    size_t start = proc*num_nodes_per_processor;
    size_t end = (proc+1)*num_nodes_per_processor;
    if (proc == nproc-1) end = num_nodes;

    if (proc == 0) std::cout << "*** Medial Axis: evaluating "<<num_nodes << " nodes"<<std::endl;
	// where the checking starts
    for (size_t idx = start; idx<end; idx++)
    {
      Point p = points_[idx];	//start picking up points

      double dist1;
      double dist2;
      bool inside;
		// result checks the refinement points, returns distances and if it is inside or not, most time is spent here, projections found using a search grid
      bool res = find_closest_two_distances(surfmesh,p,
                                            level_distance_diff_,
                                            max_normal_dot_product_,
                                            max_axis_dot_product_,
                                            max_distance_diff_,
											forward_mult_,
											backward_mult_,
                                            dist1, dist2, inside );

      if (res == true && inside == false)
      {
        if (dist2 > level_distance_diff_) res = false;
      }

      values_[idx] = static_cast<char>(res);	//might be MA if its in here
      diff_[idx] = dist2-dist1;					//distance diff
      inside_[idx] = static_cast<char>(inside);	//inside or outside

    }
	//update progress
    if (proc == 0)
    {
      pr_->update_progress(level,max_level_);
    }
	//at this point we know all processes are done
    barrier_.wait(nproc);

	//check to see if we want to keep the points or not
    if (proc == 0)
    {
      size_t size = points_.size();
      for (size_t idx=0; idx<size;idx++)
      {
        if (diff_[idx] < max_distance_diff_ && values_[idx] && inside_[idx])  // max_distance_diff_ defined upstream, based on box size
        {
          pointcloud_mesh_->add_node(points_[idx]); // keep these
        }
	  }

      if (level < max_level_)
      {
        std::vector<Point> old_points = points_;
        size_t size = old_points.size();

        points_.clear();
        points_.reserve(old_points.size());
        cell_length_ = cell_length_ * 0.5;
        double hcl = cell_length_ * 0.5;

        level_distance_diff_ = sqrt(3.0)*cell_length_;

        for (size_t idx=0; idx<size;idx++)
        {
          if (values_[idx])
          {
            Point p = old_points[idx];					    // at the points that we kept, we add 8 more points
            points_.push_back(p+Vector(-hcl,-hcl,-hcl));	// try to refine around the point that we got
            points_.push_back(p+Vector( hcl,-hcl,-hcl));
            points_.push_back(p+Vector(-hcl, hcl,-hcl));
            points_.push_back(p+Vector( hcl, hcl,-hcl));
            points_.push_back(p+Vector(-hcl,-hcl, hcl));
            points_.push_back(p+Vector( hcl,-hcl, hcl));
            points_.push_back(p+Vector(-hcl, hcl, hcl));
            points_.push_back(p+Vector( hcl, hcl, hcl));
          }
        }
        values_.resize(points_.size());
        diff_.resize(points_.size());
        inside_.resize(points_.size());
      }
    }
    barrier_.wait(nproc);
  }
}

bool
MedialAxisP::find_closest_two_distances(VMesh* surfmesh,
                                        Point p,
                                        double level_distance_diff,
                                        double max_normal_dot_product,
                                        double max_axis_dot_product,
                                        double max_distance_diff,
										double forward_mult,
									    double backward_mult,
                                        double& dist1,
                                        double& dist2,
                                        bool& inside)
{
  // Get internal search grid
  LockingHandle< SearchGridT<index_type> > elem_grid =
                                            surfmesh->get_elem_search_grid();

  // get grid sizes
  const size_type ni = elem_grid->get_ni();
  const size_type nj = elem_grid->get_nj();
  const size_type nk = elem_grid->get_nk();

  Point *points            = surfmesh->get_points_pointer();
  VMesh::index_type *faces = surfmesh->get_elems_pointer();

  const double epsilon = surfmesh->get_epsilon();
  const double epsilon2 = epsilon*epsilon;

  // ================================================================
  // Compute closest distance

  // Convert to grid coordinates.
  VMesh::index_type bi, ei, bj, ej, bk, ek;
  elem_grid->unsafe_locate(bi, bj, bk, p);	// locate where pt is in search grid
	// bi is begin and ei is end
  if (bi >= ni) bi = ni-1; if (bi < 0) bi = 0;
  if (bj >= nj) bj = nj-1; if (bj < 0) bj = 0;
  if (bk >= nk) bk = nk-1; if (bk < 0) bk = 0;
  ei= bi; ej = bj; ek = bk;

  double dmin = DBL_MAX;
  bool found = true;
  bool found_one = false;

  VMesh::Elem::index_type face = -1;
  Point first_point;
  int edge = -1;
  int node = -1;
  // loops through looking at all the immediate neighbor cells, if it doesn't find anything to make it true, it expands its search to two cells out and so on
  do   // if it doesn't find anything it will go to false.
  {
    found = true;
    /// We need to do a full shell without any elements that are closer
    /// to make sure there no closer elements in neighboring searchgrid cells
    for (index_type i = bi; i <= ei; i++) // finds closest distance to the surface
    {
      if (i < 0 || i >= ni) continue;

      for (index_type j = bj; j <= ej; j++)
      {
        if (j < 0 || j >= nj) continue;
        for (index_type k = bk; k <= ek; k++)
        {
          if (k < 0 || k >= nk) continue;
          if (i == bi || i == ei || j == bj || j == ej || k == bk || k == ek)
          {
            if (elem_grid->min_distance_squared(p, i, j, k) < dmin)
            {
              found = false;
              SearchGridT<index_type>::iterator it, eit;
              elem_grid->lookup_ijk(it,eit, i, j, k);

              while (it != eit)
              {
                index_type idx = (*it) * 3;
                Point r; // candidate for second-closest-point
                int tedge;
                int tnode;
                closest_point_on_tri(r, tedge, tnode, p, points[faces[idx]],
                                                  points[faces[idx+1]],
                                                  points[faces[idx+2]]);

                const double dtmp = (p - r).length2();
                if (dtmp < dmin)
                {
                  found_one = true;
                  first_point = r;
                  face = VMesh::Elem::index_type(*it);
                  edge = tedge;
                  dmin = dtmp;
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

  if (!found_one)
  {
    std::cerr << "could not find a single surface inside mesh"<<std::endl;
    return (false);
  }
  dist1 = sqrt(dmin);

  index_type fidx = face * 3;
  Point f1 = points[faces[fidx]];
  Point f2 = points[faces[fidx+1]];
  Point f3 = points[faces[fidx+2]];

  Vector fnorm1 = Cross(f2-f1,f3-f2);
  fnorm1.normalize();

  Vector fnorm2;

  // special case

  if (node > -1)  // can determine whether if point is inside or outside
  {
    VMesh::Node::array_type nodes;
    surfmesh->get_nodes(nodes,face);

    VMesh::Elem::array_type elems;
    surfmesh->get_elems(elems,nodes[node]);

    Vector anorm = Vector(0,0,0);
    for (size_t q=0; q<elems.size();q++)
    {
      index_type dfidx = elems[q] * 3;
      Point df1 = points[faces[dfidx]];
      Point df2 = points[faces[dfidx+1]];
      Point df3 = points[faces[dfidx+2]];

      Vector dfnorm1 = Cross(df2-df1,df3-df2);
      dfnorm1.normalize();
      anorm += dfnorm1;
    }

    fnorm1 = anorm;
    fnorm1.normalize();
  }
  else if (edge > -1)
  {
    VMesh::DElem::array_type delems;
    surfmesh->get_delems(delems,face);
    VMesh::DElem::index_type fedge = delems[edge];

    VMesh::Elem::index_type dface;
    surfmesh->get_neighbor(dface,face,fedge);

    index_type dfidx = dface * 3;
    Point df1 = points[faces[dfidx]];
    Point df2 = points[faces[dfidx+1]];
    Point df3 = points[faces[dfidx+2]];

    Vector dfnorm1 = Cross(df2-df1,df3-df2);
    dfnorm1.normalize();

    fnorm1 = fnorm1 + dfnorm1;
    fnorm1.normalize();
  }


  // ================================================================
  // Compute whether node is inside or outside the surface


  Vector n = fnorm1;
  Vector k = Vector(p-first_point); k.normalize();

  double angle = Dot(n,k);
  if (angle < 0.0)
  {
    inside = false;
  }
  else if (angle >= 0.0)
  {
    inside = true;
  }


  // ================================================================
  // Find second point now

  // Convert to grid coordinates.
  elem_grid->unsafe_locate(bi, bj, bk, p);

  if (bi >= ni) bi = ni-1; if (bi < 0) bi = 0;
  if (bj >= nj) bj = nj-1; if (bj < 0) bj = 0;
  if (bk >= nk) bk = nk-1; if (bk < 0) bk = 0;
  ei= bi; ej = bj; ek = bk;
  dmin = (dist1+level_distance_diff)*(dist1+level_distance_diff);

  found = true;
  found_one = false;

  Point second_point;
  face = -1;

  Vector norm1 = (first_point-p);
  norm1.normalize();

  do
  {
    found = true;
    /// We need to do a full shell without any elements that are closer
    /// to make sure there no closer elements in neighboring searchgrid cells
    for (index_type i = bi; i <= ei; i++)
    {
      if (i < 0 || i >= ni) continue;
      for (index_type j = bj; j <= ej; j++)
      {
        if (j < 0 || j >= nj) continue;

        for (index_type k = bk; k <= ek; k++)
        {
          if (k < 0 || k >= nk) continue;

          if (i == bi || i == ei || j == bj || j == ej || k == bk || k == ek)
          {
            if (elem_grid->min_distance_squared(p, i, j, k) < dmin)
            {
              found = false;
              SearchGridT<index_type>::iterator it, eit;
              elem_grid->lookup_ijk(it,eit, i, j, k);

              while (it != eit)
              {
                index_type idx = (*it) * 3;
                Point r; // candidate for second-closest-point
                Point f1 = points[faces[idx]];
                Point f2 = points[faces[idx+1]];
                Point f3 = points[faces[idx+2]];

                int tedge, tnode;
                closest_point_on_tri(r, tedge, tnode, p, f1, f2, f3);

                Vector norm2 = r-p;
                norm2.normalize();
			    if (Dot(norm1,norm2) < max_axis_dot_product)
                {
                  Vector tfnorm2 = Cross(f2-f1,f3-f2);
                  tfnorm2.normalize();
                  if (Dot(fnorm1,tfnorm2) < max_normal_dot_product)	// check whether normals are at certain angle and alpha angles are at certain angle
                  {													// make sure we're measuring surfaces facing diff directions, and that there is a good enough angle between pts.
                    Vector k = r-first_point;
                    double hl = k.length()*0.5;
                    k.normalize();
                    Vector rp = r-p;
                    double d = Abs(hl-Abs(Dot(k,rp)));

                    if (d < level_distance_diff)
                    {
                      const double dtmp = (r-p).length2();
                      if (dtmp < dmin)
                      {
                        found_one = true;
                        face = VMesh::Elem::index_type(*it);
                        edge = tedge;
                        node = tnode;
                        fnorm2 = tfnorm2;
                        second_point = r;
                        dmin = dtmp;
                      }
                    }
                  }
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

  if (node > -1)
  {
    VMesh::Node::array_type nodes;
    surfmesh->get_nodes(nodes,face);

    VMesh::Elem::array_type elems;
    surfmesh->get_elems(elems,nodes[node]);

    Vector anorm = Vector(0,0,0);
    for (size_t q=0; q<elems.size();q++)
    {
      index_type dfidx = elems[q] * 3;
      Point df1 = points[faces[dfidx]];
      Point df2 = points[faces[dfidx+1]];
      Point df3 = points[faces[dfidx+2]];

      Vector dfnorm1 = Cross(df2-df1,df3-df2);
      dfnorm1.normalize();
      anorm += dfnorm1;
    }

    fnorm1 = anorm;
    fnorm1.normalize();
  }
  else if (edge > -1)
  {
    VMesh::DElem::array_type delems;
    surfmesh->get_delems(delems,face);
    VMesh::DElem::index_type fedge = delems[edge];

    VMesh::Elem::index_type dface;
    surfmesh->get_neighbor(dface,face,fedge);

    index_type dfidx = dface * 3;
    Point df1 = points[faces[dfidx]];
    Point df2 = points[faces[dfidx+1]];
    Point df3 = points[faces[dfidx+2]];

    Vector dfnorm2 = Cross(df2-df1,df3-df2);
    dfnorm2.normalize();

    fnorm2 = fnorm2 + dfnorm2;
    fnorm2.normalize();
  }

  k = Vector(p-second_point); k.normalize();
  angle = Dot(fnorm2,k);
  if (angle < 0.0)
  {
    inside = false;
  }


  dist2 = sqrt(dmin);

  // If we are close to surface, do a back projection
  // if we find a surface close to the other side we keep the
  // point. Otherwise throw it out

  if (dist1 < forward_mult*max_distance_diff)	// check how close point is close to the surface
  {
    // close to border, need to do special check

    double max_back_dot_product = cos(135.0/180.0*M_PI);

    // Convert to grid coordinates.
    elem_grid->unsafe_locate(bi, bj, bk, p);

    if (bi >= ni) bi = ni-1; if (bi < 0) bi = 0;
    if (bj >= nj) bj = nj-1; if (bj < 0) bj = 0;
    if (bk >= nk) bk = nk-1; if (bk < 0) bk = 0;
    ei= bi; ej = bj; ek = bk;

    dmin = (backward_mult*max_distance_diff)*(backward_mult*max_distance_diff);	// if the forward distance diff is 2* and backward is 4* keep it

    found = true;
    found_one = false;

    face = -1;

    Vector norm1 = (first_point-p);
    norm1.normalize();

    Vector norm2 = (second_point-p);
    norm2.normalize();

    if (Dot(norm1,norm2) < max_back_dot_product)
    {
      // Points are at opposite directions
      return (true);
    }

    Vector norm3 = norm1+norm2;
    norm3.normalize();

    do
    {
      found = true;
      /// We need to do a full shell without any elements that are closer
      /// to make sure there no closer elements in neighboring searchgrid cells
      for (index_type i = bi; i <= ei; i++)
      {
        if (i < 0 || i >= ni) continue;
        for (index_type j = bj; j <= ej; j++)
        {
          if (j < 0 || j >= nj) continue;

          for (index_type k = bk; k <= ek; k++)
          {
            if (k < 0 || k >= nk) continue;
            if (i == bi || i == ei || j == bj || j == ej || k == bk || k == ek)
            {
              if (elem_grid->min_distance_squared(p, i, j, k) < dmin)
              {
                found = false;
                SearchGridT<index_type>::iterator it, eit;
                elem_grid->lookup_ijk(it,eit, i, j, k);

                while (it != eit)
                {
                  index_type idx = (*it) * 3;
                  Point r; // candidate for second-closest-point
                  Point f1 = points[faces[idx]];
                  Point f2 = points[faces[idx+1]];
                  Point f3 = points[faces[idx+2]];

                  int tedge, tnode;
                  closest_point_on_tri(r, tedge, tnode, p, f1, f2, f3);

                  Vector norm4 = r-p;
                  if (norm4.length2() < dmin)
                  {
                    norm4.normalize();
                    if (Dot(norm3,norm4) < max_back_dot_product)
                    {
                      return (true);
                    }
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

    // This is only to mark the point as bad. It is inside, but too close
    // to the border and with the backward projection far away. Hence this
    // will effectively toss the point out.
    inside = false;
  }

  return (true);
}



bool
MedialAxis2Algo::run(FieldHandle Surface, FieldHandle& MedialAxis)
{
  algo_start("MedialAxis");

  FieldInformation surface_fi(Surface);

  if (!surface_fi.is_trisurfmesh())
  {
    error("Medial Axis can only be computed for a TriSurfMesh\n");
    algo_end(); return (false);
  }

  int    refinement_levels = get_int("refinement_levels");

  double axis_min_angle = get_scalar("axis_minimum_angle");
  double axis_max_dot_product;
  if (axis_min_angle > 0.0) axis_max_dot_product = cos((axis_min_angle/180.0)*M_PI);
  else axis_max_dot_product = cos((35.0/180.0)*M_PI);

  double normal_min_angle = get_scalar("normal_minimum_angle");
  double normal_max_dot_product;
  if (normal_min_angle > 0.0) normal_max_dot_product = cos((normal_min_angle/180.0)*M_PI);
  else normal_max_dot_product = cos((35.0/180.0)*M_PI);

  double forward_proj_mult = get_scalar("forward_projection_multiplier");
  double back_proj_mult = get_scalar("backward_projection_multiplier");
  double forward_multiplier,backward_multiplier;
  if (forward_proj_mult > 0.0) forward_multiplier = forward_proj_mult;
  else forward_multiplier = 2.0;
  if (back_proj_mult > 0.0) backward_multiplier = back_proj_mult;
  else backward_multiplier = 4.0;


  double max_distance_diff = get_scalar("maximum_distance_difference");
  int start_num_bins = get_int("start_num_bins");

  MedialAxis = CreateField(POINTCLOUDMESH_E,NODATA_E,DOUBLE_E); // output field
	// parallel class
  MedialAxisP palgo(Surface,
                    MedialAxis,
                    max_distance_diff,
                    normal_max_dot_product,
                    axis_max_dot_product,
					forward_multiplier,
					backward_multiplier,
                    refinement_levels,
                    start_num_bins,
                    get_progress_reporter());

// starting point for parllel algo, based off of palgo class, running parallel, on numProcessors (1st on is the current processor, next is additional processors that we are running on)
  Thread::parallel(&palgo,&MedialAxisP::parallel,Thread::numProcessors(),Thread::numProcessors()); //parallel version
//  Thread::parallel(&palgo,&MedialAxisP::parallel,1,1); //serial version


//down sample nodes
  SCIRunAlgo::JoinFieldsAlgo join_;

  VMesh* smesh = Surface->vmesh();
  VMesh::size_type numelems = smesh->num_elems();
  double area = 0.0;

  for (VMesh::Elem::index_type idx=0; idx<numelems; idx++)
  {
    area += smesh->get_size(idx);
  }
  area = area / static_cast<double>(numelems);
  double tolerance = 0.5*sqrt(area);

  bool   mergenodes = true;

  join_.set_scalar("tolerance",tolerance);
  join_.set_bool("merge_nodes",mergenodes);

  join_.set_progress_reporter(get_progress_reporter());

  std::vector<FieldHandle> input(1);

  if (MedialAxis->vmesh()->num_nodes())
  {
    input[0] = MedialAxis;
    if(!(join_.run(input,MedialAxis))) return(false);
  }

  algo_end();
  return (true);
}

} // namespace SCIRunAlgo
