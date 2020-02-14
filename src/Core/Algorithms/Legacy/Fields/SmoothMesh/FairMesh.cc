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


#include <Core/Algorithms/Legacy/Fields/SmoothMesh/FairMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

ALGORITHM_PARAMETER_DEF(Fields, FairMeshMethod);
ALGORITHM_PARAMETER_DEF(Fields, NumIterations);
ALGORITHM_PARAMETER_DEF(Fields, Lambda);
ALGORITHM_PARAMETER_DEF(Fields, FilterCutoff);

FairMeshAlgo::FairMeshAlgo()
{
  addOption(Parameters::FairMeshMethod,"fast","fast|desbrun");
  addParameter(Parameters::NumIterations,50);
  addParameter(Parameters::Lambda,0.6307);
  addParameter(Parameters::FilterCutoff,0.1);
}

bool FairMeshAlgo::runImpl(FieldHandle input,FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "fairmesh");

  if (!input)
  {
    error("No input field");
    return (false);
  }

  FieldInformation fi(input);
  if (!fi.is_surface())
  {
    error("This algorithm only works on a surface mesh");
    return (false);
  }

  if (fi.is_imagemesh())
  {
    warning("An image mesh is byt default smooth, skipping mesh smoothing");
    output = input;
    return (true);
  }

  std::string method = getOption(Parameters::FairMeshMethod);
  int num_iter = 2*get(Parameters::NumIterations).toInt();
  double lambda = get(Parameters::Lambda).toDouble();
  double filter_cutoff = get(Parameters::FilterCutoff).toDouble();

  double mu = 1.0/(filter_cutoff - 1.0/lambda);

  output.reset(input->deep_clone());

  VMesh* mesh = output->vmesh();
  VMesh::size_type num_nodes = mesh->num_nodes();
  mesh->unsynchronize(Mesh::NORMALS_E);

  if (method == "fast")
  {
    // Fast neighborhoods
    std::vector<VMesh::Node::array_type > neighborhoods(num_nodes);
    mesh->synchronize(Mesh::NODE_NEIGHBORS_E);

    for (VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
    {
      mesh->get_neighbors(neighborhoods[idx],idx);
    }

    std::vector<Vector> disp(num_nodes);
    Point*  point = mesh->get_points_pointer();
    Point p0;
    for (int it = 0; it<num_iter; it++)
    {
      for (VMesh::index_type idx=0; idx<num_nodes;idx++)
      {
        p0 = point[idx];
        Vector d(0.0,0.0,0.0);
        VMesh::Node::array_type &neighbors = neighborhoods[idx];
        double w = 1.0/(neighbors.size());
        for (size_t j=0; j<neighbors.size(); j++)
        {
          d += w* (point[neighbors[j]]-p0);
        }
        disp[idx] = d;
      }

      if (it % 2 == 0)
      {
        for (VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
          point[idx] = point[idx]+ lambda*disp[idx];
      }
      else
      {
        for (VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
          point[idx] = point[idx]+  mu*disp[idx];
      }
      update_progress_max(it,num_iter);
    }
  }
  else
  {
    // desbrun method
    std::vector<std::vector<std::pair<VMesh::index_type,VMesh::index_type> > > neighborhoods(num_nodes);
    mesh->synchronize(Mesh::NODE_NEIGHBORS_E|Mesh::EPSILON_E);

    VMesh::Elem::array_type elems;
    VMesh::Node::array_type nodes;

    for (VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
    {
      std::vector<std::pair<VMesh::index_type,VMesh::index_type> > &neighborhood = neighborhoods[idx];
      mesh->get_elems(elems,idx);
      for (size_t j = 0; j<elems.size(); j++)
      {
        mesh->get_nodes(nodes,elems[j]);
        // make it circular
        nodes.push_back(nodes[0]);

        for (size_t k=1;k < nodes.size();k++)
        {
          // get all edges that are not connected to the node itself
          if(nodes[k-1] != idx && nodes[k] != idx)
          {
            neighborhood.push_back(std::pair<VMesh::index_type,VMesh::index_type>(nodes[k-1],nodes[k]));
          }
        }
      }
    }

    std::vector<Vector> disp(num_nodes);
    Point*  point = mesh->get_points_pointer();
    Point p0;

    double epsilon = mesh->get_epsilon();

    for (int it = 0; it<num_iter; it++)
    {

      for (VMesh::index_type idx=0; idx<num_nodes;idx++)
      {
        // Center location of this node
        p0 = point[idx];
        Vector d(0.0,0.0,0.0);

        // neighborhood points
        Point p1, p2, p3;
        Vector p12;

        // get local neighborhood pairs
        const std::vector<std::pair<VMesh::index_type,VMesh::index_type> > &neighborhood = neighborhoods[idx];

        // if no neighborhood continue
        if (neighborhood.empty()) continue;

        // total weight
        double totw = 0.0;

        for (size_t j = 0; j < neighborhood.size(); j++)
        {
          p1 = point[neighborhood[j].first];
          p2 = point[neighborhood[j].second];

          // vectors pointing to the two neighbor nodes
          Vector e1 = p2-p0;
          Vector e2 = p1-p0;

          // Get vector between neighbors
          p12 = p1-p2;

          // Squared distance between neighbors
          double e = Dot(p12,p12);

          if (e > 0.0)
          {
            double dot = Dot(p1-p0,p12)/e;
            p3 = p1 - dot*p12;

            double A = (p1-p3).length();
            double B = (p0-p3).length();
            double C = (p2-p3).length();

            // if B approaches zero, we have a flat
            // triangle, hence we need to bounce back the node
            // towards the other side. Hence ignoring these
            // directions
            if (B >= 10*epsilon)
            {
              if (dot < 0.0) A = -A;
              if (dot > 1.0) C = -C;
              totw += (A+C)/B;

              d += (A/B)*e1 + (C/B)*e2;
            }
          }
        }

        /// set the displacement vector for this node.
        if (totw != 0.0) disp[idx] = d * (1.0 / totw);
      }

      if (it % 2 == 0)
      {
        for (VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
          point[idx] = point[idx]+ lambda*disp[idx];
      }
      else
      {
        for (VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
          point[idx] = point[idx]+  mu*disp[idx];
      }
      update_progress_max(it,num_iter);
    }
  }

  return (true);
}

const AlgorithmInputName FairMeshAlgo::Input_Mesh("Input_Mesh");
const AlgorithmOutputName FairMeshAlgo::Faired_Mesh("Faired_Mesh");

AlgorithmOutput FairMeshAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Input_Mesh);

  FieldHandle outputField;
  if (!runImpl(field, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Faired_Mesh] = outputField;
  return output;
}
