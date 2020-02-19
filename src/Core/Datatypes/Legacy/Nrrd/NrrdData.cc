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
 *@file  NrrdData.cc
 *@brief Interface to Gordon's Nrrd package
 *
 *@author
 *   David Weinstein
 *   School of Computing
 *   University of Utah
 *@date  February 2001
 */

#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>
#include <Core/Persistent/Pstreams.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>
#include <Core/Utils/Legacy/Debug.h>
#include <Core/Thread/Mutex.h>

using namespace SCIRun;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Geometry;


Persistent *
NrrdData::maker()
{
  return new NrrdData;
}

static Mutex teem_library_lock("TEEM Library lock");

NrrdGuard::NrrdGuard()
{
  // Since Teem is not thread safe, this Guard protects Teem from being
  // called by multiple threads. It is however optional, is it reduces performance
  //if (sci_getenv("FORCE_TEEM_LOCK"))
    teem_library_lock.lock();
}

NrrdGuard::~NrrdGuard()
{
  //if (sci_getenv("FORCE_TEEM_LOCK"))
    teem_library_lock.unlock();
}

void
NrrdData::lock_teem()
{
  teem_library_lock.lock();
}

void
NrrdData::unlock_teem()
{
  teem_library_lock.unlock();
}

PersistentTypeID NrrdData::type_id("NrrdData", "PropertyManager", maker);


NrrdData::NrrdData() :
  nrrd_(nrrdNew()),
  write_nrrd_(true),
  embed_object_(false)
  //data_owner_(0)
{
  DEBUG_CONSTRUCTOR("NrrdData")
}


NrrdData::NrrdData(Nrrd *n) :
  nrrd_(n),
  write_nrrd_(true),
  embed_object_(false)
  //data_owner_(0)
{
  DEBUG_CONSTRUCTOR("NrrdData")
}

/*
NrrdData::NrrdData(LockingHandle<Datatype> data_owner) :
  nrrd_(nrrdNew()),
  write_nrrd_(true),
  embed_object_(false),
  data_owner_(data_owner)
{
  DEBUG_CONSTRUCTOR("NrrdData")
}
*/

NrrdData::NrrdData(const NrrdData &copy) :
  Datatype(copy),
  nrrd_(nrrdNew()),
  //data_owner_(0),
  nrrd_fname_(copy.nrrd_fname_)
{
  DEBUG_CONSTRUCTOR("NrrdData")
  nrrdCopy(nrrd_, copy.nrrd_);
}


NrrdData::~NrrdData()
{
  DEBUG_DESTRUCTOR("NrrdData")

//  if(!data_owner_)
//  {
    nrrdNuke(nrrd_);
//  }
//  else
//  {
//    nrrdNix(nrrd_);
//    data_owner_ = 0;
//  }
}


NrrdData*
NrrdData::clone() const
{
  return new NrrdData(*this);
}


// This would be much easier to check with a regular expression lib
// A valid label has the following format:
// type = one of the valid types (Scalar, Vector, Tensor)
// elem = [A-Za-z0-9\-]+:type
// (elem,?)+

bool
NrrdData::in_name_set(const std::string &s) const
{
  for (std::string::size_type i = 0; i < s.size(); i++)
  {
    if (!(isalnum(s[i]) || s[i] == '-' || s[i] == '_'))
    {
      return false;
    }
  }
  return true;
}


#define NRRDDATA_VERSION 6

//////////
// PIO for NrrdData objects
void NrrdData::io(Piostream& stream)
{
  // THREAD SAFETY: As TEEM is not thread safe lock down all of teem while
  // doing calls to the TEEM library
  NrrdData::lock_teem();

  int version =  stream.begin_class("NrrdData", NRRDDATA_VERSION);
  // Do the base class first.
  if (version > 2)
  {
    PropertyManager().io(stream);
  }

  // In version 4 and higher we denote by a bool whether the object
  // is embedded or not. In case it is it is handled by the new
  // reader and writer version that comes with version 4.
  if (version > 3) stream.io(embed_object_);

  if (stream.reading())
  {

    if ((version < 4)||(!embed_object_))
    {

      // Added a check against dumping a pointer without deallocation
      // memory.
      if (nrrd_)
      {   // make sure we free any existing Nrrd Data set
        //if (!data_owner_.get_rep())
        //{
          nrrdNuke(nrrd_);
        //}
        //else
        //{
        //  nrrdNix(nrrd_);
        //  data_owner_ = 0;
        //}
        // Make sure we put a zero pointer in the field. There is no nrrd
        nrrd_ = nrrdNew();
      }

      // This is the old code, which needs some update in the way
      // errors are reported
      // Functions have been added to supply a filename for external
      // nrrds

      // saved out filename basically indicates whether it will be
      // a .nrrd or .nhdr file because we need to attach the path
      // that was part of the stream filename
      Pio(stream, nrrd_fname_);

      // versions before 6 wrote out a full path so use that
      // for reading in the nrrd file.  version 6 and after should
      // be writing out a relative path so strip off the ./ and
      // prepend the path given from the .nd file
      if (version >= 6)
      {
        std::string path = stream.file_name;
        std::string::size_type e = path.find_last_of("/");
        if (e == std::string::npos) e = path.find_last_of("\\");
        if (e != std::string::npos) path = stream.file_name.substr(0,e+1);
        nrrd_fname_ = path + nrrd_fname_.substr(2,nrrd_fname_.length());
      }

      if (nrrdLoad(nrrd_, nrrd_fname_.c_str(), 0))
      {
        // Need to upgrade error reporting
        char *err = biffGet(NRRD);
        std::cerr << "Error reading nrrd " << nrrd_fname_ << ": " << err << std::endl;
        free(err);
        biffDone(NRRD);
        NrrdData::unlock_teem();
        return;
      }
    }
    else
    {   // Allow for raw embedded nrrds in SCIRun streams

      // Added a check against dumping a pointer without deallocation
      // memory.
      // Any existing data will be purged, so we do not have a memory leak

      if (nrrd_)
      {   // make sure we free any existing Nrrd Data set
        //if(!data_owner_.get_rep())
        //{
          nrrdNuke(nrrd_);
        //}
        //else
        //{
        //  nrrdNix(nrrd_);
        //  data_owner_ = 0;
        //}
      }

      // Create a new nrrd structure
      if (!(nrrd_ = nrrdNew()))
      {   // Needs to be replaced with proper exception code
        std::cerr << "Error allocating memory for nrrd" << std::endl;
      }

      stream.begin_cheap_delim();
      stream.io(nrrd_->type);  // the type of the nrrd

      // We dump the dimensions right at the start, so when reading
      // the data we can directly allocate the proper amount of memory

      stream.begin_cheap_delim();
      stream.io(nrrd_->dim);

      size_t nrrddims[NRRD_DIM_MAX]; // defined in nrrd.h
      for (unsigned int p = 0; p<nrrd_->dim; p++)
      {
        stream.io(nrrddims[p]);
      }
      stream.end_cheap_delim();

      // Allocate memory using the nrrd allocator
      // Need some error checking here

      // Need to upgade error reporting


      if(nrrdAlloc_nva(nrrd_,nrrd_->type,nrrd_->dim,nrrddims))
      {
        char *err = biffGet(NRRD);
        std::cerr << "Error reading nrrd: " << err << std::endl;
        free(err);
        biffDone(NRRD);
      }
      //data_owner_ = 0;

      stream.begin_cheap_delim();
      // Read the contents of the axis

      // Pio uses std::string and nrrd char*
      // These object are used as intermediates
      std::string label, unit;

      for (unsigned int q=0; q< nrrd_->dim; q++)
      {
        stream.begin_cheap_delim();
        stream.io(nrrd_->axis[q].size);
        stream.io(nrrd_->axis[q].spacing);
        stream.io(nrrd_->axis[q].min);
        stream.io(nrrd_->axis[q].max);
        stream.io(nrrd_->axis[q].center);
        stream.io(nrrd_->axis[q].kind);
        stream.io(label);
        stream.io(unit);
        // dupiclate the strings so they are not deallocated when label and
        // unit are destroyed. This uses the nrrd allocato for obtaining memory
        // for the strings, we should not mix malloc and new..

        // Need error checking here as well
        nrrd_->axis[q].label= airStrdup(label.c_str());
        nrrd_->axis[q].units= airStrdup(unit.c_str());
        stream.end_cheap_delim();
      }
      stream.end_cheap_delim();


      // Same construct as above for label and unit
      std::string content;
      stream.io(content);
      nrrd_->content = airStrdup(content.c_str());
      stream.io(nrrd_->blockSize);
      stream.io(nrrd_->oldMin);
      stream.io(nrrd_->oldMax);

      // Dummies for the moment until I figure out how to read
      // AirArrays
      int numcmts;
      int numkeys;

      // Place holders for extending the reader to the comment and
      // keyvalue pair arrays. Currently zeros are written to indicate
      // the length of the arrays
      stream.begin_cheap_delim();
      stream.io(numcmts);
      stream.end_cheap_delim();

      stream.begin_cheap_delim();
      stream.io(numkeys);
      stream.end_cheap_delim();

      stream.begin_cheap_delim();
      int size;
      stream.io(size);

      // Ugly but necessary:
      // big switch statement going over every type of the nrrd structure
      switch(nrrd_->type)
      {
      case nrrdTypeChar:
        {
          char *ptr = static_cast<char *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeUChar:
        {
          unsigned char *ptr = static_cast<unsigned char *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeShort:
        {
          short *ptr = static_cast<short *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeUShort:
        {
          unsigned short *ptr = static_cast<unsigned short *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeInt:
        {
          int *ptr = static_cast<int *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeUInt:
        {
          unsigned short *ptr = static_cast<unsigned short *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeLLong:
        {
          long long *ptr = static_cast<long long *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeULLong:
        {
          // Currently PIO does not support unsigned long long
          // Need to fix this bug in the Persistent.h
          long long *ptr = static_cast<long long *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeFloat:
        {
          float *ptr = static_cast<float *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeDouble:
        {
          double *ptr = static_cast<double *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeBlock:
        {
          char *ptr = static_cast<char *>(nrrd_->data);
          for (unsigned int p=0; p < (size*nrrd_->blockSize); p ++)
            stream.io(ptr[p]);
        }
        break;
      default:
        // We should not get here, but it outputs a statement in case
        // we reach this one due to some other bug elsewhere
        NrrdData::unlock_teem();
        THROW_INVALID_ARGUMENT("Unknown nrrd string type");
      }
      stream.end_cheap_delim();
      stream.end_cheap_delim();
    }
  }
  else
  { // writing

    // the nrrd file name will just append .nrrd

    if ((version < 4)||(!embed_object_))
    {

      std::string::size_type e = stream.file_name.rfind('.');
      //remove the .nd
      nrrd_fname_ = stream.file_name.substr(0, e);

      // figure out file to save out in .nd file (relative path)
      std::string full_filename = stream.file_name;
      e = full_filename.find_last_of("/");
      if (e == std::string::npos) e = full_filename.find_last_of("\\");

      std::string filename = full_filename;
      if (e != std::string::npos) filename = full_filename.substr(e+1,full_filename.length());

      e = filename.find(".");
      std::string root = std::string("./") + filename;

      if (e != std::string::npos)
      {
        root = std::string("./") + filename.substr(0,e);
      }

      if (write_nrrd_)
      {
        nrrd_fname_ += std::string(".nrrd");
        root += std::string (".nrrd");
      }
      else
      {
        nrrd_fname_ += std::string(".nhdr");
        root += std::string (".nhdr");
      }
      Pio(stream, root);

      NrrdIoState *no = 0;
      TextPiostream *text = dynamic_cast<TextPiostream*>(&stream);
      if (text)
      {
        no = nrrdIoStateNew();
        no->encoding = nrrdEncodingAscii;
      }
      if (nrrdSave(nrrd_fname_.c_str(), nrrd_, no))
      {
        char *err = biffGet(NRRD);
        std::cerr << "Error writing nrrd " << nrrd_fname_ << ": "<< err << std::endl;
        free(err);
        biffDone(NRRD);
        NrrdData::unlock_teem();
        return;
      }
      if (text) { nrrdIoStateNix(no); }
    }
    else
    {
      // Save the type of data
      stream.begin_cheap_delim();
      stream.io(nrrd_->type);

      // We dump the dimensions right at the start, so when reading
      // the data we can directly allocate the proper amount of memory

      stream.begin_cheap_delim();
      stream.io(nrrd_->dim);
      for (unsigned int q=0; q < nrrd_->dim; q++)
      {
        stream.io(nrrd_->axis[q].size);
      }

      stream.end_cheap_delim();
      // Save the contents of the axis

      stream.begin_cheap_delim();
      for (unsigned int q=0; q< nrrd_->dim; q++)
      {
        stream.begin_cheap_delim();
        stream.io(nrrd_->axis[q].size);
        stream.io(nrrd_->axis[q].spacing);
        stream.io(nrrd_->axis[q].min);
        stream.io(nrrd_->axis[q].max);
        stream.io(nrrd_->axis[q].center);
        stream.io(nrrd_->axis[q].kind);
        std::string label, unit;
        if ( nrrd_->axis[q].label) { label = nrrd_->axis[q].label; } else { label = ""; };
        if ( nrrd_->axis[q].units) { label = nrrd_->axis[q].units; } else { unit = ""; };
        stream.io(label);
        stream.io(unit);
        stream.end_cheap_delim();
      }
      stream.end_cheap_delim();

      if (nrrd_->content)
      {
        std::string content = nrrd_->content;
        stream.io(content);
      }
      else
      {
        std::string content = "";
        stream.io(content);
      }
      stream.io(nrrd_->blockSize);
      stream.io(nrrd_->oldMin);
      stream.io(nrrd_->oldMax);

      // Make entry point for comments and keyvalue pair
      // arrays

      int numcmts = 0;
      int numkeys = 0;

      stream.begin_cheap_delim();
      stream.io(numcmts);
      stream.end_cheap_delim();

      stream.begin_cheap_delim();
      stream.io(numkeys);
      stream.end_cheap_delim();

      // Figure out how many data bytes we have

      int dim = nrrd_->dim;
      int size = 1;
      for (int p = 0; p < dim ; p++)
      {
        size *= nrrd_->axis[p].size;
      }

      stream.begin_cheap_delim();
      stream.io(size);
      switch(nrrd_->type)
      {
      case nrrdTypeChar:
        {
          char *ptr = static_cast<char *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeUChar:
        {
          unsigned char *ptr = static_cast<unsigned char *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeShort:
        {
          short *ptr = static_cast<short *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeUShort:
        {
          unsigned short *ptr = static_cast<unsigned short *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeInt:
        {
          int *ptr = static_cast<int *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeUInt:
        {
          unsigned short *ptr = static_cast<unsigned short *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeLLong:
        {
          long long *ptr = static_cast<long long *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeULLong:
        {
          unsigned long long *ptr = static_cast<unsigned long long *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeFloat:
        {
          float *ptr = static_cast<float *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeDouble:
        {
          double *ptr = static_cast<double *>(nrrd_->data);
          for (int p=0; p <size; p ++) stream.io(ptr[p]);
        }
        break;
      case nrrdTypeBlock:
        {
          char *ptr = static_cast<char *>(nrrd_->data);
          for (unsigned int p=0; p < (size*nrrd_->blockSize); p ++)
            stream.io(ptr[p]);
        }
        break;
      default:
        NrrdData::unlock_teem();
        THROW_INVALID_ARGUMENT("Unknown nrrd string type");
      }
      stream.end_cheap_delim();
      stream.end_cheap_delim();
    }
  }
  if (version > 1 && version < 5)
  {
    // Somehow a statement got saved whether the nrrd owned the data
    // or not. Although it might not own the data while writing, when
    // creating a new object in case of reading the data, it always
    // will be ownED by that nrrd, who else would own it. Hence in the
    // new version it will write a dummy variable and as well read a
    // dummy. This dummy is set to one, so when older versions of
    // SCIRun read the data, they properly assume they own the data
    //
    // Bumped version number. version > 1 is backwards, should
    // probably have been version == 1.
    bool own_data = true;
    Pio(stream, own_data);
  }
  stream.end_class();

  // Unlock TEEM for other threads
  NrrdData::unlock_teem();
}


template <>
int SCIRun::get_nrrd_type<Tensor>()
{
  return nrrdTypeFloat;
}


template <>
int SCIRun::get_nrrd_type<char>()
{
  return nrrdTypeChar;
}


template <>
int SCIRun::get_nrrd_type<unsigned char>()
{
  return nrrdTypeUChar;
}


template <>
int SCIRun::get_nrrd_type<short>()
{
  return nrrdTypeShort;
}


template <>
int SCIRun::get_nrrd_type<unsigned short>()
{
  return nrrdTypeUShort;
}


template <>
int SCIRun::get_nrrd_type<int>()
{
  return nrrdTypeInt;
}


template <>
int SCIRun::get_nrrd_type<unsigned int>()
{
  return nrrdTypeUInt;
}


template <>
int SCIRun::get_nrrd_type<long long>()
{
  return nrrdTypeLLong;
}


template <>
int SCIRun::get_nrrd_type<unsigned long long>()
{
  return nrrdTypeULLong;
}


template <>
int SCIRun::get_nrrd_type<float>()
{
  return nrrdTypeFloat;
}


void SCIRun::get_nrrd_compile_type( const unsigned int type,
			    std::string & typeStr,
			    std::string & typeName )
{
  switch (type) {
  case nrrdTypeChar :
    typeStr = "char";
    typeName = "char";
    break;
  case nrrdTypeUChar :
    typeStr = "unsigned char";
    typeName = "unsigned_char";
    break;
  case nrrdTypeShort :
    typeStr = "short";
    typeName = "short";
    break;
  case nrrdTypeUShort :
    typeStr = "unsigned short";
    typeName = "unsigned_short";
    break;
  case nrrdTypeInt :
    typeStr = "int";
    typeName = "int";
    break;
  case nrrdTypeUInt :
    typeStr = "unsigned int";
    typeName = "unsigned_int";
    break;
  case nrrdTypeLLong :
    typeStr = "long long";
    typeName = "long_long";
    break;
  case nrrdTypeULLong :
    typeStr = "unsigned long long";
    typeName = "unsigned_long_long";
    break;
  case nrrdTypeFloat :
    typeStr = "float";
    typeName = "float";
    break;
  case nrrdTypeDouble :
    typeStr = "double";
    typeName = "double";
    break;
  default:
    typeStr = "float";
    typeName = "float";
  }
}

std::string
nrrd_type_to_string(unsigned int ntype)
{
  std::string result, dummy;
  get_nrrd_compile_type(ntype, result, dummy);
  return result;
}


int
string_to_nrrd_type(const std::string &str)
{
  if (str == "nrrdTypeChar")
    return nrrdTypeChar;
  else if (str == "nrrdTypeUChar")
    return nrrdTypeUChar;
  else if (str == "nrrdTypeShort")
    return nrrdTypeShort;
  else if (str == "nrrdTypeUShort")
    return nrrdTypeUShort;
  else if (str == "nrrdTypeInt")
    return nrrdTypeInt;
  else if (str == "nrrdTypeUInt")
    return nrrdTypeUInt;
  else if (str == "nrrdTypeLLong")
    return nrrdTypeLLong;
  else if (str == "nrrdTypeULLong")
    return nrrdTypeULLong;
  else if (str == "nrrdTypeFloat")
    return nrrdTypeFloat;
  else if (str == "nrrdTypeDouble")
    return nrrdTypeDouble;
  else
  {
    THROW_INVALID_ARGUMENT("Unknown nrrd string type");
    return nrrdTypeFloat;
  }
}


double SCIRun::get_nrrd_value( Nrrd* nrrd,
		       unsigned int p ) {

  switch(nrrd->type)
  {
  case nrrdTypeChar:
    {
      char *ptr = static_cast<char *>(nrrd->data);
      return (double) (ptr[p]);
    }
    break;
  case nrrdTypeUChar:
    {
      unsigned char *ptr = static_cast<unsigned char *>(nrrd->data);
      return (double) (ptr[p]);
    }
    break;
  case nrrdTypeShort:
    {
      short *ptr = static_cast<short *>(nrrd->data);
      return (double) (ptr[p]);
    }
    break;
  case nrrdTypeUShort:
    {
      unsigned short *ptr = static_cast<unsigned short *>(nrrd->data);
      return (double) (ptr[p]);
    }
    break;
  case nrrdTypeInt:
    {
      int *ptr = static_cast<int *>(nrrd->data);
      return (double) (ptr[p]);
    }
    break;
  case nrrdTypeUInt:
    {
      unsigned short *ptr = static_cast<unsigned short *>(nrrd->data);
      return (double) (ptr[p]);
    }
    break;
  case nrrdTypeLLong:
    {
      long long *ptr = static_cast<long long *>(nrrd->data);
      THROW_INVALID_ARGUMENT("Long long nrrd is out of range for a double");
      return (double) (ptr[p]);
    }
    break;
  case nrrdTypeULLong:
    {
      unsigned long long *ptr = static_cast<unsigned long long *>(nrrd->data);
      THROW_INVALID_ARGUMENT("Unsigned long long nrrd is out of range for a double");
      return (double) (ptr[p]);
    }
    break;
  case nrrdTypeFloat:
    {
      float *ptr = static_cast<float *>(nrrd->data);
      return (double) (ptr[p]);
    }
    break;
  case nrrdTypeDouble:
    {
      double *ptr = static_cast<double *>(nrrd->data);
      return (double) (ptr[p]);
    }
    break;
  default:
    THROW_INVALID_ARGUMENT("Unknown nrrd string type");
  }
}
