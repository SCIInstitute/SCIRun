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


/*
 * FILE: matlabconverter.cc
 * AUTH: Jeroen G Stinstra
 * DATE: 17 OCT 2004
 */

#include <Core/Matlab/matlabconverter.h>
#include <Core/Matlab/fieldtomatlab.h>
#include <Core/Matlab/matlabtofield.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <boost/assign/std/vector.hpp>

using namespace SCIRun;
using namespace SCIRun::MatlabIO;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;
using namespace boost::assign;

// Currently the property converter only manages strings
// all other data is ignored both on Matlab side as well
// as on the property manager side.

/* DESIGN NOTES : */

/*
 * The BIG Problem in SCIRun is that it is hard to know what
 * kind of object a SCIRun property is and how it should be
 * translated. For example there are countless array classes and
 * no attempt has been made to standardize the data formats
 * which leads to a huge conversion problem. Hence only the useful
 * objects are translated the rest is just discarded. Until there
 * is a better data management structure within SCIRun, the conversion
 * process is basically a big switch statement scanning for each possible
 * object structure
 */

/*
 * With the hope in mind that a better data management system will be
 * in place in the future, all conversion algorithms are grouped in
 * this one object. All the other Matlab classes function independent
 * of the main SCIRun structure, it has its own memory and data management
 * making it easier to adapt to future changes. The separation of Matlab
 * and SCIRun data management has the advantage that the Matlab side can be
 * further enhanced to deal with different future Matlab file formats, without
 * having to comb through the conversion code. Though there is a little
 * memory overhead. Especially with the V7 compressed files, more memory
 * is needed to maintain the integrity of the Matlab reader. Some changes
 * in this converter may be needed to enhance performance. Currently the
 * a compressed file will be decompressed and scanned for suitable objects.
 * Upon loading the matrix, the matrix will be decompressed again as after
 * scanning the file, nothing will remain in memory.
 */


// Manage converter options

// Set defaults in the constructor
matlabconverter::matlabconverter() :
    pr_(0),
    numericarray_(false),
    indexbase_(1),
    datatype_(matlabarray::miSAMEASDATA),
    disable_transpose_(false),
    prefer_nrrds(false),
    prefer_bundles(false)
{
}

matlabconverter::matlabconverter(LoggerHandle pr) :
    pr_(pr),
    numericarray_(false),
    indexbase_(1),
    datatype_(matlabarray::miSAMEASDATA),
    disable_transpose_(false),
    prefer_nrrds(false),
    prefer_bundles(false)
{
}

///// FOR N(E)RRD BUSINESS ////////////////////////

unsigned int matlabconverter::convertmitype(matlabarray::mitype type)
{
  switch (type)
    {
    case matlabarray::miINT8:   return(nrrdTypeChar);
    case matlabarray::miUINT8:  return(nrrdTypeUChar);
    case matlabarray::miINT16:  return(nrrdTypeShort);
    case matlabarray::miUINT16: return(nrrdTypeUShort);
    case matlabarray::miINT32:  return(nrrdTypeInt);
    case matlabarray::miUINT32: return(nrrdTypeUInt);
    case matlabarray::miINT64:  return(nrrdTypeLLong);
    case matlabarray::miUINT64: return(nrrdTypeULLong);
    case matlabarray::miSINGLE: return(nrrdTypeFloat);
    case matlabarray::miDOUBLE: return(nrrdTypeDouble);
    default: return(nrrdTypeUnknown);
    }
}


// This function converts nrrds into Matlab matrices
// only the data is being transformed into a Matlab array

matlabarray::mitype matlabconverter::convertnrrdtype(int type)
{
  switch (type)
    {
    case nrrdTypeChar : return(matlabarray::miINT8);
    case nrrdTypeUChar : return(matlabarray::miUINT8);
    case nrrdTypeShort : return(matlabarray::miINT16);
    case nrrdTypeUShort : return(matlabarray::miUINT16);
    case nrrdTypeInt : return(matlabarray::miINT32);
    case nrrdTypeUInt : return(matlabarray::miUINT32);
    case nrrdTypeLLong : return(matlabarray::miINT64);
    case nrrdTypeULLong : return(matlabarray::miUINT64);
    case nrrdTypeFloat : return(matlabarray::miSINGLE);
    case nrrdTypeDouble : return(matlabarray::miDOUBLE);
    default: return(matlabarray::miUNKNOWN);
    }
}

// Support function to check whether the names supplied are within the
// rules that Matlab allows. Otherwise we could save the file, but Matlab
// would complain it could not read the file.

bool matlabconverter::isvalidmatrixname(const std::string& name)
{
  const std::string validchar("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
  const std::string validstartchar("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

  bool valid = true;
  bool foundchar = false;

  for (int p=0; p < static_cast<int>(name.size()); p++)
  {
    if (p == 0)
    {
      // A variable name is not allowed to start with a number
      foundchar = false;
      for (int q = 0; q < static_cast<int>(validstartchar.size()); q++)
      {
        if (name[p] == validstartchar[q]) { foundchar = true; break; }
      }
    }
    else
    {
      foundchar = false;
      for (int q = 0; q < static_cast<int>(validchar.size()); q++)
      {
        if (name[p] == validchar[q]) { foundchar = true; break; }
      }
    }
    if (foundchar == false) { valid = false; break; }
  }
  return(valid);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
void matlabconverter::mlPropertyTOsciProperty(matlabarray &ma,PropertyManager *handle)
{
  int numfields;
  matlabarray::mlclass mclass;
  matlabarray subarray;
  std::string propname;
  std::string propval;
  matlabarray proparray;

  std::string dummyinfo;
  int matrixscore;
  int fieldscore;

  NrrdDataHandle  nrrd;
  MatrixHandle    matrix;
  FieldHandle     field;

  // properties are stored in field property
  int propindex = ma.getfieldnameindexCI("property");

  if (propindex > -1)
  { // field property exists

    proparray = ma.getfield(0,propindex);
    if (proparray.isempty()) return;

    numfields = proparray.getnumfields();

    for (int p=0; p<numfields; p++)
    {
      subarray = proparray.getfield(0,p);
      mclass = subarray.getclass();

      // Check whether property is string
      // In the property manager string are STL strings
      if (mclass == matlabarray::mlSTRING)
      {   // only string arrays are converted
        propname = proparray.getfieldname(p);
        propval = subarray.getstring();
        handle->set_property(propname,propval,false);
        continue;
      }

      if ((fieldscore = sciFieldCompatible(subarray,dummyinfo)))
      {
        if (fieldscore > 1)
        {
           propname = proparray.getfieldname(p);
           mlArrayTOsciField(subarray,field);
           handle->set_property(propname,field,false);
           continue;
        }
      }

      if ((matrixscore = sciMatrixCompatible(subarray,dummyinfo)))
      {
        if (matrixscore > 1)
        {
           propname = proparray.getfieldname(p);
           mlArrayTOsciMatrix(subarray,matrix);
           handle->set_property(propname,matrix,false);
           continue;
        }
        else
        {
          if (sciNrrdDataCompatible(subarray,dummyinfo))
          {
            propname = proparray.getfieldname(p);
            mlArrayTOsciNrrdData(subarray,nrrd);
            handle->set_property(propname,nrrd,false);
            continue;
          }
          propname = proparray.getfieldname(p);
          mlArrayTOsciMatrix(subarray,matrix);
          handle->set_property(propname,matrix,false);
          continue;
        }
      }

      if (sciNrrdDataCompatible(subarray,dummyinfo))
      {
        propname = proparray.getfieldname(p);
        mlArrayTOsciNrrdData(subarray,nrrd);
        handle->set_property(propname,nrrd,false);
        continue;
      }
      if (fieldscore > 0)
      {
        propname = proparray.getfieldname(p);
        mlArrayTOsciField(subarray,field);
        handle->set_property(propname,field,false);
        continue;
      }
    }
  }
}

void matlabconverter::sciPropertyTOmlProperty(PropertyManager *handle,matlabarray &ma)
{
  size_t numfields;
  matlabarray proparray;
  std::string propname;
  std::string propvalue;
  matlabarray subarray;

  StringHandle    str;
  MatrixHandle    matrix;
  NrrdDataHandle  nrrd;
  FieldHandle     field;

  proparray.createstructarray();
  numfields = handle->nproperties();

  for (size_t p=0;p<numfields;p++)
  {
    propname = handle->get_property_name(p);
    if (handle->get_property(propname,propvalue))
    {
      subarray.createstringarray(propvalue);
      proparray.setfield(0,propname,subarray);
    }
    if (handle->get_property(propname,nrrd))
    {
      subarray.clear();
      bool oldnumericarray_ = numericarray_;
      numericarray_ = true;
      sciNrrdDataTOmlArray(nrrd,subarray);
      numericarray_ = oldnumericarray_;
      proparray.setfield(0,propname,subarray);
    }
    if (handle->get_property(propname,matrix))
    {
      subarray.clear();
      bool oldnumericarray_ = numericarray_;
      numericarray_ = true;
      sciMatrixTOmlArray(matrix,subarray);
      numericarray_ = oldnumericarray_;
      proparray.setfield(0,propname,subarray);
    }
    if (handle->get_property(propname,field))
    {
      subarray.clear();
      sciFieldTOmlArray(field,subarray);
      proparray.setfield(0,propname,subarray);
    }
    if (handle->get_property(propname,str))
    {
      subarray.clear();
      sciStringTOmlArray(str,subarray);
      proparray.setfield(0,propname,subarray);
    }
  }
  ma.setfield(0,"property",proparray);
}

int matlabconverter::sciColorMapCompatible(matlabarray &ma, std::string &infotext, bool postremark)
{
  infotext = "";
  if (ma.isempty()) return(0);
  if (ma.getnumelements() == 0) return(0);

  matlabarray::mlclass mclass;
  mclass = ma.getclass();

  switch (mclass)
  {
    case matlabarray::mlDENSE:
      {
        // check whether the data is of a proper format

        std::vector<int> dims;
        dims = ma.getdims();
        if (dims.size() > 2)
        {
          if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun ColorMap (dimensions > 2)."));
          return(0); // no multidimensional arrays supported yet in the SCIRun Matrix classes
        }
        if (ma.getnumelements() == 0)
        {
          if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun ColorMap (0x0 matrix)."));
          return(0); // no multidimensional arrays supported yet in the SCIRun Matrix classes
        }

        if ((dims[0]!=3)&&(dims[0]!=4)&&(dims[1]!=3)&&(dims[1]!=4))
        {
          if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun ColorMap (improper dimensions)."));
          return(0); // no multidimensional arrays supported yet in the SCIRun Matrix classes
        }

        std::ostringstream oss;
        oss << dims[0];
        infotext = ma.getname() + "  COLORMAP [" + oss.str() + " COLORS]";
        return(2);
      }
    break;
    default:
    break;
  }

  if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun ColorMap (matrix is not a dense array)."));
  return (0);
}
#endif
// The next function checks whether
// the program knows how to convert
// the matlabarray into a scirun matrix

int matlabconverter::sciMatrixCompatible(const matlabarray &ma, std::string &infotext, bool postremark)
{
  infotext = "";
  if (ma.isempty())
  {
    if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun Matrix (empty matrix)."));
    return(0);
  }

  matlabarray::mlclass mclass;
  mclass = ma.getclass();

  switch (mclass) {
  case matlabarray::mlDENSE:
  case matlabarray::mlSPARSE:
    {
      // check whether the data is of a proper format

      std::vector<int> dims;
      dims = ma.getdims();
      if (dims.size() > 2)
      {
        if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun Matrix (dimensions > 2)."));
        return(0); // no multidimensional arrays supported yet in the SCIRun Matrix classes
      }
      if ((ma.getnumelements() == 0)&&(!ma.issparse()))
      {
        if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun Matrix (0x0 matrix)."));
        return(0); // no multidimensional arrays supported yet in the SCIRun Matrix classes
      }

      matlabarray::mitype type;
      type = ma.gettype();

      infotext = ma.getinfotext();

      // We expect a double array.
      // This classification is pure for convenience
      // Any integer array can be dealt with as well

      // doubles are most likely to be the data wanted by the users
      if (type == matlabarray::miDOUBLE) return(4);
      // though singles should work as well
      if (type == matlabarray::miSINGLE) return(3);
      // all other numeric formats should be integer types, which
      // can be converted using a simple cast
      return(2);
    }
  case matlabarray::mlSTRUCT:
    {
      int index;
      /* A lot of different names can be used for the data:
         This has mainly historical reasons: a lot of different
         names have been used at CVRTI to store data, to be
         compatible with all, we allow all of them. Though we
         suggest the use of : "data"
      */

      index = ma.getfieldnameindexCI("data");
      if (index == -1) index = ma.getfieldnameindexCI("potvals");       // in case it is a saved TSDF file
      if (index == -1) index = ma.getfieldnameindexCI("field");
      if (index == -1) index = ma.getfieldnameindexCI("scalarfield");
      if (index == -1) index = ma.getfieldnameindexCI("vectorfield");
      if (index == -1) index = ma.getfieldnameindexCI("tensorfield");
      if (index == -1)
      {
        if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun Matrix (cannot find a field with data: create a .data field)."));
        return(0); // incompatible
      }

      int numel;
      numel = ma.getnumelements();
      if (numel > 1)
      {
        if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun Matrix (the struct matrix is not 1x1: do not define more than one matrix)."));
        return(0); // incompatible
      }

      matlabarray subarray;
      subarray = ma.getfield(0,index);

      // check whether the data is of a proper format

      if (subarray.isempty())
      {
        if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun Matrix (no data: matrix is empty)."));
        return(0); // not compatible
      }

      std::vector<int> dims;
      dims = subarray.getdims();
      if (dims.size() > 2)
        {
          if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun Matrix (dimensions > 2)."));
          return(0); // no multidimensional arrays supported yet in the SCIRun Matrix classes
        }

      matlabarray::mitype type;
      type = subarray.gettype();

      infotext = subarray.getinfotext(ma.getname());
      // We expect a double array.
      // This classification is pure for convenience
      // Any integer array can be dealt with as well

      // doubles are most likely to be the data wanted by the users
      if (type == matlabarray::miDOUBLE) return(4);
      // though singles should work as well
      if (type == matlabarray::miSINGLE) return(3);
      // all other numeric formats should be integer types, which
      // can be converted using a simple cast
      return(1);
    }
  default:
    break;
  }

  if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun Matrix (matrix is not struct, dense or sparse array)."));
  return (0);
}


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
int matlabconverter::sciStringCompatible(const matlabarray &ma, std::string &infotext, bool postremark)
{
  infotext = "";
  if (ma.isempty()) return(0);
  if (ma.getnumelements() == 0) return(0);

  matlabarray::mlclass mclass = ma.getclass();

  switch (mclass)
  {
    case matlabarray::mlSTRING:
      {
        infotext = ma.getinfotext();
        return(3);
      }
      break;
    default:
      break;
  }

  if (postremark) remark(std::string("Matrix '" + ma.getname() + "' cannot be translated into a SCIRun String (matrix is not a string)"));
  return (0);
}

void matlabconverter::mlArrayTOsciString(const matlabarray &ma, StringHandle &handle)
{
  matlabarray::mlclass mclass = ma.getclass();

  switch(mclass)
  {
    case matlabarray::mlSTRING:
      {
        handle = dynamic_cast<String *>(new String(ma.getstring()));
        if (handle.get_rep() == 0) throw error_type();
      }
      break;
    default:
      {   // The program should not get here
        throw error_type();
      }
  }
}

void matlabconverter::sciStringTOmlArray(StringHandle scistr,matlabarray &mlmat)
{
  mlmat.createstringarray(scistr->get());
}
#endif

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
void matlabconverter::mlArrayTOsciColorMap(const matlabarray &ma,ColorMapHandle &handle)
{
  int m,n;
  m = static_cast<int>(ma.getm());

  if ((m != 4) && (m != 3)) ma.transpose();
  m = static_cast<int>(ma.getm());
  n = static_cast<int>(ma.getn());

  std::vector<double> data;
  ma.getnumericarray(data);

  int q = 0;
  float v = 0.0f;
  float step = 1.0f;
  float alpha = 1.0f;
  if(n > 1) step = 1.0f/static_cast<float>(n-1);

  std::vector<SCIRun::Color> rgb(n);
  std::vector<float> rgbT(n);
  std::vector<float> alph(n);

  for (int p = 0; p < n; p++)
  {
    SCIRun::Color color(data[q],data[q+1],data[q+2]); q+=3;
    rgb[p] = color;
    if (m == 4) alpha = static_cast<float>(data[q++]);
    rgbT[p] = v;
    alph[p] = alpha;
    v += step;
  }

  handle = dynamic_cast<SCIRun::ColorMap *>(new SCIRun::ColorMap(rgb,rgbT,alph,rgbT,static_cast<unsigned int>(n)));
}
#endif

void matlabconverter::mlArrayTOsciMatrix(const matlabarray &ma,MatrixHandle &handle)
{
  matlabarray::mlclass mclass = ma.getclass();

  switch(mclass)
  {
    case matlabarray::mlDENSE:
      {
        int m = static_cast<int>(ma.getm());
        int n = static_cast<int>(ma.getn());

        if (disable_transpose_)
        {
          DenseMatrixHandle dmptr(new DenseMatrix(n,m));
          // copy and cast elements:
          // getnumericarray is a templated function that casts the data to the supplied pointer
          // type. It needs the dimensions of the memory block (in elements) to make sure
          // everything is still OK.
          ma.getnumericarray(dmptr->data(), dmptr->size());

          handle = dmptr;
        }
        else
        {
          DenseMatrix dm(n,m);
          ma.getnumericarray(dm.data(), dm.size());

          // There is no transpose function to operate on the same memory block
          // Hence, it is a little memory inefficient.

          handle = boost::make_shared<DenseMatrix>(dm.transpose()); // SCIRun has a C++-style matrix and Matlab a FORTRAN-style matrix;
        }
      }
      break;

    case matlabarray::mlSPARSE:
      {
        // Since the SparseRowMatrix does not allocate memory but on the
        // other hand frees it in the destructor. The memory needs to be
        // allocated outside of the object and then linked to the object
        // to have it freed later on by the object.

        // in the matlabio classes they are defined as long, hence
        // the casting operators
        size_type nnz = static_cast<size_type>(ma.getnnz());
        size_type m = static_cast<size_type>(ma.getm());
        size_type n = static_cast<size_type>(ma.getn());

        LegacySparseDataContainer<double> sparseData(n + 1, nnz, nnz);

        // NOTE: this was flipped around in the old code: the arrays rows/cols were passed as cols/rows into the SparseRowMatrix ctor.  Hence the screwy order here, and the transpose call below.
        // REASON: SCIRun uses Row sparse matrices and Matlab Column sparse matrices.
        ma.getnumericarray(sparseData.data().get(), nnz);
        ma.getrowsarray(sparseData.columns().get(), nnz);
        ma.getcolsarray(sparseData.rows().get(), (n + 1));

        SparseRowMatrixHandle sparse(new SparseRowMatrix(n, m, sparseData.rows().get(), sparseData.columns().get(), sparseData.data().get(), nnz));

        if (disable_transpose_)
        {
          handle = sparse;
        }
        else
        {
          handle = boost::make_shared<SparseRowMatrix>(sparse->transpose());
        }
      }
      break;

    case matlabarray::mlSTRUCT:
      {
        // A compatible struct has the following fields
        // - data:     submatrix with the actual matrix in it
        // - property: optional extra struct array with key
        //             /value pairs for the property manager

        int dataindex, propertyindex;
        dataindex = ma.getfieldnameindexCI("data");
        if (dataindex == -1) dataindex = ma.getfieldnameindex("potvals");
        if (dataindex == -1) dataindex = ma.getfieldnameindexCI("field");
        if (dataindex == -1) dataindex = ma.getfieldnameindexCI("scalarfield");
        if (dataindex == -1) dataindex = ma.getfieldnameindexCI("vectorfield");
        if (dataindex == -1) dataindex = ma.getfieldnameindexCI("tensorfield");

        propertyindex = ma.getfieldnameindexCI("property");

        if (dataindex == -1)
        {
          throw error_type();
        }

        matlabarray subarray;
        subarray = ma.getfield(0,dataindex);
        mlArrayTOsciMatrix(subarray,handle);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
        if (propertyindex != -1)
        {
          mlPropertyTOsciProperty(ma,static_cast<PropertyManager *>(handle.get_rep()));
        }
#endif

      }
      break;
    default:
      {   // The program should not get here
        throw error_type();
      }
  }
}

void matlabconverter::sciMatrixTOmlMatrix(MatrixHandle scimat, matlabarray &mlmat)
{
  // Get the format for exporting data
  matlabarray::mitype dataformat = datatype_;

  // SCIRun matrices are always (up till now) doubles
  if (dataformat == matlabarray::miSAMEASDATA) dataformat = matlabarray::miDOUBLE;

  if (matrixIs::dense(scimat))
  {
    DenseMatrix tmatrix = castMatrix::toDense(scimat)->transpose();
    std::vector<int> dims(2);
    dims[1] = tmatrix.nrows();
    dims[0] = tmatrix.ncols();
    mlmat.createdensearray(dims,dataformat);
    mlmat.setnumericarray(tmatrix.data(),mlmat.getnumelements());
  }
  else if (matrixIs::column(scimat))
  {
    std::vector<int> dims(2);
    DenseColumnMatrixHandle cmatrix = castMatrix::toColumn(scimat);
    dims[0] = cmatrix->nrows();
    dims[1] = cmatrix->ncols();
    mlmat.createdensearray(dims,dataformat);
    mlmat.setnumericarray(cmatrix->data(),mlmat.getnumelements());
  }
  else if (matrixIs::sparse(scimat))
  {
    SparseRowMatrix tmatrix = castMatrix::toSparse(scimat)->transpose();

    std::vector<int> dims(2);
    dims[1] = tmatrix.nrows();
    dims[0] = tmatrix.ncols();
    mlmat.createsparsearray(dims,dataformat);
    mlmat.setnumericarray(tmatrix.valuePtr(),tmatrix.nonZeros());
    mlmat.setrowsarray(tmatrix.get_cols(),tmatrix.nonZeros());
    mlmat.setcolsarray(tmatrix.get_rows(),tmatrix.nrows()+1);
  }
}


void matlabconverter::sciMatrixTOmlArray(MatrixHandle scimat,matlabarray &mlmat)
{
  if (numericarray_)
  {
    sciMatrixTOmlMatrix(scimat,mlmat);
  }
  else
  {
    matlabarray dataarray;
    mlmat.createstructarray();
    sciMatrixTOmlMatrix(scimat,dataarray);
    mlmat.setfield(0,"data",dataarray);
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    sciPropertyTOmlProperty(static_cast<PropertyManager *>(scimat.get_rep()),mlmat);
#endif
  }
}


///// N(E)RRD STUFF /////////////////
// We support this because the internal matrix class of SCIRun
// does not support multi dimensional matrices.
// WHen SCIRun does support that, this will hopefully become obsolete
// as nrrd is nerdy...

// Test the compatibility of the matlabarray witha nrrd structure
// in case it is compatible return a positive value and write
// out an infostring with a summary of the contents of the matrix

int matlabconverter::sciNrrdDataCompatible(const matlabarray &mlarray, std::string &infostring, bool postremark)
{
  infostring = "";
  if (mlarray.isempty()) return(0);
  if (mlarray.getnumelements() == 0) return(0);

  matlabarray::mlclass mclass;
  mclass = mlarray.getclass();

  // parse matrices are dealt with in a separate
  // pr as the the data needs to be divided over
  // three separate Nrrds

  // Support for importing strings into SCIRun
  if (mclass == matlabarray::mlSTRING)
  {
    infostring = mlarray.getinfotext(mlarray.getname());
    return(1);
  }

  if ((mclass == matlabarray::mlSTRUCT)||(mclass == matlabarray::mlOBJECT))
  {
    int fieldnameindex;
    matlabarray subarray;

    fieldnameindex = mlarray.getfieldnameindexCI("data");
    if (fieldnameindex == -1) fieldnameindex = mlarray.getfieldnameindexCI("potvals");
    if (fieldnameindex == -1) fieldnameindex = mlarray.getfieldnameindexCI("field");
    if (fieldnameindex == -1) fieldnameindex = mlarray.getfieldnameindexCI("scalarfield");
    if (fieldnameindex == -1) fieldnameindex = mlarray.getfieldnameindexCI("vectorfield");
    if (fieldnameindex == -1) fieldnameindex = mlarray.getfieldnameindexCI("tensorfield");

    if (fieldnameindex == -1)
    {
      if (postremark) remark(std::string("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Nrrd Object (cannot find field with data: create a .data field)."));
      return(0);
    }

    subarray = mlarray.getfield(0,fieldnameindex);
    if (subarray.isempty())
    {
      if (postremark) remark(std::string("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Nrrd Object (field with data is empty)"));
      return(0);
    }

    infostring = subarray.getinfotext(mlarray.getname());
    matlabarray::mitype type;
    type = subarray.gettype();

    matlabarray::mlclass mclass;
    mclass = subarray.getclass();

    if ((mclass != matlabarray::mlDENSE)&&(mclass != matlabarray::mlSPARSE))
    {
      if (postremark) remark(std::string("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Nrrd Object (matrix is not dense or structured array)"));
      return(0);
    }

    if (subarray.getnumdims() > 10)
    {
      if (postremark) remark(std::string("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Nrrd Object (matrix dimension is larger than 10)"));
      return(0);
    }

    if (subarray.getnumelements() == 0)
    {
      if (postremark) remark(std::string("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Nrrd Object (0x0 matrix)"));
      return(0); // no multidimensional arrays supported yet in the SCIRun Matrix classes
    }

    // We expect a double array.
    // This classification is pure for convenience
    // Any integer array can be dealt with as well

    // doubles are most likely to be the data wanted by the users
    if (type == matlabarray::miDOUBLE) return(3);
    // though singles should work as well
    if (type == matlabarray::miSINGLE) return(2);
    // all other numeric formats should be integer types, which
    // can be converted using a simple cast
    return(1);
  }

  if ((mclass != matlabarray::mlDENSE))
  {
    if (postremark) remark(std::string("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Nrrd Object (the data is not dense matrix or a structured array)"));
    return(0); // incompatible for the moment, no converter written for this type yet
  }

  // Need to enhance this code to squeeze out dimensions of size one

  // Nrrds can be multi dimensional hence no limit on the dimensions is
  // needed

  if (mlarray.isempty())
  {
    if (postremark) remark(std::string("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Nrrd Object (matrix is empty)"));
    return(0);
  }

  if (mlarray.getnumelements() == 0)
  {
    if (postremark) remark(std::string("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Nrrd Object (0x0 matrix)"));
    return(0); // no multidimensional arrays supported yet in the SCIRun Matrix classes
  }

  matlabarray::mitype type;
  type = mlarray.gettype();

  infostring = mlarray.getinfotext();

  // We expect a double array.
  // This classification is pure for convenience
  // Any integer array can be dealt with as well

  // doubles are most likely to be the data wanted by the users
  if (type == matlabarray::miDOUBLE) return(4);
  // though singles should work as well
  if (type == matlabarray::miSINGLE) return(3);
  // all other numeric formats should be integer types, which
  // can be converted using a simple cast
  return(2);
}

void matlabconverter::mlArrayTOsciNrrdData(const matlabarray &mlarray,NrrdDataHandle &scinrrd)
{
  // Depending on the matlabclass there are several converters
  // for converting the data from Matlab into a SCIRun Nrrd object

  matlabarray::mlclass mclass;
  mclass = mlarray.getclass();

  // In case no converter is found return 0
  // Hence initialise scinrrd as a NULL ptr

  scinrrd.reset();

  switch(mclass)
  {

    case matlabarray::mlDENSE:
      {   // new environment so I can create new variables

        try
        {
          // new nrrd data handle
          scinrrd.reset(new NrrdData()); // nrrd is owned by the object

          // obtain the type of the new nrrd
          // we want to keep the nrrd type the same
          // as the original Matlab type

          unsigned int nrrdtype = convertmitype(mlarray.gettype());

          // obtain the dimensions of the new nrrd
          size_t nrrddims[NRRD_DIM_MAX];
          std::vector<int> dims = mlarray.getdims();
          int nrrddim = static_cast<int>(dims.size());

          // NRRD cannot do more then 10 dimensions... NRRD
          ASSERT(nrrddim <= NRRD_DIM_MAX);
          for (int p=0;p<nrrddim;p++) nrrddims[p] = dims[p];

          nrrdAlloc_nva(scinrrd->getNrrd(),nrrdtype,nrrddim,nrrddims);

          switch (nrrdtype)
          {
            case nrrdTypeChar:
              mlarray.getnumericarray(static_cast<signed char *>(scinrrd->getNrrd()->data),static_cast<int>(nrrdElementNumber(scinrrd->getNrrd())));
              break;
            case nrrdTypeUChar:
              mlarray.getnumericarray(static_cast<unsigned char *>(scinrrd->getNrrd()->data),static_cast<int>(nrrdElementNumber(scinrrd->getNrrd())));
              break;
            case nrrdTypeShort:
              mlarray.getnumericarray(static_cast<signed short *>(scinrrd->getNrrd()->data),static_cast<int>(nrrdElementNumber(scinrrd->getNrrd())));
              break;
            case nrrdTypeUShort:
              mlarray.getnumericarray(static_cast<unsigned short *>(scinrrd->getNrrd()->data),static_cast<int>(nrrdElementNumber(scinrrd->getNrrd())));
              break;
            case nrrdTypeInt:
              mlarray.getnumericarray(static_cast<signed int *>(scinrrd->getNrrd()->data),static_cast<int>(nrrdElementNumber(scinrrd->getNrrd())));
              break;
            case nrrdTypeUInt:
              mlarray.getnumericarray(static_cast<unsigned int *>(scinrrd->getNrrd()->data),static_cast<int>(nrrdElementNumber(scinrrd->getNrrd())));
              break;
            case nrrdTypeLLong:
              mlarray.getnumericarray(static_cast<int64_t *>(scinrrd->getNrrd()->data),static_cast<int>(nrrdElementNumber(scinrrd->getNrrd())));
              break;
            case nrrdTypeULLong:
              mlarray.getnumericarray(static_cast<uint64_t *>(scinrrd->getNrrd()->data),static_cast<int>(nrrdElementNumber(scinrrd->getNrrd())));
              break;
            case nrrdTypeFloat:
              mlarray.getnumericarray(static_cast<float *>(scinrrd->getNrrd()->data),static_cast<int>(nrrdElementNumber(scinrrd->getNrrd())));
              break;
            case nrrdTypeDouble:
              mlarray.getnumericarray(static_cast<double *>(scinrrd->getNrrd()->data),static_cast<int>(nrrdElementNumber(scinrrd->getNrrd())));
              break;
            default:
              throw error_type();
          }

          // set some info on the axis as not all SCIRun prs check whether there is any
          // data and may crash if there is no label

          // Nrrd lib is C and needs a list with pointers to C-style strings
          // The following C++ code does this without the need of the need for
          // explicit dynamic memory allocation.

          std::vector<std::string> labels;
          labels.resize(nrrddim);
          const char *labelptr[NRRD_DIM_MAX];

          for (int p=0;p<nrrddim;p++)
          {
            std::ostringstream oss;
            oss << "dimension " << (p+1);
            labels[p] = oss.str();
            labelptr[p] = labels[p].c_str();
            // labelptr contains ptrs into labels and
            // the memory it points to will be destroyed with
            // the labels object. The const cast is needed as
            // nrrd
          }

          nrrdAxisInfoSet_nva(scinrrd->getNrrd(),nrrdAxisInfoLabel,labelptr);

          double spacing[NRRD_DIM_MAX];
          for (int p=0;p<NRRD_DIM_MAX;p++)
          {
            spacing[p] = 1.0;
          }

          nrrdAxisInfoSet_nva(scinrrd->getNrrd(),nrrdAxisInfoSpacing,spacing);

          double mindata[NRRD_DIM_MAX];
          for (int p=0;p<NRRD_DIM_MAX;p++)
          {
            mindata[p] = 0.0;
          }

          nrrdAxisInfoSet_nva(scinrrd->getNrrd(),nrrdAxisInfoMin,mindata);

          double maxdata[NRRD_DIM_MAX];
          for (int p=0;p<NRRD_DIM_MAX;p++)
          {
            if (p < static_cast<int>(dims.size())) maxdata[p] = static_cast<double>(dims[p]);
            else maxdata[p] = 1.0;
          }

          nrrdAxisInfoSet_nva(scinrrd->getNrrd(),nrrdAxisInfoMax,maxdata);

          int centerdata[NRRD_DIM_MAX];
          for (int p=0;p<NRRD_DIM_MAX;p++)
          {
            centerdata[p] = 2;
          }


          nrrdAxisInfoSet_nva(scinrrd->getNrrd(),nrrdAxisInfoCenter,centerdata);

          //scinrrd = scinrrd;
        }
        catch (...)
        {
          // in case something went wrong
          // release the datablock attached to
          // the nrrdhandle ... NRRD ....

          scinrrd.reset();
          throw;
        }
      }

      if (scinrrd != 0)
      {
        std::string str = mlarray.getname();
        scinrrd->set_filename(str);
      }

      break;
      // END CONVERSION OF MATLAB MATRIX

    case matlabarray::mlSTRUCT:
    case matlabarray::mlOBJECT:
      {
        int dataindex;
        dataindex = mlarray.getfieldnameindexCI("data");
        if (dataindex == -1) dataindex = mlarray.getfieldnameindexCI("potvals");
        if (dataindex == -1) dataindex = mlarray.getfieldnameindexCI("field");
        if (dataindex == -1) dataindex = mlarray.getfieldnameindexCI("scalarfield");
        if (dataindex == -1) dataindex = mlarray.getfieldnameindexCI("vectorfield");
        if (dataindex == -1) dataindex = mlarray.getfieldnameindexCI("tensorfield");


        // We need data to create an object
        // if no data field is found return
        // an error

        if (dataindex == -1)
        {
          throw error_type();
        }

        matlabarray subarray;
        subarray = mlarray.getfieldCI(0,"data");

        matlabarray::mlclass subclass;
        subclass = subarray.getclass();

        if (subclass != matlabarray::mlDENSE)
        {
          throw error_type();
        }

        mlArrayTOsciNrrdData(subarray,scinrrd);

        if (scinrrd == 0)
        {
          throw error_type();
        }

        // Add axes properties if they are specified

        int axisindex;
        axisindex = mlarray.getfieldnameindexCI("axis");

        if (axisindex != -1)
        {
          matlabarray::mlclass axisarrayclass;
          matlabarray axisarray;
          int numaxis;
          int fnindex;
          matlabarray farray;

          axisarray = mlarray.getfieldCI(0,"axis");

          if (!axisarray.isempty())
          {
            numaxis = axisarray.getm();
            axisarrayclass =axisarray.getclass();

            if ((axisarrayclass != matlabarray::mlSTRUCT)&&(axisarrayclass != matlabarray::mlOBJECT))
            {
              throw error_type();
            }


            // insert labels into nnrd
            // labels can be defined in axis(n).label

            fnindex = axisarray.getfieldnameindexCI("label");

            if (fnindex != -1)
            {
              std::vector<std::string> labels(NRRD_DIM_MAX);
              const char *clabels[NRRD_DIM_MAX];

              // Set some default values in case the
              // label is not defined by the matlabarray

              for (int p=0;p<NRRD_DIM_MAX;p++)
              {
                std::ostringstream oss;
                oss << "dimension " << (p+1);
                labels[p] = oss.str();
                clabels[p] = labels[p].c_str();
              }

              // In case the label is set in the matlabarray
              // add this label to the clabels array

              for (int p=0;p<numaxis;p++)
              {
                farray = axisarray.getfield(p,fnindex);
                if (farray.getclass() == matlabarray::mlSTRING)
                {
                  labels[p] = farray.getstring();
                  clabels[p] = labels[p].c_str();
                }
              }

              nrrdAxisInfoSet_nva(scinrrd->getNrrd(),nrrdAxisInfoLabel,clabels);
            }


            // insert unit names
            // into nrrd object

            fnindex = axisarray.getfieldnameindexCI("unit");
            if (fnindex != -1)
            {
              std::vector<std::string> units(NRRD_DIM_MAX);
              const char *cunits[NRRD_DIM_MAX];

              // Set some default values in case the
              // unit is not defined by the matlabarray

              for (int p=0;p<NRRD_DIM_MAX;p++)
                {
                  units[p] = "unknown";
                  cunits[p] = units[p].c_str();
                }

              // In case the unit is set in the matlabarray
              // add this unit to the clabels array

              for (int p=0;p<numaxis;p++)
                {
                  farray = axisarray.getfield(p,fnindex);
                  if (farray.getclass() == matlabarray::mlSTRING)
                    {
                      units[p] = farray.getstring();
                      cunits[p] = units[p].c_str();
                    }
                }

              nrrdAxisInfoSet_nva(scinrrd->getNrrd(),nrrdAxisInfoUnits,cunits);
            }

                // insert spacing information

                fnindex = axisarray.getfieldnameindexCI("spacing");
                if (fnindex != -1)
                  {
                    double spacing[NRRD_DIM_MAX];
                    std::vector<double> data(1);

                    // Set some default values in case the
                    // spacing is not defined by the matlabarray

                    for (int p=0;p<NRRD_DIM_MAX;p++)
                      {
                        spacing[p] = 1.0;
                      }

                    for (int p=0;p<numaxis;p++)
                      {
                        farray = axisarray.getfield(p,fnindex);
                        if ((farray.getclass() == matlabarray::mlDENSE)&&(farray.getnumelements() > 0))
                          {
                            farray.getnumericarray(data);
                            spacing[p] = data[0];
                          }
                      }

                    nrrdAxisInfoSet_nva(scinrrd->getNrrd(),nrrdAxisInfoSpacing,spacing);
                  }

                // insert minimum information

                fnindex = axisarray.getfieldnameindexCI("min");
                if (fnindex != -1)
                  {
                    double mindata[NRRD_DIM_MAX];
                    std::vector<double> data;

                    // Set some default values in case the
                    // minimum is not defined by the matlabarray

                    for (int p=0;p<NRRD_DIM_MAX;p++)
                      {
                        mindata[p] = AIR_NAN;
                      }

                    for (int p=0;p<numaxis;p++)
                      {
                        farray = axisarray.getfield(p,fnindex);
                        if ((farray.getclass() == matlabarray::mlDENSE)&&(farray.getnumelements() > 0))
                          {
                            farray.getnumericarray(data);
                            mindata[p] = data[0];
                          }
                      }

                    nrrdAxisInfoSet_nva(scinrrd->getNrrd(),nrrdAxisInfoMin,mindata);
                  }


                // insert maximum information

                fnindex = axisarray.getfieldnameindexCI("max");
                if (fnindex != -1)
                {
                  double maxdata[NRRD_DIM_MAX];
                  std::vector<double> data;

                  // Set some default values in case the
                  // maximum is not defined by the matlabarray

                  for (int p=0;p<NRRD_DIM_MAX;p++)
                  {
                    maxdata[p] = AIR_NAN;
                  }

                  for (int p=0;p<numaxis;p++)
                  {
                    farray = axisarray.getfield(p,fnindex);
                    if ((farray.getclass() == matlabarray::mlDENSE)&&(farray.getnumelements() > 0))
                    {
                      farray.getnumericarray(data);
                      maxdata[p] = data[0];
                    }
                  }

                  nrrdAxisInfoSet_nva(scinrrd->getNrrd(),nrrdAxisInfoMax,maxdata);
                }

                // insert centering information
                fnindex = axisarray.getfieldnameindexCI("center");
                if (fnindex != -1)
                {
                  int centerdata[NRRD_DIM_MAX];
                  std::vector<int> data;

                  // Set some default values in case the
                  // maximum is not defined by the matlabarray

                  for (int p=0;p<NRRD_DIM_MAX;p++)
                  {
                    centerdata[p] = 2;
                  }

                  for (int p=0;p<numaxis;p++)
                  {
                    farray = axisarray.getfield(p,fnindex);
                    if ((farray.getclass() == matlabarray::mlDENSE)&&(farray.getnumelements() > 0))
                    {
                      farray.getnumericarray(data);
                      centerdata[p] = data[0];
                    }
                  }

                  nrrdAxisInfoSet_nva(scinrrd->getNrrd(),nrrdAxisInfoCenter,centerdata);
                }
              }
        }

        int propertyindex;
        propertyindex = mlarray.getfieldnameindexCI("property");

        if (propertyindex != -1)
        {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
          mlPropertyTOsciProperty(mlarray,static_cast<PropertyManager *>(scinrrd.get_rep()));
#endif
        }

        if (mlarray.isfieldCI("name"))
        {
          if (scinrrd != 0)
            {
              matlabarray matname;
              matname = mlarray.getfieldCI(0,"name");
              std::string str = matname.getstring();
              if (matname.isstring()) scinrrd->set_filename(str);
            }

        }
        else
        {
          if (scinrrd != 0)
            {
              std::string str = mlarray.getname();
              scinrrd->set_filename(str);
            }
        }
      }

      break;

    default:
      {   // The program should not get here
        throw error_type();
      }
    }
}

void matlabconverter::sciNrrdDataTOmlMatrix(NrrdDataHandle scinrrd, matlabarray &mlarray)
{
  matlabarray::mitype dataformat = datatype_;

  mlarray.clear();

  // first determine the size of a nrrd
  std::vector<int> dims;
  const Nrrd *nrrdptr = scinrrd->getNrrd();

  if (!nrrdptr)
  {
    return;
  }

  // check if there is any data
  if (nrrdptr->dim == 0) return;

  dims.resize(nrrdptr->dim);

  int totsize = 1; // this one is used as an internal check and is handy to have
  for (int p=0; p<static_cast<int>(nrrdptr->dim);p++)
  {
    dims[p] = nrrdptr->axis[p].size;
    totsize *= dims[p];
  }

  // if there is no data leave the object empty
  if (totsize == 0) return;

  // we now have to determine the type of the Matlab array
  // It can be either the same as in the nrrd array or casted
  // to a more appropriate type
  // type will store the new Matlab array type

  if(dataformat == matlabarray::miSAMEASDATA) dataformat = convertnrrdtype(nrrdptr->type);

  // create the Matlab array structure
  mlarray.createdensearray(dims,dataformat);

  // having the correct pointer type will automatically invoke
  // the proper template function for casting and storing the data

  if (nrrdptr->data)
  {
    switch (nrrdptr->type)
    {
    case nrrdTypeDouble  : mlarray.setnumericarray(static_cast<double *>(nrrdptr->data),totsize,dataformat); break;
    case nrrdTypeFloat   : mlarray.setnumericarray(static_cast<float *>(nrrdptr->data),totsize,dataformat); break;
    case nrrdTypeChar    : mlarray.setnumericarray(static_cast<signed char *>(nrrdptr->data),totsize,dataformat); break;
    case nrrdTypeUChar   : mlarray.setnumericarray(static_cast<unsigned char *>(nrrdptr->data),totsize,dataformat); break;
    case nrrdTypeShort   : mlarray.setnumericarray(static_cast<signed short *>(nrrdptr->data),totsize,dataformat); break;
    case nrrdTypeUShort  : mlarray.setnumericarray(static_cast<unsigned short *>(nrrdptr->data),totsize,dataformat); break;
    case nrrdTypeInt     : mlarray.setnumericarray(static_cast<signed int *>(nrrdptr->data),totsize,dataformat); break;
    case nrrdTypeUInt    : mlarray.setnumericarray(static_cast<unsigned int *>(nrrdptr->data),totsize,dataformat); break;
    case nrrdTypeLLong   : mlarray.setnumericarray(static_cast<int64_t *>(nrrdptr->data),totsize,dataformat); break;
    case nrrdTypeULLong  : mlarray.setnumericarray(static_cast<uint64_t *>(nrrdptr->data),totsize,dataformat); break;
    }
  }
}

void matlabconverter::sciNrrdDataTOmlArray(NrrdDataHandle scinrrd, matlabarray &mlarray)
{
  if (numericarray_)
  {
    sciNrrdDataTOmlMatrix(scinrrd,mlarray);
    return;
  }

  matlabarray matrix;
  sciNrrdDataTOmlMatrix(scinrrd,matrix);

  mlarray.createstructarray();
  mlarray.setfield(0,"data",matrix);

  // Set the properties of the axis
  std::vector<std::string> axisfieldnames;
  axisfieldnames += "size", "spacing", "min", "max", "center", "label", "unit";

  Nrrd  *nrrdptr;
  nrrdptr = scinrrd->getNrrd();

  matlabarray axisma;
  std::vector<int> dims(2);
  dims[0] = nrrdptr->dim;
  dims[1] = 1;

  axisma.createstructarray(dims,axisfieldnames);

  for (int p=0; p<static_cast<int>(nrrdptr->dim); p++ )
  {
    matlabarray sizema;
    matlabarray spacingma;
    matlabarray minma;
    matlabarray maxma;
    matlabarray centerma;
    matlabarray labelma;
    matlabarray unitma;

    sizema.createdoublescalar(static_cast<double>(nrrdptr->axis[p].size));
    axisma.setfield(p,"size",sizema);

    spacingma.createdoublescalar(static_cast<double>(nrrdptr->axis[p].spacing));
    axisma.setfield(p,"spacing",spacingma);

    minma.createdoublescalar(static_cast<double>(nrrdptr->axis[p].min));
    axisma.setfield(p,"min",minma);

    maxma.createdoublescalar(static_cast<double>(nrrdptr->axis[p].max));
    axisma.setfield(p,"max",maxma);

    centerma.createdoublescalar(static_cast<double>(nrrdptr->axis[p].center));
    axisma.setfield(p,"center",centerma);

    if (nrrdptr->axis[p].label == 0)
    {
      std::ostringstream oss;
      oss << "axis " << p;
      labelma.createstringarray(oss.str());
    }
    else
    {
      labelma.createstringarray(nrrdptr->axis[p].label);
    }
    axisma.setfield(p,"label",labelma);

    if (nrrdptr->axis[p].units == 0)
    {
      unitma.createstringarray("no unit");
    }
    else
    {
      unitma.createstringarray(nrrdptr->axis[p].units);
    }
    axisma.setfield(p,"unit",unitma);

  }

  mlarray.setfield(0,"axis",axisma);
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  sciPropertyTOmlProperty(static_cast<PropertyManager *>(scinrrd.get_rep()),mlarray);
#endif
}

// Routine for discovering which kind of mesh is being supplied
// It reads the matlabarray and looks for certain fields to see whether
// it is convertible into a SCIRun mesh.
// Currently there are dozens of mesh and field types being used in
// SCIRun, each having a similar but different interface. This function
// only supports the most used ones:
//   PointCloudMesh
//   CurveMesh
//   TriSurfMesh
//   QuadSurfMesh
//   TetVolMesh
//   HexVolMesh
//   PrismVolMesh
//   StructCurveMesh
//   StructQuadSurfMesh
//   StructHexVolMesh
//   ScanlineMesh
//   ImageMesh
//   LatVolMesh
//   any suggestions for other types that need support ??

int matlabconverter::sciFieldCompatible(const matlabarray& mlarray,std::string &infostring, bool postremark)
{
  MatlabToFieldAlgo algo;
  algo.setreporter(pr_);
  return(algo.analyze_iscompatible(mlarray,infostring,postremark));
}

void matlabconverter::mlArrayTOsciField(const matlabarray& mlarray, FieldHandle &scifield)
{
  scifield.reset();

  MatlabToFieldAlgo algo;

  algo.setreporter(pr_);

  std::string fielddesc;
  try
  {
    algo.analyze_fieldtype(mlarray, fielddesc);
  }
  catch (error_type& e)
  {
    error("matlabconverter: Could not determine the output field type");
    throw;
  }

  if (!(algo.execute(scifield)))
  {
    // The algorithm has an builtin sanity check. If a specific converter cannot be built
    // it will create an algorithm that returns a false. Hence instead of failing at the
    // compiler level a proper description will be issued to the user
    error("mlArrayTOsciField: The dynamically compiled matlabconverter does not function properly; most probably some specific mesh or field converters are missing or have not yet been implemented\n");
    throw error_type();
  }

  if (!scifield)
  {
    error("mlArrayTOsciField: The dynamically compiled matlabconverter does not function properly\n");
    throw error_type();
  }

  if (mlarray.isstruct())
  {
    if (mlarray.isfieldCI("property"))
    {
      matlabarray mlproperty = mlarray.getfieldCI(0,"property");
      if (mlproperty.isstruct())
      {
        if (scifield)
        {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
          mlPropertyTOsciProperty(mlarray,static_cast<PropertyManager *>(scifield.get_rep()));
#endif
        }
      }
    }

    if (mlarray.isfieldCI("name"))
    {
      matlabarray mlname = mlarray.getfieldCI(0,"name");
      if (mlname.isstring())
      {
        if (scifield)
        {
          scifield->properties().set_property("name",mlname.getstring(),false);
        }
      }
    }
    else
    {
      if (scifield)
      {
        scifield->properties().set_property("name",mlarray.getname(),false);
      }
    }
  }
  else
  {
    if (scifield)
    {
      scifield->properties().set_property("name",mlarray.getname(),false);
    }
  }

  return;
}

void matlabconverter::sciFieldTOmlArray(FieldHandle scifield,matlabarray &mlarray)
{
  FieldToMatlabAlgo algo ;
  // Do the magic, internally algo will now refer to the proper dynamic class, which will be
  // loaded by this function as well

  algo.setreporter(pr_);

  // The function takes the matlabconverter pointer again, which we need to re-enter the object, which we will
  // leave in the dynamic compilation. The later was done to assure small filenames in the on-the-fly libs
  // Filenames over 31 chars will cause problems on certain systems
  // Since the matlabconverter holds all our converter settings, we don't want to lose it, hence it is added
  // here. The only disadvantage is that some of the functions in the matlabconverter class nedd to be public

  // create a new structured Matlab array
  mlarray.createstructarray();

  if(!(algo.execute(scifield,mlarray)))
  {
    // The algorithm has an builtin sanity check. If a specific converter cannot be built
    // it will create an algorithm that returns a false. Hence instead of failing at the
    // compiler level a proper description will be issued to the user of the pr
    error("The dynamically compiled matlabconverter does not function properly; most probably some specific mesh or field converters are missing or have not yet been implemented.");
    mlarray.clear();
    throw error_type();
  }

  if (mlarray.isempty())
  {
    // Apparently my sanity check did not work, we did not get a Matlab object
    error("Converter did not result in a useful translation, something went wrong, giving up.");
    throw error_type();
  }

  // This code is not the most efficient one: we first create the complete structured one and then
  // strip out one field. But it seems to be the easiest one.

  if (numericarray_)
  {
    if (mlarray.isfield("field"))
    {
      // strip the whole data structure
      mlarray = mlarray.getfield(0,"field");
      return; // leave since we are not dealing with a struct
    }
    else
    {
      error("There is no field data in Field.");
      throw error_type();
    }
  }

  // add the properties
  if (scifield)
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    sciPropertyTOmlProperty(static_cast<PropertyManager *>(scifield.get_rep()),mlarray);
#endif
  }

  // Parse the namefield separately
  if (scifield->properties().is_property("name"))
  {
    std::string name;
    matlabarray namearray;
    scifield->properties().get_property("name",name);
    namearray.createstringarray(name);
    mlarray.setfield(0,"name",namearray);
  }
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
int matlabconverter::sciBundleCompatible(matlabarray &mlarray, std::string &infostring, bool postremark)
{
  infostring = "";
  if (!mlarray.isstruct()) return(0);
  if (mlarray.getnumelements()==0) return(0);
  int nfields = mlarray.getnumfields();

  std::string dummyinfo;
  int numfields = 0;
  int nummatrices = 0;
  int numnrrds = 0;
  int numbundles = 0;
  int numstrings = 0;

  matlabarray subarray;
  for (int p = 0; p < nfields; p++)
  {
    subarray = mlarray.getfield(0,p);
    if (sciStringCompatible(subarray,dummyinfo,false)) { numstrings++; continue; }
    if (prefer_bundles)  {if (sciBundleCompatible(subarray,dummyinfo,false)) { numbundles++; continue; } }
    int score = sciFieldCompatible(subarray,dummyinfo,false);
    if (prefer_fields)
      if (score > 0)  { numfields++; continue; }
    else
      if (score > 1)  { numfields++; continue; }
    if (prefer_nrrds) { if (sciNrrdDataCompatible(subarray,dummyinfo,false))   { numnrrds++; continue; } }
    if (sciMatrixCompatible(subarray,dummyinfo,false)) { nummatrices++; continue; }
    if (!prefer_nrrds) { if (sciNrrdDataCompatible(subarray,dummyinfo,false))   { numnrrds++; continue; } }
    if (score) { numfields++; continue; }
    if (sciBundleCompatible(subarray,dummyinfo,false)) { numbundles++; continue; }
  }

  if (numfields+nummatrices+numnrrds+numbundles+numstrings == 0)
  {
    if (postremark) remark(std::string("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Bundle (none of the fields matches a SCIRun object)."));
    return(0);
  }

  std::ostringstream oss;
  oss << mlarray.getname() << " BUNDLE [ " << nummatrices << " MATRICES, " << numnrrds << " NRRDS, " << numfields << " FIELDS, " << numstrings << "STRINGS, "<< numbundles << " BUNDLES]";
  infostring = oss.str();
  return(1);
}

void matlabconverter::mlArrayTOsciBundle(matlabarray &mlarray,BundleHandle &scibundle)
{
  if (!mlarray.isstruct())
  {
    error("Matlab array is not a structured array: cannot translate it into a bundle.");
    throw error_type();
  }

  if (mlarray.getnumelements()==0)
  {
    error("Matlab array does not contain any fields.");
    throw error_type();
  }
  int numfields = mlarray.getnumfields();

  scibundle = new Bundle;
  if (scibundle.get_rep() == 0)
  {
    error("Could not allocate bundle (not enough memory).");
    throw error_type();
  }

  std::string dummyinfo;
  std::string fname;
  matlabarray subarray;

  // We do not want explanations why a field cannot be translated
  // We are testing here whether they can

  for (int p = 0; p < numfields; p++)
  {
    subarray = mlarray.getfield(0,p);
    fname = mlarray.getfieldname(p);

    //! STRINGS Can always be translated
    //! into strings
    if (subarray.isstring())
    {
      if (sciStringCompatible(subarray,dummyinfo,false))
      {
        StringHandle strhandle;
        mlArrayTOsciString(subarray,strhandle);
        scibundle->setString(fname,strhandle);
        continue;
      }
    }

    if (subarray.isstruct())
    {
      if (prefer_bundles == true)
      {
        if (sciBundleCompatible(subarray,dummyinfo,false))
        {
          BundleHandle subbundle;
          mlArrayTOsciBundle(subarray,subbundle);
          scibundle->setBundle(fname,subbundle);
          continue;
        }
      }
    }

    int score = sciFieldCompatible(subarray,dummyinfo,false);
    if (score > 1)
    {
      FieldHandle field;
      mlArrayTOsciField(subarray,field);
      scibundle->setField(fname,field);
      continue;
    }
    if (prefer_nrrds)
    {
      if (sciNrrdDataCompatible(subarray,dummyinfo,false))
      {
        NrrdDataHandle nrrd;
        mlArrayTOsciNrrdData(subarray,nrrd);
        scibundle->setNrrd(fname,nrrd);
        continue;
      }
    }
    if (sciMatrixCompatible(subarray,dummyinfo,false))
    {
      MatrixHandle  matrix;
      mlArrayTOsciMatrix(subarray,matrix);
      scibundle->setMatrix(fname,matrix);
      continue;
    }
    if (!prefer_nrrds)
    {
      if (sciNrrdDataCompatible(subarray,dummyinfo,false))
      {
        NrrdDataHandle nrrd;
        mlArrayTOsciNrrdData(subarray,nrrd);
        scibundle->setNrrd(fname,nrrd);
        continue;
      }
    }
    if (score)
    {
      FieldHandle field;
      mlArrayTOsciField(subarray,field);
      scibundle->setField(fname,field);
      continue;
    }
    if (sciBundleCompatible(subarray,dummyinfo,false))
    {
      BundleHandle subbundle;
      mlArrayTOsciBundle(subarray,subbundle);
      scibundle->setBundle(fname,subbundle);
      continue;
    }
  }

}

void matlabconverter::sciBundleTOmlArray(BundleHandle &scibundle, matlabarray &mlmat)
{
  int numhandles = scibundle->getNumHandles();
  LockingHandle<PropertyManager> handle;
  std::string name;
  Field* field = 0;
  Matrix<double>* matrix = 0;
  String* str = 0;
  NrrdData* nrrd = 0;
  Bundle* bundle = 0;

  mlmat.clear();
  mlmat.createstructarray();

  // This routine scans systematically whether certain
  // SCIRun objects are contained in the bundle and then
  // converts them into Matlab objects

  for (int p=0; p < numhandles; p++)
  {
    handle = scibundle->gethandle(p);
    name = scibundle->getHandleName(p);
    field = dynamic_cast<Field *>(handle.get_rep());
    if (field)
    {
      FieldHandle  fhandle = field;
      matlabarray subarray;
      bool numericarray = numericarray_;
      numericarray_ = false;
      sciFieldTOmlArray(fhandle,subarray);
      mlmat.setfield(0,name,subarray);
      numericarray_ = numericarray;
    }
    matrix = dynamic_cast<Matrix<double>*>(handle.get_rep());
    if (matrix)
    {
      MatrixHandle  fhandle = matrix;
      matlabarray subarray;
      sciMatrixTOmlArray(fhandle,subarray);
      mlmat.setfield(0,name,subarray);
    }
    str = dynamic_cast<String *>(handle.get_rep());
    if (str)
    {
      StringHandle fhandle = str;
      matlabarray subarray;
      sciStringTOmlArray(fhandle,subarray);
      mlmat.setfield(0,name,subarray);
    }
    nrrd = dynamic_cast<NrrdData *>(handle.get_rep());
    if (nrrd)
    {
      NrrdDataHandle  fhandle = nrrd;
      matlabarray subarray;
      sciNrrdDataTOmlArray(fhandle,subarray);
      mlmat.setfield(0,name,subarray);
    }
    bundle = dynamic_cast<Bundle *>(handle.get_rep());
    if (bundle)
    {
      BundleHandle fhandle = bundle;
      matlabarray subarray;
      sciBundleTOmlArray(fhandle,subarray);
      mlmat.setfield(0,name,subarray);
    }
  }
}
#endif

void matlabconverter::error(const std::string& error)
{
  if(pr_) pr_->error(error);
}

void matlabconverter::warning(const std::string& warning)
{
  if(pr_) pr_->warning(warning);
}

void matlabconverter::remark(const std::string& remark)
{
  if(pr_) pr_->remark(remark);
}

void matlabconverter::setdatatype(matlabarray::mitype dataformat)
{
  datatype_ = dataformat;
}

void matlabconverter::setindexbase(int indexbase)
{
  indexbase_ = indexbase;
}

void matlabconverter::converttonumericmatrix()
{
  numericarray_ = true;
}

void matlabconverter::converttostructmatrix()
{
  numericarray_ = false;
}

void matlabconverter::setdisabletranspose(bool disabletranspose)
{
  disable_transpose_ = disabletranspose;
}


void matlabconverter::preferfields()
{
  prefer_fields = true;
  prefer_nrrds = false;
}

void matlabconverter::prefernrrds()
{
  prefer_nrrds = true;
  prefer_fields = false;
}

void matlabconverter::prefermatrices()
{
  prefer_nrrds = false;
  prefer_fields = false;
}

void matlabconverter::preferbundles()
{
  prefer_bundles = true;
}

void matlabconverter::prefersciobjects()
{
  prefer_bundles = false;
}
