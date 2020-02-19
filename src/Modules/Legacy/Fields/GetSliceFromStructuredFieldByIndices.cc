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
///     @file    GetSliceFromStructuredFieldByIndices.h
///
///     @author  Michael Callahan &&
///              Allen Sanderson
///              SCI Institute
///              University of Utah
///     @date    March 2006
///

#include <Modules/Legacy/Fields/GetSliceFromStructuredFieldByIndices.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

/// @class GetSliceFromStructuredFieldByIndices
/// @brief This module reduces the dimension of a topologically regular field by 1 dimension.

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields::Parameters;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Fields, Dim_i);
ALGORITHM_PARAMETER_DEF(Fields, Dim_j);
ALGORITHM_PARAMETER_DEF(Fields, Dim_k);
ALGORITHM_PARAMETER_DEF(Fields, Index_i);
ALGORITHM_PARAMETER_DEF(Fields, Index_j);
ALGORITHM_PARAMETER_DEF(Fields, Index_k);
ALGORITHM_PARAMETER_DEF(Fields, Axis_ijk);
ALGORITHM_PARAMETER_DEF(Fields, SpinBoxReexecute);
ALGORITHM_PARAMETER_DEF(Fields, AxisReexecute);
ALGORITHM_PARAMETER_DEF(Fields, SliderReexecute);

MODULE_INFO_DEF(GetSliceFromStructuredFieldByIndices, NewField, SCIRun)

GetSliceFromStructuredFieldByIndices::GetSliceFromStructuredFieldByIndices() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(OutputMatrix);
}

void GetSliceFromStructuredFieldByIndices::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Dim_i, 0);
  state->setValue(Dim_j, 0);
  state->setValue(Dim_k, 0);
  state->setValue(Index_i, 0);
  state->setValue(Index_j, 0);
  state->setValue(Index_k, 0);
  state->setValue(Axis_ijk, 2);
  state->setValue(SpinBoxReexecute, false);
  state->setValue(AxisReexecute, false);
  state->setValue(SliderReexecute, false);
}

void GetSliceFromStructuredFieldByIndices::execute()
{
  auto inputField = getRequiredInput(InputField);

  /// Get the optional matrix handle from the port. Note if a matrix is
  /// present it is sent down stream. Otherwise it will be created.
  auto inputMatrixOption = getOptionalInput(InputMatrix);

  bool indexesChanged = false;

  VField* ifield = inputField->vfield();
  VMesh*  imesh = inputField->vmesh();

  // Because the field slicer is index based it can only work on
  // structured data. For unstructured data SamplePlane should be used.
  if (imesh->is_unstructuredmesh())
  {
    error("This module is only available for topologically structured data.");
    return;
  }

  // For cell based data the max index is one less than the dimension in each direction
  unsigned int offset;

  if (ifield->basis_order() == 0)
    offset = 1;
  else
    offset = 0;

  // Get the dimensions of the mesh.
  VMesh::dimension_type dims;

  imesh->get_dimensions(dims);

  auto state = get_state();

  /// Check to see if the gui dimensions are different than the field.
  if (dims.size() >= 1)
  {
    if (state->getValue(Dim_i).toInt() != static_cast<int>(dims[0] - offset))
    {
      state->setValue(Dim_i, static_cast<int>(dims[0] - offset));
    }
  }

  if (dims.size() >= 2)
  {
    if (state->getValue(Dim_j).toInt() != static_cast<int>(dims[1] - offset))
    {
      state->setValue(Dim_j, static_cast<int>(dims[1] - offset));
    }
  }

  if (dims.size() >= 3)
  {
    if (state->getValue(Dim_k).toInt() != static_cast<int>(dims[2] - offset))
    {
      state->setValue(Dim_k, static_cast<int>(dims[2] - offset));
    }
  }

  /// An input matrix is present so use the values in it to override
  /// the variables set in the gui.
  /// Column 0 selected axis to slice.
  /// Column 1 index of the axis to slice.
  /// Column 2 dimensions of the data.
  if (inputMatrixOption && *inputMatrixOption)
  {
    auto inputMatrix = *inputMatrixOption;
    if ((inputMatrix->nrows() == 1 && inputMatrix->ncols() == 1))
    {
      /// Check to see what index has been selected.
      if (state->getValue(Axis_ijk).toInt() == 0)
      {
        state->setValue(Index_i, static_cast<int>(inputMatrix->get(0, 0)));
      }

      if (state->getValue(Axis_ijk).toInt() == 1)
      {
        state->setValue(Index_j, static_cast<int>(inputMatrix->get(0, 0)));
      }

      if (state->getValue(Axis_ijk).toInt() == 2)
      {
        state->setValue(Index_k, static_cast<int>(inputMatrix->get(0, 0)));
      }

      indexesChanged = true;
    }

    /// The matrix is optional. If present make sure it is a 3x3 matrix.
    /// The row indices is the axis index. The column is the data.
    else if ((inputMatrix->nrows() == 3 &&
      inputMatrix->ncols() == 3))
    {
      /// Sanity check. Make sure the gui dimensions match the matrix
      /// dimensions.
      if (state->getValue(Dim_i).toInt() != inputMatrix->get(0, 2) ||
        state->getValue(Dim_j).toInt() != inputMatrix->get(1, 2) ||
        state->getValue(Dim_k).toInt() != inputMatrix->get(2, 2))
      {
        std::ostringstream str;
        str << "The dimensions of the matrix slicing do not match the field. "
          << " Expected "
          << state->getValue(Dim_i).toInt() << " "
          << state->getValue(Dim_j).toInt() << " "
          << state->getValue(Dim_k).toInt()
          << " Got "
          << inputMatrix->get(0, 2) << " "
          << inputMatrix->get(1, 2) << " "
          << inputMatrix->get(2, 2);

        error(str.str());
        return;
      }

      /// Check to see what axis has been selected. Only one should be
      /// selected.
      for (index_type i = 0; i < inputMatrix->nrows(); i++)
      {
        if (inputMatrix->get(i, 0) == 1 && state->getValue(Axis_ijk).toInt() != i)
        {
          state->setValue(Axis_ijk, static_cast<int>(i));
          indexesChanged = true;
        }
      }

      /// Check to see what index has been selected and if it matches
      /// the gui index.
      if (state->getValue(Index_i).toInt() != inputMatrix->get(0, 1) ||
        state->getValue(Index_j).toInt() != inputMatrix->get(1, 1) ||
        state->getValue(Index_k).toInt() != inputMatrix->get(2, 1))
      {
        state->setValue(Index_i, static_cast<int>(inputMatrix->get(0, 1)));
        state->setValue(Index_j, static_cast<int>(inputMatrix->get(1, 1)));
        state->setValue(Index_k, static_cast<int>(inputMatrix->get(2, 1)));

        indexesChanged = true;
      }
    }
    else
    {
      error("Input matrix is not a 1x1 or a 3x3 matrix");
      return;
    }
  }

  /// If no data or an input change recreate the field. I.e Only
  /// execute when neeed.

  if (indexesChanged || needToExecute())
  {
    if (ifield->basis_order() == 0)
    {
      VMesh::index_type i_start = 0;
      VMesh::index_type j_start = 0;
      VMesh::index_type k_start = 0;

      VMesh::index_type i_stop = state->getValue(Dim_i).toInt();
      VMesh::index_type j_stop = state->getValue(Dim_j).toInt();
      VMesh::index_type k_stop = state->getValue(Dim_k).toInt();

      VMesh::index_type i_stride = 1;
      VMesh::index_type j_stride = 1;
      VMesh::index_type k_stride = 1;

      /// Get the index for the axis selected.
      if (state->getValue(Axis_ijk).toInt() == 0)
      {
        i_start = state->getValue(Index_i).toInt();
        i_stop = state->getValue(Index_i).toInt() + 1;
      }
      else if (state->getValue(Axis_ijk).toInt() == 1)
      {
        j_start = state->getValue(Index_j).toInt();
        j_stop = state->getValue(Index_j).toInt() + 1;
      }
      else
      {
        k_start = state->getValue(Index_k).toInt();
        k_stop = state->getValue(Index_k).toInt() + 1;
      }

      FieldHandle field_out_handle;

      VMesh::size_type idim_in, jdim_in, kdim_in;
      VMesh::index_type i, j, k, inode, jnode, knode;

      VMesh::dimension_type dims;
      imesh->get_dimensions(dims);

      size_t rank = dims.size();

      if (rank == 3)
      {
        idim_in = dims[0];
        jdim_in = dims[1];
        kdim_in = dims[2];
      }
      else if (rank == 2)
      {
        idim_in = dims[0];
        jdim_in = dims[1];
        kdim_in = 1;
      }
      else if (rank == 1)
      {
        idim_in = dims[0];
        jdim_in = 1;
        kdim_in = 1;
      }

      /// This happens when wrapping.
      if (i_stop <= i_start) i_stop += idim_in;
      if (j_stop <= j_start) j_stop += jdim_in;
      if (k_stop <= k_start) k_stop += kdim_in;

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
        if ((i_stop - i_start) % i_stride) idim_out += (rank >= 1 ? 1 : 0);
        if ((j_stop - j_start) % j_stride) jdim_out += (rank >= 2 ? 1 : 0);
        if ((k_stop - k_start) % k_stride) kdim_out += (rank >= 3 ? 1 : 0);

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

      FieldInformation fi(inputField);
      MeshHandle mesh;

      if (rank == 3)
      {
        dims[0] = idim_out;
        dims[1] = jdim_out;
        dims[2] = kdim_out;
        mesh = CreateMesh(fi, dims[0], dims[1], dims[2]);
      }
      else if (rank == 2)
      {
        dims[0] = idim_out;
        dims[1] = jdim_out;
        mesh = CreateMesh(fi, dims[0], dims[1]);
      }
      else if (rank == 1)
      {
        dims[0] = idim_out;
        mesh = CreateMesh(fi, dims[0]);
      }

      VMesh* omesh = mesh->vmesh();

      field_out_handle = CreateField(fi, mesh);
      VField* ofield = field_out_handle->vfield();
      CopyProperties(*inputField, *field_out_handle);

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
        omesh->set_transform(imesh->get_transform());
        inodeIdx = 0;

        /// Get the orgin of mesh. */
        imesh->get_center(o, inodeIdx);

        /// Set the iterator to the first point.
        inodeIdx += (k_start*(jdim_in*idim_in) + j_start*(idim_in)+i_start);

        /// Get the point.
        imesh->get_center(p, inodeIdx);

        /// Put the new field into the correct location.
        Transform trans;

        trans.pre_translate((Vector)(-o));
        trans.pre_scale(Vector(i_stride, j_stride, k_stride));
        trans.pre_translate((Vector)(o));
        trans.pre_translate((Vector)(p - o));

        omesh->transform(trans);
      }

      /// Index based on the old mesh so that we are assured of getting the last
      /// node even if it forms a "partial" elem.
      for (k = k_start; k < k_stop_stride; k += k_stride)
      {

        /// Check for going past the stop.
        if (k > k_stop) k = k_stop;

        /// Check for overlap.
        if (k - k_stride <= k_start + kdim_in && k_start + kdim_in <= k)
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
        kelemIdx = knode*(jdim_in - 1)*(idim_in - 1);

        for (j = j_start; j < j_stop_stride; j += j_stride)
        {

          /// Check for going past the stop.
          if (j > j_stop) j = j_stop;

          /// Check for overlap.
          if (j - j_stride <= j_start + jdim_in && j_start + jdim_in <= j)
            jnode = j_start;
          else
            jnode = j % jdim_in;

          /// A hack here so that an iterator can be used.
          /// Set this iterator to be at the correct jth index.
          jnodeIdx = knodeIdx;
          jelemIdx = kelemIdx;

          jnodeIdx += jnode*idim_in;
          jelemIdx += jnode*(idim_in - 1);

          for (i = i_start; i < i_stop_stride; i += i_stride)
          {

            /// Check for going past the stop.
            if (i > i_stop)
              i = i_stop;

            /// Check for overlap.
            if (i - i_stride <= i_start + idim_in && i_start + idim_in <= i)
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

            switch (ifield->basis_order())
            {
              case 0:

                if (i + i_stride < i_stop_stride &&
                  j + j_stride < j_stop_stride &&
                  k + k_stride < k_stop_stride)
                {
                  ofield->copy_value(ifield, ielemIdx, oelemIdx);
                  oelemIdx++;
                }
                break;

              case 1:
                ofield->copy_value(ifield, inodeIdx, onodeIdx);
                break;

              default:
                break;
            }

            onodeIdx++;
          }
        }
      }

      // Send the data downstream
      sendOutput(OutputField, field_out_handle);
    }
    else
    {

      FieldHandle field_out_handle;

      index_type old_i, old_j, old_k;
      index_type new_i, new_j;

      VMesh::dimension_type dim;
      imesh->get_dimensions(dim);

      /// Get the dimensions of the old field.
      if (dim.size() == 3)
      {
        old_i = dim[0];
        old_j = dim[1];
        old_k = dim[2];

      }
      else if (dim.size() == 2)
      {
        old_i = dim[0];
        old_j = dim[1];
        old_k = 1;
      }
      else if (dim.size() == 1)
      {
        old_i = dim[0];
        old_j = 1;
        old_k = 1;
      }

      int axis = state->getValue(Axis_ijk).toInt();

      /// Get the dimensions of the new field.
      if (axis == 0)
      {
        new_i = old_j;
        new_j = old_k;
      }
      else if (axis == 1)
      {
        new_i = old_i;
        new_j = old_k;
      }
      else if (axis == 2)
      {
        new_i = old_i;
        new_j = old_j;
      }

      FieldInformation fi(inputField);

      /// 3D LatVol to 2D Image
      if (imesh->is_latvolmesh())
      {
        fi.make_imagemesh();
        MeshHandle mesh = CreateMesh(fi, new_i, new_j);
        field_out_handle = CreateField(fi, mesh);
      }
      /// 3D StructHexVol to 2D StructQuadSurf
      else if (imesh->is_structhexvolmesh())
      {
        fi.make_structquadsurfmesh();
        MeshHandle mesh = CreateMesh(fi, new_i, new_j);
        field_out_handle = CreateField(fi, mesh);
      }
      /// 2D Image to 1D Scanline or
      /// 1D Scanline to 0D Scanline (perhaps it should be pointcloud).
      else if (imesh->is_imagemesh() || imesh->is_scanlinemesh())
      {
        fi.make_scanlinemesh();
        MeshHandle mesh = CreateMesh(fi, new_i);
        field_out_handle = CreateField(fi, mesh);
      }
      /// 2D StructQuadSurf to 1D StructCurve
      else if (imesh->is_structquadsurfmesh())
      {
        fi.make_structcurvemesh();
        MeshHandle mesh = CreateMesh(fi, new_i);
        field_out_handle = CreateField(fi, mesh);
      }
      // 1D StructCurve to 0D PointCloud
      else if (imesh->is_structcurvemesh())
      {
        fi.make_pointcloudmesh();
        field_out_handle = CreateField(fi);

        VMesh* omesh = field_out_handle->vmesh();
        VField* ofield = field_out_handle->vfield();
        omesh->resize_nodes(new_i);
        ofield->resize_values();
      }

      VField* ofield = field_out_handle->vfield();
      VMesh* omesh = field_out_handle->vmesh();
      CopyProperties(*inputField, *field_out_handle);

      auto indexCheck = [&state](const AlgorithmParameterName& name)
      {
        return [&state, &name](int index)
        {
          if (index < 0 || index >= state->getValue(name).toInt())
          {
            std::ostringstream str;
            str << "The selected index slice (" << index << ") is out of range of the field dimensions (0.." << state->getValue(name).toInt() << ").";
            return std::make_tuple(false, str.str());
          }
          else
            return std::make_tuple(true, std::string());
        };
      };
      /// Get the index for the axis selected.
      index_type index;
      if (state->getValue(Axis_ijk).toInt() == 0)
      {
        index = state->getValue(Index_i).toInt();
        auto check = indexCheck(Dim_i)(index);
        IF_CHECK_FAILED_THROW_ALGORITHM_INPUT_ERROR(check);
      }
      else if (state->getValue(Axis_ijk).toInt() == 1)
      {
        index = state->getValue(Index_j).toInt();
        auto check = indexCheck(Dim_j)(index);
        IF_CHECK_FAILED_THROW_ALGORITHM_INPUT_ERROR(check);
      }
      else
      {
        index = state->getValue(Index_k).toInt();
        auto check = indexCheck(Dim_k)(index);
        IF_CHECK_FAILED_THROW_ALGORITHM_INPUT_ERROR(check);
      }

      if (dim.size() == 3)
      {
        old_i = dim[0];
        old_j = dim[1];
        old_k = dim[2];
      }
      else if (dim.size() == 2)
      {
        old_i = dim[0];
        old_j = dim[1];
        old_k = 1;      /// This makes it is possible to slice from 1D to 0D easily.
      }
      else if (dim.size() == 1)
      {
        old_i = dim[0];
        old_j = 1;
        old_k = 1;      /// This makes it is possible to slice from 1D to 0D easily.
      }

      if (axis == 0)
      {
        new_i = old_j;
        new_j = old_k;
      }
      else if (axis == 1)
      {
        new_i = old_i;
        new_j = old_k;
      }
      else if (axis == 2)
      {
        new_i = old_i;
        new_j = old_j;
      }

      VMesh::Node::index_type inodeIdx = 0;
      VMesh::Node::index_type onodeIdx = 0;

      Point p;

      VMesh::index_type i, j;

      if (imesh->is_regularmesh())
      {
        Transform trans = imesh->get_transform();
        double offset = 0.0;
        if (axis == 0)
        {
          trans.post_permute(2, 3, 1);
          offset = index / (double)old_i;
        }
        else if (axis == 1)
        {
          trans.post_permute(1, 3, 2);
          offset = index / (double)old_j;
        }
        else
        {
          offset = index / (double)old_k;
        }
        trans.post_translate(Vector(0.0, 0.0, index));

        omesh->set_transform(trans);
      }

      inodeIdx = 0;
      onodeIdx = 0;

      /// Slicing along the i axis. In order to get the slice in this
      /// direction only one location can be obtained at a time. So the
      /// iterator must increment to the correct column (i) index, the
      /// location samples, and then incremented to the end of the row.
      if (axis == 0)
      {
        for (j = 0; j < new_j; j++)
        {
          for (i = 0; i < new_i; i++)
          {
            // Set the iterator to the correct column (i).
            inodeIdx += index;

            // Get the point and value at this location
            if (omesh->is_irregularmesh())
            {
              imesh->get_center(p, inodeIdx);
              omesh->set_point(p, onodeIdx);
            }

            ofield->copy_value(ifield, inodeIdx, onodeIdx);
            onodeIdx++;

            inodeIdx += old_i - index;
          }
        }
      }
      /// Slicing along the j axis. In order to get the slice in this
      /// direction a complete row can be obtained at a time. So the
      /// iterator must increment to the correct row (j) index, the
      /// location samples, and then incremented to the end of the slice.
      else if (axis == 1)
      {
        for (j = 0; j < new_j; j++)
        {
          /// Set the iterator to the correct row (j).
          inodeIdx += index*old_i;

          /// Get all of the points and values along this row.
          for (i = 0; i < new_i; i++)
          {
            /// Get the point and value at this location
            if (omesh->is_irregularmesh())
            {
              imesh->get_center(p, inodeIdx);
              omesh->set_point(p, onodeIdx);
            }

            ofield->copy_value(ifield, inodeIdx, onodeIdx);
            onodeIdx++;
            inodeIdx++;
          }

          /// Move to the end of all the rows.
          inodeIdx += (old_j - index - 1)*(old_i);
        }
      }
      /// Slicing along the k axis.
      else if (axis == 2)
      {

        inodeIdx += index*old_j*old_i;

        /// Get all of the points and values along this slice.
        for (j = 0; j < new_j; j++)
        {
          for (i = 0; i < new_i; i++)
          {
            /// Get the point and value at this location
            if (omesh->is_irregularmesh())
            {
              imesh->get_center(p, inodeIdx);
              omesh->set_point(p, onodeIdx);
            }

            ofield->copy_value(ifield, inodeIdx, onodeIdx);

            onodeIdx++;
            inodeIdx++;
          }
        }
      }

      // Send the data downstream.
      sendOutput(OutputField, field_out_handle);
    }

    /// Create the output matrix with the axis selected, index, and
    /// dimensions.
    if (!(inputMatrixOption && *inputMatrixOption))
    {
      DenseMatrixHandle selected(new DenseMatrix(3, 3));

      for (int i = 0; i < 3; i++)
        selected->put(i, 0, (double)(state->getValue(Axis_ijk).toInt() == i));

      selected->put(0, 1, state->getValue(Index_i).toInt());
      selected->put(1, 1, state->getValue(Index_j).toInt());
      selected->put(2, 1, state->getValue(Index_k).toInt());

      selected->put(0, 2, state->getValue(Dim_i).toInt());
      selected->put(1, 2, state->getValue(Dim_j).toInt());
      selected->put(2, 2, state->getValue(Dim_k).toInt());

      sendOutput(OutputMatrix, selected);
    }
  }
}
