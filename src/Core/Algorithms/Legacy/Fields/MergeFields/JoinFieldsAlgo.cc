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


#include <Core/Algorithms/Legacy/Fields/MergeFields/JoinFieldsAlgo.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Transform.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/SearchGridT.h>
#include <boost/scoped_ptr.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

AlgorithmParameterName JoinFieldsAlgo::MergeNodes("merge_nodes");
AlgorithmParameterName JoinFieldsAlgo::MergeElems("merge_elems");
AlgorithmParameterName JoinFieldsAlgo::Tolerance("tolerance");
AlgorithmParameterName JoinFieldsAlgo::MatchNodeValues("match_node_values");
AlgorithmParameterName JoinFieldsAlgo::MakeNoData("make_no_data");

JoinFieldsAlgo::JoinFieldsAlgo()
{
  /// Merge duplicate nodes?
  addParameter(MergeNodes, true);
  /// Merge duplicate elements?
  addParameter(MergeElems, false);
  /// Tolerance for merging duplicate nodes?
  addParameter(Tolerance, 1e-6);
  /// Only merge nodes whose value is the same
  addParameter(MatchNodeValues, false);
  /// Create a field with no data
  addParameter(MakeNoData, false);
}

bool
JoinFieldsAlgo::runImpl(const FieldList& input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "JoinFields");

  std::vector<FieldHandle> inputs;
  std::copy_if(input.begin(), input.end(), std::back_inserter(inputs), [](FieldHandle f) { return f; });

  if (inputs.empty())
  {
    error("No input fields given");
    return (false);
  }

  bool match_node_values = get(MatchNodeValues).toBool();
  bool make_no_data = get(MakeNoData).toBool();
  bool merge_nodes = get(MergeNodes).toBool();
  bool merge_elems = get(MergeElems).toBool();

  double tol = get(Tolerance).toDouble();
  const double tol2 = tol*tol;

  // Check whether mesh types are the same
  FieldInformation first(inputs[0]);
  first.make_unstructuredmesh();

  /// Make sure mesh and mesh basis order are equal
  for (size_t p=1; p<inputs.size(); p++)
  {
    FieldInformation fi(inputs[p]);
    fi.make_unstructuredmesh();
    if (fi.get_mesh_type() != first.get_mesh_type())
    {
      error("Mesh elements need to be equal in order to join multiple fields together");
      return (false);
    }

    if (fi.mesh_basis_order() != first.mesh_basis_order())
    {
      error("Mesh elements need to be of the same order in for joining multiple fields together");
      return (false);
    }
  }

  if (make_no_data)
  {
    first.make_nodata();
  }
  else
  {
    for (size_t p=1; p<inputs.size(); p++)
    {
      FieldInformation fi(input[p]);
      if (fi.field_basis_order() != first.field_basis_order())
      {
        error("Fields need to have the same basis order");
        return (false);
      }

      if (fi.get_data_type() != first.get_data_type())
      {
        if (fi.is_scalar() && first.is_scalar())
        {
          remark("Converting field to double as fields use different datatypes");
          first.make_double();
        }
        else
        {
          error("Fields have different data types");
          return (false);
        }
      }
    }
  }

  if (match_node_values)
  {
    for (size_t p=0; p<inputs.size(); p++)
    {
      VField* ifield = inputs[p]->vfield();
      if (ifield->num_values() == 0)
      {
        error("Node values can only be matched if all fields contain data values.");
        return (false);
      }

      FieldInformation fi(input[p]);
      if (!(fi.is_scalar()))
      {
        error("Node values can only be matched for scalar values");
        return (false);
      }

      if (fi.is_float() || fi.is_double())
      {
        remark("Converting floating point values into integers for matching values");
        first.make_int();
      }
    }
  }

  BBox box;
  boost::scoped_ptr<SearchGridT<index_type> > node_grid;
  boost::scoped_ptr<SearchGridT<index_type> > elem_grid;

  size_type ni = 0, nj = 0, nk = 0;

  size_type tot_num_nodes = 0;
  size_type tot_num_elems = 0;

  if (merge_elems) merge_nodes = true;

  // Compute bounding box and number of nodes
  // and elements
  for (size_t p = 0; p < inputs.size(); p++)
  {
    VMesh* imesh = inputs[p]->vmesh();
    if (merge_nodes)
    {
      box.extend(imesh->get_bounding_box());
    }
    tot_num_nodes += imesh->num_nodes();
    tot_num_elems += imesh->num_elems();
  }

//  if (merge_elems) merge_nodes = true;

  // Add an epsilon so all nodes will be inside
  if (merge_nodes)
  {
    if (!box.valid())
      THROW_ALGORITHM_PROCESSING_ERROR("Merging nodes will fail: BBox is empty or invalid, diagonal not provided.");
    box.extend(1e-5*box.diagonal().length());

    const size_type s =  3*static_cast<size_type>
      ((ceil(pow(static_cast<double>(tot_num_nodes) , (1.0/3.0))))/2.0 + 1.0);

    Vector diag  = box.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(diag.z()/trace*s));

    node_grid.reset(new SearchGridT<index_type>(sx, sy, sz, box.get_min(), box.get_max()));

    if (sx == 0) sx = 1;
    if (sy == 0) sy = 1;
    if (sz == 0) sz = 1;

    ni = node_grid->get_ni()-1;
    nj = node_grid->get_nj()-1;
    nk = node_grid->get_nk()-1;
  }

  if (merge_elems)
  {
    const size_type s =  3*static_cast<size_type>
      ((ceil(pow(static_cast<double>(tot_num_nodes) , (1.0/3.0))))/2.0 + 1.0);

    Vector diag  = box.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(diag.z()/trace*s));

    if (sx == 0) sx = 1;
    if (sy == 0) sy = 1;
    if (sz == 0) sz = 1;

    elem_grid.reset(new SearchGridT<index_type>(sx, sy, sz, box.get_min(), box.get_max()));

    ni = node_grid->get_ni()-1;
    nj = node_grid->get_nj()-1;
    nk = node_grid->get_nk()-1;
  }

  MeshHandle mesh = CreateMesh(first);
  if (!mesh)
  {
    error("Could not create output mesh");
    return (false);
  }

  output = CreateField(first,mesh);
  if (!output)
  {
    error("Could not create output field");
    return (false);
  }

  VMesh* omesh = output->vmesh();
  VField* ofield = output->vfield();

  omesh->node_reserve(tot_num_nodes);
  omesh->elem_reserve(tot_num_elems);

  size_type nodes_offset = 0;
  size_type elems_offset = 0;
  size_type nodes_count = 0;
  size_type elems_count = 0;

  Point P;
  std::vector<int> values;
  int curval;
  if (match_node_values) values.resize(tot_num_nodes);

  for (size_t p = 0; p < inputs.size(); p++)
  {
    if (inputs[p]->vmesh()->is_pointcloudmesh())
    {
      merge_elems = false;
    }
  }

  for (size_t p = 0; p < inputs.size(); p++)
  {
    elems_count = 0;
    nodes_count = 0;

    VMesh* imesh = inputs[p]->vmesh();
    VField* ifield = inputs[p]->vfield();

    VMesh::Node::array_type nodes, newnodes, nodes2;
    std::vector<index_type> newnodes_sorted;

    size_type num_elems = imesh->num_elems();
    size_type num_nodes = imesh->num_nodes();
    std::vector<VMesh::Node::index_type> local_to_global(num_nodes,-1);
    std::vector<VMesh::Elem::index_type> local_to_global_elem;

    if (merge_elems)
    {
      local_to_global_elem.resize(num_elems,-1);
    }

    for (VMesh::Elem::index_type idx=0; idx<num_elems;idx++)
    {
      imesh->get_nodes(nodes,idx);

      newnodes.resize(nodes.size());
      for(size_t q=0; q< nodes.size(); q++)
      {
        VMesh::Node::index_type nodeq = nodes[q];

        ASSERT(nodeq<local_to_global.size())
        ASSERT(q<newnodes.size())

        if (local_to_global[nodeq] >= 0)
        {
          newnodes[q] = local_to_global[nodeq];
        }
        else
        {
          if (merge_nodes)
          {
            imesh->get_center(P,nodeq);

            if (match_node_values) ifield->get_value(curval,nodeq);

            // Convert to grid coordinates.
            index_type bi, bj, bk, ei, ej, ek;
            node_grid->unsafe_locate(bi, bj, bk, P);

            // Clamp to closest point on the grid.
            if (bi > ni) bi = ni;
            if (bi < 0) bi = 0;

            if (bj > nj) bj = nj;
            if (bj < 0) bj = 0;

            if (bk > nk) bk = nk;
            if (bk < 0) bk = 0;

            ei = bi; ej = bj; ek = bk;

            index_type cidx = -1;
            double dmin = tol2;
            bool found = true;

            do
            {
              found = true;
              for (index_type i = bi; i <= ei; i++)
              {
                if (i < 0 || i > ni) continue;
                for (index_type j = bj; j <= ej; j++)
                {
                if (j < 0 || j > nj) continue;
                  for (index_type k = bk; k <= ek; k++)
                  {
                    if (k < 0 || k > nk) continue;
                    if (i == bi || i == ei || j == bj || j == ej || k == bk || k == ek)
                    {
                      if (node_grid->min_distance_squared(P, i, j, k) < dmin)
                      {
                        found = false;
                        SearchGridT<index_type>::iterator it, eit;
                        node_grid->lookup_ijk(it, eit, i, j, k);

                        if (match_node_values)
                        {
                          while (it != eit)
                          {
                            if (values[*it] == curval)
                            {
                              Point point;
                              omesh->get_center(point,VMesh::Node::index_type(*it));
                              const double dist  = (P-point).length2();

                              if (dist < dmin)
                              {
                                cidx = *it;
                                dmin = dist;
                              }
                            }
                            ++it;
                          }
                        }
                        else
                        {
                          while (it != eit)
                          {
                            Point point;
                            omesh->get_center(point,VMesh::Node::index_type(*it));
                            const double dist  = (P-point).length2();

                            if (dist < dmin)
                            {
                              cidx = *it;
                              dmin = dist;
                            }
                            ++it;
                          }
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
            while (!found);

            if (cidx >= 0)
            {
              newnodes[q] = cidx;
              local_to_global[nodeq] = cidx;
            }
            else
            {
              index_type nidx = omesh->add_point(P);
              nodes_count++;
              newnodes[q] = nidx;
              local_to_global[nodeq] = nidx;
              node_grid->insert(nidx,P);
              if (match_node_values)
              {
                values[nidx] = curval;
              }
            }
          }
          else
          {
            Point P;
            imesh->get_center(P,nodeq);

            index_type nidx = omesh->add_point(P);
            nodes_count++;
            newnodes[q] = nidx;
            local_to_global[nodeq] = nidx;
          }
        }
      }

      if (merge_elems)
      {
        newnodes_sorted.resize(newnodes.size());
        for (size_t r=0; r<newnodes.size();r++)
        {
          newnodes_sorted[r] = newnodes[r];
        }
        std::sort(newnodes_sorted.begin(),newnodes_sorted.end());

        Point p(0.0,0.0,0.0);
        for (size_t r=0; r< newnodes_sorted.size(); r++)
        {
          Point q;
          omesh->get_center(q,VMesh::Node::index_type(newnodes_sorted[r]));
          p = (p + q).asPoint();
        }
        p = p*(1.0/newnodes_sorted.size());

        // Convert to grid coordinates.
        index_type i, j, k;
        elem_grid->unsafe_locate(i, j, k, p);

        SearchGridT<index_type>::iterator it, eit;
        elem_grid->lookup_ijk(it, eit, i, j, k);

        bool found = false;
        VMesh::Elem::index_type oelem = -1;
        while (it != eit)
        {
          omesh->get_nodes(nodes2,VMesh::Elem::index_type(*it));
          std::sort(nodes2.begin(),nodes2.end());

          // compare

          bool similar = true;
          for (size_t h=0;h<nodes2.size();h++)
          {
            if (nodes2[h] != newnodes_sorted[h]) { similar = false; break;}
          }

          if (similar) { found = true; oelem = VMesh::Elem::index_type(*it); break; }
          ++it;
        }

        if (!found)
        {
          oelem = omesh->add_elem(newnodes);
          elem_grid->insert(oelem,p);
          elems_count++;
          local_to_global_elem[idx] = oelem;
        }

      }
      else
      {
        omesh->add_elem(newnodes);
        elems_count++;
      }
    }

    if (ifield->num_values() > 0)
    {
      if (ofield->basis_order() == 0 && ifield->basis_order() == 0)
      {
        ofield->resize_values();
        if (merge_elems)
        {
          for (VMesh::Elem::index_type j=0;j<num_elems;j++)
          {
            if (local_to_global_elem[j] >= 0)
            {
              ofield->copy_value(ifield,j,local_to_global_elem[j]);
            }
          }
        }
        else
        {
          ofield->copy_values(ifield,0,elems_offset,num_elems);
        }
      }
      else if (ofield->basis_order() == 1 && ifield->basis_order() == 1)
      {
        ofield->resize_values();
        for (VMesh::Node::index_type j=0;j<num_nodes;j++)
        {
          if (local_to_global[j] >= 0)
          {
            ofield->copy_value(ifield,j,local_to_global[j]);
          }
        }
      }
    }

    elems_offset += elems_count;
    nodes_offset += nodes_count;

    update_progress_max(p+1, inputs.size());
  }


  return (true);
}

AlgorithmOutput JoinFieldsAlgo::run(const AlgorithmInput& input) const
{
  auto inputFields = input.getList<Field>(Variables::InputFields);

  FieldHandle outputField;
  if (!runImpl(inputFields, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;
  return output;
}
