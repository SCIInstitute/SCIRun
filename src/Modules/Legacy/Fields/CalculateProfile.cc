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


#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <math.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>
#include <float.h>
#include <vector>
#include <iostream>
#include <Core/Thread/Thread.h>
#include <Core/Algorithms/Fields/Mapping/MapFieldDataOntoNodes.h>
#include <Core/Algorithms/Fields/Mapping/MappingDataSource.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

namespace SCIRunAlgo {

	using namespace SCIRun;

	/// @class CalculateProfile
	/// @brief Samples image data surrounding mesh generated from segmentation.
	class CalculateProfile : public Module {
	public:
		CalculateProfile(GuiContext*);
		virtual ~CalculateProfile() {}
		virtual void execute();

		GuiInt gui_depth_;
		GuiInt gui_samples_;

	};

	DECLARE_MAKER(CalculateProfile)

	CalculateProfile::CalculateProfile(GuiContext* ctx) :
	Module("CalculateProfile", ctx, Source, "ChangeFieldData", "SCIRun"),
	gui_depth_(ctx->subVar("depth"),4),
	gui_samples_(ctx->subVar("samples"),4)
	{
	}

	void
	CalculateProfile::execute()
	{
		FieldHandle source, destination;

		get_input_handle("Source",source,true);
		get_input_handle("Destination",destination,true);

		if (inputs_changed_ || gui_depth_.changed() || gui_samples_.changed())

		{
			update_state(Executing);

			FieldInformation fis(source);

			if (!(fis.is_latvolmesh()))
			{
				error("Source needs to be a LatVolMesh");
				return;
			}

			if (!(fis.is_constantdata()))
			{
				error("Source needs to have data at the elements");
				return;
			}

			FieldInformation fid(destination);
			if (!(fid.is_trisurfmesh()))
			{
				error("Destination needs to be a TriSurfMesh");
				return;
			}
			// Algorithm goes here....

			fid.make_lineardata();
			fid.make_double();

			// Generate output field

			MappingDataSourceHandle datasource;

			FieldHandle wfield;

			SCIRunAlgo::MapFieldDataOntoNodesAlgo algo;


			algo.set_option("quantity","value");
			//algo.set_option("value","closestinterpolateddata");
			algo.set_option("value","interpolateddata");
			algo.set_scalar("outside_value",0);
			algo.set_scalar("max_distance",50000000);

			if(!(CreateDataSource(datasource,source,wfield,&algo)))
			{
				error("Could not make data source for mapping");
				return;
			}


			VMesh* dmesh = destination->vmesh();
			VMesh* smesh = source->vmesh();

			dmesh->synchronize(Mesh::NORMALS_E);
			smesh->synchronize(Mesh::ELEM_LOCATE_E);

			VMesh::Node::size_type num_nodes = dmesh->num_nodes();

			size_type depth = gui_depth_.get();
			size_type Nsamples = gui_samples_.get();

			double ddepth = static_cast<double> (depth);
			double scale = ddepth/ static_cast<double> (Nsamples);

			// Create std::vector of length ArLen to define regularly spaced intervals within + and -
			//	bounds "depth"
			int ArLen = 2*Nsamples +1;
			std::vector<double> incre;
			for (int inx = 0; inx < ArLen; inx++) {
				incre.push_back( -1 * ddepth + inx * scale);
			}

			MatrixHandle provals= new DenseMatrix(num_nodes, ArLen);

			Point SampLoc;
			double val;
			Point  point;
			Vector normal;
			int cnt = 0;
			double* mprt= provals->get_data_pointer();

			// Write sample locations associated with each node to a txt file in the bin directory
			//std::ofstream outfilename;
			//outfilename.open("LineNodes");
			int count=0;
			for (VMesh::Node::index_type idx=0;idx<num_nodes;idx++)
			{
				dmesh->get_center(point,idx);
				dmesh->get_normal(normal,idx);

				for (int inx = 0; inx <ArLen; inx++) {
					// Define a point along the line profile
					SampLoc = normal * incre[inx] + point;
					// Interpolate surrounding data onto point(SampLoc)
					datasource->get_data(val,SampLoc);
					// Write interpolated value to correct position within the line profile
					mprt[count] = val;
					count++;
					//Write out each sample location along profile to new line
					//outfilename << SampLoc << " ";
					//outfilename << endl;
				}

				// progress bar
				cnt++; if (cnt == 30) { cnt = 0; update_progress(idx,num_nodes); }

			}
			//outfilename.close();


			send_output_handle("Result",provals);
		}
	}

} // End namespace SCIRun
