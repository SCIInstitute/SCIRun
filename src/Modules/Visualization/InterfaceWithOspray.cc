/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
University of Utah.

License for the specific language governing rights and limitations under
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

#include <Modules/Visualization/InterfaceWithOspray.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/utility.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/cstdlib.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/lambda/lambda.hpp>


#ifdef WITH_OSPRAY
#include <ospray/ospray.h>
#endif

using namespace SCIRun;
using namespace Dataflow::Networks;
using namespace Modules::Visualization;
using namespace Core;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Visualization;
using namespace Datatypes;

ALGORITHM_PARAMETER_DEF(Visualization, ImageHeight);
ALGORITHM_PARAMETER_DEF(Visualization, ImageWidth);
ALGORITHM_PARAMETER_DEF(Visualization, CameraPositionX);
ALGORITHM_PARAMETER_DEF(Visualization, CameraPositionY);
ALGORITHM_PARAMETER_DEF(Visualization, CameraPositionZ);
ALGORITHM_PARAMETER_DEF(Visualization, CameraUpX);
ALGORITHM_PARAMETER_DEF(Visualization, CameraUpY);
ALGORITHM_PARAMETER_DEF(Visualization, CameraUpZ);
ALGORITHM_PARAMETER_DEF(Visualization, CameraViewX);
ALGORITHM_PARAMETER_DEF(Visualization, CameraViewY);
ALGORITHM_PARAMETER_DEF(Visualization, CameraViewZ);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorR);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorG);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorB);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorA);
ALGORITHM_PARAMETER_DEF(Visualization, BackgroundColorR);
ALGORITHM_PARAMETER_DEF(Visualization, BackgroundColorG);
ALGORITHM_PARAMETER_DEF(Visualization, BackgroundColorB);
ALGORITHM_PARAMETER_DEF(Visualization, FrameCount);
ALGORITHM_PARAMETER_DEF(Visualization, ShowImageInWindow);
ALGORITHM_PARAMETER_DEF(Visualization, LightColorR);
ALGORITHM_PARAMETER_DEF(Visualization, LightColorG);
ALGORITHM_PARAMETER_DEF(Visualization, LightColorB);
ALGORITHM_PARAMETER_DEF(Visualization, LightIntensity);
ALGORITHM_PARAMETER_DEF(Visualization, LightVisible);
ALGORITHM_PARAMETER_DEF(Visualization, LightType);
ALGORITHM_PARAMETER_DEF(Visualization, RendererType);
ALGORITHM_PARAMETER_DEF(Visualization, AutoCameraView);
ALGORITHM_PARAMETER_DEF(Visualization, StreamlineRadius);
ALGORITHM_PARAMETER_DEF(Visualization, SphereRadius);
ALGORITHM_PARAMETER_DEF(Visualization, OneSidedLighting);
ALGORITHM_PARAMETER_DEF(Visualization, ShadowsEnabled);
ALGORITHM_PARAMETER_DEF(Visualization, UseNormals);
ALGORITHM_PARAMETER_DEF(Visualization, LightPositionX);
ALGORITHM_PARAMETER_DEF(Visualization, LightPositionY);
ALGORITHM_PARAMETER_DEF(Visualization, LightPositionZ);
ALGORITHM_PARAMETER_DEF(Visualization, LightDirectionX);
ALGORITHM_PARAMETER_DEF(Visualization, LightDirectionY);
ALGORITHM_PARAMETER_DEF(Visualization, LightDirectionZ);
ALGORITHM_PARAMETER_DEF(Visualization, LightAngle);
ALGORITHM_PARAMETER_DEF(Visualization, LightRadius);
ALGORITHM_PARAMETER_DEF(Visualization, LightLockCamera);

MODULE_INFO_DEF(InterfaceWithOspray, Visualization, SCIRun)

void InterfaceWithOspray::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::ImageHeight, 768);
  state->setValue(Parameters::ImageWidth, 1024);
  state->setValue(Parameters::CameraPositionX, 5.0);
  state->setValue(Parameters::CameraPositionY, 5.0);
  state->setValue(Parameters::CameraPositionZ, 5.0);
  state->setValue(Parameters::CameraUpX, 0.0);
  state->setValue(Parameters::CameraUpY, 0.0);
  state->setValue(Parameters::CameraUpZ, 1.0);
  state->setValue(Parameters::CameraViewX, 0.0);
  state->setValue(Parameters::CameraViewY, 0.0);
  state->setValue(Parameters::CameraViewZ, 0.0);
  state->setValue(Parameters::DefaultColorR, 0.5);
  state->setValue(Parameters::DefaultColorG, 0.5);
  state->setValue(Parameters::DefaultColorB, 0.5);
  state->setValue(Parameters::DefaultColorA, 1.0);
  state->setValue(Parameters::BackgroundColorR, 0.0);
  state->setValue(Parameters::BackgroundColorG, 0.0);
  state->setValue(Parameters::BackgroundColorB, 0.0);
  state->setValue(Parameters::FrameCount, 10);
  state->setValue(Parameters::ShowImageInWindow, true);
  state->setValue(Parameters::LightColorR, 1.0);
  state->setValue(Parameters::LightColorG, 1.0);
  state->setValue(Parameters::LightColorB, 1.0);
  state->setValue(Parameters::LightIntensity, 50.0);
  state->setValue(Parameters::LightVisible, false);
  state->setValue(Parameters::LightLockCamera, false);
  state->setValue(Parameters::LightType, std::string("distant"));
  state->setValue(Parameters::RendererType, std::string("scivis"));
  state->setValue(Parameters::AutoCameraView, true);
  state->setValue(Parameters::OneSidedLighting, false);
  state->setValue(Parameters::ShadowsEnabled, true);
  state->setValue(Parameters::UseNormals, false);
  state->setValue(Parameters::StreamlineRadius, 0.1);
  state->setValue(Parameters::SphereRadius, 0.3);
  state->setValue(Variables::Filename, std::string(""));
  state->setValue(Parameters::LightPositionX, 0.0);
  state->setValue(Parameters::LightPositionY, 0.0);
  state->setValue(Parameters::LightPositionZ, 500.0);
  state->setValue(Parameters::LightDirectionX, 0.0);
  state->setValue(Parameters::LightDirectionY, 0.0);
  state->setValue(Parameters::LightDirectionZ, -1);
  state->setValue(Parameters::LightAngle, 60);
  state->setValue(Parameters::LightRadius, 1.0);
}

namespace detail
{
  #ifdef WITH_OSPRAY
  class OsprayImpl
  {
  private:
    static bool initialized_;
    static Core::Thread::Mutex lock_;
    static void initialize()
    {
      if (!initialized_)
      {
        const char* argv[] = { "" };
        int argc = 0;
        int init_error = ospInit(&argc, argv);
        if (init_error != OSP_NO_ERROR)
          throw init_error;
        initialized_ = true;
      }
    }

    Core::Thread::Guard guard_;
    Core::Geometry::BBox imageBox_;
    ModuleStateHandle state_;
    osp::vec2i imgSize_;
    OSPCamera camera_;
    OSPModel world_;
    std::vector<OSPGeometry> meshes_;
    OSPRenderer renderer_;
    OSPFrameBuffer framebuffer_;

    float toFloat(const Name& name) const
    {
      return static_cast<float>(state_->getValue(name).toDouble());
    }

    std::array<float,3> toArray(const Vector& v) const
    {
      return { static_cast<float>(v.x()), static_cast<float>(v.y()), static_cast<float>(v.z()) };
    }

    struct FieldData
    {
      std::vector<float> vertex, color, vertex_normal;
      std::vector<int32_t> index;
    };
    
    std::vector<FieldData> fieldData_;
    
    
    


  public:
    explicit OsprayImpl(ModuleStateHandle state) : guard_(lock_.get()), state_(state)
    {
      initialize();
    }

    void setup()
    {
      imgSize_.x = state_->getValue(Parameters::ImageWidth).toInt();
      imgSize_.y = state_->getValue(Parameters::ImageHeight).toInt();

      // camera
      float cam_pos[] = { toFloat(Parameters::CameraPositionX), toFloat(Parameters::CameraPositionY), toFloat(Parameters::CameraPositionZ) };
      float cam_up[] = { toFloat(Parameters::CameraUpX), toFloat(Parameters::CameraUpY), toFloat(Parameters::CameraUpZ) };
      float cam_view[] = { toFloat(Parameters::CameraViewX), toFloat(Parameters::CameraViewY), toFloat(Parameters::CameraViewZ) };

      // create and setup camera
      camera_ = ospNewCamera("perspective");
      ospSetf(camera_, "aspect", imgSize_.x / (float)imgSize_.y);
      ospSet3fv(camera_, "pos", cam_pos);
      ospSet3fv(camera_, "dir", cam_view);
      ospSet3fv(camera_, "up", cam_up);
      ospCommit(camera_); // commit each object to indicate modifications are done

      world_ = ospNewModel();
      ospCommit(world_);
    }

    void adjustCameraPosition(FieldHandle field)
    {
      if (state_->getValue(Parameters::AutoCameraView).toBool())
      {
        auto vmesh = field->vmesh();
        auto bbox = vmesh->get_bounding_box();
        imageBox_.extend(bbox);
        auto center = imageBox_.center();
        float position[] = { toFloat(Parameters::CameraPositionX), toFloat(Parameters::CameraPositionY), toFloat(Parameters::CameraPositionZ) };
        float cam_up[] = { toFloat(Parameters::CameraUpX), toFloat(Parameters::CameraUpY), toFloat(Parameters::CameraUpZ) };
        float newDir[] = { static_cast<float>(center.x()) - position[0],
           static_cast<float>(center.y()) - position[1],
           static_cast<float>(center.z()) - position[2]};

        state_->setValue(Parameters::CameraViewX, center.x());
        state_->setValue(Parameters::CameraViewY, center.y());
        state_->setValue(Parameters::CameraViewZ, center.z());
        ospSet3fv(camera_, "dir", newDir);
        auto newUp = getCameraUp(newDir, cam_up);
        state_->setValue(Parameters::CameraUpX, newUp.x());
        state_->setValue(Parameters::CameraUpY, newUp.y());
        state_->setValue(Parameters::CameraUpZ, newUp.z());
        ospSet3fv(camera_, "up", toArray(newUp).begin());
        ospCommit(camera_);
      }
    }

    Vector getCameraUp(float* newDir, float* cam_up)
    {
      Vector side(newDir[1]*cam_up[2] - newDir[2]*cam_up[1],
        newDir[2]*cam_up[0] - newDir[0]*cam_up[2],
        newDir[0]*cam_up[1] - newDir[1]*cam_up[0]);
      auto norm_side = side.length();
      if (norm_side <= 1e-3)
      {
        side = Vector(newDir[1], -newDir[0], 0.0);
        norm_side = side.length();
        if (norm_side <= 1e-3)
        {
          side = Vector(-newDir[2], 0.0, newDir[0]);
          norm_side = side.length();
        }
      }
      side /= norm_side;

      return Vector(side[1]*newDir[2] - side[2]*newDir[1],
        side[2]*newDir[0] - side[0]*newDir[2],
        side[0]*newDir[1] - side[1]*newDir[0]);
    }

    void fillDataBuffers(FieldHandle field, ColorMapHandle colorMap)
    {
      auto facade(field->mesh()->getFacade());

      fieldData_.push_back({});
      auto& fieldData = fieldData_.back();
      auto& vertex = fieldData.vertex;
      auto& color = fieldData.color;

      auto vfield = field->vfield();
      
      

      {
        double value;
        ColorRGB nodeColor(state_->getValue(Parameters::DefaultColorR).toDouble(), state_->getValue(Parameters::DefaultColorG).toDouble(), state_->getValue(Parameters::DefaultColorB).toDouble());
        auto alpha = toFloat(Parameters::DefaultColorA);

        for (const auto& node : facade->nodes())
        {
          auto point = node.point();
          vertex.push_back(static_cast<float>(point.x()));
          vertex.push_back(static_cast<float>(point.y()));
          vertex.push_back(static_cast<float>(point.z()));
          vertex.push_back(0);
          
//          auto edges = node.edgeIndices();
//          std::cout << "Node " << node.index() << " point=" << node.point().get_string() << " edges=[" << edges << "]" << std::endl;
//          std::cout << "Node " << node.index() << " point=" << node.point().get_string() << std::endl;

          vfield->get_value(value, node.index());
          if (colorMap)
          {
            nodeColor = colorMap->valueToColor(value);
          }
          color.push_back(static_cast<float>(nodeColor.r()));
          color.push_back(static_cast<float>(nodeColor.g()));
          color.push_back(static_cast<float>(nodeColor.b()));
          color.push_back(alpha);
        }
      }

      auto& index = fieldData.index;
      {
        for (const auto& face : facade->faces())
        {
          auto nodes = face.nodeIndices();
          index.push_back(static_cast<int32_t>(nodes[0]));
          index.push_back(static_cast<int32_t>(nodes[1]));
          index.push_back(static_cast<int32_t>(nodes[2]));
        }
      }
      
      FieldInformation info(field);
      if (state_->getValue(Parameters::UseNormals).toBool() && info.is_trisurfmesh())
      {
        auto& vertex_normal = fieldData.vertex_normal;
        {
          Vector norm;
          
          for (const auto& node : facade->nodes())
          {
            field->vmesh()->get_normal(norm,node.index());
            
            vertex_normal.push_back(static_cast<float>(norm.x()));
            vertex_normal.push_back(static_cast<float>(norm.y()));
            vertex_normal.push_back(static_cast<float>(norm.z()));
            vertex_normal.push_back(0);
          }
        }
      }
    }

    void addField(FieldHandle field, ColorMapHandle colorMap)
    {
      adjustCameraPosition(field);

      fillDataBuffers(field, colorMap);

      const auto& fieldData = fieldData_.back();
      const auto& vertex = fieldData.vertex;
      
      const auto& color = fieldData.color;
      const auto& index = fieldData.index;

      // create and setup model and mesh
      OSPGeometry mesh = ospNewGeometry("triangles");
      OSPData data = ospNewData(vertex.size() / 4, OSP_FLOAT3A, &vertex[0]); // OSP_FLOAT3 format is also supported for vertex positions
      ospCommit(data);
      ospSetData(mesh, "vertex", data);
      
      FieldInformation info(field);
      if (state_->getValue(Parameters::UseNormals).toBool() && info.is_trisurfmesh())
      {
        const auto& vertex_normal = fieldData.vertex_normal;
        data = ospNewData(vertex_normal.size() / 4, OSP_FLOAT3A, &vertex_normal[0]);
        ospCommit(data);
        ospSetData(mesh, "vertex.normal", data);
      }
      
      data = ospNewData(color.size() / 4, OSP_FLOAT4, &color[0]);
      ospCommit(data);
      ospSetData(mesh, "vertex.color", data);
      data = ospNewData(index.size() / 3, OSP_INT3, &index[0]); // OSP_INT4 format is also supported for triangle indices
      ospCommit(data);
      ospSetData(mesh, "index", data);
      ospCommit(mesh);

      meshes_.push_back(mesh);
      ospAddGeometry(world_, mesh);
      ospCommit(world_);
    }
    
    void addSpheres(FieldHandle field, ColorMapHandle colorMap)
    {
      adjustCameraPosition(field);
      
      fillDataBuffers(field, colorMap);
      
      const auto& fieldData = fieldData_.back();
      const auto& vertex = fieldData.vertex;
      const auto& color = fieldData.color;
      
      OSPGeometry spheres = ospNewGeometry("spheres");
      OSPData data = ospNewData(vertex.size() / 4, OSP_FLOAT3A, &vertex[0]);
      ospCommit(data);
      ospSetData(spheres, "spheres", data);
      
      data = ospNewData(color.size() / 4, OSP_FLOAT4, &color[0]);
      ospCommit(data);
      ospSetData(spheres, "color", data);
      
      ospSet1f(spheres, "radius", toFloat(Parameters::StreamlineRadius)*1.3);
      
      ospCommit(spheres);
      
      meshes_.push_back(spheres);
      ospAddGeometry(world_, spheres);
      ospCommit(world_);
      
    }
    

    void addStreamline(FieldHandle field, ColorMapHandle colorMap)
    {
      adjustCameraPosition(field);

      fillDataBuffers(field, colorMap);

      auto& fieldData = fieldData_.back();
      const auto& vertex = fieldData.vertex;
      const auto& color = fieldData.color;

      EdgeVector all_edges;
      std::list<Vertex> order, order_test;
      
//      std::vector<int32_t> check_edges(field->mesh()->getFacade()->numEdges(),0);
//       std::list<int32_t> index_list;
      //bool connected = false;
      
      auto& v_index = fieldData.index;
      {
        auto facade(field->mesh()->getFacade());
        for (const auto& edge : facade->edges())
        {
        
          //check_edges[edge.index()]=1;
          auto nodesFromEdge = edge.nodeIndices();
          v_index.push_back(nodesFromEdge[0]);
          auto nodePoints = edge.nodePoints();
          
//          std::cout << "Edge " << edge.index() << " nodes=[" << nodesFromEdge[0] << " point=" << nodePoints[0].get_string()
//          << ", " << nodesFromEdge[1] << " point=" << nodePoints[1].get_string() << "]" << std::endl;
//
          all_edges.push_back(std::make_pair(nodesFromEdge[0],nodesFromEdge[1]));

        }
      }
      
      
      std::vector<int32_t> cc_index;
      std::vector<int32_t> index = sort_points(all_edges,cc_index);
//      std::cout<<"cc_index = "<<cc_index<<std::endl;
      
      std::vector<float> vertex_new, color_new;
      std::vector<int32_t> index_new;
      
//      std::cout<<"index size ="<<index.size()<<std::endl;
//      std::cout<<"index = [";
//      for (auto i : index)
//      {
//        std::cout<<" "<<i;
//      }
//      std::cout<<" ]"<<std::endl;
      
      ReorderNodes(index, cc_index, vertex, color, index_new, vertex_new, color_new);
      
//      std::cout<<"index_new size ="<<index_new.size()<<std::endl;
//      std::cout<<"index_new = [";
//      for (auto i : index_new)
//      {
//        std::cout<<" "<<i;
//      }
//      std::cout<<" ]"<<std::endl;
//      
      
      
//      std::vector<Vertex_u> source_vertex_tot;
//      detect_loops vis1(source_vertex_tot);
//      std::vector<boost::default_color_type> vertex_color( boost::num_vertices(graph) );
//      auto idmap = boost::get( boost::vertex_index, graph );
//      auto vcmap = make_iterator_property_map( vertex_color.begin(), idmap );
//      std::map<typename UndirectedGraph::edge_descriptor, boost::default_color_type> edge_color;
//      auto ecmap = boost::make_assoc_property_map( edge_color );
//
//      boost::undirected_dfs(graph,vis1,vcmap,ecmap);
//      std::cout << std::endl;
//
//      std::cout<<"total points "<<vis1.LoopDetected()<<std::endl;
//      std::cout<<"loop detected? "<<vis1.LoopDetected()<<std::endl;
//      std::cout<<"source_vertex = " << source_vertex_tot<<std::endl;
//
      
      

      OSPGeometry streamlines = ospNewGeometry("streamlines");
      OSPData data = ospNewData(vertex_new.size() / 4, OSP_FLOAT3A, &vertex_new[0]);
      ospCommit(data);
      ospSetData(streamlines, "vertex", data);

      data = ospNewData(color_new.size() / 4, OSP_FLOAT4, &color_new[0]);
      ospCommit(data);
      ospSetData(streamlines, "vertex.color", data);

      data = ospNewData(index_new.size(), OSP_INT, &index_new[0]);
      ospCommit(data);
      ospSetData(streamlines, "index", data);

      ospSet1f(streamlines, "radius", toFloat(Parameters::StreamlineRadius));

      ospCommit(streamlines);

      meshes_.push_back(streamlines);
      ospAddGeometry(world_, streamlines);
      ospCommit(world_);
    }
    
    void ReorderNodes(std::vector<int32_t> index, std::vector<int32_t> cc_index, std::vector<float> vertex, std::vector<float> color, std::vector<int32_t>& index_new, std::vector<float>& vertex_new,std::vector<float>& color_new)
    {
//      std::cout<<"Reordering nodes"<<std::endl;
      
      int cc_cnt = 0;
      for (size_t k=0;k<index.size();k++)
      {
        if (k!=cc_index[cc_cnt])
        {
          index_new.push_back(k);
        }
        else
        {
//          std::cout<<"end point"<<std::endl;
          cc_cnt++;
        }
        
//        std::cout<<k<<std::endl;
//        std::cout<<"vertex = [ "<<vertex[k*4]<<" , "<<vertex[k*4+1]<<" , "<<vertex[k*4+2]<<" ]"<<std::endl;
//        std::cout<<"color = [ "<<color[k*4]<<" , "<<color[k*4+1]<<" , "<<color[k*4+2]<<" , "<<color[k*4+3]<<" ]"<<std::endl;
        
        vertex_new.push_back(vertex[index[k]*4]);
        vertex_new.push_back(vertex[index[k]*4+1]);
        vertex_new.push_back(vertex[index[k]*4+2]);
        vertex_new.push_back(0);
        color_new.push_back(color[index[k]*4]);
        color_new.push_back(color[index[k]*4+1]);
        color_new.push_back(color[index[k]*4+2]);
        color_new.push_back(color[index[k]*4+3]);
      }
    }
    
    void connected_component_edges(EdgeVector all_edges, std::vector<EdgeVector>& subsets, std::vector<int>& size_regions)
    {
      UndirectedGraph graph = UndirectedGraph(all_edges.begin(), all_edges.end(), all_edges.size());
      std::vector<int> component(boost::num_vertices(graph));
      boost::connected_components(graph, &component[0]);
//      std::cout<<"conn comp ="<<component<<std::endl;
      
      int max_comp=0;
      for (size_t i = 0; i < component.size(); ++i) if (component[i]>max_comp) max_comp = component[i];
      size_regions.clear();
      size_regions.resize(max_comp+1,0);
      for (size_t i = 0; i < component.size(); ++i) size_regions[component[i]]++;
      
//      std::cout<<"num of cc = "<<max_comp+1<<std::endl;
//      std::cout<<"size of ccs = "<<size_regions<<std::endl;
      
      subsets.clear();
      subsets.resize(max_comp+1);
      boost::graph_traits<UndirectedGraph>::edge_iterator ei, ei_end;
      for (tie(ei,ei_end)= edges(graph); ei != ei_end; ++ei)
      {
        //        std::cout <<"edge["<<*ei<< "]=(" << source(*ei, graph)
        //        << "," << target(*ei, graph) << ") ";
        subsets[component[source(*ei, graph)]].push_back(std::make_pair(source(*ei, graph),target(*ei, graph)));
        
      }
//      std::cout<<"Subsets created.  Size = "<<subsets.size()<<std::endl;
      //      std::cout << std::endl;
      
      
    }
    
    std::list<Vertex_u> sort_cc(EdgeVector sub_edges,Vertex_u vend)
    {
      UndirectedGraph graph = UndirectedGraph(sub_edges.begin(), sub_edges.end(), sub_edges.size());
//      std::cout << "back edges:\n";
      std::vector<Vertex_u> source_vertex;
      
      detect_loops vis(source_vertex);
      std::vector<boost::default_color_type> vertex_color( boost::num_vertices(graph) );
      auto idmap = boost::get( boost::vertex_index, graph );
      auto vcmap = make_iterator_property_map( vertex_color.begin(), idmap );
      std::map<typename UndirectedGraph::edge_descriptor, boost::default_color_type> edge_color;
      auto ecmap = boost::make_assoc_property_map( edge_color );
      boost::undirected_dfs(graph,vis,vcmap,ecmap);
//      std::cout << std::endl;
      
//      std::cout<<"loop detected? "<<vis.LoopDetected()<<std::endl;
//      std::cout<<"source_vertex = " << source_vertex<<std::endl;
      
      
      std::list<Vertex_u> v_path;
      v_path.push_back(vend);
      
      FindPath(graph,vend,v_path);
      
      if (vis.LoopDetected())
      {
        v_path.push_back(v_path.front());
      }
      
      
      return v_path;
      
    }
    
    bool FindPath(UndirectedGraph& graph, Vertex_u& curr_v, std::list<Vertex_u>& v_path)
    {
      typename boost::graph_traits<UndirectedGraph>::out_edge_iterator ei, ei_end;
      size_t edge_idx = 0;
      int cnt = 0;
      for ( tie(ei, ei_end)=out_edges( curr_v, graph ); ei != ei_end; ++ei, ++edge_idx )
      {
        cnt++;
        Vertex_u v2a = source(*ei, graph);
        Vertex_u v2b = target(*ei, graph);
//        std::cout<<"edge = (" << v2a<<","<<v2b<<")"<<std::endl;
        
        if (cnt>2)
        {
          std::cout<<"branch detected"<<std::endl;
          continue;
        }
        
        if ( std::find( v_path.cbegin(), v_path.cend(), v2b ) == v_path.cend() )
        {
          v_path.push_back(v2b);
          FindPath(graph, v2b, v_path);
        }
      }
      
      return true;
    }
    
    
    std::vector<int32_t> sort_points(EdgeVector edges, std::vector<int32_t>& cc_index)
    {
      std::vector<EdgeVector> subsets;
      std::vector<int> size_regions;
      connected_component_edges(edges, subsets, size_regions);
      std::list<Vertex> order;
      
      int cnt=-1;
      for (auto edges_subset : subsets)
      {
        
        cnt++;
        
//        std::cout<<"subset size ="<<edges_subset.size()<<std::endl;
//        std::cout<<"edge_subset["<<cnt<<"] = [ ";
        for (auto e : edges_subset)
        {
          std::cout<<" ["<<e.first<<","<<e.second<<"]";
        }
        std::cout<<" ]"<<std::endl;
        
        int sum_regions = 0;
        for (int it=0; it<=cnt; it++) { sum_regions+=size_regions[it];}
        Vertex_u vend=sum_regions-1;
//        std::cout<<"ending vertex = "<<vend<<std::endl;
        
        
        std::list<Vertex_u> order_subset = sort_cc(edges_subset,vend);

        
        
//        std::cout<<"order size ="<<order_subset.size()<<std::endl;
//        std::cout<<"order_subset["<<cnt<<"] = [ ";
//        for (auto o : order_subset)
//        {
//          std::cout<<" "<<o;
//        }
//
//        std::cout<<" ]"<<std::endl;
//        std::cout<<"splicing lists"<<std::endl;
        
        order_subset.reverse();
        if (cnt ==0) cc_index.push_back(order_subset.size()-1);
        else cc_index.push_back(cc_index.back()+order_subset.size());
//        std::cout<<"cc_index = "<<cc_index.back()<<std::endl;
//        order_subset.pop_back();
        order.splice(order.end(), order_subset);
      }
      
      
      std::vector<int32_t> index{ std::make_move_iterator(std::begin(order)),
        std::make_move_iterator(std::end(order)) };
      
      return index;
      
      
    }
    
    struct detect_loops : public boost::dfs_visitor<>
    {
      detect_loops(std::vector<Vertex_u>& _source_vertex) : source_vertex(_source_vertex) { }
      
      bool LoopDetected() const {return !source_vertex.empty();}
    
      template <class Edge, class Graph>
      void back_edge(Edge e, const Graph& g)
      {
        
        source_vertex.push_back( source(e, g) );
//        std::cout << source(e, g)<< " -- " << target(e, g) << "\n";
//        std::cout<<"source_vertex = " << source_vertex<<std::endl;
//        std::cout<<"source_vertex empty?" << source_vertex.empty()<<std::endl;
        
      }
      std::vector<Vertex_u>& source_vertex;
    };

    void render()
    {
      renderer_ = ospNewRenderer(state_->getValue(Parameters::RendererType).toString().c_str()); // choose Scientific Visualization renderer

      OSPData lights;
      
      // create and setup light for Ambient Occlusion
      OSPLight a_light = ospNewLight(renderer_, "ambient");
      
      float lightColor[] = { toFloat(Parameters::LightColorR), toFloat(Parameters::LightColorG), toFloat(Parameters::LightColorB) };
      
      ospSet3fv(a_light, "color", lightColor);
      ospSet1f(a_light, "intensity", 1.0);
      ospCommit(a_light);
      lights = ospNewData(1, OSP_LIGHT, &a_light);
      
      ospCommit(lights);
      
      OSPLight light = ospNewLight(renderer_, state_->getValue(Parameters::LightType).toString().c_str());
      if (light)
      {
        float lightColor[] = { toFloat(Parameters::LightColorR), toFloat(Parameters::LightColorG), toFloat(Parameters::LightColorB) };
        
        if (state_->getValue(Parameters::LightLockCamera).toBool())
        {
          state_->setValue(Parameters::LightPositionX,toFloat(Parameters::CameraPositionX));
          state_->setValue(Parameters::LightPositionY,toFloat(Parameters::CameraPositionY));
          state_->setValue(Parameters::LightPositionZ,toFloat(Parameters::CameraPositionZ));
          state_->setValue(Parameters::LightDirectionX,toFloat(Parameters::CameraViewX)-toFloat(Parameters::CameraPositionX));
          state_->setValue(Parameters::LightDirectionY,toFloat(Parameters::CameraViewY)-toFloat(Parameters::CameraPositionY));
          state_->setValue(Parameters::LightDirectionZ,toFloat(Parameters::CameraViewZ)-toFloat(Parameters::CameraPositionZ));
        }
        
        float lightPosition[] = { toFloat(Parameters::LightPositionX), toFloat(Parameters::LightPositionY), toFloat(Parameters::LightPositionZ) };
        float lightDirection[] = { toFloat(Parameters::LightDirectionX), toFloat(Parameters::LightDirectionY), toFloat(Parameters::LightDirectionZ) };
        
        
        ospSet3fv(light, "color", lightColor);
        ospSet3fv(light, "position", lightPosition);
        ospSet3fv(light, "direction", lightDirection);
        ospSet1f(light, "intensity", toFloat(Parameters::LightIntensity));
        ospSet1f(light, "angularDiameter", toFloat(Parameters::LightRadius)/2.0);
        ospSet1f(light, "penumbraAngle", toFloat(Parameters::LightAngle)/5.0);
        ospSet1f(light, "openingAngle", toFloat(Parameters::LightAngle));
        ospSet1f(light, "radius", toFloat(Parameters::LightRadius));
        ospSet1i(light, "isVisible", state_->getValue(Parameters::LightVisible).toBool() ? 1 : 0);
        ospCommit(light);
        lights = ospNewData(1, OSP_LIGHT, &light);
        ospCommit(lights);
      }

      //material
      OSPMaterial material = ospNewMaterial(renderer_, "OBJMaterial");
      ospSet3f(material, "Kd", 0.2f, 0.2f, 0.2f);
      ospSet3f(material, "Ks", 0.4f, 0.4f, 0.4f);
      ospSet1f(material, "Ns", 100.0f);
      ospCommit(renderer_);

      // complete setup of renderer
      ospSet1i(renderer_, "aoSamples", 10);
      ospSet1i(renderer_, "oneSidedLighting",state_->getValue(Parameters::OneSidedLighting).toBool() ? 1 : 0);
      ospSet1i(renderer_, "shadowsEnabled",state_->getValue(Parameters::ShadowsEnabled).toBool() ? 1 : 0);
      ospSet3f(renderer_, "bgColor", toFloat(Parameters::BackgroundColorR),
        toFloat(Parameters::BackgroundColorG),
        toFloat(Parameters::BackgroundColorB));
      ospSetObject(renderer_, "model", world_);
      ospSetObject(renderer_, "camera", camera_);
      if (light)
        ospSetObject(renderer_, "lights", lights);
      ospSetObject(renderer_, "material", material);
      ospCommit(renderer_);

      // create and setup framebuffer
      framebuffer_ = ospNewFrameBuffer(imgSize_, OSP_FB_SRGBA, OSP_FB_COLOR | /*OSP_FB_DEPTH |*/ OSP_FB_ACCUM);
      ospFrameBufferClear(framebuffer_, OSP_FB_COLOR | OSP_FB_ACCUM);

      // render one frame
      ospRenderFrame(framebuffer_, renderer_, OSP_FB_COLOR | OSP_FB_ACCUM);

      const int frameCount = state_->getValue(Parameters::FrameCount).toInt();
      // render N more frames, which are accumulated to result in a better converged image
      for (int frames = 0; frames < frameCount-1; frames++)
        ospRenderFrame(framebuffer_, renderer_, OSP_FB_COLOR | OSP_FB_ACCUM);
    }

    void writeImage(const std::string& filename)
    {
      // access framebuffer and write its content as PPM file
      const uint32_t * fb = (uint32_t*)ospMapFrameBuffer(framebuffer_, OSP_FB_COLOR);
      writePPM(filename.c_str(), imgSize_, fb);
      ospUnmapFrameBuffer(fb, framebuffer_);
    }
  private:
    void writePPM(const char *fileName, const osp::vec2i &size, const uint32_t *pixel) const
    {
      FILE *file = fopen(fileName, "wb");
      fprintf(file, "P6\n%i %i\n255\n", size.x, size.y);
      std::vector<unsigned char> out(3 * size.x);
      for (int y = 0; y < size.y; y++)
      {
        const unsigned char *in = (const unsigned char *)&pixel[(size.y - 1 - y)*size.x];
        for (int x = 0; x < size.x; x++)
        {
          out[3 * x + 0] = in[4 * x + 0];
          out[3 * x + 1] = in[4 * x + 1];
          out[3 * x + 2] = in[4 * x + 2];
        }
        fwrite(&out[0], 3 * size.x, sizeof(char), file);
      }
      fprintf(file, "\n");
      fclose(file);
    }
  };

  bool OsprayImpl::initialized_(false);
  Core::Thread::Mutex OsprayImpl::lock_("ospray lock");

  #else
  class OsprayImpl {};
  #endif
}

InterfaceWithOspray::InterfaceWithOspray() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(Streamlines);
  INITIALIZE_PORT(SceneGraph);
}

void InterfaceWithOspray::execute()
{
  #ifdef WITH_OSPRAY
  auto fields = getOptionalDynamicInputs(Field);
  auto colorMaps = getOptionalDynamicInputs(ColorMapObject);
  auto streamlines = getOptionalDynamicInputs(Streamlines);

  if (needToExecute())
  {
    detail::OsprayImpl ospray(get_state());
    ospray.setup();

    if (!fields.empty())
    {
      if (colorMaps.size() < fields.size())
        colorMaps.resize(fields.size());

      for (auto&& fieldColor : zip(fields, colorMaps))
      {
        FieldHandle field;
        ColorMapHandle color;
        boost::tie(field, color) = fieldColor;

        FieldInformation info(field);

        if (info.is_trisurfmesh())
        {
          ospray.addField(field, color);
        }
        else if (info.is_pointcloudmesh())
        {
          ospray.addSpheres(field, color);
        }
        else if (info.is_curvemesh())
        {
          ospray.addStreamline(field,color);
        }
        else
        {
          THROW_INVALID_ARGUMENT("Module currently only works with trisurfs or pointclouds.");
        }
        
      }
    }

    for (auto& streamline : streamlines)
    {
      FieldInformation info(streamline);

      if (!info.is_curvemesh())
        THROW_INVALID_ARGUMENT("Module currently only works with curvemesh streamlines.");

      ospray.addStreamline(streamline,nullptr);
    }

    ospray.render();

    auto isoString = boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::universal_time());
    auto filename = "scirunOsprayOutput_" + isoString + ".ppm";
    auto filePath = get_state()->getValue(Variables::Filename).toString() / boost::filesystem::path(filename);
    ospray.writeImage(filePath.string());
    remark("Saving output to " + filePath.string());

    get_state()->setTransientValue(Variables::Filename, filePath.string());

    //auto geom = builder_->buildGeometryObject(field, colorMap, *this, this);
    //sendOutput(SceneGraph, geom);
  }
  #else
  error("Build SCIRun with WITH_OSPRAY set to true to enable this module.");
  #endif
}
