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


// NOTE: This MatlabIO file is used in different projects as well. Please, do not
// make it depend on other scirun code. This way it is easier to maintain matlabIO
// code among different projects. Thank you.

/*
 * FILE: matlabarray.h
 * AUTH: Jeroen G Stinstra
 * DATE: 16 MAY 2005
 */

#include <Core/Matlab/matlabarray.h>
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
#include <sstream>

using namespace SCIRun::MatlabIO;


// matlabarray management functions
// constructor/destructor
// assignment/copy constructor
// clear

// constructor
// creates on the place holder for the matlabarray
matlabarray::matlabarray()
  : m_(0)
{
}

// destructor
matlabarray::~matlabarray()
{
  if (m_ != 0)
  {	// delete the attached structure if necessary
    m_->ref_--;
    if (m_->ref_ == 0) { delete m_; }
    m_ = 0;
  }
}

// copy constructor
matlabarray::matlabarray(const matlabarray &m)
{
  m_ = 0;

  // reference the same structure as the matlabarray we are copying
  if (m.m_ != 0)
  {
    m_ = m.m_;
    m_->ref_++;
  }
}



// assignment
matlabarray& matlabarray::operator= (const matlabarray &m)
{ // the same construction as the previous function is used
  // to create a copy

  // if m = m we have to do nothing
  if (this != &m)
  {
    if (m_ != 0)
    {	// delete the attached structure if necessary
      m_->ref_--;
      if (m_->ref_ == 0) { delete m_; }
      m_ = 0;
    }

    if (m.m_ != 0)
    {
      m_ = m.m_;
      m_->ref_++;
    }
  }

  return *this;
}

// clear and empty

// Clear: this function is for clearing the contents of the matrix and dereferencing
// the handle properly. In the matrix nothing needs to be destroyed as all elements
// have their own destructors, hence deleting the object will invoke all routines
// for cleaning up memory

void matlabarray::clear()
{

  if (m_ != 0)
  {	// delete the attached structure if necessary

    m_->ref_--;
    if (m_->ref_ == 0) { delete m_; }
    m_ = 0;
  }
}

// This function can be used to test whether the matrix contains any data
// If not do not try to access the matrix components. This will result in
// an internal_error exception

bool matlabarray::isempty() const
{
  if (m_ == 0) return(true);
  if (!issparse()) if (getnumelements() == 0) return(true);
  return(false);
}

matlabarray matlabarray::clone() const
{
	matlabarray ma;
	if (isempty()) return(ma);

  ma.m_ = new mxarray;
  ma.m_->ref_ = 1;
	ma.m_->class_ = m_->class_;
	ma.m_->type_ = m_->type_;
	ma.m_->flags_ = m_->flags_;
	ma.m_->dims_ = m_->dims_;
	ma.m_->name_ = m_->name_;
	ma.m_->classname_ = m_->classname_;
	ma.m_->string_ = m_->string_;
	ma.m_->fieldnames_ = m_->fieldnames_;

	ma.m_->subarray_.resize(m_->subarray_.size());
	for (int p=0;p<static_cast<int>(m_->subarray_.size());p++)
	{
		ma.m_->subarray_[p] = m_->subarray_[p].clone();
	}

	ma.m_->preal_ = m_->preal_.clone();
	ma.m_->pimag_ = m_->pimag_.clone();
	ma.m_->prows_ = m_->prows_.clone();
	ma.m_->pcols_ = m_->pcols_.clone();

	return(ma);
}


// functions to maintain the matlabarray
// General functions to maintain matrix names class names and
// matrix types/ dimensions. These functions can only be
// called if there is a matrix associated with the matlabarray
// handle.
//
// If an internal_error is thrown it means that the emptiness
// is not checked before calling these functions

matlabarray::mlclass matlabarray::getclass() const
{
  if (m_ == 0) return(mlUNKNOWN);
  return(m_->class_);
}

matlabarray::mitype matlabarray::gettype() const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in gettype()\n";
    throw internal_error();
  }
  return(m_->type_);
}


std::string matlabarray::getclassname() const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getclassname()\n";
    throw internal_error();
  }
  return(m_->classname_);
}

void matlabarray::setclassname(const std::string& classname)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in setclassname()\n";
    throw internal_error();
  }
  m_->classname_ = classname;
}

std::string matlabarray::getname() const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getname()\n";
    throw internal_error();
  }
  return(m_->name_);
}


void matlabarray::setname(const std::string& name)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in setname()\n";
    throw internal_error();
  }
  m_->name_ = name;
}

void matlabarray::setdims(const std::vector<int> &dims)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in setdims()\n";
    throw internal_error();
  }
  if (dims.size() == 0)
	{
		std::vector<int> ndims(2);
		ndims[0] = 0; ndims[1] = 0;
		m_->dims_ = ndims;
		return;
	}
  if (dims.size() == 1)
	{
		std::vector<int> ndims(2);
		ndims[0] = dims[0]; ndims[1] = 1;
		m_->dims_ = ndims;
		return;
	}
  m_->dims_ = dims;
}

void matlabarray::settype(matlabarray::mitype type)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in settype()\n";
    throw internal_error();
	}
  m_->type_ = type;
}

std::vector<int> matlabarray::getdims() const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getdims()\n";
    throw internal_error();
  }
  return(m_->dims_);
}

int matlabarray::getnumdims() const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getnumdims()\n";
    throw internal_error();
  }
  return(static_cast<int>(m_->dims_.size()));
}



int matlabarray::getnumelements() const
{
  if (m_ == 0) return(0);
	if ((m_->class_ == mlSTRUCT)||(m_->class_ == mlOBJECT))
	{
    if(m_->fieldnames_.size() == 0) return(0);
  }
  if (m_->class_ != mlSPARSE)
  {
    int numel = 1;
    int ndims = static_cast<int>(m_->dims_.size());
    for (int p=0; p<ndims;p++) { numel *= m_->dims_[p]; }
    return (numel);
  }
  else
  {
    return(getnnz());
  }
}

int matlabarray::getm() const
{
  if (m_ == 0) return(0);
  return(m_->dims_[0]);
}

int matlabarray::getn() const
{
  if (m_ == 0) return(0);
  return(m_->dims_[1]);
}

// Functions to calculate the index into an array
// Matlab uses the fortran way of numbering the dimensions
// No attempt is made in this to transpose matrices at
// this point.

int matlabarray::sub2index(const std::vector<int> &vec) const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in sub2index()\n";
    throw internal_error();
  }

  int index = 0;
  int ndims = static_cast<int>(m_->dims_.size());

  std::vector<int> proddims(ndims);
  proddims[0] = 1;
  for (int p=1;p<ndims;p++) { proddims[p] = m_->dims_[p-1]*proddims[p-1]; }

  for (int p=0;p<ndims;p++) { index += proddims[p]*vec[p];}
  return(index);
}

std::vector<int> matlabarray::index2sub(int index) const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in index2sub()\n";
    throw internal_error();
  }

  int ndims = static_cast<int>(m_->dims_.size());
  std::vector<int> vec(ndims);

	std::vector<int> proddims(ndims);
	proddims[0] = 1;
	for (int p=1;p<ndims;p++) { proddims[p] = m_->dims_[p-1]*proddims[p-1]; }

  ldiv_t q;
  for (int p=(ndims-1);p>=0;p--)
  {
    q = ldiv(index,proddims[p]);
    index = static_cast<int>(q.rem);
    vec[p]= static_cast<int>(q.quot);
  }
  return(vec);
}


// cell specific functions

matlabarray matlabarray::getcell(int index) const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getcell()\n";
    throw internal_error();
	}

  // No more out of range, but an empty array
  if ((index >= static_cast<int>(m_->subarray_.size()))||(index < 0)) {matlabarray ma; return(ma);}
  return(m_->subarray_[index]);
}

matlabarray matlabarray::getcell(const std::vector<int> &indexvec) const
{
  return(getcell(sub2index(indexvec)));
}

void matlabarray::setcell(int index, const matlabarray& m)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in setcell()\n";
    throw internal_error();
  }

  if ((index >= static_cast<int>(m_->subarray_.size()))||(index < 0)) throw out_of_range();
  m_->subarray_[index] = m;
}

void matlabarray::setcell(const std::vector<int> &indexvec, const matlabarray& m)
{
  setcell(sub2index(indexvec),m);
}



// struct specific functions

std::vector<std::string> matlabarray::getfieldnames() const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getfieldnames()\n";
    throw internal_error();
	}

  if ((m_->class_ != mlSTRUCT)&&(m_->class_ != mlOBJECT)) throw internal_error();
	return(m_->fieldnames_);
}

int matlabarray::getnumfields() const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getnumfields()\n";
    throw internal_error();
  }
  return(static_cast<int>(m_->fieldnames_.size()));
}

std::string matlabarray::getfieldname(int index) const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getfieldname()\n";
    throw internal_error();
	}
  if ((m_->class_ != mlSTRUCT)&&(m_->class_ != mlOBJECT)) throw internal_error();
  if ((index >= static_cast<int>(m_->fieldnames_.size()))||(index < 0)) throw out_of_range();
  return(m_->fieldnames_[index]);
}

int matlabarray::getfieldnameindex(const std::string& fieldname) const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getfieldnameindex()\n";
    throw internal_error();
  }
  if ((m_->class_ != mlSTRUCT)&&(m_->class_ != mlOBJECT)) throw internal_error();
  int index = -1;
  for (int p = 0;p<static_cast<int>(m_->fieldnames_.size());p++) { if (m_->fieldnames_[p] == fieldname) { index = p; break;} }
  return(index);
}

int matlabarray::getfieldnameindexCI(const std::string& fieldname) const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getfieldnameindexCI()\n";
    throw internal_error();
  }
  if ((m_->class_ != mlSTRUCT)&&(m_->class_ != mlOBJECT)) throw internal_error();
  int index = -1;
  for (int p = 0;p< static_cast<int>(m_->fieldnames_.size());p++) { if ( boost::iequals(m_->fieldnames_[p],fieldname)) { index = p; break;} }
  return(index);
}

void matlabarray::setfieldname(int index,const std::string& fieldname)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in setfieldname()\n";
    throw internal_error();
  }
  if ((m_->class_ != mlSTRUCT)&&(m_->class_ != mlOBJECT)) throw internal_error();
  if ((index >= static_cast<int>(m_->fieldnames_.size()))||(index < 0)) throw out_of_range();
  m_->fieldnames_[index] = fieldname;
}

matlabarray matlabarray::getfield(int index,int fieldnameindex) const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getfield()\n";
    throw internal_error();
  }
  if ((m_->class_ != mlSTRUCT)&&(m_->class_ != mlOBJECT))
  {
    std::cerr << "internal error in getfield()\n";
    throw internal_error();
	}
  // if out of range just return an empty array
  if ((fieldnameindex < 0)||(fieldnameindex > static_cast<int>(m_->fieldnames_.size()))) { matlabarray ma; return(ma);}

  index = (index * static_cast<int>(m_->fieldnames_.size())) + fieldnameindex;
  return(getcell(index));
}

matlabarray matlabarray::getfield(int index,const std::string& fieldname) const
{
  return(getfield(index,getfieldnameindex(fieldname)));
}

matlabarray matlabarray::getfield(const std::vector<int> &indexvec,const std::string& fieldname) const
{
  return(getfield(sub2index(indexvec),getfieldnameindex(fieldname)));
}

matlabarray matlabarray::getfieldCI(int index,const std::string& fieldname) const
{
  return(getfield(index,getfieldnameindexCI(fieldname)));
}

matlabarray matlabarray::getfieldCI(const std::vector<int> &indexvec,const std::string& fieldname) const
{
  return(getfield(sub2index(indexvec),getfieldnameindexCI(fieldname)));
}

matlabarray matlabarray::getfield(const std::vector<int> &indexvec,int fieldnameindex) const
{
  return(getfield(sub2index(indexvec),fieldnameindex));
}

void matlabarray::setfield(int index,int fieldnameindex,const matlabarray& m)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in setfield()\n";
    throw internal_error();
  }
  if ((m_->class_ != mlSTRUCT)&&(m_->class_ != mlOBJECT))
  {
    std::cerr << "internal error in setfield()\n";
    throw internal_error();
  }
  if ((fieldnameindex < 0)||(fieldnameindex > static_cast<int>(m_->fieldnames_.size()))) throw out_of_range();

  index = (index * static_cast<int>(m_->fieldnames_.size())) + fieldnameindex;
  setcell(index,m);
}

void matlabarray::setfield(int index,const std::string& fieldname,const matlabarray& m)
{
  int fieldindex = getfieldnameindex(fieldname);
	if (fieldindex == -1)
	{
		fieldindex = addfieldname(fieldname);
	}
	setfield(index,fieldindex,m);
}

void matlabarray::setfield(const std::vector<int> &indexvec,const std::string& fieldname,const matlabarray& m)
{
  int fieldindex = getfieldnameindex(fieldname);
	if (fieldindex == -1)
	{
		fieldindex = addfieldname(fieldname);
	}
  setfield(sub2index(indexvec),fieldindex,m);
}

void matlabarray::setfield(const std::vector<int> &indexvec,int fieldnameindex,const matlabarray& m)
{
  setfield(sub2index(indexvec),fieldnameindex,m);
}


int matlabarray::addfieldname(const std::string& fieldname)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in addfieldname()\n";
    throw internal_error();
  }
  if ((m_->class_ != mlSTRUCT)&&(m_->class_ != mlOBJECT))
  {
    std::cerr << "internal error in addfieldname()\n";
    throw internal_error();
  }

  // Reorder the fieldname array
  int newfieldnum = static_cast<int>(m_->fieldnames_.size());

	m_->fieldnames_.resize(newfieldnum+1);
	m_->fieldnames_[newfieldnum] = fieldname;

  // Reorder the subarray
  int newsize = (newfieldnum+1)*getnumelements();
  std::vector<matlabarray> subarray(newsize);
  for (int p=0,q=0,r=0;p<newsize;p++,q++)
  {
    if (q == newfieldnum)
        { q =0; }
    else
        { subarray[p] = m_->subarray_[r]; r++; }
  }
  m_->subarray_ = subarray;

  return(newfieldnum);
}


void matlabarray::removefieldname(int fieldnameindex)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in removefieldname()\n";
    throw internal_error();
	}
  if ((m_->class_ != mlSTRUCT)&&(m_->class_ != mlOBJECT))
  {
    std::cerr << "internal error in removefieldname()\n";
    throw internal_error();
  }

  // Reorder the fieldname vector
  int numfields = static_cast<int>(m_->fieldnames_.size());

  std::vector<std::string> fieldnames(numfields-1);
  for (int p=0,r=0;p<numfields;p++)
    { if (r != fieldnameindex) { fieldnames[r] = m_->fieldnames_[p]; r++; }}
  m_->fieldnames_ = fieldnames;

  // Reorder the subarray vector
  numfields = (static_cast<int>(m_->fieldnames_.size())*getnumelements());
  std::vector<matlabarray> subarray;
  for (int p=0,q=0,r=0;p<numfields;p++,q++)
  {
    if (q == numfields) {q=0;}
    if (q != fieldnameindex)
    {
      subarray[r] = m_->subarray_[p];
      r++;
    }
  }
  m_->subarray_ = subarray;
}

void matlabarray::removefieldname(const std::string& fieldname)
{
	removefieldname(getfieldnameindex(fieldname));
}

// String specific functions
// The user can alter the contents of a
// string array any time

std::string matlabarray::getstring() const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getstring()\n";
    throw internal_error();
  }
  return(m_->string_);
}

void matlabarray::setstring(const std::string& str)
{
  if (m_ == 0)
  {
    std::cerr << "internal error setstring()\n";
    throw internal_error();
  }
  m_->string_ = str;
  std::vector<int> dims(2);
  dims[0] = 1; dims[1] = static_cast<int>(str.size());
  setdims(dims);
}


// quick function for creating a double scalar
void matlabarray::createdoublescalar(double value)
{
	std::vector<int> dims(2); dims[0] = 1; dims[1] = 1;
	createdensearray(dims,miDOUBLE);
	setnumericarray(&value,1);
}

void matlabarray::createdoublevector(const std::vector<double> &values)
{
	std::vector<int> dims(2); dims[0] = 1; dims[1] = static_cast<int>(values.size());
	createdensearray(dims,miDOUBLE);
	setnumericarray(values);
}

void matlabarray::createdoublevector(int n, const double *values)
{
	std::vector<int> dims(2); dims[0] = 1; dims[1] = n;
	createdensearray(dims,miDOUBLE);
	setnumericarray(values,n);
}


void matlabarray::createdoublematrix(const std::vector<double> &values, const std::vector<int> &dims)
{
	createdensearray(dims,miDOUBLE);
	setnumericarray(values);
}


void matlabarray::createdoublematrix(int m,int n, const double *values)
{
	std::vector<int> dims(2); dims[0] = m; dims[1] = n;
	createdensearray(dims,miDOUBLE);
	setnumericarray(values,m*n);
}

// quick function for creating a int scalar
void matlabarray::createintscalar(int value)
{
	std::vector<int> dims(2); dims[0] = 1; dims[1] = 1;
	createdensearray(dims,miINT32);
	setnumericarray(&value,1);
}

void matlabarray::createintvector(const std::vector<int> &values)
{
	std::vector<int> dims(2); dims[0] = 1; dims[1] = static_cast<int>(values.size());
	createdensearray(dims,miINT32);
	setnumericarray(values);
}

void matlabarray::createintvector(int n, const int *values)
{
	std::vector<int> dims(2); dims[0] = 1; dims[1] = n;
	createdensearray(dims,miINT32);
	setnumericarray(values,n);
}


void matlabarray::createintmatrix(const std::vector<int> &values, const std::vector<int> &dims)
{
	createdensearray(dims,miINT32);
	setnumericarray(values);
}


void matlabarray::createintmatrix(int m,int n, const int *values)
{
	std::vector<int> dims(2); dims[0] = m; dims[1] = n;
	createdensearray(dims,miINT32);
	setnumericarray(values,m*n);
}

// creation functions

void matlabarray::createdensearray(const std::vector<int> &dims,mitype type)
{
  // function only works for numeric types

  clear(); // make sure there is no data

  m_ = new mxarray;
  m_->ref_ = 1;

  m_->class_ = mlDENSE;
  m_->type_ = type;
  m_->flags_ = 0;

  setdims(dims);

  // the numeric data can be inserted later
}

void matlabarray::createdensearray(int m,int n,mitype type)
{
	std::vector<int> dims(2); dims[0] = m; dims[1] = n;
  // function only works for numeric types

  clear(); // make sure there is no data

  m_ = new mxarray;
  m_->ref_ = 1;

  m_->class_ = mlDENSE;
  m_->type_ = type;
  m_->flags_ = 0;

  setdims(dims);

  // the numeric data can be inserted later
}

void matlabarray::createsparsearray(const std::vector<int> &dims,mitype type)
{
  clear(); // make sure there is no data

  m_ = new mxarray;
  m_->ref_ = 1;
  m_->class_ = mlSPARSE;
  m_->type_ = type;  // need to add some type checking here
  m_->flags_ = 0;

  setdims(dims);

  // actual data can be added lateron
}

void matlabarray::createsparsearray(int m,int n,mitype type)
{
	std::vector<int> dims(2); dims[0] = m; dims[1] = n;

  clear(); // make sure there is no data

  m_ = new mxarray;
  m_->ref_ = 1;
  m_->class_ = mlSPARSE;
  m_->type_ = type;  // need to add some type checking here
  m_->flags_ = 0;

  setdims(dims);

  // actual data can be added lateron
}


void matlabarray::createcellarray(const std::vector<int> &dims)
{
  clear(); // make sure there is no data

  m_ = new mxarray;
  m_->ref_ = 1;

  m_->class_ = mlCELL;
  m_->type_ = miMATRIX;  // need to add some type checking here
  m_->flags_ = 0;

  setdims(dims);

	m_->subarray_.clear();
  m_->subarray_.resize(getnumelements());
}

void matlabarray::createstructarray(const std::vector<std::string> &fieldnames)
{
	std::vector<int> dims(2);
	dims[0] = 1;
	dims[1] = 1;
	createstructarray(dims,fieldnames);
}

void matlabarray::createstructarray(const std::vector<int> &dims, const std::vector<std::string> &fieldnames)
{
  clear(); // make sure there is no data

  m_ = new mxarray;
  m_->ref_ = 1;
  m_->class_ = mlSTRUCT;
  m_->type_ = miMATRIX;  // need to add some type checking here
  m_->flags_ = 0;

  setdims(dims);
  m_->fieldnames_ = fieldnames;
	m_->subarray_.clear();
  m_->subarray_.resize(getnumelements()*getnumfields());
}

void matlabarray::createstructarray()
{
  clear(); // make sure there is no data

  m_ = new mxarray;
  m_->ref_ = 1;
  m_->class_ = mlSTRUCT;
  m_->type_ = miMATRIX;  // need to add some type checking here
  m_->flags_ = 0;

	std::vector<int> dims(2);
	dims[0] = 1;
	dims[1] = 1;
  setdims(dims);

	m_->fieldnames_.resize(0);
	m_->subarray_.clear();
  m_->subarray_.resize(0);
}

void matlabarray::createclassarray(const std::vector<std::string> &fieldnames,const std::string& classname)
{
  std::vector<int> dims(2);
  dims[0] = 1;
  dims[1] = 1;
  createclassarray(dims,fieldnames,classname);
}


void matlabarray::createclassarray(const std::vector<int> &dims,const std::vector<std::string> &fieldnames,const std::string& classname)
{
  clear(); // make sure there is no data

  m_ = new mxarray;
  m_->ref_ = 1;
  m_->class_ = mlOBJECT;
  m_->type_ = miMATRIX;  // need to add some type checking here
  m_->flags_ = 0;
  setdims(dims);
  m_->classname_ = classname;
  m_->fieldnames_ = fieldnames;
  m_->subarray_.clear();
  m_->subarray_.resize(getnumelements()*getnumfields());
}

void matlabarray::createstringarray()
{
	std::string str("");
  createstringarray(str);
}

void matlabarray::createstringarray(const std::string& str)
{
  clear(); // make sure there is no data

  m_ = new mxarray;
  m_->ref_ = 1;
  m_->class_ = mlSTRING;
  m_->type_ = miUINT8;
  m_->flags_ = 0;

	setstring(str);
}


// sparse functions

int matlabarray::getnnz() const
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getnnz()\n";
    throw internal_error();
  }
  if (m_->dims_.size() < 2) { return(0); }
  int n = m_->dims_[1];
  if (m_->pcols_.size() < n) { return(0); }
  return(m_->pcols_.getandcastvalue<int>(n));
}


// Raw access to data

matfiledata matlabarray::getpreal()
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getpreal()\n";
    throw internal_error();
  }
  return( m_->preal_);
}

matfiledata matlabarray::getpimag()
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getpimag()\n";
    throw internal_error();
  }
  return( m_->pimag_);
}

matfiledata matlabarray::getprows()
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getprows()\n";
    throw internal_error();
  }
  return( m_->prows_);
}

matfiledata matlabarray::getpcols()
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getpcols()\n";
    throw internal_error();
  }
  return( m_->pcols_);
}

std::string matlabarray::getinfotext() const
{
	return(getinfotext(""));
}

std::string matlabarray::getinfotext(const std::string& name) const
{
  if (m_ == 0) return(std::string("[EMPTY MATRIX]"));

	std::ostringstream oss;
	if (name.size() == 0)
	{
		oss << m_->name_ << "  ";
		if (m_->name_.length() < 40) oss << std::string(40-(m_->name_.length()),' ');
	}
	else
	{
		oss << name << "  ";
		if (name.length() < 40) oss << std::string(40-(name.length()),' ');
	}

	oss << "[ ";
	for (int p=0;p<static_cast<int>(m_->dims_.size());p++)  { oss << m_->dims_[p]; if (p<(static_cast<int>(m_->dims_.size())-1)) oss << "x "; }

	switch (m_->class_)
	{
		case mlDENSE:
			switch(m_->type_)
			{
				case miINT8: oss << " INT8"; break;
				case miUINT8: oss << " UINT8"; break;
				case miINT16: oss << " INT16"; break;
				case miUINT16: oss << " UINT16"; break;
				case miINT32: oss << " INT32"; break;
				case miUINT32: oss << " UINT32"; break;
				case miINT64: oss << " INT64"; break;
				case miUINT64: oss << " UINT64"; break;
				case miSINGLE: oss << " SINGLE"; break;
				case miDOUBLE: oss << " DOUBLE"; break;
				case miUTF8: oss << " UTF8"; break;
				case miUTF16: oss << " UTF16"; break;
				case miUTF32: oss << " UTF32"; break;
				default: oss << " UNKNOWN"; break;
			}
			oss << " ]";
			break;
		case mlSPARSE:
			oss << " SPARSE ]";
			break;
		case mlCELL:
			oss << " CELL ]";
			break;
		case mlSTRUCT:
			oss << " STRUCT ]";
			break;
		case mlSTRING:
			oss << " STRING ]";
			break;
		case mlOBJECT:
			oss << " OBJECT ]";
			break;
		default:
			oss << " UNKNOWN ]";
			break;
	}
	return(oss.str());
}


void matlabarray::setcomplex(bool val)
{
	if (m_ == 0)
  {
    std::cerr << "internal error in setcomplex()\n";
    throw internal_error();
	}
  m_->flags_ &= 0xFE;
	if (val) m_->flags_ |= 0x01;
}

void matlabarray::setlogical(bool val)
{
	if (m_ == 0)
  {
    std::cerr << "internal error in setlogical()\n";
    throw internal_error();
	}
  m_->flags_ &= 0xFD;
	if (val) m_->flags_ |= 0x02;
}

void matlabarray::setglobal(bool val)
{
	if (m_ == 0)
  {
    std::cerr << "internal error in setglobal()\n";
    throw internal_error();
	}
  m_->flags_ &= 0xFB;
	if (val) m_->flags_ |= 0x04;
}

bool matlabarray::iscomplex() const
{
	if (m_ == 0) return(false);
  return(m_->flags_ & 0x01) != 0;
}

bool matlabarray::islogical() const
{
	if (m_ == 0) return(false);
	return(m_->flags_ & 0x02) != 0;
}

bool matlabarray::isglobal() const
{
	if (m_ == 0) return(false);
	return(m_->flags_ & 0x04) != 0;
}

bool matlabarray::isnumeric() const
{
	if (m_ == 0) return(false);
	return(((m_->class_ == mlSPARSE)||(m_->class_ == mlDENSE)));
}

bool matlabarray::isstruct() const
{
	if (m_ == 0) return(false);
	return(((m_->class_ == mlSTRUCT)||(m_->class_ == mlOBJECT)));
}

bool matlabarray::iscell() const
{
	if (m_ == 0) return(false);
	return((m_->class_ == mlCELL));
}

bool matlabarray::isclass() const
{
	if (m_ == 0) return(false);
	return((m_->class_ == mlOBJECT));
}

bool matlabarray::isstring() const
{
	if (m_ == 0) return(false);
	return((m_->class_ == mlSTRING));
}

bool matlabarray::isdense() const
{
	if (m_ == 0) return(false);
	return((m_->class_ == mlDENSE));
}

bool matlabarray::issparse() const
{
	if (m_ == 0) return(false);
	return((m_->class_ == mlSPARSE));
}

bool matlabarray::isfield(const std::string& fieldname) const
{
	return(getfieldnameindex(fieldname) != -1);
}

bool matlabarray::isfieldCI(const std::string& fieldname) const
{
	return(getfieldnameindexCI(fieldname) != -1);
}


// Private functions
//
//int matlabarray::cmp_nocase(const std::string &s1,const std::string &s2)
//{
//	std::string::const_iterator p1 = s1.begin();
//	std::string::const_iterator p2 = s2.begin();
//
//	while (p1!=s1.end() && p2 != s2.end())
//	{
//		if (toupper(*p1) != toupper(*p2)) return (toupper(*p1) < toupper(*p2)) ? -1 : 1;
//		p1++; p2++;
//	}
//
//	return((s2.size()==s1.size()) ? 0 : (s1.size() < s2.size()) ? -1 :  1);
//}

void matlabarray::permute(const std::vector<int>& permorder)
{
	if (m_ == 0)
  {
    std::cerr << "internal error in permute()\n";
    throw internal_error();
	}
  if (m_->class_ != mlDENSE)
  {
    std::cerr << "internal error in permute()\n";
    throw internal_error(); // Other types are not yet implemented
	}

  // In case we are in information mode, there may be no data stored in
  // the matrix. Hence if there is no data only swap the header
  if (m_->preal_.size() > 0)
  {
    std::vector<int> neworder;
    reorder_permute(neworder,permorder);

    // this construction creates a new reordered matfiledata object
    // and destroys the old one.
    m_->preal_ = m_->preal_.reorder(neworder);
    if (iscomplex()) m_->pimag_ = m_->pimag_.reorder(neworder);
	}

	int dsize = static_cast<int>(m_->dims_.size());
	std::vector<int> dims(dsize);
	for (int p = 0;  p< dsize;p++) dims[p] = m_->dims_[permorder[p]];
	m_->dims_ = dims;
}

void matlabarray::transpose()
{
	if (m_ == 0)
  {
    std::cerr << "internal error in transpose()\n";
    throw internal_error();
	}
  if (m_->dims_.size() != 2)
  {
    std::cerr << "internal error in transpose()\n";
    throw internal_error();
	}
  if (m_->class_ != mlDENSE)
  {
    std::cerr << "internal error in transpose()\n";
    throw internal_error(); // Other types are not yet implemented
	}

	std::vector<int> permorder(2);
	permorder[0] = 1;
	permorder[1] = 0;

  if (m_->preal_.size() > 0)
  {
    std::vector<int> neworder;
    reorder_permute(neworder,permorder);

    // this construction creates a new reordered matfiledata object
    // and destroys the old one.
    m_->preal_ = m_->preal_.reorder(neworder);
    if (iscomplex()) m_->pimag_ =m_->pimag_.reorder(neworder);
	}

	int dsize = static_cast<int>(m_->dims_.size());
	std::vector<int> dims(dsize);
	for (int p = 0 ; p < dsize ; p++) dims[p] = m_->dims_[permorder[p]];
	m_->dims_ = dims;
}

void matlabarray::reorder_permute(std::vector<int> &newindices, const std::vector<int>& permorder)
{
	// check whether we can permute this matrix
	if (m_ == 0)
  {
    std::cerr << "internal error in reorder_permute()\n";
    throw internal_error();
	}
  if (m_->dims_.size() != permorder.size())
  {
    std::cerr << "internal error in reorder_permute()\n";
    throw internal_error();
	}
  newindices.resize(getnumelements());

	int size = static_cast<int>(m_->dims_.size());

	for (int p = 0; p < size; p++)
	{
    if ((permorder[p] < 0)||(permorder[p] >= size)) throw out_of_range();
  }

	std::vector<int> dims(size);
	std::vector<int> ndims(size);
	std::vector<int> index(size);
	std::vector<int> cdims(size);
	std::vector<int> ncdims(size);

	int m = 1;
	for (int p = 0; p < size; p++)
	{
		cdims[p] = m;
		dims[p] = m_->dims_[p];
		m *= dims[p];
	}

	for (int p = 0; p < size; p++)
	{
		ncdims[p] = cdims[permorder[p]];
		ndims[p] = dims[permorder[p]];
	}

	size_t numel = newindices.size();
	for (size_t p = 0; p < numel; p++)
	{
		newindices[p] = 0;
		for (int q = 0;q < size; q++) {newindices[p] += index[q]*ncdims[q];}

		index[0]++;
		if (index[0] == ndims[0])
		{
			int q = 0;
			while ((q < size)&&(index[q] == ndims[q]))
			{
				index[q] = 0;
				q++;
				if (q == size) break;
				index[q]++;
			}
		}
	}

}

bool matlabarray::compare(const std::string& str) const
{
	if (m_ == 0) return(false);
	if (str == m_->string_) return(true);
	return(false);
}

bool matlabarray::compareCI(const std::string& str) const
{
	if (m_ == 0) return(false);
	return boost::iequals(str,m_->string_);
}
