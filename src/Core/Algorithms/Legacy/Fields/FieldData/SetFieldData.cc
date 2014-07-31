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
   Author            : Moritz Dannhauer
   Last modification : March 24 2014 (ported from SCIRun4)
   TODO: Nrrd input 
*/

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldData.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;

AlgorithmParameterName SetFieldDataAlgo::keepTypeCheckBox("keepTypeCheckBox");
SetFieldDataAlgo::SetFieldDataAlgo()
{ 
  addParameter(keepTypeCheckBox, false);
}

bool SetFieldDataAlgo::verify_input_data(FieldHandle& input_field, DenseMatrixHandle& data, size_type& numvals, FieldInformation& fi) const
{
  VMesh* imesh = input_field->vmesh();
  
  VMesh::size_type numnodes = imesh->num_nodes();
  VMesh::size_type numelems = imesh->num_elems();
  VMesh::size_type numenodes = numnodes+numelems;
  
  bool found = false;

 if ((data->nrows() >= numnodes+numelems) ||
      (data->ncols() >= numnodes+numelems))
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
     if (get(keepTypeCheckBox).getBool()) 
       fi.set_data_type(input_field->vfield()->get_data_type()); 
     else
       fi.set_data_type("double");    
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
  else if ( ((!found) && ((data->ncols() == numnodes) || (data->ncols() == numelems))) || (data->ncols() == numenodes)) 
  {
    found = true;
    
    /// do we have a scalar, vector, or tensor  ?
    if (data->nrows() == 1) 
    {     
     if (get(keepTypeCheckBox).getBool()) 
       fi.set_data_type(input_field->vfield()->get_data_type()); 
     else
       fi.set_data_type("double");   
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
        fi.set_data_type("scalar");
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
        fi.set_data_type("scalar");
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
  
  return found;
}

bool SetFieldDataAlgo::setscalardata(VField* ofield, DenseMatrixHandle& data, size_type numvals, size_type nrows, size_type ncols, size_type numnvals, size_type numevals) const
{
      if (((nrows == 1)&&(ncols == numvals))||((ncols == 1)&&(nrows == numvals)))
      {
       std::vector<double> values(numvals);
       if (((nrows == 1)&&(ncols == numvals))) for (VField::index_type j=0; j<numvals; j++) values[j] = (* data)(0,j);
       if (((ncols == 1)&&(nrows == numvals))) for (VField::index_type j=0; j<numvals; j++) values[j] = (* data)(j,0);
       
       ofield->set_values(values);
       #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
       if (numevals) 
         {
	  std::vector<double> values2(numevals);
	  if (((nrows == 1)&&(ncols == numvals))) for (VField::index_type j=numnvals; j<numvals+numevals; j++) values[j] = (* data)(0,j);
	  if (((ncols == 1)&&(nrows == numvals))) for (VField::index_type j=numnvals; j<numvals+numevals; j++) values[j] = (* data)(j,0);
	  ofield->set_evalues(values2);    
	  //if (numevals) ofield->set_evalues(matrixdata,numevals);
	 }
	#endif 
      }
      else if ((nrows == 1)&&(ncols == 1))
      {
        ofield->set_all_values((* data)(0,0));
      }
      else
      {
	THROW_ALGORITHM_INPUT_ERROR("Internal error (data not scalar)");
        return false;     
      }
    
   return true;   
}

bool SetFieldDataAlgo::setvectordata(VField* ofield, DenseMatrixHandle& data, size_type numvals, size_type nrows, size_type ncols, size_type numnvals, size_type numevals) const
{
    /// Handle Vector values
    if ((ncols == 3)&&(nrows == numvals))
    {
    Vector v;
      for (VMesh::index_type i=0; i< numnvals; i++)
      {
        v[0]=(* data)(i,0); v[1]=(* data)(i,1); v[2]=(* data)(i,2);
        ofield->set_value(v,i);
      }
      #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
      {
        Vector v((* data)(i,0),(* data)(i,1),(* data)(i,2));
        ofield->set_evalue(v,i);
      }
      #endif
    }    
    else if ((nrows == 3)&&(ncols == numvals))
    {
      for (VMesh::index_type i=0; i< numnvals; i++)
      {
  	Vector v((* data)(0,i),(* data)(1,i),(* data)(2,i));
        ofield->set_value(v,i);
      }    
      #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
      {
	Vector v((* data)(0,i),(* data)(1,i),(* data)(2,i));
        ofield->set_evalue(v,i);
      } 
      #endif   
    }    
    else if (((nrows == 1)&&(ncols == 3))||((ncols == 1)&&(nrows == 3)))
    {
      Vector v;
      if ((nrows == 1)&&(nrows == 3)) { v[0]=(* data)(0,0); v[1]=(* data)(0,1); v[2]=(* data)(0,2);}
      if ((ncols == 1)&&(ncols == 3)) { v[0]=(* data)(0,0); v[1]=(* data)(1,0); v[2]=(* data)(2,0);}
      ofield->set_all_values(v);
    }
    else
    {
      THROW_ALGORITHM_INPUT_ERROR("Internal error (data not vector)");
      return false;         
    }
   
 return true;
}

bool SetFieldDataAlgo::settensordata(VField* ofield, DenseMatrixHandle& data, size_type numvals, size_type nrows, size_type ncols, size_type numnvals, size_type numevals) const
{
    /// Fill field with Tensor values
    /// Handle 6 by n data 
    if ((ncols == 6)&&(nrows == numvals))
    {
      Vector v[6];
      for (VMesh::index_type i=0; i< numnvals; i++)
      {		  
	v[0]=(* data)(i,0); v[1]=(* data)(i,1);	v[2]=(* data)(i,2);	  
	v[3]=(* data)(i,3); v[4]=(* data)(i,4);	v[5]=(* data)(i,5);		  
        ofield->set_values(v,i);
      }
      #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
      {
        v[0]=(* data)(i,0); v[1]=(* data)(i,1);	v[2]=(* data)(i,2);	  
	v[3]=(* data)(i,3); v[4]=(* data)(i,4);	v[5]=(* data)(i,5);		  
        ofield->set_evalues(v,i);
      }
      #endif
    }
    else if ((nrows == 6)&&(ncols == numvals))
    {
     Vector v[6];
     for (VMesh::index_type i=0; i< numnvals; i++)
      {
       	v[0]=(* data)(0,i); v[1]=(* data)(1,i);	v[2]=(* data)(2,i);	  
	v[3]=(* data)(3,i); v[4]=(* data)(4,i);	v[5]=(* data)(5,i);		  
        ofield->set_values(v,i);    
      }
     #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
     for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
      {
        v[0]=(* data)(0,i); v[1]=(* data)(1,i);	v[2]=(* data)(2,i);	  
	v[3]=(* data)(3,i); v[4]=(* data)(4,i);	v[5]=(* data)(5,i);		  
        ofield->set_evalues(v,i);
      }
     #endif
    }
    else if (((nrows == 1)&&(ncols == 6))||((ncols == 1)&&(nrows == 6)))
    {
      Vector v;
      if ((nrows == 1)&&(ncols == 6)) {v[0]=(* data)(0,0); v[1]=(* data)(0,1); v[2]=(* data)(0,2); v[3]=(* data)(0,3); v[4]=(* data)(0,4); v[5]=(* data)(0,5);}
      if ((ncols == 1)&&(nrows == 6)) {v[0]=(* data)(0,0); v[1]=(* data)(1,0); v[2]=(* data)(2,0); v[3]=(* data)(3,0); v[4]=(* data)(4,0); v[5]=(* data)(5,0);}
      ofield->set_all_values(v);
    }
    /// Handle 9 by n data 
    else if ((ncols == 9)&&(nrows == numvals))
    {
     Vector v[9];
      for (VMesh::index_type i=0; i< numnvals; i++)
      {		  
	v[0]=(* data)(i,0); v[1]=(* data)(i,1);	v[2]=(* data)(i,2);	  
	v[3]=(* data)(i,3); v[4]=(* data)(i,4);	v[5]=(* data)(i,5);
	v[6]=(* data)(i,6); v[7]=(* data)(i,7);	v[8]=(* data)(i,8);		  
        ofield->set_values(v,i);
      }
      #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
      {
        v[0]=(* data)(i,0); v[1]=(* data)(i,1);	v[2]=(* data)(i,2);	  
	v[3]=(* data)(i,3); v[4]=(* data)(i,4);	v[5]=(* data)(i,5);
	v[6]=(* data)(i,6); v[7]=(* data)(i,7);	v[8]=(* data)(i,8);		  
        ofield->set_evalues(v,i);
      }
      #endif
    }
    else if ((nrows == 9)&&(ncols == numvals))
    {
     Vector v[9];
     for (VMesh::index_type i=0; i< numnvals; i++)
      {
       	v[0]=(* data)(0,i); v[1]=(* data)(1,i);	v[2]=(* data)(2,i);	  
	v[3]=(* data)(3,i); v[4]=(* data)(4,i);	v[5]=(* data)(5,i);
	v[6]=(* data)(6,i); v[7]=(* data)(7,i);	v[8]=(* data)(8,i);		  
        ofield->set_values(v,i);    
      }
     #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER 
     for (VMesh::index_type i=numnvals; i< numevals+numnvals; i++)
      {
        v[0]=(* data)(0,i); v[1]=(* data)(1,i);	v[2]=(* data)(2,i);	  
	v[3]=(* data)(3,i); v[4]=(* data)(4,i);	v[5]=(* data)(5,i);
	v[6]=(* data)(6,i); v[7]=(* data)(7,i);	v[8]=(* data)(8,i);		  
        ofield->set_evalues(v,i);
      }
     #endif
    }
    else if (((nrows == 1)&&(ncols == 9))||((ncols == 1)&&(nrows == 9)))
    {
     Vector v;
     if ((nrows == 1)&&(ncols == 9)) {v[0]=(* data)(0,0); v[1]=(* data)(0,1); v[2]=(* data)(0,2); v[3]=(* data)(0,3); v[4]=(* data)(0,4); v[5]=(* data)(0,5);}
     if ((ncols == 1)&&(nrows == 9)) {v[0]=(* data)(0,0); v[1]=(* data)(1,0); v[2]=(* data)(2,0); v[3]=(* data)(3,0); v[4]=(* data)(4,0); v[5]=(* data)(5,0);}
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
  VMesh::size_type numenodes = numnodes+numelems;
  
  size_type numvals=0; 

  bool found = verify_input_data(input_field, data, numvals, fi);
 
  if (!found)
  {
    THROW_ALGORITHM_INPUT_ERROR("Matrix dimensions do not match any of the fields dimensions");
    return FieldHandle();
  }
  
  FieldHandle output = CreateField(fi, input_field->mesh());

  if (!output) 
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not create output field and output interface");
    return FieldHandle();
  }  

  VField* ofield = output->vfield();
  
  size_type nrows = data->nrows();
  size_type ncols = data->ncols(); 
  
  size_type numnvals = numvals;
  size_type numevals = 0;
  
  if (numvals == numenodes)
  {
    numnvals = numnodes;
    numevals = numvals-numnodes;
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

  FieldHandle output_field;
  output_field = run(input_field, input_matrix);
  
  AlgorithmOutput output;
  output[Variables::OutputField] = output_field;

  return output;
}
