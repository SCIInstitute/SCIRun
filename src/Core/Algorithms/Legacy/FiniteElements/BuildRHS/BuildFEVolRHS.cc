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


#include <Core/Algorithms/Legacy/FiniteElements/BuildRHS/BuildFEVolRHS.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Thread/Barrier.h>
#include <Core/Thread/Parallel.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
//#include <Core/Geometry/Point.h>
//#include <Core/Geometry/Tensor.h>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Logging;

class FEMVolRHSBuilder
{
 public:

  FEMVolRHSBuilder(const AlgorithmBase *algo) :
      algo_(algo), numprocessors_(Parallel::NumCores()),
      barrier_("FEMVolRHSBuilder Barrier", numprocessors_),
      mesh_(0), field_(0),
      domain_dimension(0), local_dimension_nodes(0),
      local_dimension_add_nodes(0),
      local_dimension_derivatives(0),
      local_dimension(0),
      global_dimension_nodes(0),
      global_dimension_add_nodes(0),
      global_dimension_derivatives(0),
      global_dimension(0)
    {
    }

  DenseMatrixHandle build_RHS_Vol(FieldHandle input);

  private:
  const AlgorithmBase *algo_;
  int numprocessors_;
  Barrier barrier_;

  VMesh* mesh_;
  VField *field_;

  DenseMatrixHandle rhsmatrix_;

  std::vector<bool> success_;

  index_type domain_dimension;

  index_type local_dimension_nodes;
  index_type local_dimension_add_nodes;
  index_type local_dimension_derivatives;
  index_type local_dimension;

  index_type global_dimension_nodes;
  index_type global_dimension_add_nodes;
  index_type global_dimension_derivatives;
  index_type global_dimension;

  std::vector<std::pair<std::string, Vector> > vectors_;

  std::vector<std::pair<std::string, double> > scalars_;

  // Entry point for the parallel version
  void parallel(int proc);

 private:

    void create_numerical_integration(std::vector<VMesh::coords_type > &p,
                                      std::vector<double> &w,
                                      std::vector<std::vector<double> > &d);
    bool build_local_matrix(VMesh::Elem::index_type c_ind,
                            index_type row,
                            double &l_val,
                            std::vector<VMesh::coords_type> &p,
                            std::vector<double> &w,
                            std::vector<std::vector<double> >  &d);
    bool build_local_matrix_regular(VMesh::Elem::index_type c_ind,
                                    index_type row,
                                    double &l_val,
                                    std::vector<VMesh::coords_type> &p,
                                    std::vector<double> &w,
                                    std::vector<std::vector<double> >  &d,
                                    std::vector<std::vector<double> > &precompute);
    bool setup();

};

AlgorithmInputName BuildFEVolRHSAlgo::Mesh("Mesh");
AlgorithmOutputName BuildFEVolRHSAlgo::RHS("RHS");

BuildFEVolRHSAlgo::BuildFEVolRHSAlgo()
{

}

bool FEMVolRHSBuilder::build_local_matrix(VMesh::Elem::index_type c_ind,
                   index_type row, double &l_val,
                   std::vector<VMesh::coords_type > &p,
                   std::vector<double> &w,
		   std::vector<std::vector<double> >  &d)
{
  Vector V;

  if (vectors_.size() == 0)
  {
    field_->get_value(V,c_ind);
  }
  else
  {
    int vector_index;
    field_->get_value(vector_index,c_ind);
    V = vectors_[vector_index].second;
  }

  double Ca = V[0];
  double Cb = V[1];
  double Cc = V[2];

  if ((Ca==0)&&(Cb==0)&&(Cc==0))
  {
    l_val = 0.0;
  }
  else
  {
    l_val = 0.0;

    int local_dimension2=2*local_dimension;

    // These calls are direct lookups in the base of the VMesh
    // The compiler should optimize these well
    double vol = mesh_->get_element_size();
    const int dim = mesh_->dimensionality();


    if(dim <1 || dim >3)
    {
      algo_->error("Mesh dimension is 0 or larger than 3, for which no FE implementation is available");
      return (false);
    }
    for (size_t i = 0; i < d.size(); i++)
    {
      double Ji[9];
      // Call to virtual interface, this should be one internal call
      double detJ = mesh_->inverse_jacobian(p[i],c_ind,Ji);

      // If Jacobian is negative there is a problem with the mesh
      if (detJ <= 0.0)
      {
        algo_->error("Mesh has elements with negative jacobians, check the order of the nodes that define an element");
        return (false);
      }

      // Volume associated with the local Gaussian Quadrature point:
      // weightfactor * Volume Unit element * Volume ratio (real element/unit element)
      detJ*=w[i]*vol;

      // Build local RHS value
      // Get the local derivatives of the basis functions in the basis element
      // They are all the same and are thus precomputed in matrix d
      const double *Nxi = &d[i][0];
      const double *Nyi = &d[i][local_dimension];
      const double *Nzi = &d[i][local_dimension2];
      // Gradients associated with the node we are calculating
      const double &Nxip = Nxi[row];
      const double &Nyip = Nyi[row];
      const double &Nzip = Nzi[row];
      // Calculating gradient shape function * inverse Jacobian * volume scaling factor
      const double uxp = detJ*(Nxip*Ji[0]+Nyip*Ji[1]+Nzip*Ji[2]);
      const double uyp = detJ*(Nxip*Ji[3]+Nyip*Ji[4]+Nzip*Ji[5]);
      const double uzp = detJ*(Nxip*Ji[6]+Nyip*Ji[7]+Nzip*Ji[8]);

      // Dot product with input vector :
      l_val += uxp * Ca + uyp * Cb + uzp * Cc;
    }
  }

  return (true);
}

bool FEMVolRHSBuilder::build_local_matrix_regular(VMesh::Elem::index_type c_ind,
                             index_type row, double &l_val,
                             std::vector<VMesh::coords_type> &p,
                             std::vector<double> &w,
                             std::vector<std::vector<double> >  &d,
                             std::vector<std::vector<double> > &precompute)
{
  Vector V;

  if (vectors_.size() == 0)
  {
    // Call to virtual interface. Get the vector value. Actually this call relies
    // on the automatic casting feature of the virtual interface to convert scalar
    // values into a vector.
    field_->get_value(V,c_ind);
  }
  else
  {
    int vector_index;
    field_->get_value(vector_index,c_ind);
    V = vectors_[vector_index].second;
  }

  double Ca = V[0];
  double Cb = V[1];
  double Cc = V[2];

  if ((Ca==0)&&(Cb==0)&&(Cc==0))
  {
    l_val = 0.0;
  }
  else
  {

    if (precompute.size() == 0)
    {
      precompute.resize(d.size());
      for (int m=0; m < static_cast<int>(d.size()); m++)
      {
        precompute[m].resize(10);
      }

      l_val = 0.0;

      int local_dimension2=2*local_dimension;

      double vol = mesh_->get_element_size();

      for (size_t i = 0; i < d.size(); i++)
      {
        std::vector<double>& pc = precompute[i];

        double Ji[9];
        double detJ = mesh_->inverse_jacobian(p[i],c_ind,Ji);

        // Volume elements can return negative determinants if the order of elements
        // is put in a different order
        /// @todo: It seems to be that a negative determinant is not necessarily bad,
        // we should be more flexible on thiis point
        if (detJ <= 0.0)
        {
          algo_->error("Mesh has elements with negative jacobians, check the order of the nodes that define an element");
          return (false);
        }
        // Volume associated with the local Gaussian Quadrature point:
        // weightfactor * Volume Unit element * Volume ratio (real element/unit element)
        detJ*=w[i]*vol;

        pc[0] = Ji[0];
        pc[1] = Ji[1];
        pc[2] = Ji[2];
        pc[3] = Ji[3];
        pc[4] = Ji[4];
        pc[5] = Ji[5];
        pc[6] = Ji[6];
        pc[7] = Ji[7];
        pc[8] = Ji[8];
        pc[9] = detJ;

        // Build local RHS Value
        // Get the local derivatives of the basis functions in the basis element
        // They are all the same and are thus precomputed in matrix d
        const double *Nxi = &d[i][0];
        const double *Nyi = &d[i][local_dimension];
        const double *Nzi = &d[i][local_dimension2];
        // Gradients associated with the node we are calculating
        const double &Nxip = Nxi[row];
        const double &Nyip = Nyi[row];
        const double &Nzip = Nzi[row];
        // Calculating gradient shape function * inverse Jacobian * volume scaling factor
        const double uxp = pc[9]*(Nxip*pc[0]+Nyip*pc[1]+Nzip*pc[2]);
        const double uyp = pc[9]*(Nxip*pc[3]+Nyip*pc[4]+Nzip*pc[5]);
        const double uzp = pc[9]*(Nxip*pc[6]+Nyip*pc[7]+Nzip*pc[8]);

        // Dot product with input vector :
        l_val += uxp * Ca + uyp * Cb + uzp * Cc;

      }
    }
    else
    {
      l_val = 0.0;

      int local_dimension2=2*local_dimension;

      for (size_t i = 0; i < d.size(); i++)
      {
        std::vector<double>& pc = precompute[i];

        // Build local RHS Value
        // Get the local derivatives of the basis functions in the basis element
        // They are all the same and are thus precomputed in matrix d
        const double *Nxi = &d[i][0];
        const double *Nyi = &d[i][local_dimension];
        const double *Nzi = &d[i][local_dimension2];
        // Gradients associated with the node we are calculating
        const double &Nxip = Nxi[row];
        const double &Nyip = Nyi[row];
        const double &Nzip = Nzi[row];
        // Calculating gradient shape function * inverse Jacobian * volume scaling factor
        const double uxp = pc[9]*(Nxip*pc[0]+Nyip*pc[1]+Nzip*pc[2]);
        const double uyp = pc[9]*(Nxip*pc[3]+Nyip*pc[4]+Nzip*pc[5]);
        const double uzp = pc[9]*(Nxip*pc[6]+Nyip*pc[7]+Nzip*pc[8]);

        // Dot product with input vector :
        l_val += uxp * Ca + uyp * Cb + uzp * Cc;
      }
    }
  }

  return (true);
}

void FEMVolRHSBuilder::create_numerical_integration(std::vector<VMesh::coords_type> &p,
                             std::vector<double> &w,
                             std::vector<std::vector<double> > &d)
{
  int int_basis = 1;
  if (mesh_->is_quad_element() ||
      mesh_->is_hex_element() ||
      mesh_->is_prism_element())
  {
    int_basis = 2;
  }
  mesh_->get_gaussian_scheme(p,w,int_basis);
  d.resize(p.size());
  for (size_t j=0; j<p.size();j++)
    mesh_->get_derivate_weights(p[j],d[j],1);

}

bool FEMVolRHSBuilder::setup()
{
  // The domain dimension
  domain_dimension = mesh_->dimensionality();
  if (domain_dimension < 1)
  {
    algo_->error("This mesh type cannot be used for FE computations");
    return (false);
  }

  local_dimension_nodes = mesh_->num_nodes_per_elem();
  if (field_->basis_order() == 2)
  {
    local_dimension_add_nodes = mesh_->num_enodes_per_elem();
  }
  else
  {
    local_dimension_add_nodes = 0;
  }

  local_dimension_derivatives = 0;

  // Local degrees of freedom per element
  local_dimension = local_dimension_nodes +
                    local_dimension_add_nodes +
                    local_dimension_derivatives; ///< degrees of freedom (dofs) of system

  VMesh::Node::size_type mns;
  mesh_->size(mns);
  // Number of mesh points (not necessarily number of nodes)
  global_dimension_nodes = mns;
  if (field_->basis_order() == 2)
  {
    mesh_->synchronize(Mesh::ENODES_E);
    global_dimension_add_nodes =  mesh_->num_enodes();
  }
  else
  {
    global_dimension_add_nodes = 0;
  }
  global_dimension_derivatives = 0;
  global_dimension = global_dimension_nodes+
                     global_dimension_add_nodes+
                     global_dimension_derivatives;

    if (mns > 0)
	{
		// We only need edges for the higher order basis in case of quatric lagrangian
		// Hence we should only synchronize it for this case
		if (global_dimension_add_nodes > 0)
      mesh_->synchronize(Mesh::EDGES_E|Mesh::NODE_NEIGHBORS_E);
		else
      mesh_->synchronize(Mesh::NODE_NEIGHBORS_E);
  }
	else
	{
	  success_[0] = false;
	}

  return (true);
}

void FEMVolRHSBuilder::parallel(int proc_num)
{
  success_[proc_num] = true;

  if (proc_num == 0)
  {
    try
    {
      success_[proc_num] = setup();
    }
    catch (...)
    {
      algo_->error(std::string("BuildFEVolRHS could not setup FE computation"));
      success_[proc_num] = false;
    }
  }


  barrier_.wait();

  // In case one of the threads fails, we should have them fail all
	for (int q=0; q<numprocessors_;q++)
	if (success_[q] == false) return;

	/// distributing dofs among processors
	const index_type start_gd = (global_dimension * proc_num)/numprocessors_;
	const index_type end_gd  = (global_dimension * (proc_num+1))/numprocessors_;

	VMesh::Elem::array_type ca;
	VMesh::Node::array_type na;
	VMesh::Edge::array_type ea;
	std::vector<index_type> neib_dofs;

	/// loop over system dofs for this thread
	int cnt = 0;
 	size_type size_gd = end_gd-start_gd;
     try
	{

		for (VMesh::Node::index_type i = start_gd; i<end_gd; i++)
		{
			/// check for nodes
			if (i<global_dimension_nodes)
			{
				/// get neighboring cells for node
				mesh_->get_elems(ca, i);
			}
			else if (i<global_dimension_nodes+global_dimension_add_nodes)
			{
				/// check for additional nodes at edges
				/// get neighboring cells for node
				VMesh::Edge::index_type ii(i-global_dimension_nodes);
                                mesh_->get_elems(ca,ii);
			}
			else
			{
				/// check for derivatives - to do
			}

			for(size_t j = 0; j < ca.size(); j++)
			{
				/// check for additional nodes at edges
				if (global_dimension_add_nodes)
				{
					/// get neighboring edges
					mesh_->get_edges(ea, ca[j]);
				}
			}

			if (proc_num == 0)
			{
				cnt++;
			      #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
				if (cnt == 200) { cnt = 0; algo_->update_progress(i,2*size_gd); }
			     #endif
			}
		}

	  success_[proc_num] = true;
	}
	catch (...)
	{
		algo_->error(std::string("BuildFEVolRHS crashed while mapping"));
	  success_[proc_num] = false;
	}

	std::vector<std::vector<double> > precompute;
	//index_type st = 0;

        /// check point
	barrier_.wait();

	// Bail out if one of the processes failed
	for (int q=0; q<numprocessors_;q++)
		if (success_[q] == false) return;

	try
	{
		/// the main thread makes the matrix
		if (proc_num == 0)
		{
		 rhsmatrix_ = boost::make_shared<DenseMatrix>(global_dimension,1);
		}
		success_[proc_num] = true;
	}
	catch (...)
	{
	  algo_->error(std::string("BuildVolRHS crashed while creating final output matrix"));
	  success_[proc_num] = false;
	}

        /// check point
        barrier_.wait();

	// Bail out if one of the processes failed
	for (int q=0; q<numprocessors_;q++)
		if (success_[q] == false) return;

	try
	{
		std::vector<VMesh::coords_type > ni_points;
		std::vector<double> ni_weights;
		std::vector<std::vector<double> > ni_derivatives;

		create_numerical_integration(ni_points, ni_weights, ni_derivatives);

                double l_val;

		/// loop over system dofs for this thread
		cnt = 0;

		size_gd = end_gd-start_gd;

		for (VMesh::Node::index_type i = start_gd; i<end_gd; i++)
		{

                        //zero output vector
                        (*rhsmatrix_)(i,0)=0.0;

			if (i < global_dimension_nodes)
			{
				/// check for nodes
				/// get neighboring cells for node
				mesh_->get_elems(ca,i);
			}
			else if (i < global_dimension_nodes + global_dimension_add_nodes)
			{
				/// check for additional nodes at edges
				/// get neighboring cells for additional nodes
				VMesh::Edge::index_type ii(i-global_dimension_nodes);
				mesh_->get_elems(ca,ii);
			}
			else
			{
				/// check for derivatives - to do
			}

			/// loop over elements attributed elements

			if (mesh_->is_regularmesh())
			{

				for (size_t j = 0; j < ca.size(); j++)
				{
					mesh_->get_nodes(na, ca[j]); ///< get neighboring nodes
                                    for(size_t k = 0; k < na.size(); k++)
					{
						if (na[k] == i)
						{
					          build_local_matrix_regular(ca[j], k , l_val, ni_points, ni_weights, ni_derivatives,precompute);
                                                  (*rhsmatrix_)(i,0)=(*rhsmatrix_)(i,0)+l_val;
						}
					}
				}
			}
			else
			{

				for (size_t j = 0; j < ca.size(); j++)
				{
					mesh_->get_nodes(na, ca[j]); ///< get neighboring nodes

					/// check for additional nodes at edges
					if (global_dimension_add_nodes)
					{
					    mesh_->get_edges(ea, ca[j]); ///< get neighboring edges
					}

					//ASSERT(static_cast<int>(neib_dofs.size()) == local_dimension);
					for(size_t k = 0; k < na.size(); k++)
					{
						if (na[k] == i)
						{
					            build_local_matrix(ca[j], k , l_val, ni_points, ni_weights, ni_derivatives);
                                                    (*rhsmatrix_)(i,0)=(*rhsmatrix_)(i,0)+l_val;  //rhsmatrix_->add(i, 0, l_val);
						}
					}

					if (global_dimension_add_nodes)
					{
						for(size_t k = 0; k < ea.size(); k++)
						{
							if (global_dimension + static_cast<int>(ea[k]) == i)
							{
							     build_local_matrix(ca[j], k+na.size() , l_val, ni_points, ni_weights, ni_derivatives);
                                                             (*rhsmatrix_)(i,0)=(*rhsmatrix_)(i,0)+l_val;
							}
						}
					}
				}
			}

			if (proc_num == 0)
			{
				cnt++;
				//if (cnt == 200) { cnt = 0; algo_->update_progress(i+size_gd,2*size_gd); }
			}
		}

		success_[proc_num] = true;

	}
	catch (...)
	{
	  algo_->error(std::string("BuildFEVolRHS crashed while filling out output matrix"));
	  success_[proc_num] = false;
	}

        barrier_.wait();

	// Bail out if one of the processes failed
	for (int q=0; q<numprocessors_;q++)
		if (success_[q] == false) return;
}


DenseMatrixHandle FEMVolRHSBuilder::build_RHS_Vol(FieldHandle input)
{
  // Get virtual interface to data
  field_ = input->vfield();
  mesh_  = input->vmesh();

 #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  // We added a second system of adding a vector table, using a matrix
  // Convert that matrix into the vector table
  if (vtable)
  {
    vectors_.clear();
    DenseMatrix* mat = vtable->dense();
    MatrixHandle temphandle = mat;
    // Only if we can convert it into a dense matrix, otherwise skip it
    if (mat)
    {
      double* data = mat->get_data_pointer();
      size_type m = mat->nrows();
      size_type n = mat->ncols();
      Vector V;

      // Case the table has isotropic values
      if (mat->ncols() == 1)
      {
        for (size_type p=0; p<m;p++)
        {
          V[0] = data[p*n+0];
          V[1] = data[p*n+0];
          V[2] = data[p*n+0];

          vectors_.push_back(std::pair<std::string, Vector>("",V));
        }
      }
      else if (mat->ncols() == 3)
      {
        for (size_type p=0; p<m;p++)
        {
          V[0] = data[0+p*n];
          V[1] = data[1+p*n];
          V[2] = data[2+p*n];

          vectors_.push_back(std::pair<std::string, Vector>("",V));
        }
      }

    }
  }
  #endif

  success_.resize(numprocessors_,true);

  // Start the multi threaded FEMVolRHS builder.
  Parallel::RunTasks([this](int i) { parallel(i); }, numprocessors_);

  for (size_t j=0; j<success_.size(); j++)
  {
    if (success_[j] == false) return nullptr;
  }

  return rhsmatrix_;
}

DenseMatrixHandle BuildFEVolRHSAlgo::run(FieldHandle input) const
{

 if (!input)
 {
    THROW_ALGORITHM_INPUT_ERROR("Could not obtain input field");
 }

 if (!input->vfield()->is_vector())
 {
    THROW_ALGORITHM_INPUT_ERROR("This function is only defined for elements with vector data");
 }

 if (input->vfield()->basis_order()!=0)
 {
    THROW_ALGORITHM_INPUT_ERROR("This function has only been defined for data that is located at the elements");
 }

 #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 if (ctable)
 {
    if ((ctable->ncols() != 1)&&(ctable->ncols() != 3))
    {
      THROW_ALGORITHM_INPUT_ERROR("Vector table needs to have 1 or 3");

    }
    if (ctable->nrows() == 0)
    {
      THROW_ALGORITHM_INPUT_ERROR("Vector table is empty");
    }
  }
 #endif

 LOG_DEBUG(" Note: The original SCIRun4 module looked for a field attribute ''conductivity_table'' of the second module input which could only be set outside of SCIRun4. This function is not available in SCIRun5. ");

 FEMVolRHSBuilder builder(this);

 #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   if (get_bool("generate_basis"))
  {
    BuildFEVolRHSPrivateData* privatedata;
    get_privatedata(privatedata);


    if (vtable.get_rep()==0)
    {
      std::vector<std::pair<std::string,Tensor> > tens;

      input->get_property("conductivity_table",tens);

      if (tens.size() > 0)
      {
        vtable = new DenseMatrix(tens.size(),1);
        double* data = vtable->get_data_pointer();
        for (size_t i=0; i<tens.size();i++)
        {
          double t = tens[i].second.mat_[0][0];
          data[i] = t;
        }
      }
    }

    if (vtable.get_rep())
    {
      size_type nconds = vtable->nrows();
      if ((input->vmesh()->generation() != privatedata->generation_)||
          (privatedata->basis_fevolrhs_.get_rep()==0))
      {
        MatrixHandle con = new DenseMatrix(nconds,1);
        double* data = con->get_data_pointer();
        for (size_type i=0; i<nconds;i++) data[i] = 0.0;
        builder->build_RHS_Vol(input,con,privatedata->basis_fevolrhs_);
        if (privatedata->basis_fevolrhs_.get_rep() == 0)
        {
          error("Failed to build FEVolRHS structure");
          algo_end(); return(false);
        }
        privatedata->basis_values_.resize(nconds);
        for (size_type s=0; s< nconds; s++)
        {
          MatrixHandle temp;
          data[s] = 1.0;
          builder->build_RHS_Vol(input,con,temp);
          if (temp.get_rep() == 0)
          {
            error("Failed to build FE component for one of the tissue types");
            algo_end(); return(false);
          }

          SparseRowMatrix *m = temp->sparse();
          privatedata->basis_values_[s].resize(m->get_nnz());
          for (size_type p=0; p< m->get_nnz(); p++)
          {
            privatedata->basis_values_[s][p] = m->get_value(p);
          }
          data[s] = 0.0;
        }

        privatedata->generation_ = input->vmesh()->generation();
      }

      output = privatedata->basis_fevolrhs_;
      output.detach();

      SparseRowMatrix *m = output->sparse();
      double *sum = m->get_vals();
      double *cdata = vtable->get_data_pointer();
      size_type n = vtable->ncols();

      if (privatedata->basis_values_.size() > 0)
        for (size_t p=0; p < privatedata->basis_values_[0].size(); p++) sum[p] = 0.0;

      for (int s=0; s<nconds; s++)
      {
        double weight = cdata[s*n];
        for (size_t p=0; p < privatedata->basis_values_[s].size(); p++)
        {
          sum[p] += weight * privatedata->basis_values_[s][p];
        }
      }

    }
    else
    {
      error("No vector table present: The generate_basis option only works for indexed vectors");
      return (false);
    }
  }
 #endif

  DenseMatrixHandle output = builder.build_RHS_Vol(input);

  if (!output)
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not build output matrix");
  }

 return output;
}

AlgorithmOutput BuildFEVolRHSAlgo::run(const AlgorithmInput& input) const
{
  auto mesh = input.get<Field>(Mesh);

 #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  auto ctable = input.get<DenseMatrix>(Vector_Table);
 #endif

  DenseMatrixHandle volrhs = run(mesh);

  AlgorithmOutput output;
  output[RHS] = volrhs;
  return output;
}
