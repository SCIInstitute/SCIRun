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

#ifndef CORE_ALGORITHMS_VISUALIZATION_VTKGEOMETRYBUILDER_H
#define CORE_ALGORITHMS_VISUALIZATION_VTKGEOMETRYBUILDER_H

#include <Core/Datatypes/VTK/VtkGeometry.h>
#include <boost/graph/adjacency_list.hpp>
#include <Core/Algorithms/Visualization/share.h>
#include <array>
#include <functional>

namespace SCIRun
{
  namespace Core
  {
  namespace Algorithms {
      namespace Visualization {
        class VtkDataAlgorithm;
        typedef std::pair<int, int> Edge;
        typedef std::vector<Edge> EdgeVector;
        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> DirectedGraph;
        typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, boost::no_property, boost::property<boost::edge_color_t, boost::default_color_type> > UndirectedGraph;
        typedef boost::graph_traits<DirectedGraph>::vertex_descriptor Vertex;
        typedef boost::graph_traits<UndirectedGraph>::vertex_descriptor Vertex_u;
        typedef std::map<int, int> ComponentMap;

        struct QuadFaceKey
        {
          std::array<vtkIdType, 4> ids;

          bool operator==(const QuadFaceKey& other) const { return ids == other.ids; }
        };

        struct TriFaceKey
        {
          std::array<vtkIdType, 3> ids;

          bool operator==(const TriFaceKey& other) const { return ids == other.ids; }
        };

        struct QuadFaceKeyHash
        {
          size_t operator()(const QuadFaceKey& k) const
          {
            size_t h = 0;
            for (auto id : k.ids)
            {
              h ^= std::hash<vtkIdType>{}(id) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return h;
          }
        };

        struct TriFaceKeyHash
        {
          size_t operator()(const TriFaceKey& k) const
          {
            size_t h = 0;
            for (auto id : k.ids)
            {
              h ^= std::hash<vtkIdType>{}(id) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return h;
          }
        };

        class SCISHARE VtkGeometryBuilder
        {
         public:
          explicit VtkGeometryBuilder(const VtkDataAlgorithm& algorithm);

          Core::Datatypes::VtkGeometryObjectHandle buildGeometryObject(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap);

         private:
          VtkGeometryBuilder& add(Core::Datatypes::VtkGeometryObjectHandle obj);

           Core::Datatypes::VtkGeometryObjectHandle finalize();

           // Existing VtkDataAlgorithm methods
           Core::Datatypes::VtkGeometryObjectHandle addStreamline(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;

           Core::Datatypes::VtkGeometryObjectHandle addSphere(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;

           Core::Datatypes::VtkGeometryObjectHandle addTriSurface(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;

           Core::Datatypes::VtkGeometryObjectHandle addQuadSurface(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;

           Core::Datatypes::VtkGeometryObjectHandle addStructVol(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;

           Core::Datatypes::VtkGeometryObjectHandle addUnstructVol(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;

           Core::Datatypes::VtkGeometryObjectHandle addCylinder(FieldHandle field, Core::Datatypes::ColorMapHandle colorMap) const;

           Core::Datatypes::VtkGeometryObjectHandle makeObject(FieldHandle field) const;

#ifdef WITH_VTK
           vtkSmartPointer<vtkUnstructuredGrid> buildVolumeGrid(FieldHandle field) const;

           vtkSmartPointer<vtkPolyData> buildVolumeFaces(FieldHandle field) const;

           vtkSmartPointer<vtkPolyData> buildVolumeSurface(FieldHandle field) const;

           vtkSmartPointer<vtkImageData> buildImageVolume(FieldHandle field) const;
#endif

           std::array<double, 2> computeScalarRange(FieldHandle field) const;

           void connected_component_edges(EdgeVector all_edges, std::vector<EdgeVector>& subsets, std::vector<int>& size_regions) const;

           std::list<Vertex_u> sort_cc(EdgeVector sub_edges) const;

           bool FindPath(UndirectedGraph& graph, Vertex_u& curr_v, std::list<Vertex_u>& v_path, bool front) const;

         private:
          const VtkDataAlgorithm& algorithm_;
          std::vector<Core::Datatypes::VtkGeometryObjectHandle> objects_;
        };
      }
    }
  }
}

#endif