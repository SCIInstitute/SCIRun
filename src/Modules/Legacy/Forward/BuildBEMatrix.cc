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
class bemfield
{
public:
	explicit bemfield(const FieldHandle& fieldHandle) :
    field_(fieldHandle),
    insideconductivity(0),
    outsideconductivity(0),
    surface(false),
    neumann(false),
    measurement(false),
    dirichlet(false),
    source(false) {}
  
  void set_source_dirichlet()
  {
    this->source = true;
    this->dirichlet = true;
    this->measurement = false;
    this->neumann = false;
  }

  void set_measuremen_neumann()
  {
    this->source = false;
    this->dirichlet = false;
    this->measurement = true;
    this->neumann = true;
  }

	FieldHandle field_; // handle of the field itself
	double insideconductivity; // if it applies, the conductivity inside the surface
	double outsideconductivity; // if it applies, the conductivity outside the surface
	bool surface; // true if a surface, false if just points
  // TODO: setters? should change neumann and dirichlet based on measurement and source?
  // BURAK ANSWER: Yes, for the time being they are directly connected. This structure allows someone to introduce a new algorithm with very few modifications to the module design.
	bool neumann; // true if Neumann boundary conditions are defined on this surface
	bool measurement; // true if a measurement field
	bool dirichlet; // true if Dirichlet boundary conditions are defined on this surface
	bool source; // true if a source field
};
  
typedef std::vector<bemfield> bemfield_vector;

class BuildBEMatrix : public Module, public BuildBEMatrixBase
{
public:
  
  //! Constructor
  BuildBEMatrix(GuiContext *context);
  
  //! Destructor
  virtual ~BuildBEMatrix() {}
  
  virtual void execute();
  
private:
  bool ray_triangle_intersect(double &t,
                              const Point &p,
                              const Vector &v,
                              const Point &p0,
                              const Point &p1,
                              const Point &p2) const;
  void compute_intersections(std::vector<std::pair<double, int> > &results,
                             const VMesh* mesh,
                             const Point &p, const Vector &v,
                             int marker) const;

  int compute_parent(const vector<VMesh*> &meshes, int index);

  bool compute_nesting(vector<int> &nesting,
                       const vector<VMesh*> &meshes);

  void process_field_properties(const FieldHandle& field_handle,
                                bemfield& field,
                                std::ostringstream& inside_cond,
                                std::ostringstream& outside_cond,
                                std::ostringstream& surface_type_init);
  bool
  validate_gui_var(const std::size_t gui_list_len,
                   const std::size_t expected_len,
                   const std::string& name);

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

int
BuildBEMatrixImpl::detectBEMalgo()
{
  ///////////////////////////////////////////////////////////////////////////////////////////////////
  // Check for special case where the potentials need to be evaluated at the nodes of a lead
  // This case assumes the first input is the surface mesh and the second is the location of the
  // nodes
  
  const bemfield_vector::size_type SPECIAL_CASE_LEN = 2;
  
  if ( this->fields_.size() == SPECIAL_CASE_LEN )
  {
    VMesh *surface, *nodes;
    int surfcount=0, pointcloudcount=0;
    
    bool meets_conditions = true;
    
    for (bemfield_vector::size_type i = 0; i < SPECIAL_CASE_LEN; i++)
    {
      if (this->fields_[i].surface)
      {
        surface = this->fields_[i].field_->vmesh();
        if (! surface->is_trisurfmesh() ) meets_conditions = false;
        surfcount++;
      }
      else
      {
        nodes = this->fields_[i].field_->vmesh();
        if (! ( nodes->is_pointcloudmesh() ) || nodes->is_curvemesh() )
        {
          meets_conditions = false;
        }
        pointcloudcount++;
      }
    }
    
    if ( (surfcount == 0) || (pointcloudcount == 0) )
    {
      meets_conditions = false;
    }
    
    // If all of the checks above don't flag meets_conditions as false,
    // return a value that indicates the algorithm to use is the surface-to-nodes case
    if ( meets_conditions )
      return SURFACE_AND_POINTS;
  }
  
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Check for case where all inputs are triangle surfaces, and there is at least one source and
  // one measurement surface
  
  bool allsurfaces=true, hasmeasurementsurf=false, hassourcesurf=false;
  
  for (bemfield_vector::size_type i = 0; i < this->fields_.size(); i++)
  {
    // if the current field is not marked to be used as a surface OR it's not of trisurfmesh type, this algorithm does not apply
    if ( (! this->fields_[i].field_->vmesh()->is_trisurfmesh()) || (! this->fields_[i].surface) )
    {
      allsurfaces=false;
      break;
    }
    
    if (this->fields_[i].measurement) hasmeasurementsurf = true;
    if (this->fields_[i].source) hassourcesurf = true;
  }
  
  // if all fields are surfaces, there exists a measurement and a source surface, then use the surface-to-surface algorithm... else fail
  if (allsurfaces && hasmeasurementsurf && hassourcesurf)
  {
    return SURFACES_TO_SURFACES;
  }
  else
  {
    return UNSUPPORTED;
  }
}

void
BuildBEMatrixImpl::process_field_properties(const FieldHandle& field_handle,
                                        bemfield& field,
                                        std::ostringstream& inside_cond,
                                        std::ostringstream& outside_cond,
                                        std::ostringstream& surface_type_init)
{
  
  // legacy field properties
  static const char* INSIDE_CONDUCTIVITY = "Inside Conductivity";
  static const char* INSIDE_OR_OUTSIDE = "in/out";

  std::string condStr;
  double condVal = 0;
  // If there is no field property, just use default value of 0
  if (field_handle->get_property(INSIDE_CONDUCTIVITY, condStr))
  {
    from_string(condStr, condVal);
  }

  // TODO: hopefully I interpreted this correctly...
  if (field_handle->get_property(INSIDE_OR_OUTSIDE, condStr))
  {
    if (condStr == "in")
    {
      field.insideconductivity = condVal;
      inside_cond << 0;
      outside_cond << 1;
    }
    else if  (condStr == "out")
    {
      field.outsideconductivity = condVal;
      inside_cond << 1;
      outside_cond << 0;
    }
  }
  // TODO: If there is no field property, just initialize both
  // inside and outside conductivities to 0 for now.
  else
  {
    inside_cond << 0;
    outside_cond << 0;
  }
  
  // TODO: initially setting all to source...
  surface_type_init << SOURCE;
}

bool
BuildBEMatrix::validate_gui_var(const std::size_t gui_list_len,
                                const std::size_t expected_len,
                                const std::string& name)
{
  if (gui_list_len != expected_len)
  {
    std::ostringstream oss;
    oss << "Inputs from GUI for " << name << " (" << gui_list_len
        << ") do not match number of fields (" << expected_len
        << "). GUI input will be ignored.";
    warning(oss.str());
    return false;
  }

  return true;
}

void
BuildBEMatrix::execute()
{
  std::vector<FieldHandle> inputs;
  get_dynamic_input_handles("Surface", inputs, true);

  std::vector<VMesh*> meshes;
  std::vector<double> conductivities;
  std::vector<int> field_generation_no_new;

  std::ostringstream fieldlist, fieldtype, inside_cond, outside_cond, surface_type_init;
  
  const std::size_t INPUTS_LEN = inputs.size(), INPUT_LEN_LAST_INDEX = INPUTS_LEN - 1;    

  // If any GUI variables have been set, ignore any incoming field properties - 
  // GUI variables always take precendence.
  //
  // If field changes position, we should still preserve the GUI settings from previous run.
  if ( ( guifield_inside_cond_property_.changed(true) || guifield_outside_cond_property_.changed(true) || guifield_surface_type_property_.changed(true) ) ||
      ( guifield_inside_cond_property_.get().size() > 0 && guifield_outside_cond_property_.get().size() > 0 && guifield_surface_type_property_.get().size() > 0 ) )
  {
    this->process_gui_vars_ = true;
  }
  
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

      // process legacy field properties if not set through the GUI
      //
      // legacy field properties don't support the other bemfield properties,
      // so those will have to come from GUI
      if (! this->process_gui_vars_)
      {
        process_field_properties(field_handle,
                                 field,
                                 inside_cond,
                                 outside_cond,
                                 surface_type_init);        

        if (i < INPUT_LEN_LAST_INDEX)
        {
          fieldlist << " ";
          fieldtype << " ";
          inside_cond << " ";
          outside_cond << " ";
          surface_type_init << " ";
        }
      }
      this->fields_.push_back(field);
    }

    // set GUI vars with legacy field properties
    if (! this->process_gui_vars_)
    {
      guifields_.set(fieldlist.str().c_str());
      guifield_type_property_.set(fieldtype.str().c_str());
      guifield_inside_cond_property_.set(inside_cond.str().c_str());
      guifield_outside_cond_property_.set(outside_cond.str().c_str());
      guifield_surface_type_property_.set(surface_type_init.str().c_str());
      
      TCLInterface::eval(get_id() + " update_fields");
    }
  }
  
  if (this->process_gui_vars_)
  {
    this->process_gui_vars_ = false;

    {
      std::vector<std::string> split_vector;
      std::string property_string(guifield_inside_cond_property_.get());
      balgo::split(split_vector, property_string, boost::is_any_of(" "), balgo::token_compress_on);

      if ( validate_gui_var(split_vector.size(), INPUTS_LEN, "inside conductivity") )
      {
        for (std::vector<std::string>::size_type i = 0; i < INPUTS_LEN; ++i)
        {
          this->fields_[i].insideconductivity = boost::lexical_cast<double>(split_vector[i]);
        }
      }
    }
    
    {
      std::vector<std::string> split_vector;
      std::string property_string(guifield_outside_cond_property_.get());
      balgo::split(split_vector, property_string, boost::is_any_of(" "), balgo::token_compress_on);

      if ( validate_gui_var(split_vector.size(), INPUTS_LEN, "outside conductivity") )
      {
        for (std::vector<std::string>::size_type i = 0; i < INPUTS_LEN; ++i)
        {
          this->fields_[i].outsideconductivity = boost::lexical_cast<double>(split_vector[i]);
        }
      }
    }
    
    {
      std::vector<std::string> split_vector;
      std::string property_string(guifield_surface_type_property_.get());
      balgo::split(split_vector, property_string, boost::is_any_of(" "), balgo::token_compress_on);
      
      if ( validate_gui_var(split_vector.size(), INPUTS_LEN, "surface types (measurement or source)") )
      {
        for (std::vector<std::string>::size_type i = 0; i < INPUTS_LEN; ++i)
        {
          // GUI radiobuttons mapped to the gui variable string should be constrained to 0 and 1
          // which should match up with the surface type enum declared in this file.
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
      }
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
  
  return;
  
    // Flip normals!
  // The numerical integration routines expect inward facing normals, but the convention specified as input to this module is outward facing normals
  // Therefore, this flips the outward facing normals to be inward (by reversing the order that the node indices are stored, i.e. the convention by which the normal directions are specified on these triangle surfaces)
		// TODO: Move this code into its own function and later into the appropriate datatype as a member function
  //    // Burak's "Outward Facing Normals" fix... assume we're given outward facing normals and flip them inward manually
//    // NOTE: ALL SURFACES MATTER HERE -- EVERYTHING IS EXPECTED TO BE OUTWARD-FACING!
//    VMesh::Node::array_type inodes, onodes;
//    VMesh::Face::size_type isize;
//    VMesh::Face::index_type faceindex;
//    int numnodes = 0;
//    mesh->size(isize);
//    //Reorder nodes for each face (I think this should work for both tri and quad faces)
//    for(Matrix::index_type i = 0; i < isize; i++){
//      faceindex=i;
//      mesh->get_nodes(inodes,faceindex);
//      numnodes=inodes.size();
//      for (int p=0; p<numnodes; p++) {
//        onodes[numnodes-1-p]=inodes[p];
//      }
//      //Set the reordered nodes back into the mesh through vmesh functions
//      mesh->set_nodes(onodes, faceindex);
//    }
//  
}

// C++ized MollerTrumbore97 Ray Triangle intersection test.
//#define EPSILON 1.0e-6
bool
BuildBEMatrixImpl::ray_triangle_intersect(double &t,
				       const Point &point,
				       const Vector &dir,
				       const Point &p0,
				       const Point &p1,
				       const Point &p2) const
{
  // Find vectors for two edges sharing p0.
  const Vector edge1 = p1 - p0;
  const Vector edge2 = p2 - p0;

  // begin calculating determinant - also used to calculate U parameter.
  const Vector pvec = Cross(dir, edge2);

  // if determinant is near zero, ray lies in plane of triangle.
  const double det = Dot(edge1, pvec);
  if (det > -EPSILON && det < EPSILON)
  {
    return false;
  }
  const double inv_det = 1.0 / det;

  // Calculate distance from vert0 to ray origin.
  const Vector tvec = point - p0;

  // Calculate U parameter and test bounds.
  const double u = Dot(tvec, pvec) * inv_det;
  if (u < 0.0 || u > 1.0)
  {
    return false;
  }

  // Prepare to test V parameter.
  const Vector qvec = Cross(tvec, edge1);

  // Calculate V parameter and test bounds.
  const double v = Dot(dir, qvec) * inv_det;
  if (v < 0.0 || u + v > 1.0)
  {
    return false;
  }

  // Calculate t, ray intersects triangle.
  t = Dot(edge2, qvec) * inv_det;

  return true;
}


void
BuildBEMatrixImpl::compute_intersections(std::vector<std::pair<double, int> >
              &results,
				      const VMesh* mesh,
				      const Point &p, const Vector &v,
				      int marker) const
{
  VMesh::Face::iterator itr, eitr;
  mesh->begin(itr);
  mesh->end(eitr);
  double t;
  while (itr != eitr)
  {
    VMesh::Node::array_type nodes;
    mesh->get_nodes(nodes, *itr);
    Point p0, p1, p2;
    mesh->get_center(p0, nodes[0]);
    mesh->get_center(p1, nodes[1]);
    mesh->get_center(p2, nodes[2]);
    if (ray_triangle_intersect(t, p, v, p0, p1, p2))
    {
      results.push_back(std::make_pair(t, marker));
    }
    ++itr;
  }
}

static bool
pair_less(const std::pair<double, int> &a,
	  const std::pair<double, int> &b)
{
  return a.first < b.first;
}


int
BuildBEMatrixImpl::compute_parent(const std::vector<VMesh*> &meshes,
                              int index)
{
  Point point;
  meshes[index]->get_center(point, VMesh::Node::index_type(0));
  Vector dir(1.0, 1.0, 1.0);
  std::vector<std::pair<double, int> > intersections;

  unsigned int i;
  for (i = 0; i < (unsigned int)meshes.size(); i++)
  {
    compute_intersections(intersections, meshes[i], point, dir, i);
  }

  std::sort(intersections.begin(), intersections.end(), pair_less);

  std::vector<int> counts(meshes.size(), 0);
  for (i = 0; i < intersections.size(); i++)
  {
    if (intersections[i].second == index)
    {
      // First odd count is parent.
      for (int j = i-1; j >= 0; j--)
      {
        // TODO: unusual odd/even number test?
        if (counts[intersections[j].second] & 1)
        {
          return intersections[j].second;
        }
      }
      // No odd parent, is outside.
      return static_cast<int>( meshes.size() );
    }
    counts[intersections[i].second]++;
  }

  // Indeterminant, we should intersect with ourselves.
  return static_cast<int>( meshes.size() );
}



bool
BuildBEMatrixImpl::compute_nesting(std::vector<int> &nesting,
				const std::vector<VMesh*> &meshes)
{
  nesting.resize(meshes.size());

  unsigned int i;
  for (i = 0; i < (unsigned int)meshes.size(); i++)
  {
    nesting[i] = compute_parent(meshes, i);
  }

  return true;
}
