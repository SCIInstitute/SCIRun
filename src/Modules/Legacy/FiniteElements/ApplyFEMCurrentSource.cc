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


/*
 *  ApplyFEMCurrentSource.cc:  Builds the RHS of the FE matrix for voltage sources
 *
 *  Written by:
 *   David Weinstein, University of Utah, May 1999
 *   Alexei Samsonov, March 2001
 *   Frank B. Sachse, February 2006
 */

#include <Modules/Legacy/FiniteElements/ApplyFEMCurrentSource.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::FiniteElements;


namespace detail
{
class ApplyFEMCurrentSourceImpl
{
 //   GuiInt sourceNodeTCL_;
//  GuiInt sinkNodeTCL_;
 //   GuiString modeTCL_;

    bool execute_dipole(FieldHandle &hField, FieldHandle &hSource, 
                        MatrixHandle &rhs, MatrixHandle &hWeights);

    bool execute_sources_and_sinks(FieldHandle &hField, FieldHandle &hSource, 
                          MatrixHandle &hMapping, VMesh::index_type sourceNode, 
                          VMesh::index_type sinkNode, MatrixHandle& rhs);
                          
    static bool ud_pair_less(const std::pair<index_type, double> &a,
                             const std::pair<index_type, double> &b)
    {
        return a.first < b.first;
    }
};
}

ModuleLookupInfo ApplyFEMCurrentSource::staticInfo_("ApplyFEMCurrentSource", "FiniteElements", "SCIRun");

ApplyFEMCurrentSource::ApplyFEMCurrentSource() : Module(staticInfo_)
//    sourceNodeTCL_(context->subVar("sourceNodeTCL")),
//    sinkNodeTCL_(context->subVar("sinkNodeTCL")),
//    modeTCL_(context->subVar("modeTCL"))
{
  INITIALIZE_PORT(Mesh);
  INITIALIZE_PORT(Sources);
  INITIALIZE_PORT(Mapping);
  INITIALIZE_PORT(Input_RHS);
  INITIALIZE_PORT(Output_RHS);
  INITIALIZE_PORT(Output_Weights);
}

bool
detail::ApplyFEMCurrentSourceImpl::execute_dipole(FieldHandle &hField,
                                      FieldHandle &hSource, 
                                      MatrixHandle &rhs, 
                                      MatrixHandle &hWeights)
{
  if (hField.get_rep() == 0)
  {
    error("No input field");
    return (false);
  }

  VMesh* mesh = hField->vmesh();
  
  mesh->synchronize(Mesh::ELEM_LOCATE_E);
  VMesh::Node::size_type nsize = mesh->num_nodes(); 
  VMesh::Elem::size_type sz    = mesh->num_elems();

  VMesh::Node::array_type nodes;  
  
  mesh->size(sz);

  // Create new RHS matrix
  if (rhs.get_rep() == 0)
  {
    rhs = new ColumnMatrix(nsize);
    rhs->zero();
  }
  else
  {
    rhs.detach();
  } 

  FieldInformation fi(hSource);
  if (!fi.is_vector())
  {
    error("Source field does not contain vectors");
    return (false);
  }
  
  if (!(fi.is_constantdata()) && !(fi.is_lineardata()))
  {
    error("Source field needs to have vectors at the nodes or elements");
    return (false);
  }

  VMesh*  smesh = hSource->vmesh();
  VField* sfield = hSource->vfield();

  VField::size_type num_svalues = sfield->num_values();
  VMesh::Elem::index_type loc;
  VMesh::coords_type coords;
  Vector dir;
  double Ji[9];
  StackVector<double,3> grad;
  std::vector<double> dweights;
  std::vector<std::pair<index_type, double> > weights;
    
  double* rhs_data = rhs->get_data_pointer();

  for (VField::index_type idx=0; idx < num_svalues; idx++)
  {
    // Position of the dipole.
    Point pos;
    if (sfield->basis_order() == 0) smesh->get_center(pos,VMesh::Elem::index_type(idx));
    else smesh->get_center(pos,VMesh::Node::index_type(idx));    
  
    sfield->get_value(dir,idx);
  
    if (mesh->locate(loc, coords, pos))
    {
       msg_stream() << "Source pos="<<pos<<" dir="<<dir<<
         " found in elem "<<loc<<std::endl;

      if (fabs(dir.x()) > 0.000001)
      {
        weights.push_back(std::pair<index_type, double>(loc*3+0, dir.x()));
      }
      if (fabs(dir.y()) > 0.000001)
      {
        weights.push_back(std::pair<index_type, double>(loc*3+1, dir.y()));
      }
      if (fabs(dir.z()) > 0.000001)
      {
        weights.push_back(std::pair<index_type, double>(loc*3+2, dir.z()));
      }

      mesh->inverse_jacobian(coords,loc,Ji);
      mesh->get_derivate_weights(coords,dweights,1);
      mesh->get_nodes(nodes,loc);
      int numnodes = nodes.size();
    
      int dim = mesh->dimensionality();
  
      if (dim == 3)
      {
        for(int i=0; i<numnodes; i++) 
        {
          grad[0] = dweights[i]*Ji[0]+dweights[i+numnodes]*Ji[1]+dweights[i+2*numnodes]*Ji[2];
          grad[1] = dweights[i]*Ji[3]+dweights[i+numnodes]*Ji[4]+dweights[i+2*numnodes]*Ji[5];
          grad[2] = dweights[i]*Ji[6]+dweights[i+numnodes]*Ji[7]+dweights[i+2*numnodes]*Ji[8];
          
          rhs_data[nodes[i]] += grad[0]*dir.x() + grad[1]*dir.y() + grad[2]*dir.z();
        }
      }
      else if (dim == 2)
      {
        for(int i=0; i<numnodes; i++) 
        {
          grad[0] = dweights[i]*Ji[0]+dweights[i+numnodes]*Ji[1];
          grad[1] = dweights[i]*Ji[3]+dweights[i+numnodes]*Ji[4];
          grad[2] = dweights[i]*Ji[6]+dweights[i+numnodes]*Ji[7];
          
          rhs_data[nodes[i]] += grad[0]*dir.x() + grad[1]*dir.y() + grad[2]*dir.z();
        }
      }
      else if (dim == 1)
      {
        for(int i=0; i<numnodes; i++) 
        {
          grad[0] = dweights[i]*Ji[0];
          grad[1] = dweights[i]*Ji[3];
          grad[2] = dweights[i]*Ji[6];
          rhs_data[nodes[i]] += grad[0]*dir.x() + grad[1]*dir.y() + grad[2]*dir.z();
        }
      }

    }
    else
    {
      error("Dipole not located within mesh");
      return (false);
    }
  }

  SparseRowMatrix::Data sparseData(2, weights.size());
  const SparseRowMatrix::Rows& rr = sparseData.rows();
  const SparseRowMatrix::Columns& cc = sparseData.columns();
  const SparseRowMatrix::Storage& dd = sparseData.data();

  rr[0] = 0; rr[1] = static_cast<index_type>(weights.size());
  
  std::sort(weights.begin(), weights.end(), ud_pair_less);
  
  for (size_t i=0; i < weights.size(); i++)
  {
    cc[i] = weights[i].first;
    dd[i] = weights[i].second;
  }
  
  hWeights = new SparseRowMatrix(1, 3*sz, sparseData, static_cast<size_type>(weights.size()));

  return hWeights.get_rep() != 0;
}


bool
detail::ApplyFEMCurrentSourceImpl::execute_sources_and_sinks(FieldHandle &hField,
                               FieldHandle &hSource, 
                               MatrixHandle &hMapping, 
                               VMesh::index_type sourceNode, 
                               VMesh::index_type sinkNode, 
                               MatrixHandle& rhs)
{   
  if (hField.get_rep() == 0)
  {
    error("No input field");
    return (false);
  }

  VMesh* mesh = hField->vmesh();
  VMesh::Node::size_type nsize = mesh->num_nodes(); 

  // Create new RHS matrix
  if (rhs.get_rep() == 0)
  {
    rhs = new ColumnMatrix(nsize);
    rhs->zero();
  }
  else
  {
    rhs.detach();
  } 

  mesh->synchronize(Mesh::ELEM_LOCATE_E);

  // if we have an Mapping matrix and a Source field and all types are good,
  // hCurField will be valid after this block


  if (hMapping.get_rep() && hSource.get_rep())
  {
    FieldInformation fi(hSource);
    
    if (!(fi.is_scalar()))
    {
      error("Can only use a field with scalar values as source when using an Mapping matrix and a Source field -- this mode is for specifying current densities");
      return (false);
    }
    
    if (hSource->vfield()->num_values() != hMapping->nrows())
    {
      error("Source field and Mapping matrix size mismatch.");
      return (false);
    }
    if (static_cast<unsigned int>(nsize) != static_cast<unsigned int>(hMapping->ncols()))
    {
      error("Mesh field and Mapping matrix size mismatch.");
      return (false);
    }
  }

  // if we have don't have a Mapping matrix, use the source/sink indices
  // directly as volume nodes
    
  // if we do have a Mapping matrix, but we don't have a Source field,
  // then the source/sink indices refer to the PointCloud, so use the
  // Mapping matrix to get their corresponding volume node indices.
    
  // if we have a Mapping matrix AND a Source field, then ignore the
  // source/sink indices.  The Mapping matrix defines how the PointCloud
  //  nodes map to volume mesh nodes, and the Source field gives a
  // scalar quantity (current density) for each source.

  double* rhs_data = rhs->get_data_pointer();

  if (!hMapping.get_rep())
  {
    if (sourceNode >= nsize || sinkNode >= nsize)
    {
      error("SourceNode or SinkNode was out of mesh range.");
      return (false);
    }
    rhs_data[sourceNode] += -1.0;
    rhs_data[sinkNode] += 1.0;

    return (true);
  }
    
  if (hSource.get_rep() == 0)
  {
    if (sourceNode < hMapping->nrows() &&
        sinkNode < hMapping->nrows())
    {
      index_type *cc;
      double *vv;
      index_type ccsize;
      index_type ccstride;
      hMapping->getRowNonzerosNoCopy(sourceNode, ccsize, ccstride, cc, vv);
      sourceNode = cc?cc[0]:0;
      hMapping->getRowNonzerosNoCopy(sinkNode, ccsize, ccstride, cc, vv);
      sinkNode = cc?cc[0]:0;
    }
    else
    {
      error("SourceNode or SinkNode was out of mapping range.");
      return (false); 
    }
    rhs_data[sourceNode] += -1.0;
    rhs_data[sinkNode] += 1.0;

    return(true);
  }
   
  VField* pfield = hSource->vfield();   
  VMesh* pmesh = hSource->vmesh();
  VMesh::size_type num_nodes = pmesh->num_nodes();
  
  for (VMesh::index_type idx=0; idx < num_nodes; idx++)
  {
    double currentDensity;
    pfield->get_value(currentDensity, idx);

    index_type *cc;
    double *vv;
    index_type ccsize;
    index_type ccstride;

    hMapping->getRowNonzerosNoCopy(idx, ccsize, ccstride, cc, vv);

    for (size_type j=0; j < ccsize; j++)
    {
      rhs_data[cc?cc[j*ccstride]:j] += vv[j*ccstride] * currentDensity;
    }
  }
  
  return (true);
} 


void
ApplyFEMCurrentSource::execute()
{ 
  bool dipole=false;

  if (modeTCL_.get() == "dipole") dipole=true;
  else if (modeTCL_.get() == "sources and sinks") dipole=false;
  else
    error("Unreachable code, bad mode.");

  // Get the input mesh.
  FieldHandle hField, hSource;
  get_input_handle("Mesh", hField,true);
  get_input_handle("Sources", hSource, false);
	
  // If the user passed in a vector the right size, copy it into ours.

  MatrixHandle hMapping, hWeights, hRHS;
  get_input_handle("Input RHS", hRHS, false);

  if (dipole)
  {
    if(!(execute_dipole(hField, hSource, hRHS, hWeights))) return;
  }
  else 
  {
    get_input_handle("Mapping", hMapping, false);
    if(!(execute_sources_and_sinks(hField, hSource, hMapping, 
      Max(sourceNodeTCL_.get(),0), Max(sinkNodeTCL_.get(),0), hRHS))) return;
  }

  //! Sending result
  send_output_handle("Output RHS", hRHS);
  if (hWeights.get_rep()) send_output_handle("Output Weights", hWeights);
}
