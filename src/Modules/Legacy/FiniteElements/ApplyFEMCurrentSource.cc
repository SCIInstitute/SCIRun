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
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Logging/Log.h>
#include <spdlog/fmt/ostr.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::FiniteElements;


namespace detail
{
  class ApplyFEMCurrentSourceImpl
  {
  public:
    explicit ApplyFEMCurrentSourceImpl(LegacyLoggerInterface* log) : log_(log) {}

    bool execute_dipole(FieldHandle field,
      FieldHandle source,
      DenseColumnMatrixHandle& rhs,
      SparseRowMatrixHandle& weights);

    bool execute_sources_and_sinks(FieldHandle field,
      FieldHandle source,
      VMesh::index_type sourceNode,
      VMesh::index_type sinkNode,
      DenseColumnMatrixHandle& rhs,
      MatrixHandle mapping);

    static bool ud_pair_less(const std::pair<index_type, double> &a,
      const std::pair<index_type, double> &b)
    {
      return a.first < b.first;
    }

  private:
    LegacyLoggerInterface* log_;
  };
}

MODULE_INFO_DEF(ApplyFEMCurrentSource, FiniteElements, SCIRun)
const AlgorithmParameterName ApplyFEMCurrentSource::SourceNode("SourceNode");
const AlgorithmParameterName ApplyFEMCurrentSource::SinkNode("SinkNode");
const AlgorithmParameterName ApplyFEMCurrentSource::ModelType("ModelType");

ApplyFEMCurrentSource::ApplyFEMCurrentSource() : Module(staticInfo_)
{
  INITIALIZE_PORT(Mesh);
  INITIALIZE_PORT(Sources);
  INITIALIZE_PORT(Mapping);
  INITIALIZE_PORT(Input_RHS);
  INITIALIZE_PORT(Output_RHS);
  INITIALIZE_PORT(Output_Weights);
}

void ApplyFEMCurrentSource::setStateDefaults()
{
  auto state = get_state();
  state->setValue(SourceNode, 0);
  state->setValue(SinkNode, 1);
  state->setValue(ModelType, std::string("dipole"));
}

bool detail::ApplyFEMCurrentSourceImpl::execute_dipole(FieldHandle field,
  FieldHandle source,
  DenseColumnMatrixHandle& rhs,
  SparseRowMatrixHandle& weights)
{
  if (!field)
  {
    log_->error("No input field");
    return (false);
  }

  if (!source)
  {
    log_->error("No source field");
    return (false);
  }

  VMesh* mesh = field->vmesh();

  mesh->synchronize(Mesh::ELEM_LOCATE_E);
  VMesh::Node::size_type nsize = mesh->num_nodes();
  VMesh::Elem::size_type sz    = mesh->num_elems();

  VMesh::Node::array_type nodes;

  mesh->size(sz);

  // Create new RHS matrix
  if (!rhs)
  {
    rhs.reset(new DenseColumnMatrix(nsize));
    rhs->setZero();
  }
  else
  {
    rhs.reset(rhs->clone());
  }

  FieldInformation fi(source);
  if (!fi.is_vector())
  {
    log_->error("Source field does not contain vectors");
    return (false);
  }

  if (!(fi.is_constantdata()) && !(fi.is_lineardata()))
  {
    log_->error("Source field needs to have vectors at the nodes or elements");
    return (false);
  }

  VMesh*  smesh = source->vmesh();
  VField* sfield = source->vfield();

  VField::size_type num_svalues = sfield->num_values();
  VMesh::Elem::index_type loc;
  VMesh::coords_type coords;
  Vector dir;
  double Ji[9];
  StackVector<double,3> grad;
  std::vector<double> dweights;
  std::vector<std::pair<index_type, double> > weightsVec;

  double* rhs_data = rhs->data();

  for (VField::index_type idx=0; idx < num_svalues; idx++)
  {
    // Position of the dipole.
    Point pos;
    if (sfield->basis_order() == 0) smesh->get_center(pos,VMesh::Elem::index_type(idx));
    else smesh->get_center(pos,VMesh::Node::index_type(idx));

    sfield->get_value(dir,idx);

    if (mesh->locate(loc, coords, pos))
    {
      LOG_DEBUG("Source pos={} dir={} found in elem {}", pos, dir, loc);

      if (fabs(dir.x()) > 0.000001)
      {
        weightsVec.push_back(std::make_pair(loc*3+0, dir.x()));
      }
      if (fabs(dir.y()) > 0.000001)
      {
        weightsVec.push_back(std::make_pair(loc*3+1, dir.y()));
      }
      if (fabs(dir.z()) > 0.000001)
      {
        weightsVec.push_back(std::make_pair(loc*3+2, dir.z()));
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
      log_->error("Dipole not located within mesh");
      return (false);
    }
  }

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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

  return hWeights != nullptr;
#endif
  return true;
}


bool detail::ApplyFEMCurrentSourceImpl::execute_sources_and_sinks(FieldHandle field,
  FieldHandle source,
  VMesh::index_type sourceNode,
  VMesh::index_type sinkNode,
  DenseColumnMatrixHandle& rhs,
  MatrixHandle mapping)
{
  if (!field)
  {
    log_->error("No input field");
    return (false);
  }

  VMesh* mesh = field->vmesh();
  VMesh::Node::size_type nsize = mesh->num_nodes();

  // Create new RHS matrix
  if (!rhs)
  {
    rhs.reset(new DenseColumnMatrix(nsize));
    rhs->setZero();
  }
  else
  {
    rhs.reset(rhs->clone());
  }

  mesh->synchronize(Mesh::ELEM_LOCATE_E);

  // if we have an Mapping matrix and a Source field and all types are good,
  // hCurField will be valid after this block


  if (mapping && source)
  {
    FieldInformation fi(source);

    if (!(fi.is_scalar()))
    {
      log_->error("Can only use a field with scalar values as source when using an Mapping matrix and a Source field -- this mode is for specifying current densities");
      return (false);
    }

    if (source->vfield()->num_values() != mapping->nrows())
    {
      log_->error("Source field and Mapping matrix size mismatch.");
      return (false);
    }
    if (static_cast<unsigned int>(nsize) != static_cast<unsigned int>(mapping->ncols()))
    {
      log_->error("Mesh field and Mapping matrix size mismatch.");
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

  double* rhs_data = rhs->data();

  if (!mapping || !matrixIs::sparse(mapping))
  {
    if (sourceNode >= nsize || sinkNode >= nsize)
    {
      log_->error("SourceNode or SinkNode was out of mesh range.");
      return (false);
    }
    rhs_data[sourceNode] += -1.0;
    rhs_data[sinkNode] += 1.0;

    return (true);
  }

  SparseRowMatrixHandle sparseMapping = castMatrix::toSparse(mapping);

  if (!source)
  {
    if (sourceNode < mapping->nrows() &&
      sinkNode < mapping->nrows())
    {
      index_type *cc;
      double *vv;
      index_type ccsize;
      sparseMapping->getRowNonzerosNoCopy(sourceNode, ccsize, cc, vv);
      sourceNode = cc?cc[0]:0;
      sparseMapping->getRowNonzerosNoCopy(sinkNode, ccsize, cc, vv);
      sinkNode = cc?cc[0]:0;
    }
    else
    {
      log_->error("SourceNode or SinkNode was out of mapping range.");
      return (false);
    }
    rhs_data[sourceNode] += -1.0;
    rhs_data[sinkNode] += 1.0;

    return(true);
  }

  VField* pfield = source->vfield();
  VMesh* pmesh = source->vmesh();
  VMesh::size_type num_nodes = pmesh->num_nodes();

  for (VMesh::index_type idx=0; idx < num_nodes; idx++)
  {
    double currentDensity;
    pfield->get_value(currentDensity, idx);

    index_type *cc;
    double *vv;
    index_type ccsize;

    sparseMapping->getRowNonzerosNoCopy(idx, ccsize, cc, vv);

    for (size_type j=0; j < ccsize; j++)
    {
      rhs_data[cc?cc[j]:j] += vv[j] * currentDensity;
    }
  }

  return (true);
}


void ApplyFEMCurrentSource::execute()
{
  auto state = get_state();
  auto modelType = state->getValue(ModelType).toString();
  bool dipole = (modelType == "dipole");

  auto field = getRequiredInput(Mesh);
  auto sourceOption = getOptionalInput(Sources);

  SparseRowMatrixHandle weights;
  DenseColumnMatrixHandle RHS;
  auto RHSoption = getOptionalInput(Input_RHS);
  if (RHSoption)
    RHS = castMatrix::toColumn(*RHSoption);

  if (needToExecute())
  {
    detail::ApplyFEMCurrentSourceImpl impl(this);
    if (dipole)
    {
      if (!impl.execute_dipole(field, sourceOption.get_value_or(nullptr), RHS, weights))
        return;
    }
    else
    {
      auto mapping = getOptionalInput(Mapping);
      if(!impl.execute_sources_and_sinks(field,
        sourceOption.get_value_or(nullptr),
        state->getValue(SourceNode).toInt(),
        state->getValue(SinkNode).toInt(),
        RHS, mapping.get_value_or(nullptr)))
        return;
    }

    sendOutput(Output_RHS, RHS);
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (weights)
      sendOutput(Output_Weights, weights);
#endif
  }
}
