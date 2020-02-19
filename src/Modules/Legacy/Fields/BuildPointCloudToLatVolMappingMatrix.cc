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


///@author
///   McKay Davis,
///   Scientific Computing and Imaging INstitute,
///   University of Utah
///@date  May 2005

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>

#include <Core/Math/MiscMath.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

#include <vector>
#include <map>

namespace SCIRun {

///@file  BuildPointCloudToLatVolMappingMatrix.cc
///@class BuildPointCloudToLatVolMappingMatrix
///@brief Builds mapping matrix that projects data from a PointCloud to a LatVol

class BuildPointCloudToLatVolMappingMatrix : public Module
{
  public:
    BuildPointCloudToLatVolMappingMatrix(GuiContext* ctx);
    virtual ~BuildPointCloudToLatVolMappingMatrix() {}

    virtual void execute();

  private:
    GuiDouble		epsilon_;
};


DECLARE_MAKER(BuildPointCloudToLatVolMappingMatrix)

BuildPointCloudToLatVolMappingMatrix::BuildPointCloudToLatVolMappingMatrix(GuiContext* ctx)
  : Module("BuildPointCloudToLatVolMappingMatrix", ctx, Filter, "MiscField", "SCIRun"),
    epsilon_(get_ctx()->subVar("epsilon"), 0.0)
{
}

void
BuildPointCloudToLatVolMappingMatrix::execute()
{
  FieldHandle pcf;
  FieldHandle lvf;

  // Get the PointCloudField from the first port
  get_input_handle("PointCloudField", pcf, true);

  // Get the LatVolField from the first port
  get_input_handle("LatVolField", lvf, true);

  FieldInformation pfi(pcf);
  FieldInformation lfi(lvf);

  if (!(pfi.is_pointcloudmesh()))
  {
    error("Field connected to port 1 must be PointCloudField.");
    return;
  }

  // Make sure the second input field is of type LatVolField
  if (!(lfi.is_latvolmesh()))
  {
    error("Field connected to port 2 must be LatVolField.");
    return;
  }

  if (inputs_changed_ || epsilon_.changed() || !oport_cached("MappingMatrix"))
  {
    update_state(Executing);
    double epsilon = Clamp(epsilon_.get(), 0.0, 1.0);

    // Get the meshes from the fields
    VMesh* pcm = pcf->vmesh();
    VMesh* lvm = lvf->vmesh();

    // LVMesh Node Count
    VMesh::Node::size_type lvmns;
    lvm->size(lvmns);

    // PointClouldMesh Node Count
    VMesh::Node::size_type pcmns;
    pcm->size(pcmns);

    // LVMesh Node Iterators
    VMesh::Node::iterator lvmn, lvmne;
    lvm->begin(lvmn);
    lvm->end(lvmne);

    // PCMesh Node Iterators
    VMesh::Node::iterator pcmn, pcmne;
    pcm->end(pcmne);

    // Map point to distance, sorts by point index
    typedef std::map<size_t, double> point2dist_t;

    epsilon = lvm->get_bounding_box().diagonal().length() * epsilon;

    // To automatically handle memory allocation to hold data for the
    // sparse matrix, we use STL vectors here
    std::vector<index_type> rows(lvmns+1);
    std::vector<index_type> cols;
    std::vector<double> data;
    index_type i = 0, row = 0;

    // Iterate through each point of the LVMesh
    while (lvmn != lvmne)
    {
      // Get the location of this node of the LVMesh
      Point lvp;
      lvm->get_point(lvp, *lvmn);
      // Map from PCMesh node index to distance
      point2dist_t point2dist;
      // Total holds the total distance calculation to all points for normalization
      double total = 0.0;
      // Foreach node of LVMesh, iterate through each node of PCMesh
      pcm->begin(pcmn);
      while (pcmn != pcmne)
      {
        // Get the location of this node of the PCMesh
        Point pcp;
        pcm->get_point(pcp, *pcmn);
        // Do the distance function calculation: 1/d - epsilon
        double d = 1.0/(pcp-lvp).length() - epsilon;
        // If the function is positive, the PCMesh node contributes
        // to this node of the LVMesh
        if (d > 0.0)
        {
          // Insert it and increase the normalization total
          point2dist.insert(std::make_pair((*pcmn).index_, d));
          total += d;
        }
        // Next PCMeshNode please
        ++pcmn;
      }

      // Hack to avoid divide by zero
      if (total == 0.0)
        total = 1.0;

      // Now fill up the current row of the sparse matrix
      rows[row++] = cols.size();
      // Iterate through all point that contributed in PointCloudNode index order
      point2dist_t::iterator pb = point2dist.begin(), pe = point2dist.end();
      while (pb != pe)
      {
        // Normalize, and dont add if contribution is nil
        double d = pb->second/total;
        if (d > 0.0000001)
        { /// @todo: Better epsilon checking
          // Add the data to the sparse row matrix
          cols.push_back(pb->first);
          data.push_back(d);
        }
        // Next PCMeshNode/Distance pair please
        ++pb;
      }
      // Next LVMeshNode please
      ++lvmn;
    }
    rows[row] = cols.size();

    // Send the mapping matrix downstream
    MatrixHandle matrix(new SparseRowMatrix(lvmns, pcmns, SparseRowMatrix::Data(rows, cols, data), data.size()));
    send_output_handle("MappingMatrix", matrix);
  }
}


}
