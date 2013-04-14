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

#include <Core/Thread/Thread.h>
#include <Core/Thread/Mutex.h>

#include <Core/Algorithms/Fields/MarchingCubes/MarchingCubes.h>
#include <Core/Algorithms/Math/AppendMatrix/AppendMatrices.h>
#include <Core/Algorithms/Fields/MergeFields/AppendFields.h>
#include <Core/Algorithms/Fields/MarchingCubes/HexMC.h>
#include <Core/Algorithms/Fields/MarchingCubes/UHexMC.h>
#include <Core/Algorithms/Fields/MarchingCubes/PrismMC.h>
#include <Core/Algorithms/Fields/MarchingCubes/TetMC.h>
#include <Core/Algorithms/Fields/MarchingCubes/TriMC.h>
#include <Core/Algorithms/Fields/MarchingCubes/QuadMC.h>
#include <Core/Algorithms/Fields/MarchingCubes/EdgeMC.h>

#include <Core/Geom/GeomGroup.h>
#include <Core/Geom/GeomMaterial.h>

#include <Core/Datatypes/FieldInformation.h>

#include <Core/Util/MemoryUtil.h>

namespace SCIRunAlgo {


using namespace SCIRun;

template <class TESSELATOR>
class MarchingCubesAlgoP {

  public:
    MarchingCubesAlgoP(FieldHandle input, std::vector<double>& iso_values) : 
      input_(input),
      iso_values_(iso_values) { }
    
    ~MarchingCubesAlgoP()
    {
      delete_all_items(tesselator_);
    }
    
    FieldHandle    input_;
         
    std::vector<TESSELATOR*>   tesselator_;
    std::vector<FieldHandle>  output_field_;
    std::vector<MatrixHandle> output_interpolant_matrix_;
    std::vector<MatrixHandle> output_parent_cell_matrix_;
    std::vector<GeomHandle>   output_geometry_;

    bool build_field_;
    bool build_node_interpolant_;
    bool build_elem_interpolant_;
    bool build_geometry_;
    bool transparency_;

    std::vector<double>& iso_values_;
    AlgoBase* algo_;
    
    bool run(AlgoBase* algo, FieldHandle& output, GeomHandle& geometry,
             MatrixHandle& node_interpolant,MatrixHandle& elem_interpolant );
             
    void parallel(int proc, int nproc, size_t iso);
    
  private:
    SCIRunAlgo::AppendFieldsAlgo append_fields_;
    SCIRunAlgo::AppendMatricesAlgo append_matrices_;
};


template <class TESSELATOR>
bool
MarchingCubesAlgoP<TESSELATOR>::run(AlgoBase* algo, 
                        FieldHandle& output, 
                        GeomHandle& geometry,
                        MatrixHandle& node_interpolant,
                        MatrixHandle& elem_interpolant)
{
  algo_ = algo;
  
  int np = algo->get_int("num_threads");
  //! By default (-1) choose number of processors
  if (np < 1) np = Thread::numProcessors();
  //! Cap the number of threads
  if (np > 4*Thread::numProcessors()) np = 4*Thread::numProcessors();
  
  // TODO: FIX MULTI THREADING OF MARCHING CUBES FIELDS ARE NOT PROPORLY LINKED
  
  np = 1;
  size_t num_values = iso_values_.size();

  append_fields_.set_progress_reporter(algo->get_progress_reporter());
  append_matrices_.set_progress_reporter(algo->get_progress_reporter());
  append_matrices_.set_option("method","append_rows");
  
  build_field_ = algo->get_bool("build_field");
  build_geometry_ = algo->get_bool("build_geometry");
  build_node_interpolant_ = algo->get_bool("build_node_interpolant");
  build_elem_interpolant_ = algo->get_bool("build_elem_interpolant");
  transparency_ = algo->get_bool("transparency");

  tesselator_.resize(np);
  for (size_t j=0; j<tesselator_.size(); j++)
    tesselator_[j] = new TESSELATOR(input_.get_rep());
  
  output_field_.resize(np*num_values);
  output_interpolant_matrix_.resize(np*num_values);
  output_parent_cell_matrix_.resize(np*num_values);  
  output_geometry_.resize(np*num_values);

  for (size_t j=0; j<iso_values_.size(); j++)
  {
    if (np == 1) 
    {
      parallel(0,1,j);
    }
    else
    {
      Thread::parallel(this,&MarchingCubesAlgoP<TESSELATOR>::parallel,np,np,j);
    }
  }
  
  if (output_geometry_.size() == 0)
  {
    geometry = 0;
  }
  else if (output_geometry_.size() == 1)
  {
    geometry = output_geometry_[0];
  }
  else
  {
    // link geometries
    GeomGroup* group = new GeomGroup;
    for (size_t j=0; j < output_geometry_.size(); j++) group->add(output_geometry_[j]);
    geometry = group; 
  }

  if (build_field_)
  {
    if (!(append_fields_.run(output_field_,output))) return (false);
  }


  if (build_node_interpolant_)
  {
    if (!(append_matrices_.run(output_interpolant_matrix_,node_interpolant))) 
      return (false);
  }

  if (build_elem_interpolant_)
  {
    if (!(append_matrices_.run(output_parent_cell_matrix_,elem_interpolant))) 
      return (false);
  }

  return (true);
}


template<class TESSELATOR>
void 
MarchingCubesAlgoP<TESSELATOR>::parallel( int proc, int nproc, size_t iso)
{
  tesselator_[proc]->reset(0, build_field_, build_geometry_, transparency_);

  VMesh*  imesh  = input_->vmesh();
  
  VMesh::size_type num_elems = imesh->num_elems(); 

  index_type start = (proc)*(num_elems/nproc);
  index_type end = (proc < nproc-1) ? (proc+1)*(num_elems/nproc) : num_elems;

  index_type cnt = 0;
  size_type total = (num_elems*iso_values_.size()/nproc);
  index_type offset = (num_elems*iso/nproc); 
  double isoval = iso_values_[iso];

  for(VMesh::Elem::index_type idx= start ; idx<end; idx++)
  {
    tesselator_[proc]->extract(idx, isoval);
    if (proc == 0)  
    {
      cnt++;
      if (cnt == 300)
      {
        cnt = 0;
        algo_->update_progress(idx+offset,total);
      }
    }
  }
  
  output_field_[iso*nproc+proc] = 0;
  output_interpolant_matrix_[iso*nproc+proc] = 0;
  output_parent_cell_matrix_[iso*nproc+proc] = 0;
  
  output_geometry_[iso*nproc+proc] = 0;

  if (build_field_) 
  { 
    output_field_[iso*nproc+proc] = tesselator_[proc]->get_field(isoval);
  }
  if (build_node_interpolant_)
  {
    output_interpolant_matrix_[iso*nproc+proc] = tesselator_[proc]->get_interpolant();
  }
  if (build_elem_interpolant_)
  {
    output_parent_cell_matrix_[iso*nproc+proc] = tesselator_[proc]->get_parent_cells();
  }
  
  if (build_geometry_)
  {
    MaterialHandle mathandle;
    ColorMapHandle colormap;
    colormap = algo_->get_colormap("colormap");
    if (colormap.get_rep())
    {
      mathandle = colormap->lookup(isoval);
    }
    else
    {
      Color color = algo_->get_color("color");
      mathandle = new Material(color);
    }
    if (mathandle.get_rep())
    {
      GeomHandle geom = tesselator_[proc]->get_geom();
      output_geometry_[iso*nproc+proc] = new GeomMaterial(geom,mathandle);
    }
    else
    {
      output_geometry_[iso*nproc+proc] = 0;
    }
  }
}

bool
MarchingCubesAlgo::run(FieldHandle input, std::vector<double>& isovalues, 
             GeomHandle& geometry,
             FieldHandle& field,
             MatrixHandle& node_interpolant,
             MatrixHandle& elem_interpolant )
{
  algo_start("MarchingCubes");
  
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  FieldInformation fi(input);

  bool success = false;
  
  if (fi.is_pnt_element())
  {
    error("Field needs to have elements in order to  extract isosurfaces");
  }
  else if (fi.is_crv_element())
  {
    MarchingCubesAlgoP<EdgeMC> algo(input,isovalues);
    success = algo.run(this,field,geometry,node_interpolant,elem_interpolant);
  }
  else if (fi.is_tri_element())
  {
    MarchingCubesAlgoP<TriMC> algo(input,isovalues);
    success = algo.run(this,field,geometry,node_interpolant,elem_interpolant);
  }
  else if (fi.is_quad_element())
  {
    MarchingCubesAlgoP<QuadMC> algo(input,isovalues);
    success = algo.run(this,field,geometry,node_interpolant,elem_interpolant);
  }
  else if (fi.is_tet_element())
  {
    MarchingCubesAlgoP<TetMC> algo(input,isovalues);
    success = algo.run(this,field,geometry,node_interpolant,elem_interpolant);
  }
  else if (fi.is_prism_element())
  {
    MarchingCubesAlgoP<PrismMC> algo(input,isovalues);
    success = algo.run(this,field,geometry,node_interpolant,elem_interpolant);
  }
  else if (fi.is_hex_element())
  {
    if (fi.is_structuredmesh())
    {
      MarchingCubesAlgoP<HexMC> algo(input,isovalues);
      success = algo.run(this,field,geometry,node_interpolant,elem_interpolant);
    }
    else
    {
      MarchingCubesAlgoP<UHexMC> algo(input,isovalues);
      success = algo.run(this,field,geometry,node_interpolant,elem_interpolant);
    }
  }
  
  algo_end(); return (success);
}


bool 
MarchingCubesAlgo::run(FieldHandle input, std::vector<double>& isovalues, 
                       GeomHandle& geometry )
{
  FieldHandle dummy0;
  MatrixHandle dummy1, dummy2;
  return run(input,isovalues,geometry,dummy0,dummy1,dummy2);
}

bool 
MarchingCubesAlgo::run(FieldHandle input, std::vector<double>& isovalues, 
                       FieldHandle& field )
{
  GeomHandle dummy0;
  MatrixHandle dummy1, dummy2;
  return run(input,isovalues,dummy0,field,dummy1,dummy2);
}

bool 
MarchingCubesAlgo::run(FieldHandle input, std::vector<double>& isovalues, 
                       FieldHandle& field, MatrixHandle& interpolant )
{
  GeomHandle dummy0;
  MatrixHandle dummy1;
  if (get_bool("build_node_interpolant"))
  {
    return run(input,isovalues,dummy0,field,interpolant,dummy1);
  }
  else
  {
    return run(input,isovalues,dummy0,field,dummy1,interpolant);  
  }
}

}
