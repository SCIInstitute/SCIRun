/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2011 Scientific Computing and Imaging Institute,
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
 *  BuildBEMatrix.cc: 
 *
 *  Written by:
 *   Saeed Babaeizadeh - Northeastern University
 *   Michael Callahan - Department of Computer Science - University of Utah
 *   May, 2003
 *
 *  Updated by:
 *   Burak Erem - Northeastern University
 *   January, 2012
 *
 */

//#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystem.h>

#include <Modules/Legacy/Forward/BuildBEMatrix.h>
#include <Core/Algorithms/Legacy/Forward/BuildBEMatrixAlgo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/TriSurfMesh.h>

#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Math/MiscMath.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>

using namespace SCIRun;
using namespace SCIRun::Modules::Forward;

namespace balgo=boost::algorithm;

namespace detail {

enum BEM_ALGO_TYPE {
  UNSUPPORTED = -1,
  SURFACE_AND_POINTS = 1,
  SURFACES_TO_SURFACES
};

// Unfortunately, there is no clean way to map this to TCL.
// See method ui for code that sets up radiobuttons
// in BuildBEMatrix.tcl.
enum GUI_SURFACE_TYPE {
  SOURCE = 0,
  MEASUREMENT
};

// TODO: lets use intercap for class name when it's moved to an algorithm
//
// initial conditions: first should be measurement, rest source

  

class BuildBEMatrix : public Module, public BuildBEMatrixBase
{
public:
    
  virtual void execute();
  
private:
  // BURAK EDITS:
  void algoSurfaceToNodes();
  void algoSurfacesToSurfaces();
  int detectBEMalgo();
  
  DenseMatrixHandle TransferMatrix;

  MatrixHandle hZoi_;
  int old_nodes_generation_;
  int old_surface_generation_;
  bool process_gui_vars_;

  GuiString guiselected_field_;
  GuiString guiselected_type_;

  GuiString guifields_;
  GuiString guifield_type_property_;
  GuiString guifield_inside_cond_property_;
  GuiString guifield_outside_cond_property_;
  GuiString guifield_surface_type_property_;

  bemfield_vector fields_;
};

// TODO: move into class?
std::vector<int> field_generation_no_old, old_nesting;
std::vector<double> old_conductivities;

BuildBEMatrix::BuildBEMatrix(GuiContext *context):
  Module("BuildBEMatrix", context, Source, "Forward", "BioPSE"),
  old_nodes_generation_( -1 ),
  old_surface_generation_( -1 ),
  process_gui_vars_(false),
  guiselected_field_(get_ctx()->subVar("selected_field", false), ""),
  guiselected_type_(get_ctx()->subVar("selected_field_type", false), ""),
  guifields_(get_ctx()->subVar("input_field_list"), ""),
  guifield_type_property_(get_ctx()->subVar("field_type_list"), ""),
  guifield_inside_cond_property_(get_ctx()->subVar("inside_cond_list"), ""),
  guifield_outside_cond_property_(get_ctx()->subVar("outside_cond_list"), ""),
  guifield_surface_type_property_(get_ctx()->subVar("surface_type_list"), "")
{}

// BURAK EDITS:
void
BuildBEMatrixImpl::algoSurfaceToNodes()
{
  // NOTE: This is Jeroen's code that has been adapted to fit the new module structure
  //
  // Math:
  // The boundary element formulation is based on Matlab code
  // bemMatrixPP2.m, which can be found in the matlab package
  
  // The BEM formulation assumes the following matrix equations
  // P_surf_surf * PHI_surf + G_surf_surf * J_surf =  sources_in_volume
  //
  // PHI_surf are the potentials on the surface
  // J_surf are the currents passing perpendicular to the surface
  // sources_in_volume is empty in this case
  //
  // P_surf_surf is the matrix that connects the potentials at the nodes to the integral over the 
  // potential at the surface. Its terms consist of Green's function ( 1/ ( 4pi*||r-r'|| ) ) over
  // the surface of each element. As this integral becomes singular for a node and a triangle that
  // share a corner node, we use a trick to avoid computing this integral as we know that the
  // the system should reference potential invariant. Hence the rows of the matrix need to sum to
  // to zero
  //
  // G_surf_surf is the matrix that connects the potentials at the nodes to the integral over the
  // currents flowing through the surface. 
  //
  // The second equation that we use is the expression of the potentials at an arbitrary point
  // to the potentials at the surface and the current flowing through the surface
  //
  // PHI_nodes = P_nodes_surf * PHI_surf + G_nodes_surf * J_surf
  //
  // Here matrix P_nodes_surf is the matrix that projects the contribution of the potentials of the
  // surface to the nodes within the volume
  //
  // Here G_nodes_surf is the matrix that projects the contribution of the currents flowing through
  // the surface to the nodes within the volume
  //
  // Adding both equations together will result in
  //
  // PHI_nodes = P_nodes_surf* PHI_surf - G_nodes_surf * inv( G_surf_surf) * P_surf_surf * PHI_surf
  //
  // In other words the transfer matrix is
  // P_nodes_surf - G_nodes_surf * inv( G_surf_surf) * P_surf_surf
  
  VMesh *nodes = 0;
  VMesh *surface = 0;
  
  for (int i=0; i<2; i++)
  {
    if (this->fields_[i].surface)
      surface = this->fields_[i].field_->vmesh();
    else
      nodes = this->fields_[i].field_->vmesh();
  }
  
  DenseMatrixHandle Pss;
  DenseMatrixHandle Gss;
  DenseMatrixHandle Pns;
  DenseMatrixHandle Gns;
  make_auto_P( surface, Pss, 1.0, 0.0, 1.0 );
  make_cross_P( nodes, surface, Pns, 1.0, 0.0, 1.0 );
  
  std::vector<double> area;
  pre_calc_tri_areas( surface, area );
  
  make_auto_G( surface, Gss, 1.0, 0.0, 1.0, area );
  make_cross_G( nodes, surface, Gns, 1.0, 0.0, 1.0, area );
  
  Gss->invert();
  
  MatrixHandle mPns = Pns.get_rep();
  MatrixHandle mGns = Gns.get_rep();
  MatrixHandle mGss = Gss.get_rep();
  MatrixHandle mPss = Pss.get_rep();
  
  TransferMatrix = (mPns - mGns * mGss * mPss)->dense();
}

void
BuildBEMatrixImpl::algoSurfacesToSurfaces()
{
  // Math for surface-to-surface BEM algorithm (based on Jeroen Stinstra's BEM Matlab code that's part of SCIRun)
  // -------------------------------------------------------------------------------------------------------------
  // EE = matrix relating potentials on surfaces to potentials on other surfaces
  // EJ = matrix relating current density on surfaces (normal to surface) to potentials on surfaces
  // u  = potentials on the surfaces
  // j  = current density normal to the surfaces
  // 
  // General equation: EE*u + EJ*j = (dipolar sources not on the surfaces)
  // Assuming all sources are on surfaces: EE*u + EJ*j = 0
  // (below assumes that measurement=Neumann boundary conditions and source=Dirichlet boundary conditions)
  // 
  // s = source indices
  // m = measurement indices
  // 
  // Pmm = EE(m,m)
  // Pss = EE(s,s)
  // Pms = EE(m,s)
  // Psm = EE(s,m)
  //
  // Gms = EJ(m,s)
  // Gss = EJ(s,s)
  // 
  // After some block-matrix math to eliminate j from the equation and find T s.t. u(m)=T*u(s), we get:
  // iGss = inv(Gss)
  // T = inv(Pmm - Gms*iGss*Psm)*(Gms*iGss*Pss - Pms)
  // 
  
  const int Nfields = this->fields_.size();
  double op_cond=0.0; // op_cond is not used in this formulation -- someone needs to check this math and make a better decision about how to handle this value below
  
  // Count the number of fields that have been specified as being "sources" or "measurements" (and keep track of indices)
  int Nsources = 0;
  std::vector<int> sourcefieldindices;
  int Nmeasurements = 0;
  std::vector<int> measurementfieldindices;
  
  for(int i=0; i < Nfields; i++)
  {
    if(this->fields_[i].source)
	{
		Nsources++;
		sourcefieldindices.push_back(i);
	}
	else if(this->fields_[i].measurement)
	{
		Nmeasurements++;
		measurementfieldindices.push_back(i);
	}
  }
  
  BlockMatrix EE(Nfields, Nfields);
  BlockMatrix EJ(Nfields, Nsources);
  DenseMatrixHandle tempblockelement;
  
  // Calculate EE in block matrix form
  for(int i = 0; i < Nfields; i++)
  {
	  for(int j = 0; j < Nfields; j++)
	  {
		  if (i == j)
			  BuildBEMatrix::make_auto_P(this->fields_[i].field_->vmesh(), tempblockelement, this->fields_[i].insideconductivity, this->fields_[i].outsideconductivity, op_cond);
		  else
			  BuildBEMatrix::make_cross_P(this->fields_[i].field_->vmesh(), this->fields_[j].field_->vmesh(), tempblockelement, this->fields_[i].insideconductivity, this->fields_[i].outsideconductivity, op_cond);
  			
		  EE(i,j) = *tempblockelement;
	  }
  }
  
  // Calculate EJ(:,s) in block matrix form
  // ***NOTE THE CHANGE IN INDEXING!!!***
  // (The indices of block columns of EJ correspond to field indices according to "sourcefieldindices", and this affects everything with EJ below this point too!)
  for(int j = 0; j < Nsources; j++)
  {
	  // Precalculate triangle areas for this source field/surface
	  std::vector<double> temptriangleareas;
      BuildBEMatrix::pre_calc_tri_areas(this->fields_[sourcefieldindices[j]].field_->vmesh(), temptriangleareas);

      for(int i = 0; i < Nfields; i++)
	  {
		  if (i == sourcefieldindices[j])
			  BuildBEMatrix::make_auto_G(this->fields_[i].field_->vmesh(), tempblockelement, this->fields_[i].insideconductivity, this->fields_[i].outsideconductivity, op_cond, temptriangleareas);
		  else
			  BuildBEMatrix::make_cross_G(this->fields_[i].field_->vmesh(), this->fields_[sourcefieldindices[j]].field_->vmesh(), tempblockelement, this->fields_[i].insideconductivity, this->fields_[i].outsideconductivity, op_cond, temptriangleareas);
  			
		  EJ(i,j) = *tempblockelement;
	  }
  }
  
  // Perform deflation on EE matrix
  double deflationconstant = 1/((EE.to_dense())->ncols()); // 1/(# cols of EE)
  BlockMatrix deflationmatrix(Nfields, Nfields);
  
  for(int i = 0; i < Nfields; i++)
  {
	  for(int j = 0; j < Nfields; j++)
	  {
		  tempblockelement = EE(i,j);
		  deflationmatrix(i,j) = DenseMatrix(tempblockelement->nrows(), tempblockelement->ncols(), deflationconstant);
	  }
  }
  
  EE = EE + deflationmatrix;
  
  
  // Split EE apart into Pmm, Pss, Pms, and Psm
  // -----------------------------------------------
  // Pmm:
  BlockMatrix BlockPmm(Nmeasurements, Nmeasurements);
  for(int i = 0; i < Nmeasurements; i++)
  {
    for(int j = 0; j < Nmeasurements; j++)
    {
      BlockPmm(i,j) = EE(measurementfieldindices[i],measurementfieldindices[j]);
    }
  }
  DenseMatrixHandle Pmm = BlockPmm.to_dense();
  
  // Pss:
  BlockMatrix BlockPss(Nsources, Nsources);
  for(int i = 0; i < Nsources; i++)
  {
    for(int j = 0; j < Nsources; j++)
    {
      BlockPss(i,j)=EE(sourcefieldindices[i],sourcefieldindices[j]);
    }
  }
  DenseMatrixHandle Pss = BlockPss.to_dense();
  
  // Pms:
  BlockMatrix BlockPms(Nmeasurements, Nsources);
  for(int i = 0; i < Nmeasurements; i++)
  {
    for(int j = 0; j < Nsources; j++)
    {
      BlockPms(i,j)=EE(measurementfieldindices[i],sourcefieldindices[j]);
    }
  }
  DenseMatrixHandle Pms = BlockPms.to_dense();
  
  // Psm:
  BlockMatrix BlockPsm(Nsources, Nmeasurements);
  for(int i = 0; i < Nsources; i++)
  {
    for(int j = 0; j < Nmeasurements; j++)
    {
      BlockPsm(i,j)=EE(sourcefieldindices[i],measurementfieldindices[j]);
    }
  }
  DenseMatrixHandle Psm = BlockPsm.to_dense();
  
  // Split EJ apart into Gms and Gss (see ALL-CAPS note above about differences in block row vs column indexing in EJ matrix)
  // -----------------------------------------------
  // Gms:
  BlockMatrix BlockGms(Nmeasurements, Nsources);
  for(int i = 0; i < Nmeasurements; i++)
  {
    for(int j = 0; j < Nsources; j++)
    {
      BlockGms(i,j)=EJ(measurementfieldindices[i],j);
    }
  }
  DenseMatrixHandle Gms = BlockGms.to_dense();
  
  // Gss:
  BlockMatrix BlockGss(Nsources, Nsources);
  for(int i = 0; i < Nsources; i++)
  {
    for(int j = 0; j < Nsources; j++)
    {
      BlockGss(i,j) = EJ(sourcefieldindices[i],j);
    }
  }
  DenseMatrixHandle Gss = BlockGss.to_dense();
  
  // TODO: add deflation step
  
  // Compute T here (see math in comments above)
  // TransferMatrix = T = inv(Pmm - Gms*iGss*Psm)*(Gms*iGss*Pss - Pms) = inv(C)*D
  Gss->invert(); // iGss = inv(Gss)

  MatrixHandle Y = Gms * Gss;
  MatrixHandle C = Pmm - Y * Psm; 
  MatrixHandle D = Y * Pss - Pms; 

  C->invert();
  MatrixHandle T = C * D; // T = inv(C)*D
  TransferMatrix = T->dense();

  //This could be done on one line (see below), but Y (see above) would need to be calculated twice:
  //MatrixHandle TransferMatrix1 = inv(Pmm - Gms * Gss * Psm) * (Gms * Gss * Pss - Pms);
  
}


void
BuildBEMatrix::execute()
{
  std::vector<FieldHandle> inputs;
  get_dynamic_input_handles("Surface", inputs, true);
  
  const std::size_t INPUTS_LEN = inputs.size(), INPUT_LEN_LAST_INDEX = INPUTS_LEN - 1;    
 
  if (this->inputs_changed_)
  {
    this->fields_.clear();
    this->fields_.reserve(INPUTS_LEN);

    for (std::vector<FieldHandle>::size_type i = 0; i < INPUTS_LEN; ++i)
    {
      FieldHandle field_handle = inputs[i];
      // TODO: from old code - needed?
      field_handle.detach();
      field_handle->mesh_detach();
      
      bemfield field(field_handle);
      
      // don't think the strings need to be quoted, but if they do,
      // quote using {}
      fieldlist << "field" << i;
      
      // setting field type
      VMesh* vmesh = inputs[i]->vmesh();
      if (vmesh->is_trisurfmesh())
      {
        field.surface = true;
        fieldtype << "surface";
      }
      else if (vmesh->is_pointcloudmesh())
      {
        // probably redundant...
        field.surface = false;
        fieldtype << "points";
      }
      else
      {
        // unsupported field types
        warning("Input field in not either a TriSurf mesh or a PointCloud.");
        fieldtype << "unknown";
      }

      this->fields_.push_back(field);
    }
  }
  
  if (true)
  {
   

          this->fields_[i].insideconductivity = boost::lexical_cast<double>(split_vector[i]);
  
          this->fields_[i].outsideconductivity = boost::lexical_cast<double>(split_vector[i]);
  
    

          int surface_type = boost::lexical_cast<int>(split_vector[i]);
          if (surface_type == SOURCE)
          {
            this->fields_[i].set_source_dirichlet();
          }
          else // measurement
          {
            this->fields_[i].set_measuremen_neumann();
          }
    
  }

  // The specific BEM routine (2 so far) to be called is dependent on the inputs in the fields vector,
  // so we check for the conditions and call the appropriate routine:
  int BEMalgo = detectBEMalgo();

  switch(BEMalgo)
  {
  case UNSUPPORTED:
    // We don't support the inputs and detectBEMalgo() should have reported the appropriate error, so just return
    //
    // TODO: error message needs improvement
    error("The combinations of input properties is not supported. Please see documentation for supported input field options.");
    return;

  case SURFACE_AND_POINTS:
    // BEMalgo == 1 means we've detected the special case of two fields with a surface outside and points inside
    algoSurfaceToNodes();
	break;

  case SURFACES_TO_SURFACES:
    // BEMalgo == 2 means we've detected the surfaces to surfaces case
    algoSurfacesToSurfaces();
	break;

  default:
    // Should never happen, but return with an error
    error("Unable to detect the appropriate algorithm and the reason is unknown. Please report this error to the developers.");
    return;
  }

  // The BEM algorithms populate a matrix with handle "TransferMatrix". At this point all we have to do is send it as output.
  send_output_handle("BEM Forward Matrix", TransferMatrix);

}
