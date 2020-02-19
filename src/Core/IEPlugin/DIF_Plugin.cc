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


#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Util/StringUtil.h>

#include <sci_debug.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <limits.h>

#define MAX_OUTPUT_SURFS 20    // Maximum number of output surfaces

// list of colors to cycle through
const char *colors[] = {"cc0000", "00cc00", "0000cc", "cc00cc", "99ff99", "33cccc", "cc6633", "669900", "99ccff"};

namespace SCIRun {

struct IndexHash {
  static const size_t bucket_size = 4;
  static const size_t min_buckets = 8;

  size_t operator()(const index_type &idx) const
  { return (static_cast<size_t>(idx)); }

  bool operator()(const index_type &i1, const index_type &i2) const
  { return (i1 < i2); }
};

bool DIF_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);

bool DIF_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
{
  using std::endl;
  //! Define types we need for mapping
#ifdef HAVE_HASH_MAP
  typedef hash_map<index_type,int,IndexHash> hash_map_type;
#else
  typedef std::map<index_type,int,IndexHash> hash_map_type;
#endif

  // Get interfaces to mesh and field
  VMesh* mesh = fh->vmesh();
  VField* field = fh->vfield();

  // Initialize a field information class - used for checking data types
  FieldInformation fi(fh);

  if (!fi.is_trisurfmesh()){
    if (pr) pr->error("DIF writer only works on TriSurfMesh data");
    return (false);
  }

  if (!fi.is_scalar()){
    if (pr) pr->error("DIF writer only works on scalar data");
    return (false);
  }

  if(!fi.is_integer()){
    if (pr) pr->error("DIF writer only works on integer data type");
    return (false);
  }

  if(!fi.mesh_basis_order() == 1)
  {
    if (pr) pr->error("Data needs to be located on the elements.");
    return (false);
  }

  // set up output file variables, and format file string
  std::ofstream outputfile;
  outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
  std::string var_fn(filename);
  std::string::size_type pos = var_fn.find_last_of(".");
  std::string base = var_fn.substr(0, pos);
  std::string ext = var_fn.substr(pos);

  if(pos == std::string::npos)
  {
    var_fn += ".xml";
  }
  else if (ext != ".xml")
  {
    var_fn += ".xml";
  }

  try
  {
    outputfile.open(var_fn.c_str());
  }
  catch (...)
  {
    if (pr) pr->error("Could not open file: " + var_fn);
    return (false);
  }

  // these appear to be reasonable formatting flags for output
  std::ios_base::fmtflags ff;
  ff = outputfile.flags();
  ff |= outputfile.showpoint; // write floating-point values including always the decimal point
  ff |= outputfile.fixed; // write floating point values in fixed-point notation
  outputfile.flags(ff);

  // check to see if data is correct and find unique output field values.
  // this reader assumes each file to be written has a unique integer ID associated
  // with elements of a particular surface. If only one surface is to be written,
  // all triangles should have a single output value.

  VMesh::size_type num_values = field->num_values();        // total number of elements, in our case
  std::vector<int> surf_ids;                                     // storage for surf_ids (values stored on the elements)
  for(VField::index_type idx=0; idx<num_values; idx++)      // loop through all elements
  {
    int value;
    unsigned int i;
    field->get_value(value,idx);                     // get value stored on element
    for(i = 0; i < surf_ids.size(); i++)             // search through vector of current unique surface ids
    {
      if(value == surf_ids[i])                       // already have this one
        break;
    }
    if(i == surf_ids.size())                         // we didn't find it. This ID is unique
    {
      surf_ids.push_back(value);                     // add this ID to our vector
      if(surf_ids.size() > MAX_OUTPUT_SURFS)         // maximum number of output fields reached. Throw error.
      {
        if (pr) pr->error("Maximum number of output surface indices reached. MAX: " + to_string(MAX_OUTPUT_SURFS));
        return (false);
      }
    }
  }

  sort(surf_ids.begin(), surf_ids.end());  // order output field values vector.

  // Output DIF header
  outputfile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
  outputfile << "<DIF>" << endl;
  outputfile << "   <DIFHeader>" << endl;
  outputfile << "      <Version>SCI1.0</Version>" << endl;
  outputfile << "   </DIFHeader>" << endl;
  outputfile << "   <DIFBody>" << endl;
  outputfile << "      <Volumes number=\"" << surf_ids.size() << "\">" << endl;

  // We write out normals at each node as part of DIF. We need to do a synchronize to
  // have access to these normals.
  mesh->synchronize(Mesh::NORMALS_E);

  // loop through each unique surface id. We need to output a unique <Volume> section
  // in the xml file for each unique surface.
  for(unsigned int i = 0; i < surf_ids.size(); i++)
  {
    unsigned int node_count = 0;                     // number of unique nodes in this surface
    std::vector<Point> onodes;                            // storage for nodes
    std::vector<Vector> onormals;                         // storage for normals
    std::vector<unsigned int> otris;                      // storage for triangles

    VMesh::Face::size_type fsize;                    // sizes and iterators for mesh access
    VMesh::Face::iterator fiter;
    VMesh::Face::iterator fiter_end;
    mesh->size(fsize);
    mesh->begin(fiter);
    mesh->end(fiter_end);

    hash_map_type node_map;                          // create map - allows not duplicating nodes in output

    unsigned int color_idx = i % (sizeof(colors) / sizeof(char *));   // rotate through defined colors above
    outputfile << "         <Volume color=\"" << colors[color_idx] << "00\" name=\"VOL" << surf_ids[i] << "\">" << endl;

    // loop through all faces, determine if it belongs to this surface id, then store neighboring nodes and face.
    while(fiter != fiter_end)
    {
      int id_val;
      field->get_value(id_val, *fiter);              // get value from element
      if(id_val == surf_ids[i])
      {
        // this triangle belongs to this surface.
        // Need to output nodes, normals, and triangle.
        unsigned int triangle_node_idx[3];          // storage for output node IDs
        VMesh::Node::array_type fac_nodes;          // storage to get bounding nodes of this element
        mesh->get_nodes(fac_nodes, *fiter);         // find bounding nodes of this element

        for(size_t q=0; q<fac_nodes.size(); q++)    // loop through the three nodes
        {
          VMesh::Node::index_type a = fac_nodes[q];
          hash_map_type::iterator it = node_map.find(a);  // see if node is already in our map
          if(it == node_map.end())
          {
            // if node is not in map, we need to insert it and get a unique output node ID.
            Point p;
            Vector n;

            mesh->get_center(p, a);    // get point value
            mesh->get_normal(n, a);    // get point normal

            onodes.push_back(p);       // put node in vector of output points
            onormals.push_back(n);     // put normal in vector of output Vectors

            node_count++;               // output is 1 based, increment before storing.
            node_map[a] = node_count;   // store this node in our map, along with unique output index ID.
            triangle_node_idx[q] = node_count;   // store unique ID in local storage, used for writing triangle
          }
          else{
            // node is already in map, and a unique ID has already been assigned. Just get this ID.
            triangle_node_idx[q] = it->second;
          }
        }
        // put unique output indexes into output triangle structure.
        otris.push_back(triangle_node_idx[0]);
        otris.push_back(triangle_node_idx[1]);
        otris.push_back(triangle_node_idx[2]);
      }
      ++fiter;
    } // end loop through faces

    // output node vertices
    outputfile << "            <Vertices number=\"" << onodes.size() << "\">";
    for(unsigned int i = 0; i < onodes.size(); i++)
    {
      Point p = onodes[i];
      outputfile << p.x() << " " << p.y() << " " << p.z() << " ";
    }
    outputfile << "</Vertices>" << endl;

    // output normals
    outputfile << "            <Normals number=\"" << onormals.size() << "\">";
    for(unsigned int i = 0; i < onormals.size(); i++)
    {
      Vector v = onormals[i];
      outputfile << v.x() << " " << v.y() << " " << v.z() << " ";   // Flip normal direction by changing sign of v.?
    }
    outputfile << "</Normals>" << endl;

    // output triangles
    outputfile << "            <Polygons number=\"" << otris.size() / 3 << "\">";
    for(unsigned int i = 0; i < otris.size(); i+=3)
    {
      outputfile << otris[i] << " " << otris[i+1] << " " << otris[i+2] << " ";
    }
    outputfile << "</Polygons>" << endl;
    outputfile << "         </Volume>" << endl;
  } // end loop through IDs

  outputfile << "      </Volumes>" << endl;
  outputfile << "      <Labels></Labels>" << endl;
  outputfile << "   </DIFBody>" << endl;
  outputfile << "</DIF>" << endl;

  outputfile.close();
  return true;
}

static FieldIEPlugin DIF_plugin("DIF Field", "{.xml}", "", 0, DIF_writer);

} // end namespace
