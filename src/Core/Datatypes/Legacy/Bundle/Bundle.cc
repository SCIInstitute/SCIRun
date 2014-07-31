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

/// @todo Documentation Core/Datatypes/Legacy/Bundle/Bundle.cc

#include <boost/algorithm/string.hpp>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <boost/range/algorithm/count_if.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/bind/bind.hpp>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#include <Core/Datatypes/NrrdData.h>
#endif

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
static Persistent* make_Bundle() {
  return new Bundle;
}

PersistentTypeID Bundle::type_id("Bundle", "PropertyManager", make_Bundle);
#endif

Bundle::Bundle()
{
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
Bundle::Bundle(const Bundle& copy) :
  PropertyManager(copy),
  bundleName_(copy.bundleName_),
  bundle_(copy.bundle_),
  transposeNrrd_(copy.transposeNrrd_)
{
}

#endif

Bundle* Bundle::clone() const
{
  return(new Bundle(*this));
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

void Bundle::merge(LockingHandle<Bundle> C)
{
  for (size_t p = 0; p < C->bundle_.size(); p++)
  {
    std::string name = C->bundleName_[p];
    LockingHandle<PropertyManager> handle = C->bundle_[p];
      
    int index;
    index = findName(bundleName_,name);
    if (index == -1)
    {
      bundle_.push_back(handle);
      bundleName_.push_back(name);
    }
    else
    {
      bundle_[index] = handle.get_rep();
      bundleName_[index] = name;
    }
  }
}

#define BUNDLE_VERSION 2

//////////
// PIO for NrrdData objects
void
Bundle::io(Piostream& stream)
{
  stream.begin_class("Bundle", BUNDLE_VERSION);
  // Do the base class first...
 
  PropertyManager::io(stream);
  
  if (stream.reading()) 
  {
    int size;

    stream.begin_cheap_delim();
      
    stream.io(size);

    bundle_.resize(size);
    bundleName_.resize(size);
  
    std::string type;
    for (int p = 0; p < size; p++)
    {
      stream.begin_cheap_delim();
      stream.io(bundleName_[p]);
      stream.io(type);
      stream.end_cheap_delim();		
      stream.begin_cheap_delim();
      if (type=="field")
      {
        LockingHandle<Field> handle;
        Pio(stream,handle);
        bundle_[p] = dynamic_cast<PropertyManager *>(handle.get_rep());
      }
      if (type=="matrix")
      {
        MatrixHandle handle;
        Pio(stream,handle);
        bundle_[p] = dynamic_cast<PropertyManager *>(handle.get_rep());
      }
      if (type=="string")
      {
        LockingHandle<String> handle;
        Pio(stream,handle);
        bundle_[p] = dynamic_cast<PropertyManager *>(handle.get_rep());
      }
      if (type=="nrrd")
      {
        LockingHandle<NrrdData> handle;
        Pio(stream,handle);
        bundle_[p] = dynamic_cast<PropertyManager *>(handle.get_rep());
      }
      if (type=="colormap")
      {
        LockingHandle<ColorMap> handle;
        Pio(stream,handle);
        bundle_[p] = dynamic_cast<PropertyManager *>(handle.get_rep());
      }
      if (type=="bundle")
      {
        LockingHandle<Bundle> handle;
        Pio(stream,handle);
        bundle_[p] = dynamic_cast<PropertyManager *>(handle.get_rep());
      }
      stream.end_cheap_delim();
    }
    stream.end_cheap_delim();		
  } 
  else 
  { 
    int size, tsize;
    stream.begin_cheap_delim();
    tsize = 0;
    size = bundleName_.size();
    for (int p = 0; p < size; p ++)
    {
      if (bundle_[p].get_rep()) tsize++;
    }

    stream.io(tsize);
    for (int p = 0; p < size; p++)
    {
      if (bundle_[p].get_rep())
      {
        stream.begin_cheap_delim();		
        stream.io(bundleName_[p]);

        std::string type;
        LockingHandle<Field> fieldhandle = 
                                    dynamic_cast<Field*>(bundle_[p].get_rep());
        if (fieldhandle.get_rep()) 
        { 
          type = "field";
          stream.io(type);
          stream.end_cheap_delim();		
          stream.begin_cheap_delim();            
          Pio(stream,fieldhandle); 
          stream.end_cheap_delim(); 
          continue; 
        }
      
        MatrixHandle matrixhandle = 
                                   dynamic_cast<Matrix<double>*>(bundle_[p].get_rep());
        if (matrixhandle.get_rep()) 
        { 
          type = "matrix";
          stream.io(type);
          stream.end_cheap_delim();		
          stream.begin_cheap_delim();            
          Pio(stream,matrixhandle); 
          stream.end_cheap_delim(); 
          continue; 
        }
        
        LockingHandle<String> stringhandle = 
                                   dynamic_cast<String*>(bundle_[p].get_rep());
        if (stringhandle.get_rep()) 
        { 
          type = "string";
          stream.io(type);
          stream.end_cheap_delim();		
          stream.begin_cheap_delim();            
          Pio(stream,stringhandle); 
          stream.end_cheap_delim(); 
          continue; 
        }
    
                  
        LockingHandle<NrrdData> nrrdhandle = 
                                 dynamic_cast<NrrdData*>(bundle_[p].get_rep());
        if (nrrdhandle.get_rep())
        { 
          type = "nrrd";
          bool embed_old = nrrdhandle->get_embed_object();
          nrrdhandle->set_embed_object(true);
          stream.io(type);
          stream.end_cheap_delim();		
          stream.begin_cheap_delim();            
          Pio(stream,nrrdhandle); 
          stream.end_cheap_delim(); 
          nrrdhandle->set_embed_object(embed_old);
          continue; 
        }
    
      
        LockingHandle<ColorMap> colormaphandle = 
                                 dynamic_cast<ColorMap*>(bundle_[p].get_rep());
        if (colormaphandle.get_rep()) 
        { 
          type = "colormap"; 
          stream.io(type);
          stream.end_cheap_delim();		
          stream.begin_cheap_delim();            
          Pio(stream,colormaphandle); 
          stream.end_cheap_delim(); 
          continue; 
        }

        LockingHandle<Bundle> bundlehandle = 
                                  dynamic_cast<Bundle*>(bundle_[p].get_rep());
        if (bundlehandle.get_rep()) 
        { 
          type = "bundle";
          stream.io(type);
          stream.end_cheap_delim();		
          stream.begin_cheap_delim();            
          Pio(stream,bundlehandle); 
          stream.end_cheap_delim(); 
          continue; 
        }
      }
    }
    stream.end_cheap_delim();		
  }
} 


bool Bundle::NrrdToMatrixConvertible(NrrdDataHandle nrrdH)
{
  if (nrrdH.get_rep() == 0) return(false);
  if (nrrdH->nrrd_ == 0) return(false);
  switch (nrrdH->nrrd_->type)
  {
  case nrrdTypeChar: case nrrdTypeUChar:
  case nrrdTypeShort: case nrrdTypeUShort:
  case nrrdTypeInt: case nrrdTypeUInt:
  case nrrdTypeFloat: case nrrdTypeDouble:
    break;
  default:
    return(false);
  }
  if (nrrdH->nrrd_->dim < 3) return(true);
  return(false);
}


bool
Bundle::NrrdToMatrix(NrrdDataHandle nrrdH,MatrixHandle& matH)
{
  if (nrrdH.get_rep() == 0) return false;
  if (nrrdH->nrrd_ == 0) return false;
  switch(nrrdH->nrrd_->type)
    {
    case nrrdTypeChar:
      return(NrrdToMatrixHelper<char>(nrrdH,matH));
    case nrrdTypeUChar:
      return(NrrdToMatrixHelper<unsigned char>(nrrdH,matH));
    case nrrdTypeShort:
      return(NrrdToMatrixHelper<short>(nrrdH,matH));
    case nrrdTypeUShort:
      return(NrrdToMatrixHelper<unsigned short>(nrrdH,matH));
    case nrrdTypeInt:
      return(NrrdToMatrixHelper<int>(nrrdH,matH));
    case nrrdTypeUInt:
      return(NrrdToMatrixHelper<unsigned int>(nrrdH,matH));
    case nrrdTypeFloat:
      return(NrrdToMatrixHelper<float>(nrrdH,matH));
    case nrrdTypeDouble:
      return(NrrdToMatrixHelper<double>(nrrdH,matH));
    default:
      return(false);
    }
  return(false);
}

bool Bundle::MatrixToNrrdConvertible(MatrixHandle matH)
{
  if (matH.get_rep() == 0) return(false);
  if (matrix_is::dense(matH)) return(true);
  if (matrix_is::column(matH)) return(true);
  return(false);
}

bool Bundle::MatrixToNrrd(MatrixHandle matH,NrrdDataHandle &nrrdH)
{
  if (matH.get_rep() == 0) return false;
  if (matrix_is::dense(matH)) 
  {
    DenseMatrix* matrix = matH->dense();

    int rows = matrix->nrows();
    int cols = matrix->ncols();

    if (transposeNrrd_)
    {
      nrrdH = new NrrdData();
      size_t size[NRRD_DIM_MAX];
      size[0] = cols; size[1] = rows;
      nrrdAlloc_nva(nrrdH->nrrd_, nrrdTypeDouble, 2, size);


      const char *labels[NRRD_DIM_MAX];
      labels[0] = airStrdup("dense-columns");
      labels[1] = airStrdup("dense-rows");

      nrrdAxisInfoSet_nva(nrrdH->nrrd_, nrrdAxisInfoLabel, labels);
      nrrdH->nrrd_->axis[0].kind = nrrdKindDomain;
      nrrdH->nrrd_->axis[1].kind = nrrdKindDomain;

      double *val = reinterpret_cast<double*>(nrrdH->nrrd_->data);
      double *data = matrix->get_data_pointer();

      int i,j;
      i = 0;
      j = 0;
      for(int r=0; r<rows; r++) 
        {
          for(int c=0; c<cols; c++) 
            {
              i = c + cols*r;
              val[i] = data[j++];
            }
        }
    }
    else
    {
      nrrdH = new NrrdData();
      size_t size[NRRD_DIM_MAX];
      size[0] = rows; size[1] = cols;
      nrrdAlloc_nva(nrrdH->nrrd_, nrrdTypeDouble, 2, size);

      const char *labels[NRRD_DIM_MAX];
      labels[0] = airStrdup("dense-rows");
      labels[1] = airStrdup("dense-columns");
      nrrdAxisInfoSet_nva(nrrdH->nrrd_, nrrdAxisInfoLabel, labels);
      nrrdH->nrrd_->axis[0].kind = nrrdKindDomain;
      nrrdH->nrrd_->axis[1].kind = nrrdKindDomain;

      double *val = reinterpret_cast<double*>(nrrdH->nrrd_->data);
      double *data = matrix->get_data_pointer();

      for(int c=0; c<cols; c++) 
        {
          for(int r=0; r<rows; r++) 
            {
              *val++ = *data++;
            }
        }
    
    }
    return(true);
  } 
  else if (matrix_is::column(matH)) 
  {
    ColumnMatrix* matrix = matH->column();
    size_t size[NRRD_DIM_MAX];
    size[0] = matrix->nrows();

    nrrdH = new NrrdData();
    nrrdAlloc_nva(nrrdH->nrrd_, nrrdTypeDouble, 1, size);
    nrrdAxisInfoSet_nva(nrrdH->nrrd_, nrrdAxisInfoLabel, "column-data");
    nrrdH->nrrd_->axis[0].kind = nrrdKindDomain;

    double *val = reinterpret_cast<double*>(nrrdH->nrrd_->data);
    double *data = matrix->get_data_pointer();

    for(int i=0; i<matrix->nrows(); i++) 
    {
      *val = *data;
      ++data;
      ++val;
    }
    return(true);
  } 
  else 
  {
    // For the moment we do not convert this one
    // This is the SPARSE matrix one
    return(false);
  }
} // end MatrixToNrrd


MatrixHandle
Bundle::getMatrix(const std::string& name) 
{ 
  MatrixHandle matrixH;
  matrixH = get< Matrix<double> >(name);
  if (matrixH.get_rep() == 0)
  {
    NrrdDataHandle nrrdH;
    nrrdH = get<NrrdData>(name);
    if (nrrdH.get_rep())
    {
      if (NrrdToMatrixConvertible(nrrdH))
        NrrdToMatrix(nrrdH,matrixH);
    }
  }
  return matrixH;
}

bool Bundle::isMatrix(const std::string& name)  
{ 
  bool isMat;
  isMat = is< Matrix<double> >(name);
  if (!isMat)
  {
    if (is<NrrdData>(name))
    {
      NrrdDataHandle nrrdH = get<NrrdData>(name);
      if (NrrdToMatrixConvertible(nrrdH)) isMat = true;
    }
  } 
  return(isMat);
}

int Bundle::numMatrices() 
{ 
  int nummat;
  nummat = num< Matrix<double> >();
    
  int numnrrd; 
  numnrrd = num<NrrdData>();
  std::string name;
  for (int p=0;p < numnrrd; p++)
  {
    name = getName<NrrdData>(p);
    if (isMatrix(name)) nummat++;
  }
  return(nummat);
}

std::string Bundle::getMatrixName(int index) 
{
  int nummat = num< Matrix<double> >();
  if (index < nummat) return(getName< Matrix<double> >(index));

  int numnrrd; 
  numnrrd = num<NrrdData>();
  for (int p=0;p < numnrrd; p++)
  {
    std::string name = getName<NrrdData>(p);
    if (isMatrix(name))
    {
      if (index == nummat) return name;
      nummat++;
    }
  }
  return("");
}

std::string Bundle::getHandleType(int index)
{
  LockingHandle<PropertyManager> handle = gethandle(index);
  return handle->dynamic_type_name();
}


LockingHandle<NrrdData> Bundle::getNrrd(const std::string& name) 
{ 
  NrrdDataHandle nrrdH;
  nrrdH = get<NrrdData>(name);
  if (nrrdH.get_rep() == 0)
  {
    MatrixHandle matrixH;
    matrixH = get< Matrix<double> >(name);
    if (matrixH.get_rep())
    {
      if (MatrixToNrrdConvertible(matrixH))
                      MatrixToNrrd(matrixH,nrrdH);
    }
  }
  return(nrrdH);
}

bool
Bundle::isNrrd(const std::string& name)  
{ 
  bool isnrrd;
  isnrrd = is<NrrdData>(name);
  if (!isnrrd)
  {
    if (is< Matrix<double> >(name))
    {
      MatrixHandle matrixH = get< Matrix<double> >(name);
      if (MatrixToNrrdConvertible(matrixH)) isnrrd = true;
    }
  } 
  return(isnrrd);
}

int Bundle::numNrrds() 
{ 
  int numnrrd;
  numnrrd = num<NrrdData>();
    
  int nummat; 
  nummat = num< Matrix<double> >();
  std::string name;
  for (int p=0;p < nummat; p++)
  {
    name = getName< Matrix<double> >(p);
    if (isNrrd(name)) numnrrd++;
  }
  return(numnrrd);
}

std::string Bundle::getNrrdName(int index) 
{
  int numnrrd = num<NrrdData>();
  if (index < numnrrd) return(getName<NrrdData>(index));

  int nummat; 
  nummat = num< Matrix<double> >();
  for (int p=0;p < nummat; p++)
  {
    std::string name = getName< Matrix<double> >(p);
    if (isNrrd(name))
    {
      if (index == numnrrd) return(name);
      numnrrd++;
    }
  }
  return("");
}




#endif

bool Bundle::empty() const
{
  return bundle_.empty();
}

size_t Bundle::size() const
{
  return bundle_.size();
}

void Bundle::set(const std::string& name, DatatypeHandle data)
{
  bundle_[name] = data;
}

DatatypeHandle Bundle::get(const std::string& name) const
{
  auto it = bundle_.find(name);
  if (it != bundle_.end())
    return it->second;
  return DatatypeHandle();
}

/// @todo: extract into teplate impl, but do it here to avoid including every type in Bundle.h

FieldHandle Bundle::getField(const std::string& name) const
{
  return boost::dynamic_pointer_cast<Field>(get(name));
}

bool Bundle::isField(const std::string& name) const
{
  return getField(name) != nullptr;
}

size_t Bundle::numFields() const
{
  return boost::count_if(bundle_ | boost::adaptors::map_keys, boost::bind(&Bundle::isField, this, _1));
}

std::vector<FieldHandle> Bundle::getFields() const
{
  auto range = bundle_ | boost::adaptors::map_keys | boost::adaptors::transformed(boost::bind(&Bundle::getField, this, _1)) | boost::adaptors::filtered([] (FieldHandle f) { return f != nullptr; });
  return std::vector<FieldHandle>(range.begin(), range.end());
}

std::vector<std::string> Bundle::getFieldNames() const
{
  auto range = bundle_ | boost::adaptors::map_keys | boost::adaptors::filtered(boost::bind(&Bundle::isField, this, _1));
  return std::vector<std::string>(range.begin(), range.end());
}

MatrixHandle Bundle::getMatrix(const std::string& name) const
{
  return boost::dynamic_pointer_cast<Matrix>(get(name));
}

bool Bundle::isMatrix(const std::string& name) const
{
  return getMatrix(name) != nullptr;
}

size_t Bundle::numMatrices() const
{
  return boost::count_if(bundle_ | boost::adaptors::map_keys, boost::bind(&Bundle::isMatrix, this, _1));
}

std::vector<MatrixHandle> Bundle::getMatrices() const
{
  auto range = bundle_ | boost::adaptors::map_keys | boost::adaptors::transformed(boost::bind(&Bundle::getMatrix, this, _1)) | boost::adaptors::filtered([] (MatrixHandle m) { return m != nullptr; });
  return std::vector<MatrixHandle>(range.begin(), range.end());
}

std::vector<std::string> Bundle::getMatrixNames() const
{
  auto range = bundle_ | boost::adaptors::map_keys | boost::adaptors::filtered(boost::bind(&Bundle::isMatrix, this, _1));
  return std::vector<std::string>(range.begin(), range.end());
}

StringHandle Bundle::getString(const std::string& name) const
{
  return boost::dynamic_pointer_cast<String>(get(name));
}

bool Bundle::isString(const std::string& name) const
{
  return getString(name) != nullptr;
}

size_t Bundle::numStrings() const
{
  return boost::count_if(bundle_ | boost::adaptors::map_keys, boost::bind(&Bundle::isString, this, _1));
}

std::vector<StringHandle> Bundle::getStrings() const
{
  auto range = bundle_ | boost::adaptors::map_keys | boost::adaptors::transformed(boost::bind(&Bundle::getString, this, _1)) | boost::adaptors::filtered([] (StringHandle s) { return s != nullptr; });
  return std::vector<StringHandle>(range.begin(), range.end());
}

std::vector<std::string> Bundle::getStringNames() const
{
  auto range = bundle_ | boost::adaptors::map_keys | boost::adaptors::filtered(boost::bind(&Bundle::isString, this, _1));
  return std::vector<std::string>(range.begin(), range.end());
}

bool Bundle::remove(const std::string& name)
{
  return bundle_.erase(name) == 1;
}
