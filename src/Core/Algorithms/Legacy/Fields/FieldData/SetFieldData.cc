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

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldData.h>
//#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

//namespace SCIRunAlgo {

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

/*template <class T>
bool SetFieldDataV(AlgoBase *algo, FieldHandle output, 
                   NrrdDataHandle nrrd, std::string datatype)
{
  VField* vfield = output->vfield();
  T* data = reinterpret_cast<T*>(nrrd->nrrd_->data);
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
  algo->algo_end(); return (true);      
}

bool 
SetFieldDataAlgo::
run(FieldHandle input, MatrixHandle data, FieldHandle& output)
{
  algo_start("SetFieldData");
  if (!(input.get_rep()))
  {
    error("No input field was provided");
    algo_end(); return (false);  
  }

  if (!(data.get_rep()))
  {
    error("No input matrix was provided");
    algo_end(); return (false);    
  }

  FieldInformation fi(input);

  // Get the virtual interface
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
  
  // try to see whether the matrix dimensions fit the field size
  if ((data->nrows() == numnodes) ||
      (data->nrows() == numelems) ||
      (data->nrows() == numenodes))
  {
    size_type ncols = data->ncols();
    if (column_index >= 0) ncols = 1;
  
    // do we have a scalar, vector, or tensor
    if (ncols == 1) 
    {
      std::string scalardatatype;
      get_option("scalardatatype",scalardatatype);
      
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
    
     // do we have a scalar, vector, or tensor  ?
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
    // Do we have a constant that has to be fitted in every field position ?
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
    algo_end(); return (false);
  }
  
  output = CreateField(fi,input->mesh());
  output->copy_properties(input.get_rep());

  if (output.get_rep() == 0) 
  {
    error("Could not create output field and output interface");
    algo_end(); return (false);  
  }  

  VField* ofield = output->vfield();
  
  // Convert the matrix to a dense matrix if it is not
  MatrixHandle densematrix;
  if (!(matrix_is::dense(data)) && !(matrix_is::column(data)))
  {
    // store data in a new handle so it deallocates automatically
    densematrix = data->dense();
  }
  else
  {
    // handle copy
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
      // TODO: upgrade this for HO
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
        algo_end(); return (false);        
      }
    }

  }
  else if (fi.is_vector())
  {
    // Handle Vector values
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
      algo_end(); return (false);        
    }
  }
  else if (fi.is_tensor())
  {
    // Fill field with Tensor values
    // Handle 6 by n data 
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
    // Handle 9 by n data 
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
      algo_end(); return (false);        
    }  
  }

  algo_end(); return (true);
}
*/

SetFieldDataAlgo::SetFieldDataAlgo()
{
  addParameter(keepTypeCheckBox, false);
}


DenseMatrixHandle SetFieldDataAlgo::run(FieldHandle& input_field, DenseMatrixHandle& input_matrix) const
{
  DenseMatrixHandle output;
  
  return output;
}

AlgorithmParameterName SetFieldDataAlgo::keepTypeCheckBox("keepTypeCheckBox");
AlgorithmInputName SetFieldDataAlgo::InputField("InputField");
AlgorithmInputName SetFieldDataAlgo::InputMatrix("InputMatrix");
AlgorithmOutputName SetFieldDataAlgo::OutputMatrix("OutputMatrix");

AlgorithmOutput SetFieldDataAlgo::run_generic(const AlgorithmInput& input) const
{

  auto input_field = input.get<Field>(InputField);
  auto input_matrix = input.get<DenseMatrix>(InputMatrix);
 
  DenseMatrixHandle output_matrix;
  output_matrix = run(input_field,input_matrix);
  
  AlgorithmOutput output;
  output[OutputMatrix] = output_matrix;

  return output;
}

/* bool 
SetFieldDataAlgo::
run(FieldHandle input, NrrdDataHandle data, FieldHandle& output)
{
  algo_start("SetFieldData");

  algo_start("SetFieldData");
  if (!(input.get_rep()))
  {
    error("No input field was provided");
    algo_end(); return (false);  
  }

  if (!(data.get_rep()))
  {
    error("No input nrrd was provided");
    algo_end(); return (false);    
  }
    
  FieldInformation fi(input);

  VMesh* imesh = input->vmesh();
  VMesh::size_type numnodes = imesh->num_nodes();
  VMesh::size_type numelems = imesh->num_elems();

  std::string output_datatype = "";   

  VMesh::size_type numvals = 0;

  // If structured see if the nrrd looks like the mesh
  if( imesh->is_structuredmesh() ) 
  {
    VMesh::dimension_type dims;
    imesh->get_dimensions( dims );

    if( data->nrrd_->dim   == dims.size() ||
        data->nrrd_->dim-1 == dims.size() ) 
    {
      numvals = 1;

      // count number of entries, disregarding vector or tensor
      // components
      for (int d=data->nrrd_->dim-1, m=dims.size()-1; m>=0; d--, m--) 
      {
        numvals *= data->nrrd_->axis[d].size;
      }

      // These are secondary checks and are not really needed but if
      // the data is structured then it should match the mesh.

      // Node check
      if( numvals == numnodes ) 
      {
        fi.make_lineardata();

        // check dimensions
        for (int d=data->nrrd_->dim-1, m=dims.size()-1; m>=0; d--, m--) 
        {
          if (static_cast<Mesh::size_type>(data->nrrd_->axis[d].size) !=
	      dims[m]) 
          {
            numvals = 0;
            break;
          }
        }
      }

      // Element check
      else if( numvals == numelems ) 
      {
        fi.make_constantdata();

        // check dimensions
        for (int d=data->nrrd_->dim-1, m=dims.size()-1; m>=0; d--, m--) 
        {
          if (static_cast<Mesh::size_type>(data->nrrd_->axis[d].size) != dims[m]-1) 
          {
            numvals = 0;
            break;
          }
        }
      }
      // No match
      else 
      {
        numvals = 0;
      }

      if( numvals ) 
      {
        if( data->nrrd_->dim == dims.size() ||
            data->nrrd_->axis[0].size == 1 ) 
        {
          output_datatype = "Scalar";
          std::string scalardatatype;
          get_option("scalardatatype",scalardatatype);
          fi.set_data_type(scalardatatype);
        } 
        else if ( data->nrrd_->axis[0].size == 3 ) 
        {
          output_datatype = "Vector";
          fi.make_vector();
        } 
        else if ( data->nrrd_->axis[0].size == 6 )
        {
          output_datatype = "Tensor6";
          fi.make_tensor();        
        }
        else if ( data->nrrd_->axis[0].size == 7 )
        {
          output_datatype = "Tensor7";
          fi.make_tensor();        
        }
        else if ( data->nrrd_->axis[0].size == 9 )
        {
          output_datatype = "Tensor9";
          fi.make_tensor();
        }
      }
    }
  }

  // If unstructured or a single list
  else
  {
    if( data->nrrd_->dim == 1 &&
        (static_cast<Mesh::size_type>(data->nrrd_->axis[0].size) == numnodes ||
         static_cast<Mesh::size_type>(data->nrrd_->axis[0].size) == numelems) ) 
      {

      numvals = data->nrrd_->axis[0].size;

      output_datatype = "Scalar";
    } 
    else if( data->nrrd_->dim == 2 &&
	     (static_cast<Mesh::size_type>(data->nrrd_->axis[1].size) == numnodes ||
	      static_cast<Mesh::size_type>(data->nrrd_->axis[1].size) == numelems) ) 
    {

      numvals = data->nrrd_->axis[1].size;

      if( data->nrrd_->axis[0].size == 1 ) 
      {
        output_datatype = "Scalar";
      } 
      else if ( data->nrrd_->axis[0].size == 3 ) 
      {
        output_datatype = "Vector";
        fi.make_vector();
      } 
      else if ( data->nrrd_->axis[0].size == 6 )
      {
        output_datatype = "Tensor6";
        fi.make_tensor();        
      }
      else if ( data->nrrd_->axis[0].size == 7 )
      {
        output_datatype = "Tensor7";
        fi.make_tensor();        
      }
      else if ( data->nrrd_->axis[0].size == 9 )
      {
        output_datatype = "Tensor9";
        fi.make_tensor();
      }
    }

    // Nrrd data dimensions do not match but there are enough values
    // for the field so stuff them in anyways.
    else
    {
      numvals = 1;

      for( unsigned int i=0; i<data->nrrd_->dim; ++i)
      {
        numvals *= data->nrrd_->axis[i].size;
      }

      if (numvals == numnodes || numvals == numelems)
      {
        warning("NrrdData dimensions do not match any of the fields dimensions");
        warning("except the total number so reshaping the dimensions to match.");

        output_datatype = "Scalar";
      }
      else
      {
        numvals /= data->nrrd_->axis[0].size;

        if (numvals == numnodes || numvals == numelems)
        {
          warning("NrrdData dimensions do not match any of the fields dimensions");
          warning("except the total number so reshaping the dimensions to match.");

          if ( data->nrrd_->axis[0].size == 3 ) 
          {
            output_datatype = "Vector";
            fi.make_vector();
          } 
          else if ( data->nrrd_->axis[0].size == 6 )
          {
            output_datatype = "Tensor6";
            fi.make_tensor();        
          }
          else if ( data->nrrd_->axis[0].size == 7 )
          {
            output_datatype = "Tensor7";
            fi.make_tensor();        
          }
          else if ( data->nrrd_->axis[0].size == 9 )
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
    algo_end(); return (false);
  }
  
  output = CreateField(fi,input->mesh());

  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);
  }

  switch(data->nrrd_->type)
  {
    case nrrdTypeChar:
      return(SetFieldDataV<char>(this,output,data,output_datatype));
    case nrrdTypeUChar:
      return(SetFieldDataV<unsigned char>(this,output,data,output_datatype));
    case nrrdTypeShort:
      return(SetFieldDataV<short>(this,output,data,output_datatype));
    case nrrdTypeUShort:
      return(SetFieldDataV<unsigned short>(this,output,data,output_datatype));
    case nrrdTypeInt:
      return(SetFieldDataV<int>(this,output,data,output_datatype));
    case nrrdTypeUInt:
      return(SetFieldDataV<unsigned int>(this,output,data,output_datatype));
    case nrrdTypeFloat:
      return(SetFieldDataV<float>(this,output,data,output_datatype));
    case nrrdTypeDouble:
      return(SetFieldDataV<double>(this,output,data,output_datatype));
  }

  error("Nrrd datatype is not supported");
  algo_end(); return (false); 
}

} // namespace SCIRunAlgo */

