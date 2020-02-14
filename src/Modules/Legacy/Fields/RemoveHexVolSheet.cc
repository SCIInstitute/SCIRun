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


///@file  InsertHexSheet.cc
///
///@author
///   Jason Shepherd,
///   Department of Computer Science,
///   University of Utah
///@date  May 2006

#include <Core/Util/StringUtil.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#include <set>
#include <stack>
#include <vector>
#include <iostream>

namespace SCIRun {

/// @class RemoveHexVolSheetAlgo
/// @brief This module removes a layer of hexes corresponding to the input edge ids.

/// @todo: change vector param to ref
class RemoveHexVolSheetAlgo
{
public:
    /// virtual interface.
  void execute(
      ProgressReporter *reporter,
      FieldHandle hexfieldh,
      std::vector<unsigned int> edges,
      FieldHandle& keptfield,
      FieldHandle& extractedfield );

  void get_opposite_edges(
      VMesh::Edge::index_type &opp_edge1,
      VMesh::Edge::index_type &opp_edge2,
      VMesh::Edge::index_type &opp_edge3,
      VMesh *mesh,
      VMesh::Elem::index_type hex_id,
      VMesh::Edge::index_type edge_id );

  void node_get_edges(VMesh *mesh,
                      std::set<VMesh::Edge::index_type> &result,
                      VMesh::Node::index_type node );
};

void
RemoveHexVolSheetAlgo::execute(
    ProgressReporter *pr,
    FieldHandle hexfieldh, std::vector<unsigned int> edges,
    FieldHandle& keptfield, FieldHandle& extractedfield )
{
#ifdef HAVE_HASH_MAP
# if defined(__ECC) || defined(_MSC_VER)
  typedef hash_map<VMesh::index_type,
    VMesh::index_type,
    hash_compare<unsigned int, std::equal_to<unsigned int> > > hash_type;
# else
  typedef hash_map<index_type,
    VMesh::Node::index_type,
    hash<unsigned int>,
    std::equal_to<unsigned int> > hash_type;
# endif
#else
  typedef std::map<index_type,
    VMesh::Node::index_type,
    std::less<unsigned int> > hash_type;
#endif

  VMesh *original_mesh = hexfieldh->vmesh();
  VMesh *mesh_rep = hexfieldh->vmesh();

  FieldInformation fi(hexfieldh);
  fi.make_nodata();
  fi.make_hexvolmesh();

  keptfield = CreateField(fi);
  extractedfield = CreateField(fi);

  VMesh *kept_mesh = keptfield->vmesh();
  kept_mesh->copy_properties( mesh_rep );

  VMesh *extracted_mesh = extractedfield->vmesh();
  extracted_mesh->copy_properties( mesh_rep );

  original_mesh->synchronize( Mesh::EDGES_E | Mesh::NODE_NEIGHBORS_E );

  std::stack<VMesh::Edge::index_type> edge_stack;
  std::set<VMesh::Elem::index_type> extracted_hex_set;
  std::set<VMesh::Edge::index_type> used_edge_set;

  for( size_t i = 0; i < edges.size(); i++ )
  {
    if( used_edge_set.find( (VMesh::Edge::index_type)edges[i] ) != used_edge_set.end() )
        continue;

    edge_stack.push( (VMesh::Edge::index_type)edges[i] );
    used_edge_set.insert( (VMesh::Edge::index_type)edges[i] );

    while( edge_stack.size() != 0 )
    {
      VMesh::Edge::index_type edge_id = edge_stack.top();
      edge_stack.pop();

      VMesh::Elem::array_type hex_array;

      original_mesh->get_elems( hex_array, edge_id );
      if( hex_array.size() == 0 )
      {
        pr->warning("ERROR: Edge "+to_string(edge_id)+" does not exist in the mesh.\n");
        continue;
      }

      for( size_t j = 0; j < hex_array.size(); j++ )
      {
        VMesh::Edge::index_type opp_edge1, opp_edge2, opp_edge3;
        get_opposite_edges( opp_edge1, opp_edge2, opp_edge3, original_mesh, hex_array[j], edge_id );

        if( used_edge_set.find( opp_edge1 ) == used_edge_set.end() )
        {
          edge_stack.push( opp_edge1 );
          used_edge_set.insert( opp_edge1 );
        }
        if( used_edge_set.find( opp_edge2 ) == used_edge_set.end() )
        {
          edge_stack.push( opp_edge2 );
          used_edge_set.insert( opp_edge2 );
        }
        if( used_edge_set.find( opp_edge3 ) == used_edge_set.end() )
        {
          edge_stack.push( opp_edge3 );
          used_edge_set.insert( opp_edge3 );
        }

        extracted_hex_set.insert( hex_array[j] );
      }
    }
  }

  pr->remark("Extracting " +to_string(extracted_hex_set.size()) + " hexes from the mesh.\n");

  std::set<VMesh::Node::index_type> affected_node_set;
  std::set<VMesh::Edge::index_type>::iterator es = used_edge_set.begin();
  std::set<VMesh::Edge::index_type>::iterator ese = used_edge_set.end();
  while( es != ese )
  {
    VMesh::Node::array_type two_nodes;
    original_mesh->get_nodes( two_nodes, *es );
    for( int i = 0; i < 2; i++ )
    {
      affected_node_set.insert( two_nodes[i] );
    }
    ++es;
  }

  std::set<VMesh::Elem::index_type>::iterator hs = extracted_hex_set.begin();
  std::set<VMesh::Elem::index_type>::iterator hse = extracted_hex_set.end();

  hash_type extracted_nodemap;
  while( hs != hse )
  {
    VMesh::Node::array_type onodes;
    original_mesh->get_nodes( onodes, *hs );
    VMesh::Node::array_type nnodes(onodes.size());

    for (size_t k = 0; k < onodes.size(); k++)
    {
      if( extracted_nodemap.find((index_type)onodes[k]) == extracted_nodemap.end())
      {
        Point np;
        original_mesh->get_center( np, onodes[k] );
        const VMesh::Node::index_type nodeindex =
            extracted_mesh->add_point( np );
        extracted_nodemap[(index_type)onodes[k]] = nodeindex;
        nnodes[k] = nodeindex;
      }
      else
      {
        nnodes[k] = extracted_nodemap[(index_type)onodes[k]];
      }
    }
    extracted_mesh->add_elem( nnodes );
    ++hs;
  }

  VMesh::Elem::iterator citer;
  original_mesh->begin( citer );
  VMesh::Elem::iterator citere;
  original_mesh->end( citere );
  hash_type kept_nodemap;
  while( citer != citere )
  {
    VMesh::Elem::index_type hex_id = *citer;
    if( extracted_hex_set.find( hex_id ) == extracted_hex_set.end() )
    {
      VMesh::Node::array_type onodes;
      original_mesh->get_nodes( onodes, hex_id );
      VMesh::Node::array_type nnodes( onodes.size() );

      for (size_t k = 0; k < onodes.size(); k++)
      {
        if( kept_nodemap.find( (index_type)onodes[k] ) == kept_nodemap.end() )
        {
          if( affected_node_set.find( (index_type) onodes[k] ) == affected_node_set.end() )
          {
            Point np;
            original_mesh->get_center( np, onodes[k] );
            const VMesh::Node::index_type nodeindex = kept_mesh->add_point( np );
            kept_nodemap[(index_type)onodes[k]] = nodeindex;
            nnodes[k] = nodeindex;
          }
          else
          {
            std::stack<VMesh::Node::index_type> node_stack;
            std::set<VMesh::Edge::index_type> edge_string;
            std::set<VMesh::Node::index_type> node_string_set;
            node_string_set.insert( onodes[k] );
            node_stack.push( onodes[k] );
            while( node_stack.size() )
            {
              VMesh::Node::index_type stack_node = node_stack.top();
              node_stack.pop();

              std::set<VMesh::Edge::index_type> edge_set;
              node_get_edges( original_mesh, edge_set, stack_node );

              std::set<VMesh::Edge::index_type>::iterator esi = edge_set.begin();
              std::set<VMesh::Edge::index_type>::iterator esie = edge_set.end();
              while( esi != esie )
              {
                if( used_edge_set.find( *esi ) != used_edge_set.end() )
                {
                  VMesh::Node::array_type edge_nodes;
                  edge_string.insert( *esi );
                  original_mesh->get_nodes( edge_nodes, *esi );
                  for( int j = 0; j < 2; j++ )
                  {
                    if( edge_nodes[j] != stack_node &&
                        ( node_string_set.find( edge_nodes[j] ) == node_string_set.end() ) )
                    {
                      node_string_set.insert( edge_nodes[j] );
                      node_stack.push( edge_nodes[j] );
                    }
                  }
                }
                ++esi;
              }
            }

              //find the average location of the node_string_set
            std::set<VMesh::Node::index_type>::iterator nss = node_string_set.begin();
            std::set<VMesh::Node::index_type>::iterator nsse = node_string_set.end();
            Point np;
            original_mesh->get_center( np, *nss );
            ++nss;
            while( nss != nsse )
            {
              Point temp;
              original_mesh->get_center( temp, *nss );
              np += temp.vector();
              ++nss;
            }
            np /= node_string_set.size();

              //create a new point at this location
            const VMesh::Node::index_type node_index = kept_mesh->add_point( np );
              //set the kept_nodemap for all nodes in the set to this new point
            nss = node_string_set.begin();
            while( nss != nsse )
            {
              kept_nodemap[(index_type)(*nss)] = node_index;
              ++nss;
            }

              //add the point to the nnodes array
            nnodes[k] = node_index;
          }
        }
        else
        {
          nnodes[k] = kept_nodemap[(index_type)onodes[k]];
        }
      }

      kept_mesh->add_elem( nnodes );
    }
    ++citer;
  }

  keptfield->copy_properties( hexfieldh.get_rep() );
  extractedfield->copy_properties( hexfieldh.get_rep() );
}

void
RemoveHexVolSheetAlgo::get_opposite_edges(
    VMesh::Edge::index_type &opp_edge1,
    VMesh::Edge::index_type &opp_edge2,
    VMesh::Edge::index_type &opp_edge3,
    VMesh *mesh,
    VMesh::Elem::index_type hex_id,
    VMesh::Edge::index_type edge_id )
{
  VMesh::Edge::array_type all_edges;
  mesh->get_edges( all_edges, hex_id );

//  cout << all_edges[0] << " " << all_edges[1] << " " << all_edges[2] << " " << all_edges[3] << " " << all_edges[4] << " " << all_edges[5] << " " << all_edges[6] << " " << all_edges[7] << " " << all_edges[8] << " " << all_edges[9] << " " << all_edges[10] << " " << all_edges[11] << endl;

  if( edge_id == all_edges[0] )
  {
    opp_edge1 = all_edges[2];
    opp_edge2 = all_edges[4];
    opp_edge3 = all_edges[6];
  }
  else if( edge_id == all_edges[3] )
  {
    opp_edge1 = all_edges[7];
    opp_edge2 = all_edges[1];
    opp_edge3 = all_edges[5];
  }
  else if( edge_id == all_edges[8] )
  {
    opp_edge1 = all_edges[11];
    opp_edge2 = all_edges[9];
    opp_edge3 = all_edges[10];
  }
  else if( edge_id == all_edges[2] )
  {
    opp_edge1 = all_edges[0];
    opp_edge2 = all_edges[4];
    opp_edge3 = all_edges[6];
  }
  else if( edge_id == all_edges[11] )
  {
    opp_edge1 = all_edges[8];
    opp_edge2 = all_edges[9];
    opp_edge3 = all_edges[10];
  }
  else if( edge_id == all_edges[4] )
  {
    opp_edge1 = all_edges[0];
    opp_edge2 = all_edges[2];
    opp_edge3 = all_edges[6];
  }
  else if( edge_id == all_edges[7] )
  {
    opp_edge1 = all_edges[3];
    opp_edge2 = all_edges[1];
    opp_edge3 = all_edges[5];
  }
  else if( edge_id == all_edges[6] )
  {
    opp_edge1 = all_edges[0];
    opp_edge2 = all_edges[2];
    opp_edge3 = all_edges[4];
  }
  else if( edge_id == all_edges[1] )
  {
    opp_edge1 = all_edges[3];
    opp_edge2 = all_edges[7];
    opp_edge3 = all_edges[5];
  }
  else if( edge_id == all_edges[9] )
  {
    opp_edge1 = all_edges[8];
    opp_edge2 = all_edges[11];
    opp_edge3 = all_edges[10];
  }
  else if( edge_id == all_edges[10] )
  {
    opp_edge1 = all_edges[8];
    opp_edge2 = all_edges[11];
    opp_edge3 = all_edges[9];
  }
  else
  {
    opp_edge1 = all_edges[3];
    opp_edge2 = all_edges[7];
    opp_edge3 = all_edges[1];
  }
}


void
RemoveHexVolSheetAlgo::node_get_edges(VMesh *mesh,
                std::set<VMesh::Edge::index_type> &result,
                VMesh::Node::index_type node )
{
  result.clear();

 VMesh::Elem::array_type elems;
  mesh->get_elems( elems, node );

  for (size_t i = 0; i < elems.size(); i++)
  {
    VMesh::Edge::array_type edges;
    mesh->get_edges( edges, elems[i] );
    for (size_t j = 0; j < edges.size(); j++)
    {
      VMesh::Node::array_type nodes;
      mesh->get_nodes( nodes, edges[j] );

      for (size_t k = 0; k < nodes.size(); k++)
      {
        if (nodes[k] == node)
        {
          result.insert( edges[j] );
          break;
        }
      }
    }
  }
}



class RemoveHexVolSheet : public Module
{
  public:
    RemoveHexVolSheet(GuiContext* ctx);
    virtual ~RemoveHexVolSheet() {}

    virtual void execute();

  private:

    GuiString  gui_edge_list_;
    int       last_field_generation_;

    std::vector< unsigned int > edge_ids_;
};


DECLARE_MAKER(RemoveHexVolSheet)


RemoveHexVolSheet::RemoveHexVolSheet(GuiContext* ctx)
        : Module("RemoveHexVolSheet", ctx, Filter, "NewField", "SCIRun"),
          gui_edge_list_(ctx->subVar("edge-list"), "No values present."),
          last_field_generation_(0)
{
}


void
RemoveHexVolSheet::execute()
{
  // Get input fields.
  FieldHandle hexfieldhandle;
  get_input_handle("HexField", hexfieldhandle,true);

  bool changed = false;

  std::vector<unsigned int> edgeids(0);
  std::istringstream vlist(gui_edge_list_.get());
  unsigned int val;
  while(!vlist.eof())
  {
    vlist >> val;
    if (vlist.fail())
    {
      if (!vlist.eof())
      {
        vlist.clear();
        warning("List of Edge Ids was bad at character " +
                to_string((int)(vlist.tellg())) +
                "('" + ((char)(vlist.peek())) + "').");
      }
      break;
    }

    edgeids.push_back(val);
  }

    // See if any of the isovalues have changed.
  if( edge_ids_.size() != edgeids.size() )
  {
    edge_ids_.resize( edgeids.size() );
    changed = true;
  }

  for(size_t i=0; i<edgeids.size(); i++ )
  {
    if( edge_ids_[i] != edgeids[i] )
    {
      edge_ids_[i] = edgeids[i];
      changed = true;
    }
  }

  if (last_field_generation_ == hexfieldhandle->generation &&
      oport_cached( "NewHexField" )&&
      oport_cached( "ExtractedHexes" ) && !changed )
  {
    // We're up to date, return.
    return;
  }
  last_field_generation_ = hexfieldhandle->generation;


  FieldInformation fi(hexfieldhandle);

  if (!(fi.is_hex_element()))
  {
    error( "Only HexVolFields are currently supported in the RemoveHexVolSheet module.");
    return;
  }

  RemoveHexVolSheetAlgo algo;
  FieldHandle keptfield, extractedfield;
  algo.execute( this, hexfieldhandle, edgeids, keptfield, extractedfield );

  send_output_handle("NewHexField", keptfield);
  send_output_handle("ExtractedHexes", extractedfield);
}


} // End namespace SCIRun
