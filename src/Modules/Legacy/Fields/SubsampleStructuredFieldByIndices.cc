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


///
///    @file    SubsampleStructuredFieldByIndices.h
///    @author  Michael Callahan &&
///             Allen Sanderson
///             SCI Institute
///             University of Utah
///    @date    March 2006
///

#include <Core/Datatypes/DenseMatrix.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Datatypes/FieldInformation.h>


namespace SCIRun {

class SubsampleStructuredFieldByIndices : public Module {
  public:
    SubsampleStructuredFieldByIndices(GuiContext *context);
    virtual ~SubsampleStructuredFieldByIndices() {}

    virtual void execute();

  private:
    GuiInt gui_power_app_;

    GuiInt gui_wrap_;
    GuiInt gui_dims_;

    GuiInt gui_dim_i_;
    GuiInt gui_dim_j_;
    GuiInt gui_dim_k_;

    GuiInt gui_start_i_;
    GuiInt gui_start_j_;
    GuiInt gui_start_k_;

    GuiInt gui_stop_i_;
    GuiInt gui_stop_j_;
    GuiInt gui_stop_k_;

    GuiInt gui_stride_i_;
    GuiInt gui_stride_j_;
    GuiInt gui_stride_k_;

    GuiInt gui_wrap_i_;
    GuiInt gui_wrap_j_;
    GuiInt gui_wrap_k_;
};


DECLARE_MAKER(SubsampleStructuredFieldByIndices)


SubsampleStructuredFieldByIndices::SubsampleStructuredFieldByIndices(GuiContext *context)
  : Module("SubsampleStructuredFieldByIndices", context, Filter, "NewField", "SCIRun"),
    gui_power_app_(context->subVar("power_app"), 0),
    gui_wrap_(context->subVar("wrap"), 0 ),
    gui_dims_(context->subVar("dims"), 3 ),
    gui_dim_i_(context->subVar("dim-i"), 2),
    gui_dim_j_(context->subVar("dim-j"), 2),
    gui_dim_k_(context->subVar("dim-k"), 2),
    gui_start_i_(context->subVar("start-i"), 0),
    gui_start_j_(context->subVar("start-j"), 0),
    gui_start_k_(context->subVar("start-k"), 0),
    gui_stop_i_(context->subVar("stop-i"), 1),
    gui_stop_j_(context->subVar("stop-j"), 1),
    gui_stop_k_(context->subVar("stop-k"), 1),
    gui_stride_i_(context->subVar("stride-i"), 1),
    gui_stride_j_(context->subVar("stride-j"), 1),
    gui_stride_k_(context->subVar("stride-k"), 1),
    gui_wrap_i_(context->subVar("wrap-i"), 0),
    gui_wrap_j_(context->subVar("wrap-j"), 0),
    gui_wrap_k_(context->subVar("wrap-k"), 0)
{
}


void
SubsampleStructuredFieldByIndices::execute()
{
  // FieldHandle for input field
  FieldHandle field_in_handle = 0;
  // Handle for matrix input
  MatrixHandle matrix_handle = 0;

  /// Get the input field handle from the port.
  get_input_handle( "Input Field",  field_in_handle, true );

  // Because the field slicer is index based it can only work on
  // structured data. For unstructured data SamplePlane should be used.

  // Get the virtual interface
  VField* ifield = field_in_handle->vfield();
  VMesh*  imesh  = field_in_handle->vmesh();

  if( imesh->is_unstructuredmesh())
  {
    error( "This module is only availible for topologically structured data." );
    return;
  }

  // For now slice only node and cell based data.
  if( ifield->basis_order() != 0 &&
      ifield->basis_order() != 1 )
  {
    error( "This module is only available for cell or node data." );
    return;
  }

  if( inputs_changed_ )
  {
    // Get the dimensions of the mesh.
    VMesh::dimension_type dims;
    imesh->get_dimensions( dims );

    bool update_dims = false;

    /// Structured data with irregular points can be wrapped
    bool wrap = imesh->is_irregularmesh();

    /// Check to see if the gui wrap is different than the field.
    if( static_cast<bool>(gui_wrap_.get()) != wrap )
    {
      gui_wrap_.set( wrap );
      update_dims = true;
    }

    if( dims.size() >= 1 )
    {
      /// Check to see if the gui dimensions are different than the field.
      if( gui_dim_i_.get() != static_cast<int>(dims[0]) )
      {
        gui_dim_i_.set( dims[0] );
        update_dims = true;
      }
    }

    if( dims.size() >= 2 )
    {
      /// Check to see if the gui dimensions are different than the field.
      if( gui_dim_j_.get() != static_cast<int>(dims[1]) )
      {
        gui_dim_j_.set( dims[1] );
        update_dims = true;
      }
    }

    if( dims.size() >= 3 )
    {
      /// Check to see if the gui dimensions are different than the field.
      if( gui_dim_k_.get() != static_cast<int>(dims[2]) )
      {
        gui_dim_k_.set( dims[2] );
        update_dims = true;
      }
    }

    /// Check to see if the gui dimensions are different than the field.
    /// This is last because the GUI var has a callback on it.
    if( gui_dims_.get() != static_cast<int>(dims.size()) )
    {
      gui_dims_.set( dims.size() );
      update_dims = true;
    }

    /// If the gui dimensions are different than the field then update the gui.
    if( update_dims )
    {
      std::ostringstream str;
      str << get_id() << " set_size ";
      TCLInterface::execute(str.str().c_str());
      reset_vars();
    }
  }

  /// Get the optional matrix handle from the port. Note if a matrix is
  /// present it is sent down stream. Otherwise it will be created.
  get_input_handle("Input Matrix", matrix_handle, false );

  /// An input matrix is present so use the values in it to override
  /// the variables set in the gui.
  /// Column 0 start  index to subsample.
  /// Column 1 stop   index to subsample.
  /// Column 2 stride value to subsample.
  /// Column 3 wrap flag.
  /// Column 4 dimensions of the data.
  if( matrix_handle.get_rep() )
  {
    /// The matrix is optional. If present make sure it is a 3x5 matrix.
    /// The row indices is the axis index. The column is the data.
    if( (matrix_handle->nrows() != 3 || matrix_handle->ncols() != 5) )
    {
      error( "Input matrix is not a 3x5 matrix" );
      return;
    }

    /// Sanity check. Make sure the gui dimensions match the matrix
    /// dimensions.
    if( gui_dim_i_.get() != matrix_handle->get(0, 4) ||
        gui_dim_j_.get() != matrix_handle->get(1, 4) ||
        gui_dim_k_.get() != matrix_handle->get(2, 4) )
    {
      std::ostringstream str;
      str << "The dimensions of the matrix slicing do match the field. "
          << " Expected "
          << gui_dim_i_.get() << " "
          << gui_dim_j_.get() << " "
          << gui_dim_k_.get()
          << " Got "
          << matrix_handle->get(0, 4) << " "
          << matrix_handle->get(1, 4) << " "
          << matrix_handle->get(2, 4);

      error( str.str() );
      return;
    }

    /// Check to see what index has been selected and if it matches
    /// the gui index.
    if( gui_start_i_.get() != (int) matrix_handle->get(0, 0) ||
        gui_start_j_.get() != (int) matrix_handle->get(1, 0) ||
        gui_start_k_.get() != (int) matrix_handle->get(2, 0) ||

        gui_stop_i_.get() != (int) matrix_handle->get(0, 1) ||
        gui_stop_j_.get() != (int) matrix_handle->get(1, 1) ||
        gui_stop_k_.get() != (int) matrix_handle->get(2, 1) ||

        gui_stride_i_.get() != (int) matrix_handle->get(0, 2) ||
        gui_stride_j_.get() != (int) matrix_handle->get(1, 2) ||
        gui_stride_k_.get() != (int) matrix_handle->get(2, 2) ||

        gui_wrap_i_.get() != (int) matrix_handle->get(0, 3) ||
        gui_wrap_j_.get() != (int) matrix_handle->get(1, 3) ||
        gui_wrap_k_.get() != (int) matrix_handle->get(2, 3) )
      {

      gui_start_i_.set( (int) matrix_handle->get(0, 0) );
      gui_start_j_.set( (int) matrix_handle->get(1, 0) );
      gui_start_k_.set( (int) matrix_handle->get(2, 0) );

      gui_stop_i_.set( (int) matrix_handle->get(0, 1) );
      gui_stop_j_.set( (int) matrix_handle->get(1, 1) );
      gui_stop_k_.set( (int) matrix_handle->get(2, 1) );

      gui_stride_i_.set( (int) matrix_handle->get(0, 2) );
      gui_stride_j_.set( (int) matrix_handle->get(1, 2) );
      gui_stride_k_.set( (int) matrix_handle->get(2, 2) );

      gui_wrap_i_.set( (int) matrix_handle->get(0, 3) );
      gui_wrap_j_.set( (int) matrix_handle->get(1, 3) );
      gui_wrap_k_.set( (int) matrix_handle->get(2, 3) );

      std::ostringstream str;
      str << get_id() << " update_index ";

      TCLInterface::execute(str.str().c_str());

      reset_vars();

      inputs_changed_ = true;
    }
  }

  /// If no data or an input change recreate the field. I.e Only
  /// execute when neeed.
  if( inputs_changed_ ||

      !oport_cached("Output Field") ||
      !oport_cached("Output Matrix") ||

      gui_start_i_.changed(true) ||
      gui_start_j_.changed(true) ||
      gui_start_k_.changed(true) ||

      gui_stop_i_.changed(true) ||
      gui_stop_j_.changed(true) ||
      gui_stop_k_.changed(true) ||

      gui_stride_i_.changed(true) ||
      gui_stride_j_.changed(true) ||
      gui_stride_k_.changed(true) ||

      gui_wrap_i_.changed(true) ||
      gui_wrap_j_.changed(true) ||
      gui_wrap_k_.changed(true) )
  {

    // Update the state. Other state changes are handled in either
    // getting handles or in the calling method Module::do_execute.
    update_state(Executing);

    FieldHandle field_out_handle;

    VMesh::index_type i_start = gui_start_i_.get();
    VMesh::index_type j_start = gui_start_j_.get();
    VMesh::index_type k_start = gui_start_k_.get();
    VMesh::index_type i_stop = gui_stop_i_.get();
    VMesh::index_type j_stop = gui_stop_j_.get();
    VMesh::index_type k_stop = gui_stop_k_.get();
    VMesh::index_type i_stride = gui_stride_i_.get();
    VMesh::index_type j_stride = gui_stride_j_.get();
    VMesh::index_type k_stride = gui_stride_k_.get();

    VMesh::size_type idim_in, jdim_in, kdim_in;
    VMesh::index_type i, j, k, inode, jnode, knode;

    VMesh::dimension_type dims;
    imesh->get_dimensions( dims );

    size_t rank = dims.size();

    if( rank == 3 )
    {
      idim_in = dims[0];
      jdim_in = dims[1];
      kdim_in = dims[2];
    }
    else if( rank == 2 )
    {
      idim_in = dims[0];
      jdim_in = dims[1];
      kdim_in = 1;
    }
    else if( rank == 1 )
    {
      idim_in = dims[0];
      jdim_in = 1;
      kdim_in = 1;
    }

    /// This happens when wrapping.
    if( i_stop <= i_start ) i_stop += idim_in;
    if( j_stop <= j_start ) j_stop += jdim_in;
    if( k_stop <= k_start ) k_stop += kdim_in;

    /// Add one because we want the last node.
    VMesh::index_type idim_out = (i_stop - i_start) / i_stride + (rank >= 1 ? 1 : 0);
    VMesh::index_type jdim_out = (j_stop - j_start) / j_stride + (rank >= 2 ? 1 : 0);
    VMesh::index_type kdim_out = (k_stop - k_start) / k_stride + (rank >= 3 ? 1 : 0);

    VMesh::index_type i_stop_stride;
    VMesh::index_type j_stop_stride;
    VMesh::index_type k_stop_stride;

    if (imesh->is_structuredmesh() && !(imesh->is_regularmesh()))
    {
      /// Account for the modulo of stride so that the last node will be
      /// included even if it "partial" elem when compared to the others.
      if( (i_stop - i_start) % i_stride ) idim_out += (rank >= 1 ? 1 : 0);
      if( (j_stop - j_start) % j_stride ) jdim_out += (rank >= 2 ? 1 : 0);
      if( (k_stop - k_start) % k_stride ) kdim_out += (rank >= 3 ? 1 : 0);

      i_stop_stride = i_stop + (rank >= 1 ? i_stride : 0);
      j_stop_stride = j_stop + (rank >= 2 ? j_stride : 0);
      k_stop_stride = k_stop + (rank >= 3 ? k_stride : 0);
    }
    else
    {
      i_stop_stride = i_stop + (rank >= 1 ? 1 : 0);
      j_stop_stride = j_stop + (rank >= 2 ? 1 : 0);
      k_stop_stride = k_stop + (rank >= 3 ? 1 : 0);
    }

    FieldInformation fi(field_in_handle);
    MeshHandle mesh;

    if( rank == 3 )
    {
      dims[0] = idim_out;
      dims[1] = jdim_out;
      dims[2] = kdim_out;
      mesh = CreateMesh(fi,dims[0],dims[1],dims[2]);
    }
    else if( rank == 2 )
    {
      dims[0] = idim_out;
      dims[1] = jdim_out;
      mesh = CreateMesh(fi,dims[0],dims[1]);
    }
    else if( rank == 1 )
    {
      dims[0] = idim_out;
      mesh = CreateMesh(fi,dims[0]);
    }

    VMesh* omesh = mesh->vmesh();

//    omesh->copy_properties(imesh);

    field_out_handle = CreateField(fi,mesh);
    VField* ofield = field_out_handle->vfield();
    ofield->copy_properties(ifield);

    Point pt;
    Point p, o;

    VMesh::Node::index_type inodeIdx = 0, jnodeIdx = 0, knodeIdx = 0;
    VMesh::Node::index_type onodeIdx = 0;

    VMesh::Elem::index_type ielemIdx = 0, jelemIdx = 0, kelemIdx = 0;
    VMesh::Elem::index_type oelemIdx = 0;

    /// For structured uniform geometry we need to set the correct location.
    if (imesh->is_regularmesh())
    {
      /// Set the orginal transform.
      omesh->set_transform( imesh->get_transform() );
      inodeIdx = 0;

      /// Get the orgin of mesh. */
      imesh->get_center(o, inodeIdx);

      /// Set the iterator to the first point.
      inodeIdx += (k_start*(jdim_in*idim_in)+j_start*(idim_in)+i_start);

      /// Get the point.
      imesh->get_center(p, inodeIdx);

      /// Put the new field into the correct location.
      Transform trans;

      trans.pre_translate( (Vector) (-o) );
      trans.pre_scale( Vector( i_stride, j_stride, k_stride ) );
      trans.pre_translate( (Vector) (o) );
      trans.pre_translate( (Vector) (p-o) );

      omesh->transform( trans );
    }

    /// Index based on the old mesh so that we are assured of getting the last
    /// node even if it forms a "partial" elem.
    for( k=k_start; k<k_stop_stride; k+=k_stride )
    {

      /// Check for going past the stop.
      if( k > k_stop ) k = k_stop;

      /// Check for overlap.
      if( k-k_stride <= k_start+kdim_in && k_start+kdim_in <= k )
      {
        knode = k_start;
      }
      else
      {
        knode = k % kdim_in;
      }

      /// A hack here so that an iterator can be used.
      /// Set this iterator to be at the correct kth index.
      knodeIdx = 0;
      kelemIdx = 0;

      knodeIdx = knode*jdim_in*idim_in;
      kelemIdx = knode*(jdim_in-1)*(idim_in-1);

      for( j=j_start; j<j_stop_stride; j+=j_stride )
      {

        /// Check for going past the stop.
        if( j > j_stop ) j = j_stop;

        /// Check for overlap.
        if( j-j_stride <= j_start+jdim_in && j_start+jdim_in <= j )
          jnode = j_start;
        else
          jnode = j % jdim_in;

        /// A hack here so that an iterator can be used.
        /// Set this iterator to be at the correct jth index.
        jnodeIdx = knodeIdx;
        jelemIdx = kelemIdx;

        jnodeIdx += jnode*idim_in;
        jelemIdx += jnode*(idim_in-1);

        for( i=i_start; i<i_stop_stride; i+=i_stride )
        {

          /// Check for going past the stop.
          if( i > i_stop )
            i = i_stop;

          /// Check for overlap.
          if( i-i_stride <= i_start+idim_in && i_start+idim_in <= i )
            inode = i_start;
          else
            inode = i % idim_in;

          /// A hack here so that an iterator can be used.
          /// Set this iterator to be at the correct ith index.

          inodeIdx = jnodeIdx;
          ielemIdx = jelemIdx;
          inodeIdx += inode;
          ielemIdx += inode;

          if (imesh->is_irregularmesh())
          {
            imesh->get_center(pt, inodeIdx);
            omesh->set_point(pt, onodeIdx);
          }

          switch( ifield->basis_order() )
          {
            case 0:

            if( i+i_stride<i_stop_stride &&
                j+j_stride<j_stop_stride &&
                k+k_stride<k_stop_stride )
            {
              ofield->copy_value(ifield,ielemIdx,oelemIdx);
              oelemIdx++;
            }
            break;

          case 1:
              ofield->copy_value(ifield,inodeIdx,onodeIdx);
            break;

          default:
            break;
          }

          onodeIdx++;
        }
      }
    }

    // Send the data downstream
    send_output_handle( "Output Field", field_out_handle );

    if( matrix_handle == 0 )
    {
      /// Create the output matrix with the stop, stop, stride, wrap, and
      /// dimensions.
      DenseMatrix *selected = new DenseMatrix(3,5);

      selected->put(0, 0, gui_start_i_.get() );
      selected->put(0, 1, gui_stop_i_.get() );
      selected->put(0, 2, gui_stride_i_.get() );
      selected->put(0, 3, gui_wrap_i_.get() );
      selected->put(0, 4, gui_dim_i_.get() );

      selected->put(1, 0, gui_start_j_.get() );
      selected->put(1, 1, gui_stop_j_.get() );
      selected->put(1, 2, gui_stride_j_.get() );
      selected->put(1, 3, gui_wrap_j_.get() );
      selected->put(1, 4, gui_dim_j_.get() );

      selected->put(2, 0, gui_start_k_.get() );
      selected->put(2, 1, gui_stop_k_.get() );
      selected->put(2, 2, gui_stride_k_.get() );
      selected->put(2, 3, gui_wrap_k_.get() );
      selected->put(2, 4, gui_dim_k_.get() );

      matrix_handle = MatrixHandle(selected);

      // Send the data downstream
      send_output_handle( "Output Matrix", matrix_handle );
    }
  }
}

} // End namespace SCIRun
