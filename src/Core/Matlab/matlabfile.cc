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
* FILE: matlabfile.h
* AUTH: Jeroen G Stinstra
* DATE: 16 MAY 2005
*/

#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>
#include <iostream>

using namespace SCIRun::MatlabIO;

matlabfile::matlabfile()
{ }

matlabfile::matlabfile(const std::string& filename,const std::string& accessmode)
{
  open(filename,accessmode);
}

matlabfile::~matlabfile()
{ }

void matlabfile::open(const std::string& filename,const std::string& accessmode)
{
  matfile::open(filename,accessmode);

  if (isreadaccess())
  {   // scan the file for the number of matrices
    // This function will index the file and get all the matrix names
    // If it is a compressed file, it will automatically uncompress every
    // block (this behavior may be changed to become more memory efficient)

    int tagptr;
    matfiledata mfd;
    std::stack<int> ptrstack;
    std::stack<std::string> strstack;
    bool compressedmatrix = false;

    tagptr = firsttag();
    while(tagptr)
    {
      readtag(mfd);

      // Bug fix, this bool was not reset
      compressedmatrix = false;
      // If the tag tells that the next block is compressed
      // Open this compressed block before continuing
      if (mfd.type() == miCOMPRESSED)
      {
        compressedmatrix = true; // to mark that we have to close the compressed session
        opencompression(); // uncompress the data
        readtag(mfd); // read the first tag, which should be miMATRIX
      }
      if (mfd.type() != miMATRIX) throw invalid_file_format();

      openchild();
      readtag(mfd);
      nexttag();
      readtag(mfd);
      nexttag();
      readdat(mfd);
      closechild();
      if (compressedmatrix) closecompression();

      strstack.push(mfd.getstring());
      ptrstack.push(tagptr);
      tagptr = nexttag();
    }

    matrixaddress_.resize(ptrstack.size());
    matrixname_.resize(strstack.size());
    for (int p=static_cast<int>(ptrstack.size()-1);p>=0;p--)
    {
      matrixaddress_[p] = ptrstack.top();
      ptrstack.pop();
      matrixname_[p] = strstack.top();
      strstack.pop();
    }

    rewind();
  }
}


void matlabfile::close()
{
  matfile::close();
}

matlabfile::mitype matlabfile::converttype(matlabfile::mxtype type)
{  // a simple conversion function
  switch (type)
  {    // no break statements needed
  case mxUINT8: case mxCHAR: return(miUINT8);
  case mxINT8: return(miINT8);
  case mxUINT16: return(miUINT16);
  case mxINT16: return(miINT16);
  case mxUINT32: return(miUINT32);
  case mxINT32: return(miINT32);
  case mxSINGLE: return(miSINGLE);
  case mxDOUBLE: case mxSPARSE: return(miDOUBLE);
  case mxCELL: case mxSTRUCT: case mxOBJECT: return(miMATRIX);
  default: return(miUNKNOWN);
  }
}

matlabfile::mxtype matlabfile::convertclass(matlabfile::mlclass mclass,matlabfile::mitype type)
{
  switch(mclass)
  {
  case mlDENSE:
    switch(type)
    {
    case miINT8: return(mxINT8);
    case miUINT8: return(mxUINT8);
    case miINT16: return(mxINT16);
    case miUINT16: return(mxUINT16);
    case miINT32: return(mxINT32);
    case miUINT32: return(mxUINT32);
    case miSINGLE: return(mxSINGLE);
    case miUINT64: case miINT64:		// there is no mxclass for 64bit datat yet
      // hence the easiest is to convert them to double
    case miDOUBLE: return(mxDOUBLE);
    default: return(mxUNKNOWN);
    }
  case mlSPARSE:
    return(mxSPARSE);
  case mlOBJECT:
    return(mxOBJECT);
  case mlCELL:
    return(mxCELL);
  case mlSTRUCT:
    return(mxSTRUCT);
  case mlSTRING:
    return(mxCHAR);
  default:
    return(mxUNKNOWN);
  }
}



void matlabfile::exportmatlabarray(matlabarray &matrix)
{
  matfiledata matrixheader;
  matfiledata matrixclass;
  matfiledata matrixdims;
  matfiledata matrixname;

  matrixheader.setType(miMATRIX);
  writedat(matrixheader);

  std::vector<unsigned int> classinfo(2);

  if (matrix.isempty())
  {   // in case of an empty matrix goto the next one

    // Write an empty matrix if no data was given
    // nexttag();
    mxtype   m =mxDOUBLE;
    classinfo[0] = m;
    classinfo[1] = 0;

    matrixclass.putandcastvector(classinfo,miUINT32);
    std::vector<int> mdims(2);
    mdims[0] = 0; mdims[1] = 0;
    matrixdims.putandcastvector(mdims,miINT32);
    matrixname.putstring("");

    openchild();
    writedat(matrixclass);
    nexttag();
    writedat(matrixdims);
    nexttag();
    writedat(matrixname);
    nexttag();

    matfiledata mpreal;
    writedat(mpreal);
    closechild();
    return;


  }

  mxtype   mtype = convertclass(matrix.getclass(),matrix.gettype());

  classinfo[0] |= mtype;
  if (matrix.islogical()) classinfo[0] |= 0x0200;
  if (matrix.isglobal()) classinfo[0]  |= 0x0400;
  if (matrix.iscomplex()) classinfo[0] |= 0x0800;

  if (mtype == mxSPARSE) classinfo[1] = matrix.getnnz();

  // Create the different tags and data fields

  matrixheader.setType(miMATRIX);
  matrixclass.putandcastvector(classinfo,miUINT32);
  std::vector<int> mdims = matrix.getdims();
  matrixdims.putandcastvector(mdims,miINT32);
  matrixname.putstring(matrix.getname());

  openchild();
  writedat(matrixclass);
  nexttag();
  writedat(matrixdims);
  nexttag();
  writedat(matrixname);
  nexttag();

  switch(mtype)
  {
    // numeric types
  case mxINT8: case mxUINT8: case mxINT16: case mxUINT16:
  case mxINT32: case mxUINT32: case mxSINGLE: case mxDOUBLE:
    {
      matfiledata preal;
      preal = matrix.getpreal();

      writedat(preal);
      if (matrix.iscomplex())
      {
        matfiledata pimag = matrix.getpimag();
        nexttag();
        writedat(pimag);
      }
    }
    break;
  case mxSPARSE:
    {
      matfiledata prows = matrix.getprows();
      matfiledata pcols = matrix.getpcols();
      matfiledata preal = matrix.getpreal();

      writedat(prows);
      nexttag();
      writedat(pcols);
      nexttag();
      writedat(preal);
      if (matrix.iscomplex())
      {
        matfiledata pimag = matrix.getpimag();
        nexttag();
        writedat(pimag);
      }
    }
    break;

  case mxCELL:
    {
      for (int p=0;p<matrix.getnumelements();p++)
      {
        matlabarray ma;
        ma = matrix.getcell(p);
        exportmatlabarray(ma);
      }
    }
    break;

  case mxSTRUCT:
    {
      int numel = matrix.getnumelements()*matrix.getnumfields();

      std::vector<int>  fieldnamelen(1);
      matfiledata matrixfieldnamelength;
      matfiledata matrixfieldnames;

      fieldnamelen[0] = matrixfieldnames.putstringarray(matrix.getfieldnames());
      matrixfieldnamelength.putandcastvector(fieldnamelen,miINT32);

      writedat(matrixfieldnamelength);
      nexttag();
      writedat(matrixfieldnames);
      nexttag();

      for (int p=0;p<numel;p++)
      {
        matlabarray ma;
        ma = matrix.getcell(p);
        exportmatlabarray(ma);
      }
    }
    break;

  case mxOBJECT:
    {
      int numel = matrix.getnumelements()*matrix.getnumfields();

      std::vector<int>  fieldnamelen(1);
      matfiledata matrixfieldnamelength;
      matfiledata matrixfieldnames;
      matfiledata matrixclassname;

      fieldnamelen[0] = matrixfieldnames.putstringarray(matrix.getfieldnames());
      matrixfieldnamelength.putandcastvector(fieldnamelen,miINT32);
      matrixclassname.putstring(matrix.getclassname());

      writedat(matrixclassname);
      nexttag();
      writedat(matrixfieldnamelength);
      nexttag();
      writedat(matrixfieldnames);
      nexttag();

      for (int p=0;p<numel;p++)
      {
        matlabarray ma;
        ma = matrix.getcell(p);
        exportmatlabarray(ma);
      }
    }
    break;

  case mxCHAR:
    {
      matfiledata matrixstring;
      std::string str = matrix.getstring();
      matrixstring.putandcast(str.c_str(),static_cast<int>(str.size()),miUINT16);
      writedat(matrixstring);
    }
    break;

  default:
    break;

  }
  closechild();
  nexttag();

}


// **************************************************************
// This function is the main function for loading a matrix.

void matlabfile::importmatlabarray(matlabarray& matrix,int mode)
{

  // make sure the matrix is cleared
  matrix.clear();
  bool compressedmatrix = false;

  matfiledata matrixheader;
  matfiledata matrixclass;
  matfiledata matrixdims;
  matfiledata matrixname;

  // Read the main matrix header
  // If this is not a matrix header the file pointer
  // is not pointing to a matrix
  readtag(matrixheader);
  if (matrixheader.type() == miCOMPRESSED)
  {
    // A compressed matrix is encapsulated in another
    // data header. opencompressed will now open and
    // decompress part of the file if needed.
    compressedmatrix = true; // mark that we still need to close the compressed memory block
    opencompression();
    readtag(matrixheader); // This tag should now contain a miMATRIX tag
  }

  if (matrixheader.type() != miMATRIX) return;

  if (!openchild()) return;

  if (!firsttag()) { closechild(); return; }
  readdat(matrixclass);
  if (!nexttag()) { closechild(); return; }
  readdat(matrixdims);
  if (!nexttag()) { closechild(); return; }
  readdat(matrixname);

  // convert the matrix class information
  // only the lowest 8bits are used for the class identifier
  // the rest is used for future expansions

  // first element in this datasegment denotes the matrix class

  if (matrixclass.size() == 0) { closechild(); return; }
  unsigned int classinfo =  matrixclass.getandcastvalue<unsigned int>(0);
  mxtype matrixtype = static_cast<mxtype>((0x000000FF & classinfo));


  std::vector<int> dims;
  matrixdims.getandcastvector(dims);
  std::string name = matrixname.getstring();

  int numelems = 1;
  for (size_t p = 0; p < dims.size(); p ++) numelems *= dims[p];

  // All matrices contain the data pieces read so far
  // From here on it depends on the matrix class

  switch(matrixtype)
  {
    // numeric types
  case mxINT8: case mxUINT8: case mxINT16: case mxUINT16:
  case mxINT32: case mxUINT32: case mxSINGLE: case mxDOUBLE:

    {
      // create the numeric array
      matrix.createdensearray(dims,converttype(matrixtype));
      matrix.setname(name);

      // read the real and imaginary (optional) parts of the data
      if (nexttag())
      { matfiledata preal = matrix.getpreal(); if ((mode == 2)||(numelems < 10)) { readdat(preal); } else { readtag(preal); } }
      if (nexttag())
      { matfiledata pimag = matrix.getpimag(); if ((mode == 2)||(numelems < 10)) { readdat(pimag); } else { readtag(pimag); } }
    }
    break;

  case mxCELL:
    {
      // create cell matrix
      matrix.createcellarray(dims);
      matrix.setname(name);
      // the second byte contains flags.
      // logical = logical matrix
      // global  = variable was defined in the global workspace
      // complex = matrix complex
      if (classinfo & 0x0200) matrix.setlogical(1);
      if (classinfo & 0x0400) matrix.setglobal(1);
      if (classinfo & 0x0800) matrix.setcomplex(1);

      int numcells = matrix.getnumelements();
      matlabarray submatrix;

      if (mode)
      {
        for (int p=0;p<numcells;p++)
        {
          if(!nexttag()) break; // no more matrices
          importmatlabarray(submatrix,mode);
          matrix.setcell(p,submatrix);
        }
      }
    }

    break;
  case mxSTRUCT:

    {
      matfiledata matrixfieldnamelength;
      matfiledata matrixfieldnames;

      nexttag();
      readdat(matrixfieldnamelength);
      nexttag();
      readdat(matrixfieldnames);

      int fieldnamelength = matrixfieldnamelength.getandcastvalue<int>(0);
      std::vector<std::string> fieldnames = matrixfieldnames.getstringarray(fieldnamelength);

      matrix.createstructarray(dims,fieldnames);
      matrix.setname(name);
      // the second byte contains flags.
      // logical = logical matrix
      // global  = variable was defined in the global workspace
      // complex = matrix complex
      if (classinfo & 0x0200) matrix.setlogical(1);
      if (classinfo & 0x0400) matrix.setglobal(1);
      if (classinfo & 0x0800) matrix.setcomplex(1);

      int numcells = matrix.getnumelements()*matrix.getnumfields();
      matlabarray submatrix;

      if (mode)
      {
        for (int p=0;p<numcells;p++)
        {
          if(!nexttag()) break; // nomore matrices
          importmatlabarray(submatrix,mode);
          matrix.setcell(p,submatrix);
        }
      }
    }
    break;

  case mxOBJECT:
    {
      matfiledata matrixfieldnamelength;
      matfiledata matrixfieldnames;
      matfiledata matrixclassname;

      nexttag();
      readdat(matrixclassname);
      nexttag();
      readdat(matrixfieldnamelength);
      nexttag();
      readdat(matrixfieldnames);

      int fieldnamelength = matrixfieldnamelength.getandcastvalue<int>(0);
      std::vector<std::string> fieldnames = matrixfieldnames.getstringarray(fieldnamelength);
      std::string classname = matrixclassname.getstring();

      matrix.createclassarray(dims,fieldnames,classname);
      matrix.setname(name);

      // the second byte contains flags.
      // logical = logical matrix
      // global  = variable was defined in the global workspace
      // complex = matrix complex
      if (classinfo & 0x0200) matrix.setlogical(1);
      if (classinfo & 0x0400) matrix.setglobal(1);
      if (classinfo & 0x0800) matrix.setcomplex(1);

      int numcells = matrix.getnumelements()*matrix.getnumfields();
      matlabarray submatrix;

      if (mode)
      {
        for (int p=0;p<numcells;p++)
        {
          if(!nexttag()) break; // nomore matrices
          importmatlabarray(submatrix,mode);
          matrix.setcell(p,submatrix);
        }
      }
    }
    break;
  case mxCHAR:
    {
      matfiledata matrixstring;

      nexttag();
      readdat(matrixstring);

      std::string str = matrixstring.getstring();
      matrix.createstringarray(str);
      matrix.setname(name);
      // the second byte contains flags.
      // logical = logical matrix
      // global  = variable was defined in the global workspace
      // complex = matrix complex
      if (classinfo & 0x0200) matrix.setlogical(1);
      if (classinfo & 0x0400) matrix.setglobal(1);
      if (classinfo & 0x0800) matrix.setcomplex(1);


    }
    break;
  case mxSPARSE:
    {
      matrix.createsparsearray(dims,miDOUBLE);
      matrix.setname(name);
      // the second byte contains flags.
      // logical = logical matrix
      // global  = variable was defined in the global workspace
      // complex = matrix complex
      if (classinfo & 0x0200) matrix.setlogical(1);
      if (classinfo & 0x0400) matrix.setglobal(1);
      if (classinfo & 0x0800) matrix.setcomplex(1);

      if (nexttag())
      {
        matfiledata rowindex = matrix.getprows(); if (mode==2) { readdat(rowindex); } else { readtag(rowindex); }
      }
      if (nexttag())
      {
        matfiledata colindex = matrix.getpcols(); if (mode==2) { readdat(colindex); } else { readtag(colindex); }
      }
      if (nexttag())
      {
        matfiledata preal = matrix.getpreal(); if (mode==2) { readdat(preal); } else { readtag(preal); }
        matrix.settype(preal.type());
      }
      if (nexttag())
      {
        matfiledata pimag = matrix.getpimag(); if (mode==2) { readdat(pimag); } else { readtag(pimag); }
      }
    }
    break;
  default:
    break;
  }

  closechild();
  if (compressedmatrix) closecompression();

}

// ***********************************************
// access functions to the matrices in the matfile

int matlabfile::getnummatlabarrays()
{
  return(static_cast<int>(matrixaddress_.size()));
}


matlabarray matlabfile::getmatlabarrayshortinfo(int matrixindex)
{
  if (iswriteaccess()) throw invalid_file_access();
  if (matrixindex >= static_cast<int>(matrixaddress_.size())) throw out_of_range();

  matlabarray ma;
  rewind();
  if (!gototag(matrixaddress_[matrixindex]))
  {
    std::cerr << "internal error in getmatlabarrayshortinfo()\n";
    throw internal_error();
  }
  importmatlabarray(ma,0);
  return(ma);
}


matlabarray matlabfile::getmatlabarrayshortinfo(const std::string& matrixname)
{
  if (iswriteaccess()) throw invalid_file_access();
  int matrixindex = -1;
  for (int p=0;p<static_cast<int>(matrixname_.size());p++)
  {
    if (matrixname_[p] == matrixname) { matrixindex = p; break; }
  }
  if (matrixindex == -1) throw out_of_range();

  matlabarray ma;
  rewind();

  if (!gototag(matrixaddress_[matrixindex]))
  {
    std::cerr << "internal error in getmatlabarrayshortinfo()\n";
    throw internal_error();
  }
  importmatlabarray(ma,0);
  return(ma);
}

matlabarray matlabfile::getmatlabarrayinfo(int matrixindex)
{
  if (iswriteaccess()) throw invalid_file_access();
  if (matrixindex >= static_cast<int>(matrixaddress_.size())) throw out_of_range();

  matlabarray ma;
  rewind();
  if (!gototag(matrixaddress_[matrixindex]))
  {
    std::cerr << "internal error in getmatlabarrayinfo()\n";
    throw internal_error();
  }
  importmatlabarray(ma,1);
  return(ma);
}


matlabarray matlabfile::getmatlabarrayinfo(const std::string& matrixname)
{
  if (iswriteaccess()) throw invalid_file_access();
  int matrixindex = -1;
  for (int p=0;p<static_cast<int>(matrixname_.size());p++)
  {
    if (matrixname_[p] == matrixname) { matrixindex = p; break; }
  }
  if (matrixindex == -1) throw out_of_range();

  matlabarray ma;
  rewind();

  if (!gototag(matrixaddress_[matrixindex]))
  {
    std::cerr << "internal error in getmatlabarrayinfo()\n";
    throw internal_error();
  }
  importmatlabarray(ma,1);
  return(ma);
}

matlabarray matlabfile::getmatlabarray(int matrixindex)
{
  if (iswriteaccess()) throw invalid_file_access();
  if (matrixindex >= static_cast<int>(matrixaddress_.size())) throw out_of_range();

  matlabarray ma;
  rewind();
  if (!gototag(matrixaddress_[matrixindex]))
  {
    std::cerr << "internal error in getmatlabarray()\n";
    throw internal_error();
  }
  importmatlabarray(ma,2);
  return(ma);
}

matlabarray matlabfile::getmatlabarray(const std::string& matrixname)
{
  if (iswriteaccess()) throw invalid_file_access();
  int matrixindex = -1;
  for (int p=0;p<static_cast<int>(matrixname_.size());p++)
  {
    if (matrixname_[p] == matrixname) { matrixindex = p; break; }
  }
  if (matrixindex == -1) throw out_of_range();

  matlabarray ma;
  rewind();

  if (!gototag(matrixaddress_[matrixindex]))
  {
    std::cerr << "internal error in getmatlabarray()\n";
    throw internal_error();
  }
  importmatlabarray(ma,2);
  return(ma);
}

void matlabfile::putmatlabarray(matlabarray& ma,const std::string& matrixname)
{
  ma.setname(matrixname);
  exportmatlabarray(ma);
}

matlabarray SCIRun::MatlabIO::readmatlabarray(matlabfile& mfile, const std::string& matlabName)
{
  matlabarray marray;

  if (matlabName.empty())
  {
    // return an empty array
    return(marray);
  }

  if (matlabName == "<none>")
  {
    // return an empty array
    return(marray);
  }

  return mfile.getmatlabarray(matlabName);
}
