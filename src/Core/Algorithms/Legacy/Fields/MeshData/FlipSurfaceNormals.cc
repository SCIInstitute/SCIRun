/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Fields/MeshData/FlipSurfaceNormals.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/VMesh.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
FlipSurfaceNormalsAlgo::
run(FieldHandle& input, FieldHandle& output)
{
  algo_start("FlipSurfaceNormals");

  if (input.get_rep() == 0)
  {
    error("No input source field");
    algo_end();
    return (false);
  }

  FieldInformation fi(input);  
  if (! fi.is_surface())
  {
    error("This algorithm only works on a surface mesh");
    algo_end();
    return (false);
  }

  output = input;
  output.detach();
  output->mesh_detach();

  VMesh* mesh = output->vmesh();

  VMesh::Node::array_type inodes, onodes;
  VMesh::Face::size_type isize,  numnodes = 0;
  VMesh::Face::index_type faceindex;

  unsigned int cnt = 0;
  mesh->size(isize);

  // Reorder nodes for each face (I think this should work for both tri and quad faces)
  for(VMesh::Face::size_type i = 0; i < isize; ++i)
  {
    faceindex = i;
    mesh->get_nodes(inodes, faceindex);
    numnodes = inodes.size();
    for (VMesh::Face::size_type p = 0; p < numnodes; p++) 
    {
      onodes[numnodes-1-p] = inodes[p];
    }
    // Set the reordered nodes back into the mesh through vmesh functions
    mesh->set_nodes(onodes, faceindex);

    cnt++;

    if (cnt > 100)
    {
      update_progress(i, isize);
      cnt = 0;
    }
  }

  algo_end(); 
  return (true);
}

} // namespace SCIRunAlgo
