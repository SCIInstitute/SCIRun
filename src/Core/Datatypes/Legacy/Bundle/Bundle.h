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

#ifndef SCIRUN_CORE_DATATYPES_BUNDLE_H
#define SCIRUN_CORE_DATATYPES_BUNDLE_H 1

#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/NrrdData.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Core/Containers/LockingHandle.h>

#include <deque>
#include <string>

#include <Core/Datatypes/share.h>

namespace SCIRun {

class SCISHARE Bundle : public PropertyManager {
  
  public:  
  
    /// Constructor
    Bundle();
    Bundle(const Bundle& copy);
    
    /// Destructor
    virtual ~Bundle();

    /// SCIRun's way of copying
    virtual Bundle* clone();

    /// For writing bundles to file
    virtual void io(Piostream&);
    static PersistentTypeID type_id;

    /// Basic functionality
      
    /// Get handle to an object from the bundle
    template<class T> inline LockingHandle<T> get(const std::string& name);
    
    /// Add or replace an object in the bundle
    template<class T> inline void set(const std::string& name, 
                                        LockingHandle<T> &handle);
    
    /// Check whether an object is present 
    template<class T> inline bool is(const std::string& name);
    
    /// Get the number of objects of a certain type
    template<class T> inline int num();
    
    /// Get the name of an object
    template<class T> inline std::string getName(int index);

    /// Remove object
    inline void rem(const std::string& name);
   
    /// Merge two bundles together
    void merge(SCIRun::LockingHandle<Bundle> C);

    /// Transpose when doing a matrix to nrrd conversion
    void transposeNrrd(bool on);

    /// The basic functions for managing fields
    ///  getfield     -> Retrieve a Handle to a field stored in the bundle
    ///  setfield     -> Add a field with a name, if it already exists the 
    ///                  old one is overwritten
    ///  remfield     -> Remove a handle from the bundle
    ///  isfield      -> Test whether a field is present in the bundle
    ///  numfields    -> The number of fields stored in the bundle 
    ///  getfieldname -> Get the nth name in the bundle for building a contents 
    ///                  list
  
    LockingHandle<Field> getField(const std::string& name) 
      { return(get<Field>(name)); }
      
    void setField(const std::string& name, LockingHandle<Field> &field) 
      { set<Field>(name,field); }
      
    void remField(const std::string& name)
      { rem(name); }
      
    bool isField(const std::string& name)  
      { return(is<Field>(name)); }
      
    int  numFields() 
      { return(num<Field>()); }
          
    std::string getFieldName(int index) 
      { return(getName<Field>(index)); }
  
    /// The basic functions for managing matrices
    ///  getmatrix     -> Retrieve a Handle to a matrix stored in the bundle
    ///  setmatrix     -> Add a matrix with a name, if it already exists the old 
    ///                   one is overwritten
    ///  remmatrix     -> Remove a handle from the bundle
    ///  ismatrix      -> Test whether a matrix is present in the bundle
    ///  nummatrices   -> The number of matrices stored in the bundle 
    ///  getmatrixname -> Get the nth name in the bundle for building a contents 
    ///                   list
  
    MatrixHandle getMatrix(const std::string& name);
      // Implementation in cc file, with NRRD/MATRIX compatibility
      
    void setMatrix(const std::string& name, MatrixHandle& matrix)
      { set< Matrix<double> >(name,matrix); }
    
    void remMatrix(const std::string& name) 
      { rem(name); }
    
    bool isMatrix(const std::string& name); 
      // Implementation in cc file, with NRRD/MATRIX compatibility
    
    int  numMatrices();
      // Implementation in cc file, with NRRD/MATRIX compatibility
    
    std::string getMatrixName(int index);
      // Implementation in cc file, with NRRD/MATRIX compatibility

    /// The basic functions for managing matrices
    ///  getstring     -> Retrieve a Handle to a matrix stored in the bundle
    ///  setstring     -> Add a matrix with a name, if it already exists the old
    ///                   one is overwritten
    ///  remstring     -> Remove a handle from the bundle
    ///  isstring      -> Test whether a matrix is present in the bundle
    ///  numstrings    -> The number of matrices stored in the bundle 
    ///  getstringname -> Get the nth name in the bundle for building a contents 
    ///                   list
  
    LockingHandle<String> getString(const std::string& name) 
      { return(get<String>(name)); }
      
    void setString(const std::string& name, LockingHandle<String> &str) 
      { set<String>(name,str); }
      
    void remString(const std::string& name) 
      { rem(name); }
    
    bool isString(const std::string& name)  
      { return(is<String>(name)); }
    
    int  numStrings() 
      { return(num<String>()); }
    
    std::string getStringName(int index) 
      { return(getName<String>(index)); }


    /// The basic functions for managing nrrds
    ///  getnrrd     -> Retrieve a Handle to a matrix stored in the bundle
    ///  setnrrd     -> Add a nrrd with a name, if it already exists the old one 
    ///                 is overwritten
    ///  remnrrd     -> remove a handle from the bundle
    ///  isnrrd      -> Test whether a nrrd is present in the bundle
    ///  numnrrds    -> The number of nrrds stored in the bundle 
    ///  getnrrdname -> Get the nth name in the bundle for building a contents 
    ///                 list
  
    LockingHandle<NrrdData> getNrrd(const std::string& name);
      // Implementation in cc file, with NRRD/MATRIX compatibility    
    
    void setNrrd(const std::string& name, LockingHandle<NrrdData> &nrrd) 
      { set<NrrdData>(name,nrrd); }

    void remNrrd(const std::string& name) 
      { rem(name); }
      
    bool isNrrd(const std::string& name);
      // Implementation in cc file, with NRRD/MATRIX compatibility

    int  numNrrds();
      // Implementation in cc file, with NRRD/MATRIX compatibility
    
    std::string getNrrdName(int index);
      // Implementation in cc file, with NRRD/MATRIX compatibility

    /// The basic functions for managing colormaps
    ///  getcolormap     -> Retrieve a Handle to a colormap stored in the bundle
    ///  setcolormap     -> Add a colormap with a name, if it already exists the
    ///                     old one is overwritten
    ///  remcolormap     -> Remove a handle from the bundle
    ///  iscolormap      -> Test whether a colormap is present in the bundle
    ///  numcolormaps    -> The number of colormaps stored in the bundle 
    ///  getcolormapname -> Get the nth name in the bundle for building a 
    ///                     contents list

    LockingHandle<ColorMap> getColorMap(const std::string& name) 
      { return(get<ColorMap>(name)); }
    
    void setColorMap(const std::string& name, LockingHandle<ColorMap> &colormap) 
      { set<ColorMap>(name,colormap); }
    
    void remColorMap(const std::string& name) 
      { rem(name); }
    
    bool isColorMap(const std::string& name)  
      { return(is<ColorMap>(name)); }
      
    int numColorMaps() 
      { return(num<ColorMap>()); }
      
    std::string getColorMapName(int index) 
      { return(getName<ColorMap>(index)); }
  
    /// The basic functions for managing bundles
    ///  getbundle     -> Retrieve a Handle to a bundle stored in the bundle
    ///  setbundle     -> Add a bundle with a name, if it already exists the old
    ///                   one is overwritten
    ///  rembundle     -> Remove a handle from the bundle
    ///  isbundle      -> Test whether a bundle is present in the bundle
    ///  numbundles    -> The number of bundles stored in the bundle 
    ///  getbundleName -> Get the nth name in the bundle for building a contents 
    ///                   list

    LockingHandle<Bundle> getBundle(const std::string& name) 
      { return(get<Bundle>(name)); }
      
    void setBundle(const std::string& name, LockingHandle<Bundle> &bundle) 
      { set<Bundle>(name,bundle); }
      
    void remBundle(const std::string& name) 
      { rem(name); }
      
    bool isBundle(const std::string& name)  
      { return(is<Bundle>(name)); }
    
    int  numBundles() 
      { return(num<Bundle>()); }
      
    std::string getBundleName(int index) 
      { return(getName<Bundle>(index)); }
 
    
    /// Get the number of elements in the Bundle
    int getNumHandles() 
      { return static_cast<int>(bundle_.size()); }
    
    /// Get the name of the handles in the bundle     
    std::string getHandleName(int index) 
      { return bundleName_[static_cast<size_t>(index)]; }
      
    /// Get one of the handles in the bundle  
    LockingHandle<PropertyManager> gethandle(int index) 
      { return bundle_[static_cast<size_t>(index)]; }
    
    /// Get the type of a handle (for BundleInfo)
    std::string    getHandleType(int index);

    virtual std::string dynamic_type_name() const { return type_id.type; }
 
  private:
    
    /// find the handle in the bundle
    int findName(std::deque<std::string> &deq, const std::string& name);
    
    /// Functions for doing NRRD/MATRIX conversion
    template<class PTYPE> 
      inline bool NrrdToMatrixHelper(NrrdDataHandle dataH, MatrixHandle& matH);
    
    bool NrrdToMatrixConvertible(NrrdDataHandle nrrdH);
    bool NrrdToMatrix(NrrdDataHandle dataH,MatrixHandle& matH);
    bool MatrixToNrrdConvertible(MatrixHandle matH);
    bool MatrixToNrrd(MatrixHandle matH,NrrdDataHandle &nrrdH);

    /// The names of the elements in the bundle
    std::deque<std::string> bundleName_;
    
    /// An array with handle to all the objects in the bundle
    std::deque<LockingHandle<PropertyManager> > bundle_;
  
    /// Setting for the conversion between NRRD/MATRIX
    bool transposeNrrd_;
  
};

typedef LockingHandle<Bundle> BundleHandle;

inline void Bundle::transposeNrrd(bool transpose)
{
  transposeNrrd_ = transpose;
}


template<class T> inline LockingHandle<T> Bundle::get(const std::string& name)
{
  int index;
  index = findName(bundleName_,name);
  if (index == -1) return 0;
  LockingHandle<T> handle = SCI_DATATYPE_CAST<T*>(bundle_[index].get_rep());
  return(handle);
}


template<class T> inline void Bundle::set(const std::string& name,
                                              LockingHandle<T> &handle)
{
  int index;
  index = findName(bundleName_,name);
  if (index == -1)
  {
    LockingHandle<PropertyManager> lhandle = 
                            dynamic_cast<PropertyManager*>(handle.get_rep());
    bundle_.push_back(lhandle);
    bundleName_.push_back(name);
  }
  else
  {
    bundle_[index] = dynamic_cast<PropertyManager*>(handle.get_rep());
    bundleName_[index] = name;
  }
}


template<class T> inline bool Bundle::is(const std::string& name)
{
  int index;
  if ((index = findName(bundleName_,name)) > -1)
  {
    if (SCI_DATATYPE_CAST<T*>(bundle_[index].get_rep()) != 0) return(true);
  }
  return(false);
}


template<class T> inline int Bundle::num()
{
  int cnt = 0;
  for (size_t p=0;p<bundleName_.size();p++)
    if (SCI_DATATYPE_CAST<T*>(bundle_[p].get_rep()) != 0) cnt++;
  return(cnt);
}


template<class T> inline std::string Bundle::getName(int index)
{
  int cnt = -1;
  size_t p;
  for (p=0;p<bundleName_.size();p++)
  {
    if (SCI_DATATYPE_CAST<T*>(bundle_[p].get_rep()) != 0) cnt++;
    if (index == cnt) break;
  }
  if ((p < bundleName_.size())&&(cnt==index)) return bundleName_[p];
  return(std::string(""));
}


inline void Bundle::rem(const std::string& name)
{
  int index;
  index = findName(bundleName_,name);
  if (index > -1)
  {
    bundle_.erase(bundle_.begin()+index);
    bundleName_.erase(bundleName_.begin()+index);
  }
}

  
template<class PTYPE> inline bool Bundle::NrrdToMatrixHelper(
                                    NrrdDataHandle dataH, MatrixHandle& matH)
{
  if (dataH->nrrd_->dim == 1)
  {
    int cols = dataH->nrrd_->axis[0].size;

    ColumnMatrix* matrix = new ColumnMatrix(cols);

    PTYPE *val = reinterpret_cast<PTYPE*>(dataH->nrrd_->data);
    double *data = matrix->get_data_pointer();

    for(int c=0; c<cols; c++) 
    {
      *data = *val;
      data++;
      val++;
    }
    matH = matrix;
    return(true);
  }

  if (dataH->nrrd_->dim == 2)
  {
    if (transposeNrrd_)
      {
        int rows = dataH->nrrd_->axis[1].size;
        int cols = dataH->nrrd_->axis[0].size;

        DenseMatrix* matrix = new DenseMatrix(rows,cols);
  
        PTYPE *val = reinterpret_cast<PTYPE*>(dataH->nrrd_->data);
        double *data = matrix->get_data_pointer();

        int i,j;
        i = 0; j = 0;
        for(int r=0; r<rows; r++) 
          {
            for(int c=0; c<cols; c++) 
              {
                i = c + cols*r;
                data[j++] = val[i];
              }
          }
        matH = matrix;
      }
    else
    {
      int cols = dataH->nrrd_->axis[1].size;
      int rows = dataH->nrrd_->axis[0].size;

      DenseMatrix* matrix = new DenseMatrix(cols,rows);

      PTYPE *val = reinterpret_cast<PTYPE*>(dataH->nrrd_->data);
      double *data = matrix->get_data_pointer();
      
      for(int c=0; c<cols; c++) 
      {
        for(int r=0; r<rows; r++) 
        {
          *data++ = *val++;
        }
      }
      matH = matrix;  
    }
    return(true);
  }
  // Improper dimensions
  return(false);
}

} // end namespace SCIRun

#endif 
