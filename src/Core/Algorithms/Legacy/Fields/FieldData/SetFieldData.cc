/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
   Author            : Moritz Dannhauer
   Last modification : March 24 2014 (ported from SCIRun4)
   TODO: Nrrd input
   */

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldData.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>
#include <teem/nrrd.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;

ALGORITHM_PARAMETER_DEF(Fields, keepTypeCheckBox);

SetFieldDataAlgo::SetFieldDataAlgo()
{
  addParameter(Parameters::keepTypeCheckBox, false);
}

bool SetFieldDataAlgo::verify_input_data(FieldHandle input_field, DenseMatrixHandle data, size_type& numvals, FieldInformation& fi) const
{
  VMesh* imesh = input_field->vmesh();

  VMesh::size_type numnodes = imesh->num_nodes();
  VMesh::size_type numelems = imesh->num_elems();
  VMesh::size_type numenodes = numnodes + numelems;

  bool found = false;

  if ((data->nrows() >= numnodes + numelems) ||
    (data->ncols() >= numnodes + numelems))
  {
    imesh->synchronize(Mesh::EDGES_E);
    numenodes = numnodes + imesh->num_edges();
  }

  /// try to see whether the matrix dimensions fit the field size
  if ((data->nrows() == numnodes) ||
    (data->nrows() == numelems) ||
    (data->nrows() == numenodes))
  {
    size_type ncols = data->ncols();

    /// do we have a scalar, vector, or tensor
    if (ncols == 1)
    {
      found = true;
      if (get(Parameters::keepTypeCheckBox).toBool())
        fi.set_data_type(input_field->vfield()->get_data_type());
      else
        fi.set_data_type("double");
    }
    else if (ncols == 3)
    {
      fi.make_vector();
      found = true;
    }
    else if ((ncols == 6) ||
      (ncols == 7) ||
      (ncols == 9))
    {
      fi.make_tensor();
      found = true;
    }

    if (found)
    {
      numvals = data->nrows();
      if ((numnodes != numelems) || (numvals == numenodes))
      {
        if (numvals == numnodes) fi.make_lineardata();
        else if (numvals == numelems) fi.make_constantdata();
        else if (numvals == numenodes) fi.make_quadraticdata();
        else found = false;
      }
      else if ((!(fi.is_lineardata())) && (!(fi.is_constantdata())))
      {
        if (numvals == numnodes) fi.make_lineardata();
        else found = false;
      }
    }
  }
  else if (((!found) && ((data->ncols() == numnodes) || (data->ncols() == numelems))) || (data->ncols() == numenodes))
  {
    found = true;

    /// do we have a scalar, vector, or tensor  ?
    if (data->nrows() == 1)
    {
      if (get(Parameters::keepTypeCheckBox).toBool())
        fi.set_data_type(input_field->vfield()->get_data_type());
      else
        fi.set_data_type("double");
    }

    else if (data->nrows() == 3)
    {
      fi.make_vector();
    }
    else if ((data->nrows() == 6) ||
      (data->nrows() == 7) ||
      (data->nrows() == 9))
    {
      fi.make_tensor();
    }
    else
    {
      found = false;
    }

    if (found)
    {
      numvals = data->ncols();
      if ((numnodes != numelems) || (numvals == numenodes))
      {
        if (numvals == numnodes) fi.make_lineardata();
        else if (numvals == numelems) fi.make_constantdata();
        else if (numvals == numenodes) fi.make_quadraticdata();
        else found = false;
      }
      else if ((!(fi.is_lineardata())) && (!(fi.is_constantdata())))
      {
        if (numvals == numnodes) fi.make_lineardata();
        else found = false;
      }
    }
  }
  else
  {
    /// Do we have a constant that has to be fitted in every field position ?
    if (data->nrows() == 1)
    {
      found = true;
      if (data->ncols() == 1)
      {
        fi.set_data_type("scalar");
      }
      else if (data->ncols() == 3)
      {
        fi.make_vector();
      }
      else if ((data->ncols() == 6) ||
        (data->nrows() == 7) ||
        (data->ncols() == 9))
      {
        fi.make_tensor();
      }
      else
      {
        found = false;
      }
    }
    else if (data->ncols() == 1)
    {
      found = true;
      if (data->nrows() == 1)
      {
        fi.set_data_type("scalar");
      }
      else if (data->nrows() == 3)
      {
        fi.make_vector();
      }
      else if ((data->nrows() == 6) ||
        (data->nrows() == 7) ||
        (data->nrows() == 9))
      {
        fi.make_tensor();
      }
      else
      {
        found = false;
      }
    }
  }

  return found;
}

bool SetFieldDataAlgo::setscalardata(VField* ofield, DenseMatrixHandle data, size_type numvals, size_type nrows, size_type ncols, size_type numnvals, size_type numevals) const
{
  if (((nrows == 1) && (ncols == numvals)) || ((ncols == 1) && (nrows == numvals)))
  {
    std::vector<double> values(numvals);
    if (((nrows == 1) && (ncols == numvals))) for (VField::index_type j = 0; j < numvals; j++) values[j] = (*data)(0, j);
    if (((ncols == 1) && (nrows == numvals))) for (VField::index_type j = 0; j < numvals; j++) values[j] = (*data)(j, 0);

    ofield->set_values(values);

    if (numevals)
    {
      std::vector<double> values2(numevals);
      if (((nrows == 1)&&(ncols == numvals))) for (VField::index_type j=numnvals; j<numvals+numevals; j++) values[j] = (* data)(0,j);
      if (((ncols == 1)&&(nrows == numvals))) for (VField::index_type j=numnvals; j<numvals+numevals; j++) values[j] = (* data)(j,0);
      ofield->set_evalues(values2);
      //if (numevals) ofield->set_evalues(matrixdata,numevals);
    }

  }
  else if ((nrows == 1) && (ncols == 1))
  {
    ofield->set_all_values((*data)(0, 0));
  }
  else
  {
    THROW_ALGORITHM_INPUT_ERROR("Internal error (data not scalar)");
    return false;
  }

  return true;
}

bool SetFieldDataAlgo::setvectordata(VField* ofield, DenseMatrixHandle data, size_type numvals, size_type nrows, size_type ncols, size_type numnvals, size_type numevals) const
{
  /// Handle Vector values
  if ((ncols == 3) && (nrows == numvals))
  {
    Vector v;
    for (VMesh::index_type i = 0; i < numnvals; i++)
    {
      v[0] = (*data)(i, 0); v[1] = (*data)(i, 1); v[2] = (*data)(i, 2);
      ofield->set_value(v, i);
    }
    for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
    {
      Vector v((* data)(i,0),(* data)(i,1),(* data)(i,2));
      ofield->set_evalue(v,i);
    }
  }
  else if ((nrows == 3) && (ncols == numvals))
  {
    for (VMesh::index_type i = 0; i < numnvals; i++)
    {
      Vector v((*data)(0, i), (*data)(1, i), (*data)(2, i));
      ofield->set_value(v, i);
    }
    for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
    {
      Vector v((* data)(0,i),(* data)(1,i),(* data)(2,i));
      ofield->set_evalue(v,i);
    }
  }
  else if (((nrows == 1) && (ncols == 3)) || ((ncols == 1) && (nrows == 3)))
  {
    Vector v;
    if ((nrows == 1) && (nrows == 3)) { v[0] = (*data)(0, 0); v[1] = (*data)(0, 1); v[2] = (*data)(0, 2); }
    if ((ncols == 1) && (ncols == 3)) { v[0] = (*data)(0, 0); v[1] = (*data)(1, 0); v[2] = (*data)(2, 0); }
    ofield->set_all_values(v);
  }
  else
  {
    THROW_ALGORITHM_INPUT_ERROR("Internal error (data not vector)");
    return false;
  }

  return true;
}

bool SetFieldDataAlgo::settensordata(VField* ofield, DenseMatrixHandle data, size_type numvals, size_type nrows, size_type ncols, size_type numnvals, size_type numevals) const
{
  /// Fill field with Tensor values
  /// Handle 6 by n data
  if ((ncols == 6) && (nrows == numvals))
  {
    Vector v[6];
    for (VMesh::index_type i = 0; i < numnvals; i++)
    {
      v[0] = (*data)(i, 0); v[1] = (*data)(i, 1);	v[2] = (*data)(i, 2);
      v[3] = (*data)(i, 3); v[4] = (*data)(i, 4);	v[5] = (*data)(i, 5);
      ofield->set_values(v, i);
    }
    for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
    {
      v[0]=(* data)(i,0); v[1]=(* data)(i,1);	v[2]=(* data)(i,2);
      v[3]=(* data)(i,3); v[4]=(* data)(i,4);	v[5]=(* data)(i,5);
      ofield->set_evalues(v,i);
    }
  }
  else if ((nrows == 6) && (ncols == numvals))
  {
    Vector v[6];
    for (VMesh::index_type i = 0; i < numnvals; i++)
    {
      v[0] = (*data)(0, i); v[1] = (*data)(1, i);	v[2] = (*data)(2, i);
      v[3] = (*data)(3, i); v[4] = (*data)(4, i);	v[5] = (*data)(5, i);
      ofield->set_values(v, i);
    }
    for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
    {
      v[0]=(* data)(0,i); v[1]=(* data)(1,i);	v[2]=(* data)(2,i);
      v[3]=(* data)(3,i); v[4]=(* data)(4,i);	v[5]=(* data)(5,i);
      ofield->set_evalues(v,i);
    }
  }
  else if (((nrows == 1) && (ncols == 6)) || ((ncols == 1) && (nrows == 6)))
  {
    Vector v;
    if ((nrows == 1) && (ncols == 6)) { v[0] = (*data)(0, 0); v[1] = (*data)(0, 1); v[2] = (*data)(0, 2); v[3] = (*data)(0, 3); v[4] = (*data)(0, 4); v[5] = (*data)(0, 5); }
    if ((ncols == 1) && (nrows == 6)) { v[0] = (*data)(0, 0); v[1] = (*data)(1, 0); v[2] = (*data)(2, 0); v[3] = (*data)(3, 0); v[4] = (*data)(4, 0); v[5] = (*data)(5, 0); }
    ofield->set_all_values(v);
  }
  /// Handle 9 by n data
  else if ((ncols == 9) && (nrows == numvals))
  {
    Vector v[9];
    for (VMesh::index_type i = 0; i < numnvals; i++)
    {
      v[0] = (*data)(i, 0); v[1] = (*data)(i, 1);	v[2] = (*data)(i, 2);
      v[3] = (*data)(i, 3); v[4] = (*data)(i, 4);	v[5] = (*data)(i, 5);
      v[6] = (*data)(i, 6); v[7] = (*data)(i, 7);	v[8] = (*data)(i, 8);
      ofield->set_values(v, i);
    }
    for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
    {
      v[0]=(* data)(i,0); v[1]=(* data)(i,1);	v[2]=(* data)(i,2);
      v[3]=(* data)(i,3); v[4]=(* data)(i,4);	v[5]=(* data)(i,5);
      v[6]=(* data)(i,6); v[7]=(* data)(i,7);	v[8]=(* data)(i,8);
      ofield->set_evalues(v,i);
    }
  }
  else if ((nrows == 9) && (ncols == numvals))
  {
    Vector v[9];
    for (VMesh::index_type i = 0; i < numnvals; i++)
    {
      v[0] = (*data)(0, i); v[1] = (*data)(1, i);	v[2] = (*data)(2, i);
      v[3] = (*data)(3, i); v[4] = (*data)(4, i);	v[5] = (*data)(5, i);
      v[6] = (*data)(6, i); v[7] = (*data)(7, i);	v[8] = (*data)(8, i);
      ofield->set_values(v, i);
    }
    for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
    {
      v[0]=(* data)(0,i); v[1]=(* data)(1,i);	v[2]=(* data)(2,i);
      v[3]=(* data)(3,i); v[4]=(* data)(4,i);	v[5]=(* data)(5,i);
      v[6]=(* data)(6,i); v[7]=(* data)(7,i);	v[8]=(* data)(8,i);
      ofield->set_evalues(v,i);
    }
  }
  else if (((nrows == 1) && (ncols == 9)) || ((ncols == 1) && (nrows == 9)))
  {
    Vector v;
    if ((nrows == 1) && (ncols == 9)) { v[0] = (*data)(0, 0); v[1] = (*data)(0, 1); v[2] = (*data)(0, 2); v[3] = (*data)(0, 3); v[4] = (*data)(0, 4); v[5] = (*data)(0, 5); }
    if ((ncols == 1) && (nrows == 9)) { v[0] = (*data)(0, 0); v[1] = (*data)(1, 0); v[2] = (*data)(2, 0); v[3] = (*data)(3, 0); v[4] = (*data)(4, 0); v[5] = (*data)(5, 0); }
    ofield->set_all_values(v);
  }
  else
  {
    THROW_ALGORITHM_INPUT_ERROR("Internal error (data not tensor)");
    return false;
  }

  return true;
}

FieldHandle SetFieldDataAlgo::run(FieldHandle input_field, DenseMatrixHandle data) const
{
  if (!input_field)
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not obtain input field");
    return FieldHandle();
  }

  if (!data)
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not obtain input matrix");
    return FieldHandle();
  }

  FieldInformation fi(input_field);
  VMesh* imesh = input_field->vmesh();

  VMesh::size_type numnodes = imesh->num_nodes();
  VMesh::size_type numelems = imesh->num_elems();
  VMesh::size_type numenodes = numnodes + numelems;

  size_type numvals;

  bool found = verify_input_data(input_field, data, numvals, fi);

  if (!found)
  {
    THROW_ALGORITHM_INPUT_ERROR("Matrix dimensions do not match any of the fields dimensions");
  }

  FieldHandle output = CreateField(fi, input_field->mesh());

  if (!output)
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not create output field and output interface");
  }

  VField* ofield = output->vfield();

  size_type nrows = data->nrows();
  size_type ncols = data->ncols();


  size_type numnvals = numvals;
  size_type numevals = 0;

  if (numvals == numenodes)
  {
    numnvals = numnodes;
    numevals = numvals - numnodes;
  }

  if (fi.is_scalar())
  {
    if (!setscalardata(ofield, data, numvals, nrows, ncols, numnvals, numevals))
      return FieldHandle();
  }
  else if (fi.is_vector())
  {
    if (!setvectordata(ofield, data, numvals, nrows, ncols, numnvals, numevals))
      return FieldHandle();
  }
  else if (fi.is_tensor())
  {
    if (!settensordata(ofield, data, numvals, nrows, ncols, numnvals, numevals))
      return FieldHandle();
  }

  return output;
}

AlgorithmOutput SetFieldDataAlgo::run_generic(const AlgorithmInput& input) const
{
  auto input_field = input.get<Field>(Variables::InputField);
  auto input_matrix = input.get<DenseMatrix>(Variables::InputMatrix);
  auto input_nrrd = input.get<NrrdData>(Variables::InputNrrd);

  FieldHandle output_field;
  if (input_matrix)
    output_field = run(input_field, input_matrix);
  else if (input_nrrd)
    runImpl(input_field, input_nrrd, output_field);

  AlgorithmOutput output;
  output[Variables::OutputField] = output_field;

  return output;
}

template <class T>
bool SetFieldDataV(FieldHandle output, NrrdDataHandle nrrd, const std::string& datatype)
{
  VField* vfield = output->vfield();
  T* data = static_cast<T*>(nrrd->getNrrd()->data);
  VMesh::size_type num_values = vfield->num_values();

  if (datatype == "Scalar")
  {
    vfield->set_values(data,num_values);
  }
  else if (datatype == "Vector")
  {
    VMesh::index_type k = 0;
    for(VMesh::index_type idx=0; idx<num_values; idx++)
    {
      Vector v(data[k],data[k+1],data[k+2]); k += 3;
      vfield->set_value(v,idx);
    }
  }
  else if (datatype == "Tensor6")
  {
    VMesh::index_type k = 0;
    for(VMesh::index_type idx=0; idx<num_values; idx++)
    {
      Tensor tensor(data[k],data[k+1],data[k+2],data[k+3],data[k+4],data[k+5]);
      k += 6;
      vfield->set_value(tensor,idx);
    }
  }
  else if (datatype == "Tensor7")
  {
    VMesh::index_type k = 0;
    for(VMesh::index_type idx=0; idx<num_values; idx++)
    {
      Tensor tensor(data[k+1],data[k+2],data[k+3],data[k+4],data[k+5],data[k+6]);
      k += 7;
      vfield->set_value(tensor,idx);
    }
  }
  else if (datatype == "Tensor9")
  {
    VMesh::index_type k = 0;
    for(VMesh::index_type idx=0; idx<num_values; idx++)
    {
      Tensor tensor(data[k],data[k+1],data[k+2],data[k+4],data[k+5],data[k+8]);
      k += 9;
      vfield->set_value(tensor,idx);
    }
  }
  return (true);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
bool SetFieldDataAlgo::runImpl(FieldHandle input, MatrixHandle data, FieldHandle& output)
{
  algo_start("SetFieldData");
  if (!input)
  {
    error("No input field was provided");
    return (false);
  }

  if (!(data.get_rep()))
  {
    error("No input matrix was provided");
    return (false);
  }

  FieldInformation fi(input);

  /// Get the virtual interface
  VMesh* imesh = input->vmesh();

  VMesh::size_type numnodes = imesh->num_nodes();
  VMesh::size_type numelems = imesh->num_elems();
  VMesh::size_type numenodes = numnodes+numelems;

  int numvals;
  bool found = false;


  if ((data->nrows() >= numnodes+numelems) ||
    (data->ncols() >= numnodes+numelems))
  {
    imesh->synchronize(Mesh::EDGES_E);
    numenodes = numnodes + imesh->num_edges();
  }

  index_type column_index = get_index("column_index");

  /// try to see whether the matrix dimensions fit the field size
  if ((data->nrows() == numnodes) ||
    (data->nrows() == numelems) ||
    (data->nrows() == numenodes))
  {
    size_type ncols = data->ncols();
    if (column_index >= 0) ncols = 1;

    /// do we have a scalar, vector, or tensor
    if (ncols == 1)
    {
      std::string scalardatatype = get_option("scalardatatype");
      fi.set_data_type(scalardatatype);
      found = true;
    }
    else if (ncols == 3)
    {
      fi.make_vector();
      found = true;
    }
    else if ((ncols == 6)||
      (ncols == 7)||
      (ncols == 9))
    {
      fi.make_tensor();
      found = true;
    }

    if (found)
    {
      numvals = data->nrows();
      if ((numnodes != numelems)||(numvals == numenodes))
      {
        if (numvals == numnodes) fi.make_lineardata();
        else if (numvals == numelems) fi.make_constantdata();
        else if (numvals == numenodes) fi.make_quadraticdata();
        else found = false;
      }
      else if ((!(fi.is_lineardata()))&&(!(fi.is_constantdata())))
      {
        if (numvals == numnodes) fi.make_lineardata();
        else found = false;
      }
    }
  }
  else if ((!found)&&((data->ncols() == numnodes) ||
    (data->ncols() == numelems))
    ||(data->ncols() == numenodes))
  {
    found = true;

    /// do we have a scalar, vector, or tensor  ?
    if (data->nrows() == 1)
    {
      std::string scalardatatype;
      get_option("scalardatatype",scalardatatype);

      fi.set_data_type(scalardatatype);
    }
    else if (data->nrows() == 3)
    {
      fi.make_vector();
    }
    else if ((data->nrows() == 6)||
      (data->nrows() == 7)||
      (data->nrows() == 9))
    {
      fi.make_tensor();
    }
    else
    {
      found = false;
    }

    if (found)
    {
      numvals = data->ncols();
      if ((numnodes != numelems)||(numvals == numenodes))
      {
        if (numvals == numnodes) fi.make_lineardata();
        else if (numvals == numelems) fi.make_constantdata();
        else if (numvals == numenodes) fi.make_quadraticdata();
        else found = false;
      }
      else if ((!(fi.is_lineardata()))&&(!(fi.is_constantdata())))
      {
        if (numvals == numnodes) fi.make_lineardata();
        else found = false;
      }
    }
  }
  else
  {
    /// Do we have a constant that has to be fitted in every field position ?
    if (data->nrows() == 1)
    {
      found = true;
      if (data->ncols() == 1)
      {
        std::string scalardatatype;
        get_option("scalardatatype",scalardatatype);

        fi.set_data_type(scalardatatype);
      }
      else if (data->ncols() == 3)
      {
        fi.make_vector();
      }
      else if ((data->ncols() == 6)||
        (data->nrows() == 7)||
        (data->ncols() == 9))
      {
        fi.make_tensor();
      }
      else
      {
        found = false;
      }
    }
    else if (data->ncols() == 1)
    {
      found = true;
      if (data->nrows() == 1)
      {
        std::string scalardatatype;
        get_option("scalardatatype",scalardatatype);

        fi.set_data_type(scalardatatype);
      }
      else if (data->nrows() == 3)
      {
        fi.make_vector();
      }
      else if ((data->nrows() == 6)||
        (data->nrows() == 7)||
        (data->nrows() == 9))
      {
        fi.make_tensor();
      }
      else
      {
        found = false;
      }
    }
  }

  if (!found)
  {
    error("Matrix dimensions do not match any of the fields dimensions");
    return (false);
  }

  output = CreateField(fi,input->mesh());
  CopyProperties(*input, *output);

  if (!output)
  {
    error("Could not create output field and output interface");
    return (false);
  }

  VField* ofield = output->vfield();

  /// Convert the matrix to a dense matrix if it is not
  MatrixHandle densematrix;
  if (!(matrix_is::dense(data)) && !(matrix_is::column(data)))
  {
    /// store data in a new handle so it deallocates automatically
    densematrix = data->dense();
  }
  else
  {
    /// handle copy
    densematrix = data;
  }

  double* matrixdata = densematrix->get_data_pointer();
  size_type nrows = densematrix->nrows();
  size_type ncols = densematrix->ncols();

  size_type numnvals = numvals;
  size_type numevals = 0;

  if (numvals == numenodes)
  {
    numnvals = numnodes;
    numevals = numvals-numnodes;
  }

  if (fi.is_scalar())
  {
    if (column_index >= 0)
    {
      /// @todo: upgrade this for HO
      std::vector<double> values(nrows);
      for (VField::index_type j=0; j<nrows; j++) values[j] = matrixdata[column_index+j*ncols];
      ofield->set_values(values);
    }
    else
    {
      if (((nrows == 1)&&(ncols == numvals))||((ncols == 1)&&(nrows == numvals)))
      {
        ofield->set_values(matrixdata,numnvals);
        if (numevals) ofield->set_evalues(matrixdata,numevals);
      }
      else if ((nrows == 1)&&(ncols == 1))
      {
        ofield->set_all_values(matrixdata[0]);
      }
      else
      {
        error("Internal error (data not scalar)");
        return (false);
      }
    }

  }
  else if (fi.is_vector())
  {
    /// Handle Vector values
    if ((ncols == 3)&&(nrows == numvals))
    {
      int k =0;
      for (VMesh::index_type i=0; i< numnvals; i++)
      {
        Vector v(matrixdata[k],matrixdata[k+1],matrixdata[k+2]);
        ofield->set_value(v,i);
        k += 3;
      }
      for (VMesh::index_type i=0; i< numevals; i++)
      {
        Vector v(matrixdata[k],matrixdata[k+1],matrixdata[k+2]);
        ofield->set_evalue(v,i);
        k += 3;
      }
    }
    else if ((nrows == 3)&&(ncols == numvals))
    {
      for (VMesh::index_type i=0; i< numnvals; i++)
      {
        Vector v(matrixdata[i],matrixdata[i+numvals],matrixdata[i+2*numvals]);
        ofield->set_value(v,i);
      }
      for (VMesh::index_type i=0; i< numevals; i++)
      {
        Vector v(matrixdata[i],matrixdata[i+numvals],matrixdata[i+2*numvals]);
        ofield->set_evalue(v,i);
      }
    }
    else if (((nrows == 1)&&(ncols == 3))||((ncols == 1)&&(nrows == 3)))
    {
      Vector v(matrixdata[0],matrixdata[1],matrixdata[2]);
      ofield->set_all_values(v);
    }
    else
    {
      error("Internal error (data not vector)");
      return (false);
    }
  }
  else if (fi.is_tensor())
  {
    /// Fill field with Tensor values
    /// Handle 6 by n data
    if ((ncols == 6)&&(nrows == numvals))
    {
      int k = 0;
      for (VMesh::index_type i=0; i< numnvals; i++)
      {
        Tensor v(matrixdata[k],matrixdata[k+1],matrixdata[k+2],
          matrixdata[k+3],matrixdata[k+4],matrixdata[k+5]);
        ofield->set_value(v,i);
        k += 6;
      }
      for (VMesh::index_type i=0; i< numevals; i++)
      {
        Tensor v(matrixdata[k],matrixdata[k+1],matrixdata[k+2],
          matrixdata[k+3],matrixdata[k+4],matrixdata[k+5]);
        ofield->set_evalue(v,i);
        k += 6;
      }

    }
    else if ((nrows == 6)&&(ncols == numvals))
    {
      for (VMesh::index_type i=0; i< numnvals; i++)
      {
        Tensor v(matrixdata[i],matrixdata[i+numvals],
          matrixdata[i+2*numvals],matrixdata[i+3*numvals],
          matrixdata[i+4*numvals],matrixdata[i+5*numvals]);
        ofield->set_value(v,i);
      }
      for (VMesh::index_type i=0; i< numevals; i++)
      {
        Tensor v(matrixdata[i],matrixdata[i+numvals],
          matrixdata[i+2*numvals],matrixdata[i+3*numvals],
          matrixdata[i+4*numvals],matrixdata[i+5*numvals]);
        ofield->set_evalue(v,i);
      }
    }
    else if (((nrows == 1)&&(ncols == 6))||((ncols == 1)&&(nrows == 6)))
    {
      Tensor v(matrixdata[0],matrixdata[1],matrixdata[2],
        matrixdata[3],matrixdata[4],matrixdata[5]);
      ofield->set_all_values(v);
    }
    /// Handle 9 by n data
    else if ((ncols == 9)&&(nrows == numvals))
    {
      int k = 0;
      for (VMesh::index_type i=0; i< numnvals; i++)
      {
        Tensor v(matrixdata[k],matrixdata[k+1],matrixdata[k+2],
          matrixdata[k+4],matrixdata[k+5],matrixdata[k+8]);
        ofield->set_value(v,i);
        k += 9;
      }
      for (VMesh::index_type i=0; i< numevals; i++)
      {
        Tensor v(matrixdata[k],matrixdata[k+1],matrixdata[k+2],
          matrixdata[k+4],matrixdata[k+5],matrixdata[k+8]);
        ofield->set_evalue(v,i);
        k += 9;
      }
    }
    else if ((nrows == 9)&&(ncols == numvals))
    {
      for (VMesh::index_type i=0; i< numnvals; i++)
      {
        Tensor v(matrixdata[i],matrixdata[i+numvals],
          matrixdata[i+2*numvals],matrixdata[i+4*numvals],
          matrixdata[i+5*numvals],matrixdata[i+8*numvals]);
        ofield->set_value(v,i);
      }
      for (VMesh::index_type i=0; i< numevals; i++)
      {
        Tensor v(matrixdata[i],matrixdata[i+numvals],
          matrixdata[i+2*numvals],matrixdata[i+4*numvals],
          matrixdata[i+5*numvals],matrixdata[i+8*numvals]);
        ofield->set_evalue(v,i);
      }
    }
    else if (((nrows == 1)&&(ncols == 9))||((ncols == 1)&&(nrows == 9)))
    {
      Tensor v(matrixdata[0],matrixdata[1],matrixdata[2],
        matrixdata[4],matrixdata[5],matrixdata[8]);
      ofield->set_all_values(v);
    }
    else
    {
      error("Internal error (data not tensor)");
      return (false);
    }
  }

  return (true);
}
#endif

bool SetFieldDataAlgo::runImpl(FieldHandle input, NrrdDataHandle data, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "SetFieldData");

  if (!input)
  {
    error("No input field was provided");
    return (false);
  }

  if (!data)
  {
    error("No input nrrd was provided");
    return (false);
  }

  FieldInformation fi(input);

  VMesh* imesh = input->vmesh();
  VMesh::size_type numnodes = imesh->num_nodes();
  VMesh::size_type numelems = imesh->num_elems();

  std::string output_datatype = "";

  VMesh::size_type numvals = 0;

  /// If structured see if the nrrd looks like the mesh
  if( imesh->is_structuredmesh() )
  {
    VMesh::dimension_type dims;
    imesh->get_dimensions( dims );

    if( data->getNrrd()->dim   == dims.size() ||
      data->getNrrd()->dim-1 == dims.size() )
    {
      numvals = 1;

      /// count number of entries, disregarding vector or tensor
      /// components
      for (int d=data->getNrrd()->dim-1, m=dims.size()-1; m>=0; d--, m--)
      {
        numvals *= data->getNrrd()->axis[d].size;
      }

      /// These are secondary checks and are not really needed but if
      /// the data is structured then it should match the mesh.

      /// Node check
      if( numvals == numnodes )
      {
        fi.make_lineardata();

        /// check dimensions
        for (int d=data->getNrrd()->dim-1, m=dims.size()-1; m>=0; d--, m--)
        {
          if (static_cast<Mesh::size_type>(data->getNrrd()->axis[d].size) !=
            dims[m])
          {
            numvals = 0;
            break;
          }
        }
      }

      /// Element check
      else if( numvals == numelems )
      {
        fi.make_constantdata();

        /// check dimensions
        for (int d=data->getNrrd()->dim-1, m=dims.size()-1; m>=0; d--, m--)
        {
          if (static_cast<Mesh::size_type>(data->getNrrd()->axis[d].size) != dims[m]-1)
          {
            numvals = 0;
            break;
          }
        }
      }
      /// No match
      else
      {
        numvals = 0;
      }

      if( numvals )
      {
        if( data->getNrrd()->dim == dims.size() ||
          data->getNrrd()->axis[0].size == 1 )
        {
          if (get(Parameters::keepTypeCheckBox).toBool())
            fi.set_data_type(input->vfield()->get_data_type());
          else
            fi.set_data_type("double");
        }
        else if ( data->getNrrd()->axis[0].size == 3 )
        {
          output_datatype = "Vector";
          fi.make_vector();
        }
        else if ( data->getNrrd()->axis[0].size == 6 )
        {
          output_datatype = "Tensor6";
          fi.make_tensor();
        }
        else if ( data->getNrrd()->axis[0].size == 7 )
        {
          output_datatype = "Tensor7";
          fi.make_tensor();
        }
        else if ( data->getNrrd()->axis[0].size == 9 )
        {
          output_datatype = "Tensor9";
          fi.make_tensor();
        }
      }
    }
  }

  /// If unstructured or a single list
  else
  {
    if( data->getNrrd()->dim == 1 &&
      (static_cast<Mesh::size_type>(data->getNrrd()->axis[0].size) == numnodes ||
      static_cast<Mesh::size_type>(data->getNrrd()->axis[0].size) == numelems) )
    {

      numvals = data->getNrrd()->axis[0].size;

      output_datatype = "Scalar";
    }
    else if( data->getNrrd()->dim == 2 &&
      (static_cast<Mesh::size_type>(data->getNrrd()->axis[1].size) == numnodes ||
      static_cast<Mesh::size_type>(data->getNrrd()->axis[1].size) == numelems) )
    {

      numvals = data->getNrrd()->axis[1].size;

      if( data->getNrrd()->axis[0].size == 1 )
      {
        output_datatype = "Scalar";
      }
      else if ( data->getNrrd()->axis[0].size == 3 )
      {
        output_datatype = "Vector";
        fi.make_vector();
      }
      else if ( data->getNrrd()->axis[0].size == 6 )
      {
        output_datatype = "Tensor6";
        fi.make_tensor();
      }
      else if ( data->getNrrd()->axis[0].size == 7 )
      {
        output_datatype = "Tensor7";
        fi.make_tensor();
      }
      else if ( data->getNrrd()->axis[0].size == 9 )
      {
        output_datatype = "Tensor9";
        fi.make_tensor();
      }
    }

    /// Nrrd data dimensions do not match but there are enough values
    /// for the field so stuff them in anyways.
    else
    {
      numvals = 1;

      for( unsigned int i=0; i<data->getNrrd()->dim; ++i)
      {
        numvals *= data->getNrrd()->axis[i].size;
      }

      if (numvals == numnodes || numvals == numelems)
      {
        warning("NrrdData dimensions do not match any of the fields dimensions");
        warning("except the total number so reshaping the dimensions to match.");

        output_datatype = "Scalar";
      }
      else
      {
        numvals /= data->getNrrd()->axis[0].size;

        if (numvals == numnodes || numvals == numelems)
        {
          warning("NrrdData dimensions do not match any of the fields dimensions");
          warning("except the total number so reshaping the dimensions to match.");

          if ( data->getNrrd()->axis[0].size == 3 )
          {
            output_datatype = "Vector";
            fi.make_vector();
          }
          else if ( data->getNrrd()->axis[0].size == 6 )
          {
            output_datatype = "Tensor6";
            fi.make_tensor();
          }
          else if ( data->getNrrd()->axis[0].size == 7 )
          {
            output_datatype = "Tensor7";
            fi.make_tensor();
          }
          else if ( data->getNrrd()->axis[0].size == 9 )
          {
            output_datatype = "Tensor9";
            fi.make_tensor();
          }
        }
      }
    }

    if (numvals == numnodes) fi.make_lineardata();
    if (numvals == numelems) fi.make_constantdata();
  }

  if (output_datatype == "")
  {
    error("NrrdData dimensions do not match any of the fields dimensions");
    return (false);
  }

  output = CreateField(fi,input->mesh());

  if (!output)
  {
    error("Could not allocate output field");
    return (false);
  }

  switch(data->getNrrd()->type)
  {
  case nrrdTypeChar:
    return(SetFieldDataV<char>(output,data,output_datatype));
  case nrrdTypeUChar:
    return(SetFieldDataV<unsigned char>(output,data,output_datatype));
  case nrrdTypeShort:
    return(SetFieldDataV<short>(output,data,output_datatype));
  case nrrdTypeUShort:
    return(SetFieldDataV<unsigned short>(output,data,output_datatype));
  case nrrdTypeInt:
    return(SetFieldDataV<int>(output,data,output_datatype));
  case nrrdTypeUInt:
    return(SetFieldDataV<unsigned int>(output,data,output_datatype));
  case nrrdTypeFloat:
    return(SetFieldDataV<float>(output,data,output_datatype));
  case nrrdTypeDouble:
    return(SetFieldDataV<double>(output,data,output_datatype));
  }

  error("Nrrd datatype is not supported");
  return (false);
}
