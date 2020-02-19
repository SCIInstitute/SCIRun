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


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 #include <Core/Thread/Thread.h>
#endif

#include <Core/Thread/Parallel.h>
#include <Core/Thread/Mutex.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/Fields/MarchingCubes/MarchingCubes.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Math/AppendMatrix.h>
#include <Core/Algorithms/Legacy/Fields/MergeFields/AppendFieldsAlgo.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <Core/Algorithms/Legacy/Fields/MarchingCubes/HexMC.h>
#include <Core/Algorithms/Legacy/Fields/MarchingCubes/UHexMC.h>
#include <Core/Algorithms/Legacy/Fields/MarchingCubes/PrismMC.h>
#include <Core/Algorithms/Legacy/Fields/MarchingCubes/TetMC.h>
#include <Core/Algorithms/Legacy/Fields/MarchingCubes/TriMC.h>
#include <Core/Algorithms/Legacy/Fields/MarchingCubes/QuadMC.h>
#include <Core/Algorithms/Legacy/Fields/MarchingCubes/EdgeMC.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 #include <Core/Geom/GeomGroup.h>
 #include <Core/Geom/GeomMaterial.h>

 #include <Core/Util/MemoryUtil.h>
#endif

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithm::Fields;
using namespace SCIRun::Core::Algorithm::Fields::Math;

MarchingCubesAlgo::MarchingCubesAlgo()
{
  addParameter(transparency,false);
  addParameter(build_geometry,false);
  addParameter(build_field,false);
  addParameter(build_node_interpolant,false);
  addParameter(build_elem_interpolant,false);
  addParameter(num_threads,-1);
}

AlgorithmParameterName MarchingCubesAlgo::transparency("transparency");
AlgorithmParameterName MarchingCubesAlgo::build_geometry("build_geometry");
AlgorithmParameterName MarchingCubesAlgo::build_field("build_field");
AlgorithmParameterName MarchingCubesAlgo::build_node_interpolant("build_node_interpolant");
AlgorithmParameterName MarchingCubesAlgo::build_elem_interpolant("build_elem_interpolant");
AlgorithmParameterName MarchingCubesAlgo::num_threads("num_threads");

AlgorithmOutput MarchingCubesAlgo::run(const AlgorithmInput& input) const
{

  AlgorithmOutput output;
  return output;
}


template <class TESSELATOR>
class MarchingCubesAlgoP {

  public:

    MarchingCubesAlgoP(FieldHandle input,const std::vector<double>& iso_values) :
     input_(input),
     iso_values_(iso_values) { }

    ~MarchingCubesAlgoP()
    {
      #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
       delete_all_items(tesselator_);
      #endif
    }

    FieldHandle    input_;

    std::vector<TESSELATOR*>   tesselator_;
    std::vector<FieldHandle>  output_field_;
    std::vector<MatrixHandle> output_interpolant_matrix_;
    std::vector<MatrixHandle> output_parent_cell_matrix_;
    #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
     std::vector<GeomHandle>   output_geometry_;
    #endif

    bool build_field_;
    bool build_node_interpolant_;
    bool build_elem_interpolant_;
    bool build_geometry_;
    bool transparency_;

    const std::vector<double>& iso_values_;
    const AlgorithmBase* algo_;

    bool run(const AlgorithmBase* algo, FieldHandle& output,
             MatrixHandle& node_interpolant,MatrixHandle& elem_interpolant );

    void parallel(int proc, int nproc, size_t iso);

  private:
    AppendFieldsAlgorithm append_fields_;
    AppendMatrixAlgorithm append_matrices_;

};


bool
MarchingCubesAlgo::run(FieldHandle input, const std::vector<double>& isovalues)
{
  FieldHandle dummy0;
  MatrixHandle dummy1, dummy2;
  return run(input,isovalues,dummy0,dummy1,dummy2);
}

bool
MarchingCubesAlgo::run(FieldHandle input, const std::vector<double>& isovalues, FieldHandle& field )
{
  MatrixHandle dummy1, dummy2;
  return run(input,isovalues,field,dummy1,dummy2);
}

template <class TESSELATOR>
bool
MarchingCubesAlgoP<TESSELATOR>::run(const AlgorithmBase* algo,
                        FieldHandle& output,
                        MatrixHandle& node_interpolant,
                        MatrixHandle& elem_interpolant)
{
  algo_ = algo;

 #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  int np = algo->get_int("num_threads");
  /// By default (-1) choose number of processors
  if (np < 1) np = Thread::numProcessors();
  /// Cap the number of threads
  if (np > 4*Thread::numProcessors()) np = 4*Thread::numProcessors();
  */
  /// @todo: FIX MULTI THREADING OF MARCHING CUBES FIELDS ARE NOT PROPORLY LINKED
 #endif
  int np = Parallel::NumCores();

  np = 1;
  size_t num_values = iso_values_.size();

  tesselator_.resize(np);
  for (size_t j=0; j<tesselator_.size(); j++)
    tesselator_[j] = new TESSELATOR(input_);

  output_field_.resize(np*num_values);
  output_interpolant_matrix_.resize(np*num_values);
  output_parent_cell_matrix_.resize(np*num_values);
  //output_geometry_.resize(np*num_values);

  build_field_ = algo->get(MarchingCubesAlgo::build_field).toBool();
  build_geometry_ = algo->get(MarchingCubesAlgo::build_geometry).toBool();
  build_node_interpolant_ = algo->get(MarchingCubesAlgo::build_node_interpolant).toBool();
  build_elem_interpolant_ = algo->get(MarchingCubesAlgo::build_elem_interpolant).toBool();
  transparency_ = algo->get(MarchingCubesAlgo::transparency).toBool();

 #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  append_fields_.set_progress_reporter(algo->get_progress_reporter());
  append_matrices_.set_progress_reporter(algo->get_progress_reporter());
  append_matrices_.setOption("method","append_rows");
 #endif

  for (size_t j=0; j<iso_values_.size(); j++)
  {
    if (np == 1)
    {
      parallel(0,1,j);
    }
    else
    {
      #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
       Thread::parallel(this,&MarchingCubesAlgoP<TESSELATOR>::parallel,np,np,j);
      #endif
    }
  }
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
 #endif

  if (build_field_)
  {
   if (!(append_fields_.run(output_field_,output)))
      return (false);
  }

 #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
 #endif

  return (true);
}

bool MarchingCubesAlgo::run(FieldHandle input, const std::vector<double>& isovalues, FieldHandle& field, MatrixHandle& node_interpolant, MatrixHandle& elem_interpolant) const
{

  if (!input)
  {
     error("MarchingCube algorithm error: Input field is Null pointer.");
  }

  bool success = false;

  FieldInformation fi(input);

  if (fi.is_pnt_element())
  {
    error("Field needs to have elements in order to extract isosurfaces");
  }
  else if (fi.is_crv_element())
  {
    MarchingCubesAlgoP<EdgeMC> algo(input,isovalues);
    success = algo.run(this,field,node_interpolant,elem_interpolant);
  }
  else if (fi.is_tri_element())
  {
    MarchingCubesAlgoP<TriMC> algo(input,isovalues);
    success = algo.run(this,field,node_interpolant,elem_interpolant);
  }
  else if (fi.is_quad_element())
  {
    MarchingCubesAlgoP<QuadMC> algo(input,isovalues);
    success = algo.run(this,field,node_interpolant,elem_interpolant);
  }
  else if (fi.is_tet_element())
  {
    MarchingCubesAlgoP<TetMC> algo(input,isovalues);
    success = algo.run(this,field,node_interpolant,elem_interpolant);
  }
  else if (fi.is_prism_element())
  {
    MarchingCubesAlgoP<PrismMC> algo(input,isovalues);
    success = algo.run(this,field,node_interpolant,elem_interpolant);
  }
  else if (fi.is_hex_element())
  {
    if (fi.is_structuredmesh())
    {
      MarchingCubesAlgoP<HexMC> algo(input,isovalues);
      success = algo.run(this,field,node_interpolant,elem_interpolant);
    }
    else
    {
      MarchingCubesAlgoP<UHexMC> algo(input,isovalues);
      success = algo.run(this,field,node_interpolant,elem_interpolant);
    }
  }

  return true;
}


template<class TESSELATOR>
void MarchingCubesAlgoP<TESSELATOR>::parallel( int proc, int nproc, size_t iso)
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
        algo_->update_progress(idx+offset/total);
      }
    }
  }

  output_field_[iso*nproc+proc] = 0;
  output_interpolant_matrix_[iso*nproc+proc] = 0;
  output_parent_cell_matrix_[iso*nproc+proc] = 0;

  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   output_geometry_[iso*nproc+proc] = 0;
  #endif

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

  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
  #endif

}
