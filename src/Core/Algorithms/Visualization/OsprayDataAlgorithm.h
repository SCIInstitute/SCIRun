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


#ifndef CORE_ALGORITHMS_VISUALIZATION_OSPRAYDATAALGORITHM_H
#define CORE_ALGORITHMS_VISUALIZATION_OSPRAYDATAALGORITHM_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/Geometry.h>
#include <boost/graph/adjacency_list.hpp>
#include <Core/Algorithms/Visualization/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace OsprayVisualization
      {
        ALGORITHM_PARAMETER_DECL(DefaultColorR);
        ALGORITHM_PARAMETER_DECL(DefaultColorG);
        ALGORITHM_PARAMETER_DECL(DefaultColorB);
        ALGORITHM_PARAMETER_DECL(DefaultColorA);
        ALGORITHM_PARAMETER_DECL(Radius);
        ALGORITHM_PARAMETER_DECL(UseNormals);
        ALGORITHM_PARAMETER_DECL(ShowEdges);
        ALGORITHM_PARAMETER_DECL(ModuleID);
      }

      namespace Visualization
      {
        typedef std::pair<int,int> Edge;
        typedef std::vector<Edge> EdgeVector;
        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> DirectedGraph;
        typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, boost::no_property, boost::property<boost::edge_color_t, boost::default_color_type> > UndirectedGraph;
        typedef boost::graph_traits<DirectedGraph>::vertex_descriptor Vertex;
        typedef boost::graph_traits<UndirectedGraph>::vertex_descriptor Vertex_u;
        typedef std::map<int, int> ComponentMap;

        class SCISHARE OsprayDataAlgorithm : public AlgorithmBase
        {
        public:
          OsprayDataAlgorithm();
          virtual AlgorithmOutput run(const AlgorithmInput& input) const override;
        private:
          Core::Datatypes::OsprayGeometryObjectHandle addStreamline(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;
          Core::Datatypes::OsprayGeometryObjectHandle addSphere(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;
          Core::Datatypes::OsprayGeometryObjectHandle addTriSurface(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;
          Core::Datatypes::OsprayGeometryObjectHandle addQuadSurface(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;
          Core::Datatypes::OsprayGeometryObjectHandle addStructVol(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;
          Core::Datatypes::OsprayGeometryObjectHandle addUnstructVol(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;
          Core::Datatypes::OsprayGeometryObjectHandle addCylinder(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;
          void connected_component_edges(EdgeVector all_edges, std::vector<EdgeVector>& subsets, std::vector<int>& size_regions)const;
          void ReorderNodes(std::vector<uint32_t> index, std::vector<uint32_t> cc_index, std::vector<float> vertex, std::vector<float> color, std::vector<uint32_t>& index_new, std::vector<float>& vertex_new,std::vector<float>& color_new) const;
          std::list<Vertex_u> sort_cc(EdgeVector sub_edges) const;
          bool FindPath(UndirectedGraph& graph, Vertex_u& curr_v, std::list<Vertex_u>& v_path, bool front) const;
          std::vector<uint32_t> sort_points(EdgeVector edges, std::vector<uint32_t>& cc_index) const;
          Core::Datatypes::OsprayGeometryObjectHandle fillDataBuffers(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;
          Core::Datatypes::OsprayGeometryObjectHandle makeObject(FieldHandle field) const;
        };
      }
    }
  }
}

#endif
