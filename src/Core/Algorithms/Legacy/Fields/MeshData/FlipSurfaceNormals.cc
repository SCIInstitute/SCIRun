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

#include <Core/Algorithms/Legacy/Fields/MeshData/FlipSurfaceNormals.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>


using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;


AlgorithmOutput FlipSurfaceNormalsAlgo::run_generic(const AlgorithmInput& input) const
{
	auto input_field = input.get<Field>(Variables::InputField);
	
	FieldHandle output_field = input_field;
	run(input_field,output_field);
	
	AlgorithmOutput output;
	output[Variables::OutputField] = output_field;
	return output;
}


bool FlipSurfaceNormalsAlgo::run(FieldHandle& input, FieldHandle& output)  const
{
	if(!input) {
		error("No input on source field");
		return false;
	}
	
	FieldInformation fi(input);
	
	if(!fi.is_surface()) {
		error("This algorithm only works on surface mesh");
		return false;
	}
	
	std::cout << "Hello1!" << std::endl;
	
	//FieldHandle output = input;
	
	//output.detach();
	//output->mesh_detach();
	VMesh* mesh = output->vmesh();
	
	VMesh::Node::array_type inodes;
	VMesh::Node::array_type onodes;
	VMesh::Face::size_type isize, numnodes = 0;
	VMesh::Face::index_type faceindex;
	
	unsigned int cnt = 0;
	mesh->size(isize);
	
	
	unsigned int k = 0;
	unsigned int z = isize;
	
	for(VMesh::Face::size_type i = 0; i < isize; ++i)
	{
		faceindex = i;
		k = i;
		mesh->get_nodes(inodes, faceindex);
		numnodes = inodes.size();
		// Without this, when the algorithm is run there is an error saying that the vector location can not be accessed.
		// This just initializes the cells in the vector. Probably a better/more effective way to do this.
		for(VMesh::Face::size_type p = 0; p < numnodes; p++)
			onodes.push_back(0);
		for (VMesh::Face::size_type p = 0; p < numnodes; p++) 
		{
			onodes[numnodes-1-p] = inodes[p];
		}
		// Set the reordered nodes back into the mesh through vmesh functions
		mesh->set_nodes(onodes, faceindex);

		cnt++;

		if (cnt > 100)
		{
			update_progress(i/isize);
			cnt = 0;
		}
		std::cout << "Progress: " << k/z << std::endl;
	}
  
	return true;
}