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


// NOTE: This MatlabIO file is used in different projects as well. Please, do not
// make it depend on other scirun code. This way it is easier to maintain matlabIO 
// code among different projects. Thank you.

/*
 * FILE: matlabarray.h
 * AUTH: Jeroen G Stinstra
 * DATE: 16 MAY 2005
 */
 
/*
 * CLASS DESCRIPTION
 *
 * This class is a container for storing a tree of matlab
 * arrays. Structured and cell arrays are stored as a vector of
 * matlabarray objects. Hence, the object can contain sub objects of
 * the same class. In order to have an efficient memory management,
 * the object is a handle and object in one. Each function call uses
 * the handle to access the data in the object. The objects are
 * pointers to the internal data structure of a full matlab
 * array. When aquiring a pointer to one of it subfields it is a
 * pointer to that subarray in the whole three. Hence altering an
 * array at a sub level will automatically change the total array. The
 * class supports building and altering matlabarrays. Once an array
 * has been built, it can be handed over to the matlabfile class to
 * store it in a file.
 *
 * MEMORY MODEL
 *
 * The class maintains its own copies of the data. Each vector, string
 * and other data unit is copied. The object obtains its own memory
 * and frees this as well. The functions for importing and exporting
 * numeric arrays allow you to specify a pointer where data can be
 * written or is stored, the function does not free or allocate any of
 * memory blocks. These functions access the memory block you gave
 * them and copy the data from or to the internal data storage from
 * this memory block.
 *
 * ERROR HANDLING
 *
 * All errors are reported as exceptions described in the matfilebase
 * class.  Errors of external c library functions are caught and
 * forwarded as exceptions.
 *
 * COPYING/ASSIGNMENT
 *
 * Copying the object will not clone the object, but merely copy the
 * handle. Use the clone function to fully copy an object
 *
 * RESOURCE ALLOCATION
 *
 * no external resource are used
 *
 */ 
 
 
#ifndef JGS_MATLABIO_MATLABARRAY_H
#define JGS_MATLABIO_MATLABARRAY_H 1

#include "matfilebase.h"
#include "matfile.h"

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include <stdlib.h>
 
#include "share.h"
 
namespace MatlabIO {

class SCISHARE matlabarray : public matfilebase {
private:
  
  class mxarray {
  public:
    int  ref_; // reference counter: matlabarray is just a pointer to
                // this structure. By keeping the data and the pointer
                // separate a pointer (matlabarray class) can be
                // created to a subarray without having to copy this
                // subclass. Since there can be multiple pointers to
                // the subclasses the reference counter counts how
                // many instances of a matlabarray class point to this
                // one. Creating a new matlabarray class this counter
                // is increased and deleting the matlabarray class
                // will decrease the pointer. When ref_ hits zero the
                // structure is deleted.
                        
    // matrix class information
                            
    mlclass class_;      // type of the array                    
    mitype  type_;      // type of the data in the array 
    unsigned char flags_;   // matrix flags 
    
    // storage for navigating the numericc data
    
    std::vector<int> dims_;    // dimensions of the matrix
    
    std::string name_;        // name of the matrix
    std::string classname_;      // matlab class name 
    std::string string_;      // string array contents
    
    std::vector<std::string> fieldnames_;  // fieldnames
    
    std::vector<matlabarray> subarray_;  // vector with all the subarrays
    
    // storage for the numeric data. Since the data is stored in many
    // formats the data is converted upon output. The mxdata class
    // holds the type and the pointer to the data and as well its size
    // for range checking.
    
    matfiledata preal_;  // data containers (pointers) to the real and
                         // imaginary parts of the data
    matfiledata pimag_;   
    
    matfiledata prows_;  // data containers (pointers) to the row and
                         // column indices for sparse matrices
    matfiledata pcols_;     
    
  };
  
private:
  // The data is not contained inside the object, but only the pointer
  // to the object is stored. So multiple copies of the object can
  // exist, without duplicating data in memory. Especially since
  // matlabarray can be nested as a tree deleting the top one would
  // delete them all. This behaviour is circumvented by only storing a
  // pointer to the real object.
  
  mxarray* m_; 
  
  // raw access for friend classes to the data containers. These
  // functions make a copy of the container handle and will allow
  // direct access to the data stored in memory.  Data changed in a
  // copy of these handles will alter the original data directly
  
  // This access is for the matlabfile class to directly write data
  // into the matlabarray class. For user interactions use
  // getandcast<T> and putandcast<T>, which perform casting as well
  // and copy the data.
public:  
  matfiledata getpreal();
  matfiledata getpimag();
  matfiledata getprows();
  matfiledata getpcols();
  
public:
  
  // Currently this function only sets the dimension
  // vector and does not resize any of the data fields
  void setdims(std::vector<int> &dims);
  
  // Set the type of the data in the matlab array
  void settype(mitype type);
  
  // function to create, copy, and destroy the object
  matlabarray();  // constructor of an empty matrix
  ~matlabarray();  // destructor
  matlabarray(const matlabarray &m); // copy constructor
  matlabarray& operator= (const matlabarray &m); // assignment
  
  // functions to maintain the matlabarray
  
  // clear and empty
  void clear();    // empty matrix (no data stored at all)
  bool isempty();    // check emptiness
  
  // Clone the whole structure
  matlabarray clone();
  
  // use isempty() to check whether you can perform any of the next
  // operations if the matrix is empty (no data stored) and you try to
  // access some data fields the class will throw an internal_error.
  
  mlclass getclass();
  mitype  gettype();
  
  // name is the matrix name. In matlab only the top level matrix
  // names are used any submatrix can have a name, but it is not used
  // in the current versions of matlab
    
  std::string getname();
  void setname(std::string name);
  
  // matrix flags complex indicates a matrix is complex logical
  // indicates that in matlab the matrix is considered logical (zero
  // or non-zero) global indicates it was originally a global matrix
  // in the global workspace
  
  void setcomplex(bool val);
  void setlogical(bool val);
  void setglobal(bool val);
  bool iscomplex();
  bool islogical();
  bool isglobal();
  
  // classname is the name of the object class. Matlab uses this
  // classname to link it to its class "functions".
  
  std::string getclassname();
  void setclassname(std::string classname);
  
  // Everything in matlab has dimensions. The minimum amount of
  // dimensions is two. A smaller dimension will be expanded to
  // two. The dimensions are laid out as follows first the "fast"
  // dimensions and than the slower ones.
  
  std::vector<int> getdims();
  int getnumdims();  // get the number of dimensions
  int getm();    // get the first dimension
  int getn();    // get the second dimension
  
  // number of elements is the product of all the dimensions
  int getnumelements();
  
  // Some functions for indexing. A subindex is a multidimensional
  // vector pointing to the element in a multidimensional sense. A
  // index is the number of the element considering the
  // multidimensional array as one big vector in memory.  The latter
  // method is faster in addressing and is more low level
  
  int sub2index(std::vector<int> &indexvec);
  std::vector<int> index2sub(int index);
  
  // cell specific functions
  // A matrix of the cell class contains subarrays as elements. 
  // getcell() will retrieve these submatrices
  // setcell() will link new matlabarrays as submatrices
  //
  // There are multiple version of this function:
  //
  // (1) to access all fields as one big vector with just one index
  // ignoring the dimensions of the object.
  // (2) access bya vector of indices that use the
  // dimensions vector to field the location in the big vector
  //
  // The first one is direct access, where as the second needs
  // a conversion to find where a certain element is.
  // NOTE: everything is coupled through pointers, so be careful not
  // to link matrices in a loop as that would cause some unintented
  // effects.
  
  matlabarray getcell(int index);
  matlabarray getcell(std::vector<int> &indexvec); 
  void setcell(int index,matlabarray m);
  void setcell(std::vector<int> &indexvec,matlabarray m);
  
  // struct specific functions
  //
  // A matrix of the struct class is similar as a cell
  // but it now has alphanumeric fieldnames, so next to
  // an index it has an alphanumeric indicator.
  //
  // Similar to getcell() and setcell(), getfield() and setfield()
  // have similar addressing modes with a single or a vector of
  // indices.
  
  matlabarray getfield(int index,std::string fieldname);
  matlabarray getfield(std::vector<int> &indexvec,std::string fieldname);  
  matlabarray getfieldCI(int index,std::string fieldname);   // Case Insensitive version
  matlabarray getfieldCI(std::vector<int> &indexvec,std::string fieldname); // Case Insensitive version
  
  void setfield(int index,std::string fieldname,matlabarray m);
  void setfield(std::vector<int> &indexvec,std::string fieldname,matlabarray m);
  matlabarray getfield(int index,int fieldnameindex);
  matlabarray getfield(std::vector<int> &indexvec,int fieldnameindex);
  void setfield(int index,int fieldnameindex,matlabarray m);
  void setfield(std::vector<int> &index,int fieldnameindex,matlabarray m);
  
  
  // Get the names of all the fieldnames
  std::vector<std::string> getfieldnames();
  
  // Get the number of fields
  int getnumfields();
  
  // Get the fieldname from the index
  std::string getfieldname(int fieldnameindex);
  
  // Get the index of a fieldname
  // Two version case sensitive and case insensitive
  int getfieldnameindex(std::string fieldname);
  int getfieldnameindexCI(std::string fieldname);    // Case Insensitive version
  
  // Change a certain fieldname
  void setfieldname(int fieldnameindex,std::string);
  
  // Check whether a certain fieldname exists
  bool isfield(std::string fieldname);
  bool isfieldCI(std::string fieldname);  // Case Insensitive Version
  
  // Add and remove entries
  // This will internally reorder matrix
  int addfieldname(std::string);
  void removefieldname(std::string);
  void removefieldname(int fieldnameindex);
  
  // string specific functions
  std::string getstring();
  void setstring(std::string string);
  bool compare(std::string str);
  bool compareCI(std::string str);
  
  
  // creation functions
  // There are a few basic matlab array classes
  //   Dense matrices     - in various numeric formats
  //   Sparse matrices    - in matlab only in double, but data can be entered in any format
  //   Cell matrices      - matrix of pointers to submatrices
  //   Struct matrices    - similar to cell array, but with an extra dimension with fieldnames
  //   Class matrices     - based on the struct matrix with an extra field of the classname
  //   String matrices    - A charater array that needs to interpreted as a string
  
  void createdensearray(std::vector<int> &dims,mitype type);  
  void createsparsearray(std::vector<int> &dims,mitype type); 
  void createdensearray(int m, int n,mitype type);  
  void createsparsearray(int m, int n,mitype type); 
    
  void createcellarray(std::vector<int> &dims); 
  void createstructarray();
  void createstructarray(std::vector<std::string> &fieldnames); 
  void createstructarray(std::vector<int> &dims,std::vector<std::string> &fieldnames);   
  void createclassarray(std::vector<std::string> &fieldnames,std::string classname);
  void createclassarray(std::vector<int> &dims,std::vector<std::string> &fieldnames,std::string classname);
  
  // A shortway to create some basic matrices/vectors/scalars.  These
  // functions use STL vectors or the C-style pointer to a memory
  // block.
  
  void createdoublescalar(double value);
  void createdoublevector(std::vector<double> &values);
  void createdoublevector(int n, double *values);
  void createdoublematrix(std::vector<double> &values, std::vector<int> &dims);  
  void createdoublematrix(int m,int n, double *values);
  
  void createintscalar(int value);
  void createintvector(std::vector<int> &values);
  void createintvector(int n, int *values);
  void createintmatrix(std::vector<int> &values, std::vector<int> &dims);  
  void createintmatrix(int m,int n, int *values);
  
  // string arrays in this implementation will allow changing the size 
  void createstringarray(); 
  void createstringarray(std::string string); 
  
  // check type of an array
  
  bool isnumeric();
  bool isstruct();
  bool iscell();
  bool isclass();
  bool isstring();
  bool isdense();
  bool issparse();    
  
  // data collection/insertion functions
  
  // C-style access to the data. Specify the address of the databuffer
  // and the number of elements it can hold and the data will be
  // copied and casted to the right format.
  template<class T> void getnumericarray(T *data,int size);
  template<class T> void getimagnumericarray(T *data,int size);
  
  template<class T> void getnumericarray(T **data,int dim1, int dim2);
  template<class T> void getimagnumericarray(T **data,int dim1, int dim2);
  template<class T> void getnumericarray(T ***data,int dim1, int dim2, int dim3);
  template<class T> void getimagnumericarray(T ***data,int dim1, int dim2, int dim3);
  
  
  template<class T> void getnumericarray(std::vector<T> &vec);
  template<class T> void getimagnumericarray(std::vector<T> &vec);
  
  // C-style write access. The data will be copied out of the
  // databuffer and casted to the format of the matlab file. If a type
  // is specified the data will be stored in that format. Otherwise it
  // depends on the definition given in createdensearray() or in
  // createsparsearray().  Note Submatrices cannot be enterred this
  // way, use the setcell()/getcell() functions
  
  template<class T> void setnumericarray(T *data,int size);
  template<class T> void setnumericarray(T *data,int size,mitype type);
  template<class T> void setimagnumericarray(T *data,int size);
  template<class T> void setimagnumericarray(T *data,int size,mitype type);
  
  template<class T> void setnumericarray(T **data,int dim1, int dim2);
  template<class T> void setnumericarray(T **data,int dim1, int dim2 ,mitype type);
  template<class T> void setimagnumericarray(T **data,int dim1, int dim2);
  template<class T> void setimagnumericarray(T **data,int dim1, int dim2 ,mitype type);
  
  template<class T> void setnumericarray(T ***data,int dim1, int dim2, int dim3);
  template<class T> void setnumericarray(T ***data,int dim1, int dim2, int dim3 ,mitype type);
  template<class T> void setimagnumericarray(T ***data,int dim1, int dim2, int dim3);
  template<class T> void setimagnumericarray(T ***data,int dim1, int dim2 , int dim3, mitype type);
  
  template<class T> void setnumericarray(T *data,int size,std::vector<int> &dims);
  template<class T> void setnumericarray(T *data,int size,std::vector<int> &dims, mitype type);
  template<class T> void setimagnumericarray(T *data,int size,std::vector<int> &dims);
  template<class T> void setimagnumericarray(T *data,int size,std::vector<int> &dims, mitype type);
  
  template<class T> void setnumericarray(std::vector<T> &vec);
  template<class T> void setnumericarray(std::vector<T> &vec,mitype type);
  template<class T> void setimagnumericarray(std::vector<T> &vec);
  template<class T> void setimagnumericarray(std::vector<T> &vec,mitype type);
  
  template<class T> void setnumericarray(std::vector<T> &vec,std::vector<int> &dims);
  template<class T> void setnumericarray(std::vector<T> &vec,std::vector<int> &dims, mitype type);
  template<class T> void setimagnumericarray(std::vector<T> &vec,std::vector<int> &dims);
  template<class T> void setimagnumericarray(std::vector<T> &vec,std::vector<int> &dims, mitype type);
  
  // sparse functions
  int getnnz();
  template<class T> void getrowsarray(T *rows,int size);
  template<class T> void setrowsarray(T *rows,int size);
  template<class T> void getcolsarray(T *cols,int size);
  template<class T> void setcolsarray(T *cols,int size);
  
  std::string getinfotext();
  std::string getinfotext(std::string name);
  
  // reordering functions (for DENSE matrices only)
  // Useful as matlab uses Fortran ordering of matrices and
  // C++ uses the C-style ordering.
  
  void transpose();   // matrix must be 2D
  void permute(std::vector<int> permorder);  // N dimensional equivalent for transpose
  
  // Conversion tools to mitype
  
  template<class T> mitype getmitype(T &test);
  inline mitype  getmitype(char &test);
  inline mitype  getmitype(unsigned char &test);
  inline mitype  getmitype(unsigned short &test);
  inline mitype  getmitype(signed short &test);
  inline mitype  getmitype(unsigned int &test);
  inline mitype  getmitype(signed int &test);
  inline mitype  getmitype(float &test);
  inline mitype  getmitype(double &test); 
  
  
private:
  // helper functions:
  
  // case insensitive comparison between strings
  int cmp_nocase(const std::string &s1,const std::string &s2);
  
  // multi dimensional scheme for computing the new order of the
  // elements in case of switching certain dimensions.  e.g. for a 2D
  // matrix a permuation or of [1 0] wil transpose the matrix. However
  // for n dimensional matrices a transpose operation is a permuation
  // of the order of the dimensions. Based on this order this function
  // will calculate the neworder of all elements
  
  void reorder_permute(std::vector<int> &newindices,std::vector<int> permorder);
};
  
  
template<class T> inline void matlabarray::getnumericarray(T *data,int size)
{
  if(m_ == 0) throw empty_matlabarray();
  m_->preal_.getandcast(data,size);
}

template<class T> inline void matlabarray::getimagnumericarray(T *data,int size)
{
  if(m_ == 0) throw empty_matlabarray();
  m_->pimag_.getandcast(data,size);
}

template<class T> inline void matlabarray::getnumericarray(T **data,int dim1,int dim2)
{
  if(m_ == 0) throw empty_matlabarray();
  m_->preal_.getandcast(data,dim1,dim2);
}

template<class T> inline void matlabarray::getimagnumericarray(T **data,int dim1,int dim2)
{
  if(m_ == 0) throw empty_matlabarray();
  m_->pimag_.getandcast(data,dim1,dim2);
}

template<class T> inline void matlabarray::getnumericarray(T ***data,int dim1,int dim2,int dim3)
{
  if(m_ == 0) throw empty_matlabarray();
  m_->preal_.getandcast(data,dim1,dim2,dim3);
}

template<class T> inline void matlabarray::getimagnumericarray(T ***data,int dim1,int dim2,int dim3)
{
  if(m_ == 0) throw empty_matlabarray();
  m_->pimag_.getandcast(data,dim1,dim2,dim3);
}



template<class T> inline void matlabarray::getnumericarray(std::vector<T> &vec)
{
  if(m_ == 0) throw empty_matlabarray();
  m_->preal_.getandcastvector(vec);
}

template<class T> inline void matlabarray::getimagnumericarray(std::vector<T> &vec)
{
  if(m_ == 0) throw empty_matlabarray();
  m_->pimag_.getandcastvector(vec);
}


template<class T> inline void matlabarray::setnumericarray(T *data,int size,std::vector<int> &dims)
{
  if(m_ == 0) 
  {
    std::cerr << "internal error in setnumericarray(T*,int,vector<int>)\n";
    throw internal_error();
  }
  this->setdims(dims); 
  if(size != getnumelements())
  {
    std::cerr << "internal error in setnumericarray(T*,int,vector<int>)\n";
    throw internal_error();
  }
  m_->preal_.putandcast(data,size,m_->type_);
}

template<class T> inline void matlabarray::setnumericarray(T *data,int size,std::vector<int> &dims,mitype type)
{
  if(m_ == 0) 
  {
    std::cerr << "internal error in setnumericarray(T*,int,vector<int>,mitype)\n";
    throw internal_error();
  }
  this->setdims(dims); 
  if((size != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setnumericarray(T*,int,vector<int>,mitype)\n";
    throw internal_error();
  }
  
  m_->preal_.putandcast(data,size,type);
}


template<class T> inline void matlabarray::setnumericarray(T *data,int size)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setnumericarray(T*,int)\n";
    throw internal_error();
  }
  if((size != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setnumericarray(T*,int)\n";
    throw internal_error();
  }
  
  m_->preal_.putandcast(data,size,m_->type_);
}  

template<class T> inline void matlabarray::setnumericarray(T *data,int size,mitype type)
{
  if(m_ == 0) 
  {
    std::cerr << "internal error in setnumericarray(T*,int,mitype)\n";
    throw internal_error();
  }
  if((size != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setnumericarray(T*,int,mitype)\n";
    throw internal_error();
  }
  m_->preal_.putandcast(data,size,type);
}

template<class T> inline void matlabarray::setimagnumericarray(T *data,int size,std::vector<int> &dims)
{
  if(m_ == 0) 
  {
    std::cerr << "internal error in setimagnumericarray(T*,int,vector<int>)\n";
    throw internal_error();
  }
  this->setdims(dims); 
  if((size != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(T*,int,vector<int>)\n";
    throw internal_error();
  }
  m_->pimag_.putandcast(data,size,m_->type_);
}

template<class T> inline void matlabarray::setimagnumericarray(T *data,int size,std::vector<int> &dims,mitype type)
{
  if(m_ == 0) 
  {
    std::cerr << "internal error in setimagnumericarray(T*,int,vector<int>,mitype)\n";
    throw internal_error();
  }
  this->setdims(dims); 
  if((size != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(T*,int,vector<int>,mitype)\n";
    throw internal_error();
  }
  
  m_->pimag_.putandcast(data,size,type);
}



template<class T> inline void matlabarray::setnumericarray(T **data,int dim1, int dim2)
{
  if(m_ == 0) 
  {
    std::cerr << "internal error in setnumericarray(T**,int,int)\n";
    throw internal_error();
  }
  if(((dim1*dim2) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setnumericarray(T**,int,int)\n";
    throw internal_error();
  }
  
  m_->preal_.putandcast(data,dim1,dim2,m_->type_);
}  

template<class T> inline void matlabarray::setnumericarray(T **data,int dim1, int dim2 ,mitype type)
{
  if(m_ == 0) 
  { 
    std::cerr << "internal error in setnumericarray(T**,int,int,mitype)\n";
    throw internal_error();
  }
  if(((dim1*dim2) != getnumelements())&&(m_->class_ != mlSPARSE))
  { 
    std::cerr << "internal error in setnumericarray(T**,int,int,mitype)\n";
    throw internal_error();
  }
  
  m_->preal_.putandcast(data,dim1,dim2,type);
}

template<class T> inline void matlabarray::setimagnumericarray(T **data,int dim1, int dim2)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setimagnumericarray(T**,int,int)\n";
    throw internal_error();
  }
  if(((dim1*dim2) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(T**,int,int)\n";
    throw internal_error();
  }
  
  m_->pimag_.putandcast(data,dim1,dim2,m_->type_);
}  

template<class T> inline void matlabarray::setimagnumericarray(T **data,int dim1, int dim2 ,mitype type)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setimagnumericarray(T**,int,int,mitype)\n";
    throw internal_error();
  }
  if(((dim1*dim2) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(T**,int,int,mitype)\n";
    throw internal_error();
  }
  
  m_->pimag_.putandcast(data,dim1,dim2,type);
}


template<class T> inline void matlabarray::setnumericarray(T ***data,int dim1, int dim2, int dim3)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setnumericarray(T***,int,int,int)\n";
    throw internal_error();
  }
  if(((dim1*dim2*dim3) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setnumericarray(T***,int,int,int)\n";
    throw internal_error();
  }
  
  m_->preal_.putandcast(data,dim1,dim2,dim3,m_->type_);
}  

template<class T> inline void matlabarray::setnumericarray(T ***data,int dim1, int dim2, int dim3 ,mitype type)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setnumericarray(T***,int,int,int,mitype)\n";
    throw internal_error();
  }
  if(((dim1*dim2*dim3) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setnumericarray(T***,int,int,int,mitype)\n";
    throw internal_error();
  }
  
  m_->preal_.putandcast(data,dim1,dim2,dim3,type);
}

template<class T> inline void matlabarray::setimagnumericarray(T ***data,int dim1, int dim2, int dim3)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setimagnumericarray(T***,int,int,int)\n";
    throw internal_error();
  }
  if(((dim1*dim2*dim3) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(T***,int,int,int)\n";
    throw internal_error();
  }
  
  m_->pimag_.putandcast(data,dim1,dim2,dim3,m_->type_);
}  

template<class T> inline void matlabarray::setimagnumericarray(T ***data,int dim1, int dim2, int dim3 ,mitype type)
{
  if(m_ == 0) 
  {
    std::cerr << "internal error in setimagnumericarray(T***,int,int,int,mitype)\n";
    throw internal_error();
  }
  if(((dim1*dim2*dim3) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(T***,int,int,int,mitype)\n";
    throw internal_error();
  }
  
  m_->pimag_.putandcast(data,dim1,dim2,dim3,type);
}




template<class T> inline void matlabarray::setimagnumericarray(T *data,int size)
{
  if(m_ == 0) 
  {
    std::cerr << "internal error in setimagnumericarray(T*,int)\n";
    throw internal_error();
  }
  if((size != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(T*,int)\n";
    throw internal_error();
  }
  
  m_->pimag_.putandcast(data,size,m_->type_);
}  

template<class T> inline void matlabarray::setimagnumericarray(T *data,int size,mitype type)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setimagnumericarray(T*,int, mitype)\n";
    throw internal_error();
  }
  if((size != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(T*,int,mitype)\n";
    throw internal_error();
  }
  
  m_->pimag_.putandcast(data,size,type);
}



template<class T> inline void matlabarray::setnumericarray(std::vector<T> &vec,std::vector<int> &dims)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setnumericarray(vector<T>,vector<int>)\n";
    throw internal_error();
  }
  
  this->setdims(dims); 
  if(vec.size() != getnumelements())
  {
    std::cerr << "internal error in setnumericarray(vector<T>,vector<int>)\n";
    throw internal_error();
  }

  m_->preal_.putandcastvector(vec,m_->type_);
}

template<class T> inline void matlabarray::setnumericarray(std::vector<T> &vec,std::vector<int> &dims,mitype type)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setnumericarray(vector<T>,vector<int>,mitype)\n";
    throw internal_error();
  }
  
  this->setdims(dims); 
  if((vec.size() != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setnumericarray(vector<T>,vector<int>,mitype)\n";
    throw internal_error();
  }
  m_->preal_.putandcastvector(vec,type);
}


template<class T> inline void matlabarray::setnumericarray(std::vector<T> &vec)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setnumericarray(vector<T>)\n";
    throw internal_error();
  }

  if((static_cast<int>(vec.size()) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setnumericarray(vector<T>)\n";
    throw internal_error();
  }
  
  m_->preal_.putandcastvector(vec,m_->type_);
}  

template<class T> inline void matlabarray::setnumericarray(std::vector<T> &vec,mitype type)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setnumericarray(vector<T>,mitype)\n";
    throw internal_error();
  }
  if((static_cast<int>(vec.size()) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setnumericarray(vector<T>,mitype)\n";
    throw internal_error();
  }

  m_->preal_.putandcastvector(vec,type);
}

template<class T> inline void matlabarray::setimagnumericarray(std::vector<T> &vec,std::vector<int> &dims)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setimagnumericarray(vector<T>,vector<int>)\n";
    throw internal_error();
  }

  this->setdims(dims); 
  if((static_cast<int>(vec.size()) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(vector<T>,vector<int>)\n";
    throw internal_error();
  }

  m_->pimag_.putandcastvector(vec,m_->type_);
}

template<class T> inline void matlabarray::setimagnumericarray(std::vector<T> &vec,std::vector<int> &dims,mitype type)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setimagnumericarray(vector<T>,vector<int>,mitype)\n";
    throw internal_error();
  }

  this->setdims(dims); 
  if((static_cast<int>(vec.size()) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(vector<T>,vector<int>,mitype)\n";
    throw internal_error();
  }

  m_->pimag_.putandcastvector(vec,type);
}

template<class T> inline void matlabarray::setimagnumericarray(std::vector<T> &vec)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setimagnumericarray(vector<T>)\n";
    throw internal_error();
  }
  if((static_cast<int>(vec.size()) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(vector<T>)\n";
    throw internal_error();
  }

  m_->pimag_.putandcastvector(vec,m_->type_);
}

template<class T> inline void matlabarray::setimagnumericarray(std::vector<T> &vec,mitype type)
{
  if(m_ == 0)
  {
    std::cerr << "internal error in setimagnumericarray(vector<T>,mitype)\n";
    throw internal_error();
  }
  if((static_cast<int>(vec.size()) != getnumelements())&&(m_->class_ != mlSPARSE))
  {
    std::cerr << "internal error in setimagnumericarray(vector<T>,mitype)\n";
    throw internal_error();
  }

  m_->pimag_.putandcastvector(vec,type);
}

template<class T> inline void matlabarray::getrowsarray(T *rows,int size)
{
  if (m_ == 0) 
  {
    std::cerr << "internal error in getrowsarray(T*,int)\n";
    throw internal_error();
  }
  
  m_->prows_.getandcast(rows,size);
}

template<class T> inline void matlabarray::setrowsarray(T *rows,int size)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in setrowsarray(T*,int)\n";
    throw internal_error();
  }

  m_->prows_.putandcast(rows,size,miINT32);
}

template<class T>  inline void matlabarray::getcolsarray(T *cols,int size)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in getcolsarray(T*,int)\n";
    throw internal_error();
  }

  m_->pcols_.getandcast(cols,size);
}
    
template<class T>  inline void matlabarray::setcolsarray(T *cols,int size)
{
  if (m_ == 0)
  {
    std::cerr << "internal error in setcolsarray(T*,int)\n";
    throw internal_error();
  }

  m_->pcols_.putandcast(cols,size,miINT32);
}    

template<class T> inline matlabarray::mitype matlabarray::getmitype(T &/*test*/) 
{ return(matlabarray::miUNKNOWN); }

matlabarray::mitype  inline matlabarray::getmitype(unsigned char &/*test*/)
{ return(matlabarray::miUINT8); }

matlabarray::mitype  inline matlabarray::getmitype(char &/*test*/)
{ return(matlabarray::miINT8); }

matlabarray::mitype  inline matlabarray::getmitype(unsigned short &/*test*/) 
{ return(matlabarray::miUINT16); }

matlabarray::mitype  inline matlabarray::getmitype(signed short &/*test*/)
{ return(matlabarray::miINT16); }

matlabarray::mitype  inline matlabarray::getmitype(unsigned int &test) 
{ 
  if (sizeof(test) == 8) return(matlabarray::miUINT64);
  return(matlabarray::miUINT32); 
}

matlabarray::mitype  inline matlabarray::getmitype(int &test) 
{ 
  if (sizeof(test) == 8) return(matlabarray::miINT64);
  return(matlabarray::miINT32); 
}

matlabarray::mitype  inline matlabarray::getmitype(float &/*test*/) 
{ return(matlabarray::miSINGLE); }

matlabarray::mitype  inline matlabarray::getmitype(double &/*test*/) 
{ return(matlabarray::miDOUBLE); } 

} // namespace

#endif
