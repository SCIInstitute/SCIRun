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


#ifndef SCIRUN_CORE_DATATYPES_BUNDLE_H
#define SCIRUN_CORE_DATATYPES_BUNDLE_H 1

#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Legacy/Bundle/share.h>

namespace SCIRun {
  namespace Core {
    namespace Datatypes {

      /// @todo:
      // 0. Bundle I/O
      // 1. Store ColorMaps, Nrrds, and Bundles
      // 2. Matrix <-> Nrrd interop. Should go in separate class.

class SCISHARE Bundle : public Datatype
{
  public:
    Bundle();

    virtual Bundle* clone() const;

    bool empty() const;
    size_t size() const;

    DatatypeHandle get(const std::string& name) const;
    void set(const std::string& name, DatatypeHandle data);

    bool isField(const std::string& name) const;
    FieldHandle getField(const std::string& name) const;
    size_t numFields() const;
    std::vector<FieldHandle> getFields() const;
    std::vector<std::string> getFieldNames() const;

    bool isMatrix(const std::string& name) const;
    MatrixHandle getMatrix(const std::string& name) const;
    size_t numMatrices() const;
    std::vector<MatrixHandle> getMatrices() const;
    std::vector<std::string> getMatrixNames() const;

    bool isString(const std::string& name) const;
    StringHandle getString(const std::string& name) const;
    size_t numStrings() const;
    std::vector<StringHandle> getStrings() const;
    std::vector<std::string> getStringNames() const;

    bool remove(const std::string& name);

    typedef std::map<std::string, Core::Datatypes::DatatypeHandle> UnderlyingMapType;

    UnderlyingMapType::const_iterator begin() const { return bundle_.begin(); }
    UnderlyingMapType::const_iterator end() const { return bundle_.end(); }

    /// For writing bundles to file
    virtual void io(Piostream&);
    static PersistentTypeID type_id;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    /// Merge two bundles together
    void merge(SCIRun::LockingHandle<Bundle> C);

    /// Transpose when doing a matrix to nrrd conversion
    void transposeNrrd(bool on);

    // The basic functions for managing fields
    ///  getfield     -> Retrieve a Handle to a field stored in the bundle
    ///  setfield     -> Add a field with a name, if it already exists the
    ///                  old one is overwritten
    ///  remfield     -> Remove a handle from the bundle
    ///  isfield      -> Test whether a field is present in the bundle
    ///  numfields    -> The number of fields stored in the bundle
    ///  getfieldname -> Get the nth name in the bundle for building a contents
    ///                  list

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

    std::string getMatrixName(int index);
      // Implementation in cc file, with NRRD/MATRIX compatibility

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

    /// Get the name of the handles in the bundle
    std::string getHandleName(int index)
      { return bundleName_[static_cast<size_t>(index)]; }

    /// Get one of the handles in the bundle
    LockingHandle<PropertyManager> gethandle(int index)
      { return bundle_[static_cast<size_t>(index)]; }

    /// Get the type of a handle (for BundleInfo)
    std::string    getHandleType(int index);
#endif
    virtual std::string dynamic_type_name() const { return type_id.type; }

private:

  template <typename OfType>
  size_t numObjs(OfType ofType) const
  {
    return std::count_if(begin(), end(),
      [ofType](const UnderlyingMapType::value_type& p) { return ofType(p.first); }
    );
  }

  template <typename T, typename OfType>
  std::vector<T> getObjs(OfType typedGet) const
  {
    std::vector<T> objs;
    std::transform(begin(), end(), std::back_inserter(objs),
      [typedGet](const UnderlyingMapType::value_type& p)
      {
        return typedGet(p.first);
      });

    objs.erase(std::remove_if(objs.begin(), objs.end(), [](const T& t) { return t == nullptr; }), objs.end());
    return objs;
  }

  template <typename OfType>
  std::vector<std::string> getObjNames(OfType ofType) const
  {
    std::vector<std::string> names;
    for (const auto& p : bundle_)
    {
      if (ofType(p.first))
        names.push_back(p.first);
    }
    return names;
  }

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER


    /// Functions for doing NRRD/MATRIX conversion
    template<class PTYPE>
      inline bool NrrdToMatrixHelper(NrrdDataHandle dataH, MatrixHandle& matH);

    bool NrrdToMatrixConvertible(NrrdDataHandle nrrdH);
    bool NrrdToMatrix(NrrdDataHandle dataH,MatrixHandle& matH);
    bool MatrixToNrrdConvertible(MatrixHandle matH);
    bool MatrixToNrrd(MatrixHandle matH,NrrdDataHandle &nrrdH);

    /// Setting for the conversion between NRRD/MATRIX
    bool transposeNrrd_;
#endif
  UnderlyingMapType bundle_;
};

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
inline void Bundle::transposeNrrd(bool transpose)
{
  transposeNrrd_ = transpose;
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
#endif
}}}

#endif
