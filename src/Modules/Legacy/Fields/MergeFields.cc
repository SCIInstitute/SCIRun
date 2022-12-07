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


///
///@file  MergeFields.cc
///
///@author
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  January 2006
///

#include <Core/Util/StringUtil.h>
#include <Core/Geometry/CompGeom.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <vector>
#include <set>
#include <iostream>

namespace SCIRun {

/// @class MergeFields
/// @brief This module insers the elements from a field into a TetVol or TriSurf field.

class MergeFields : public Module
{
  public:
    MergeFields(GuiContext* ctx);
    virtual ~MergeFields() {}

    virtual void execute();

  private:
    int tet_generation_;
    int insert_generation_;

    FieldHandle combined_;
    FieldHandle extended_;
    MatrixHandle mapping_;
};


DECLARE_MAKER(MergeFields)

MergeFields::MergeFields(GuiContext* ctx)
  : Module("MergeFields", ctx, Filter, "NewField", "SCIRun"),
    tet_generation_(-1),
    insert_generation_(-1)
{
}


class ltvn
{
  public:
    bool operator()(const VMesh::Node::array_type &a,
                    const VMesh::Node::array_type &b) const
    {
      for (size_t i = 0; i < a.size(); i++)
      {
        if (a[i] < b[i]) return true;
        if (a[i] > b[i]) return false;
      }
      return false;
    }
};

typedef std::set<VMesh::Node::array_type, ltvn> added_set_type;


void
MergeFields::execute()
{
  FieldHandle tet_field;
  get_input_handle("Container Mesh", tet_field,true);

  FieldInformation fi(tet_field);

  if (!(fi.is_trisurfmesh()||fi.is_tetvolmesh()))
  {
    error("Container Mesh must contain a TetVolMesh or TriSurfMesh.");
    return;
  }

  bool tri = fi.is_trisurfmesh();

  // Get input field 1.
  FieldHandle insert_field;
  get_input_handle("Insert Field", insert_field,true);

  update_state(Executing);

  bool update = false;

  // Check to see if the source field has changed.
  if( tet_generation_ != tet_field->generation )
  {
    tet_generation_ = tet_field->generation;
    update = true;
  }

  // Check to see if the source field has changed.
  if( insert_generation_ != insert_field->generation )
  {
    insert_generation_ = insert_field->generation;
    update = true;
  }

  if( !combined_.get_rep() || update)
  {
    combined_ = tet_field;
    tet_field = 0;
    combined_.detach();
    combined_->mesh_detach();
    const int dim = insert_field->vmesh()->dimensionality();

    std::vector<VMesh::index_type> added_nodes;
    std::vector<VMesh::index_type> added_elems;
    if (!tri)
    {
      if (dim == 0)
      {
        VField *tfield = combined_->vfield();
        VMesh  *tmesh   = combined_->vmesh();
        VMesh  *imesh = insert_field->vmesh();

        tmesh->synchronize(Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E
                           | Mesh::FACES_E | Mesh::LOCATE_E);

        VMesh::Node::iterator ibi, iei;
        imesh->begin(ibi);
        imesh->end(iei);

        while (ibi != iei)
        {
          Point p;
          imesh->get_center(p, *ibi);

          VMesh::Elem::index_type elem;
          if (tmesh->locate(elem, p))
          {
            VMesh::Node::index_type newnode;
            VMesh::Elem::array_type newelems;
            tmesh->insert_node_into_elem(newelems, newnode, elem, p);

            added_nodes.push_back(newnode);
            for (size_t i = 0; i < newelems.size(); i++)
            {
              added_elems.push_back(newelems[i]);
            }
          }

          ++ibi;
        }

        tmesh->clear_synchronization();
        tfield->resize_values();
      }
      if (dim >= 1)
      {
        VField *tfield = combined_->vfield();
        VMesh  *tmesh   = combined_->vmesh();
        VMesh  *imesh = insert_field->vmesh();

        imesh->synchronize(Mesh::EDGES_E);
        tmesh->synchronize(Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E
                           | Mesh::FACES_E | Mesh::LOCATE_E);

        VMesh::Edge::iterator ibi, iei;
        imesh->begin(ibi);
        imesh->end(iei);

        std::vector<Point> points;

        while (ibi != iei)
        {
          VMesh::Node::array_type enodes;
          imesh->get_nodes(enodes, *ibi);
          ++ibi;

          VMesh::Elem::index_type elem, neighbor;
          VMesh::DElem::array_type faces;
          VMesh::Node::array_type nodes;
          VMesh::DElem::index_type minface;

          Point e0, e1;
          imesh->get_center(e0, enodes[0]);
          imesh->get_center(e1, enodes[1]);

          // Find our first element.  If e0 isn't inside then try e1.  Need
          // to handle falling outside of mesh better.
          if (!tmesh->locate(elem, e0))
          {
            Point tmp = e0;
            e0 = e1;
            e1 = tmp;
            if (!tmesh->locate(elem, e0)) continue;
          }

          Vector dir = e1 - e0;

          points.push_back(e0);

          index_type i;
          size_type maxsteps = 10000;
          for (i=0; i < maxsteps; i++)
          {
            tmesh->get_delems(faces, elem);
            double mindist = 1.0-1.0e-6;
            bool found = false;
            Point ecenter;
            tmesh->get_center(ecenter, elem);
            for (size_t j=0; j < faces.size(); j++)
            {
              Point p0, p1, p2;
              tmesh->get_nodes(nodes, faces[j]);
              tmesh->get_center(p0, nodes[0]);
              tmesh->get_center(p1, nodes[1]);
              tmesh->get_center(p2, nodes[2]);
              Vector normal = Cross(p1-p0, p2-p0);
              if (Dot(normal, ecenter-p0) > 0.0) { normal *= -1.0; }
              const double dist = RayPlaneIntersection(e0, dir, p0, normal);
              if (dist > -1.0e-6 && dist < mindist)
              {
                mindist = dist;
                minface = faces[j];
                found = true;
              }
            }
            if (!found) { break; }

            if (mindist > 1.0e-6) { points.push_back(e0 + dir * mindist); }

            /// @todo:  Handle falling outside of mesh better.  May not be convex.
            if (!tmesh->get_neighbor(neighbor, elem, minface)) { break; }
            elem = neighbor;
          }
          points.push_back(e1);
        }

        VMesh::Elem::index_type elem;
        VMesh::Node::index_type newnode;
        VMesh::Elem::array_type newelems;
        for (size_t i = 0; i < points.size(); i++)
        {
          if (tmesh->locate(elem, points[i]))
          {
            tmesh->insert_node_into_elem(newelems, newnode, elem, points[i]);

            added_nodes.push_back(newnode);
            for (size_t i = 0; i < newelems.size(); i++)
            {
              added_elems.push_back(newelems[i]);
            }
          }
        }

        tmesh->clear_synchronization();
        tfield->resize_values();
      }
      if (dim >= 2)
      {
        VField *tfield = combined_->vfield();
        VMesh  *tmesh   = combined_->vmesh();
        VMesh  *imesh = insert_field->vmesh();

        imesh->synchronize(Mesh::FACES_E);
        tmesh->synchronize(Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E
                           | Mesh::FACES_E | Mesh::LOCATE_E);

        VMesh::Face::iterator ibi, iei;
        imesh->begin(ibi);
        imesh->end(iei);

        std::vector<Point> points;

        while (ibi != iei)
        {
          VMesh::Node::array_type fnodes;
          imesh->get_nodes(fnodes, *ibi);

          size_t i;

          std::vector<Point> tri;
          for (i = 0; i < fnodes.size(); i++)
          {
            Point p;
            imesh->get_center(p, fnodes[i]);
            tri.push_back(p);
          }

          // Test each triangle in the face (fan the polygon).
          for (i = 2; i < tri.size(); i++)
          {
            // Intersects all of the edges in the tetvol with the triangle.
            // Add each intersection between (0,1) to the results.

            /// @todo: We only need to test the edges that are 'close', not all
            // of them.  Augment the locate grid and use that to speed this up.

            VMesh::Edge::iterator edge_iter, edge_iter_end;
            tmesh->begin(edge_iter);
            tmesh->end(edge_iter_end);
            while (edge_iter != edge_iter_end)
            {
              VMesh::Node::array_type nodes;
              tmesh->get_nodes(nodes, *edge_iter);

              Point e0, e1;
              tmesh->get_center(e0, nodes[0]);
              tmesh->get_center(e1, nodes[1]);
              Vector dir = e1 - e0;

              double t, u, v;
              const bool hit = RayTriangleIntersection(t, u, v, false, e0, dir,
                                                       tri[0], tri[i-1], tri[i]);

              if (hit && t > 0 && t < 1.0)
              {
                points.push_back(e0 + t * dir);
              }

              ++edge_iter;
            }
          }
          ++ibi;
        }

        VMesh::Elem::index_type elem;
        VMesh::Node::index_type newnode;
        VMesh::Elem::array_type newelems;
        for (size_t j = 0; j < points.size(); j++)
        {
          if (tmesh->locate(elem, points[j]))
          {
            tmesh->insert_node_into_elem(newelems, newnode, elem, points[j]);

            added_nodes.push_back(newnode);
            for (size_t k = 0; k < newelems.size(); k++)
            {
              added_elems.push_back(newelems[k]);
            }
          }
        }

        tmesh->clear_synchronization();
        tfield->resize_values();
      }
    }
    else
    {
      if (dim == 0)
      {
        VField *tfield = combined_->vfield();
        VMesh  *tmesh   = combined_->vmesh();
        VMesh  *imesh = insert_field->vmesh();

        tmesh->synchronize(Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E
                           | Mesh::FACES_E | Mesh::LOCATE_E);

        VMesh::Node::iterator ibi, iei;
        imesh->begin(ibi);
        imesh->end(iei);

        while (ibi != iei)
        {
          Point p;
          imesh->get_center(p, *ibi);

          Point cp;
          VMesh::Elem::index_type cf;
          double dist;
          tmesh->find_closest_elem(dist,cp, cf, p);

          VMesh::Node::index_type newnode;
          VMesh::Elem::array_type newelems;
          tmesh->insert_node_into_elem(newelems, newnode, cf, cp);

          added_nodes.push_back(newnode);
          for (size_t i = 0; i < newelems.size(); i++)
          {
            added_elems.push_back(newelems[i]);
          }

          ++ibi;
        }

        tmesh->synchronize(Mesh::EDGES_E);
        tfield->resize_values();
      }
      if (dim >= 1)
      {
        VField *tfield = combined_->vfield();
        VMesh  *tmesh   = combined_->vmesh();
        VMesh  *imesh = insert_field->vmesh();

        tmesh->synchronize(Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E
                           | Mesh::FACES_E | Mesh::LOCATE_E);
        imesh->synchronize(Mesh::EDGES_E);

        VMesh::Edge::iterator ibi, iei;
        imesh->begin(ibi);
        imesh->end(iei);

        while (ibi != iei)
        {
          VMesh::Node::array_type nodes;

          imesh->get_nodes(nodes, *ibi);
          Point p[2];
          imesh->get_center(p[0], nodes[0]);
          imesh->get_center(p[1], nodes[1]);

          std::vector<Point> insertpoints;

          Point closest[2];
          VMesh::Elem::index_type elem;
          VMesh::Elem::index_type elem_end;
          double dist;
          tmesh->find_closest_elem(dist,closest[0], elem, p[1]);
          insertpoints.push_back(closest[0]);
          tmesh->find_closest_elem(dist,closest[1], elem_end, p[0]);

          /// @todo: Find closest could and will land on degeneracies meaning
          // that our choice of elements there is arbitrary.  Need to walk
          // along near surface elements (breadth first search) instead of
          // exact elements.
          while (elem != elem_end)
          {
            VMesh::Node::array_type tnodes;
            tmesh->get_nodes(tnodes, elem);

            VMesh::points_type tp;
            VMesh::Elem::index_type nelem;

            bool found = false;

            VMesh::Edge::array_type edges;
            VMesh::Node::array_type nodes;
            tmesh->get_edges(edges, elem);

            for (size_t k = 0; k < edges.size(); k++)
            {
              tmesh->get_nodes(nodes,edges[k]);
              tmesh->get_centers(tp,nodes);

              double s, t;
              closest_line_to_line(s, t, closest[0], closest[1], tp[0], tp[1]);

              if (s > 1e-12 && s < 1.0 - 1e-12 &&
                  t > 1e-12 && t < 1.0 - 1e-12)
              {
                found = true;
                insertpoints.push_back(tp[0] + t * (tp[1] - tp[0]));
                break;
              }
            }
            if (!found)
            {
              std::cout << "EDGE WALKER DEAD END! " << elem << " " << *ibi << "\n";
              break;
            }
          }

          insertpoints.push_back(closest[1]);

          VMesh::Node::index_type newnode;
          VMesh::Elem::array_type newelems;
          for (size_t i = 0; i < insertpoints.size(); i++)
          {
            Point closest;
            VMesh::Elem::index_type elem;
            double dist;
            tmesh->find_closest_elem(dist,closest, elem, insertpoints[i]);
            tmesh->insert_node_into_elem(newelems, newnode, elem, closest);
            added_nodes.push_back(newnode);
            for (index_type j = static_cast<index_type>(newelems.size()-1); j >= 0; j--)
            {
              added_elems.push_back(newelems[j]);
            }
          }

          ++ibi;
        }

        tfield->resize_values();
      }
    }

    VMesh  *tmesh = combined_->vmesh();

    if (dim == 0)
    {
      FieldInformation fi("PointCloudMesh",1,"double");
      extended_ = CreateField(fi);
    }
    else if (dim == 1)
    {
      FieldInformation fi("CurveMesh",1,"double");
      extended_ = CreateField(fi);
    }
    else if (dim == 2)
    {
      FieldInformation fi("TriSurfMesh",1,"double");
      extended_ = CreateField(fi);
    }
    else if (dim == 3)
    {
      FieldInformation fi("TetVolMesh",1,"double");
      extended_ = CreateField(fi);
    }

    VMesh* omesh = extended_->vmesh();

    tmesh->synchronize(Mesh::EDGES_E | Mesh::FACES_E);

    std::sort(added_nodes.begin(), added_nodes.end());
    std::vector<VMesh::index_type>::iterator nodes_end, itr;
    nodes_end = std::unique(added_nodes.begin(), added_nodes.end());

    for (itr = added_nodes.begin(); itr != nodes_end; ++itr)
    {
      Point p;
      tmesh->get_point(p, VMesh::Node::index_type(*itr));
      omesh->add_point(p);
    }

    if (omesh->dimensionality() > 0)
    {
      added_set_type already_added;

      std::sort(added_elems.begin(), added_elems.end());
      std::vector<VMesh::index_type>::iterator elems_end;
      elems_end = std::unique(added_elems.begin(), added_elems.end());
      for (itr = added_elems.begin(); itr != elems_end; ++itr)
      {
        if (omesh->dimensionality() == 1)
        {
          VMesh::Edge::array_type edges;
          tmesh->get_edges(edges, VMesh::Elem::index_type(*itr));
          for (size_t i = 0; i < edges.size(); i++)
          {
            VMesh::Node::array_type oldnodes;
            VMesh::Node::array_type newnodes;
            tmesh->get_nodes(oldnodes, edges[i]);
            bool all_found = true;
            for (size_t j = 0; j < oldnodes.size(); j++)
            {
              std::vector<VMesh::index_type>::iterator loc =
                lower_bound(added_nodes.begin(), nodes_end, oldnodes[j]);
              if (loc != nodes_end && *loc == oldnodes[j])
              {
                newnodes.push_back(loc - added_nodes.begin());
              }
              else
              {
                all_found = false;
                break;
              }
            }

            if (all_found)
            {
              std::sort(newnodes.begin(), newnodes.end());
              added_set_type::iterator found =
                already_added.find(newnodes);
              if (found == already_added.end())
              {
                already_added.insert(newnodes);
                omesh->add_elem(newnodes);
              }
            }
          }
        }
        if (omesh->dimensionality() == 2)
        {
          VMesh::Face::array_type faces;
          tmesh->get_faces(faces, VMesh::Elem::index_type(*itr));
          for (size_t i = 0; i < faces.size(); i++)
          {
            VMesh::Node::array_type oldnodes;
            VMesh::Node::array_type newnodes;
            tmesh->get_nodes(oldnodes, faces[i]);
            bool all_found = true;
            for(size_t j = 0; j < oldnodes.size(); j++)
            {
              std::vector<VMesh::index_type>::iterator loc =
                std::lower_bound(added_nodes.begin(), nodes_end, oldnodes[j]);
              if (loc != nodes_end && *loc == oldnodes[j])
              {
                newnodes.push_back(loc - added_nodes.begin());
              }
              else
              {
                all_found = false;
                break;
              }
            }

            if (all_found)
            {
              std::sort(newnodes.begin(), newnodes.end());
              added_set_type::iterator found =
                already_added.find(newnodes);
              if (found == already_added.end())
              {
                already_added.insert(newnodes);
                omesh->add_elem(newnodes);
              }
            }
          }
        }
        if (omesh->dimensionality() == 3)
        {
          VMesh::Node::array_type oldnodes;
          VMesh::Node::array_type newnodes;
          tmesh->get_nodes(oldnodes,VMesh::Elem::index_type(*itr));
          bool all_found = true;
          for (size_t j = 0; j < oldnodes.size(); j++)
          {
            std::vector<VMesh::index_type>::iterator loc =
              std::lower_bound(added_nodes.begin(), nodes_end, oldnodes[j]);
            if (loc != nodes_end && *loc == oldnodes[j])
            {
              newnodes.push_back(loc - added_nodes.begin());
            }
            else
            {
              all_found = false;
              break;
            }
          }

          if (all_found)
          {
            omesh->add_elem(newnodes);
          }
        }
      }
    }

    // Create the output field.
    extended_->vfield()->resize_values();

    // Create the output mapping.
    VMesh::Node::size_type tnodesize;
    tmesh->size(tnodesize);
    VMesh::Node::size_type onodesize;
    omesh->size(onodesize);

    const size_type nrows = static_cast<size_type>(onodesize);
    const size_type ncols = static_cast<size_type>(tnodesize);
    SparseRowMatrix::Data sparseData(nrows+1, nrows);
    const SparseRowMatrix::Rows& rr = sparseData.rows();
    const SparseRowMatrix::Columns& cc = sparseData.columns();
    const SparseRowMatrix::Storage& d = sparseData.data();

    index_type i = 0;
    for (itr = added_nodes.begin(); itr != nodes_end; itr++)
    {
      cc[i] = *itr;
      i++;
    }

    index_type j;
    for (j = 0; j < nrows; j++)
    {
      rr[j] = j;
      d[j] = 1.0;
    }
    rr[j] = j;

    mapping_ = new SparseRowMatrix(nrows, ncols, sparseData, nrows);

  }

  send_output_handle("Combined Field", combined_, true);
  send_output_handle("Extended Insert Field", extended_, true);
  send_output_handle("Combined To Extended Mapping", mapping_, true);
}


} // End namespace SCIRun
