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

/*
 *@file  NrrdData.h
 *@brief Interface to Gordon's Nrrd class
 *
 *@author
 *   David Weinstein
 *   School of Computing
 *   University of Utah
 *@date  February 2001
 */

#if !defined(SCI_Teem_NrrdData_h)
#define SCI_Teem_NrrdData_h

#include <Core/Thread/Guard.h>

#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Field.h>
#include <Core/Containers/LockingHandle.h>
#include <teem/nrrd.h>

#include <Core/Datatypes/share.h>

namespace SCIRun {

/////////
// Structure to hold NrrdData
class SCISHARE NrrdData : public PropertyManager {
public:  
  // GROUP: public data
  //////////
  // 
  Nrrd *nrrd_;

  NrrdData();
  NrrdData(Nrrd *nrrd);
  NrrdData(LockingHandle<Datatype> data_owner);
  NrrdData(const NrrdData&);
  virtual ~NrrdData();

  virtual NrrdData* clone();

  virtual void io(Piostream&);
  static PersistentTypeID type_id;
  virtual std::string dynamic_type_name() const { return type_id.type; }

  // Separate raw files.
  void set_embed_object(bool v) { embed_object_ = v; }
  bool get_embed_object() { return embed_object_; }

  void set_filename( const std::string &f )
  { nrrd_fname_ = f; embed_object_ = false; }
  const std::string get_filename() const { return nrrd_fname_; }

  bool    write_nrrd_;

  // As parts of TEEM are by design not thread safe we need to enforce thread-safety
  // on certain functionality by forcing it to run single threaded.
  // The main issue is that error reporting uses global variables and internally
  // global variables are used.
  
  static void lock_teem();
  static void unlock_teem();

protected:
  bool    embed_object_;

  bool in_name_set(const std::string &s) const;

  /// Either the NrrdData owns the data or it wraps this external object.
  LockingHandle<Datatype> data_owner_;

  // To help with pio
  std::string                nrrd_fname_;

  static Persistent *maker();
};


typedef LockingHandle<NrrdData> NrrdDataHandle;


// nrrd Types that we need to convert to:
//  nrrdTypeChar,          
//  nrrdTypeUChar,         
//  nrrdTypeShort,         
//  nrrdTypeUShort,        
//  nrrdTypeInt,           
//  nrrdTypeUInt,          
//  nrrdTypeLLong,         
//  nrrdTypeULLong,        
//  nrrdTypeFloat,         
//  nrrdTypeDouble,


SCISHARE int string_to_nrrd_type(const std::string &str);
SCISHARE std::string nrrd_type_to_string(unsigned int ntype);

template <class T>
int get_nrrd_type();

template <>
SCISHARE int get_nrrd_type<char>();

template <>
SCISHARE int get_nrrd_type<unsigned char>();

template <>
SCISHARE int get_nrrd_type<short>();

template <>
SCISHARE int get_nrrd_type<unsigned short>();

template <>
SCISHARE int get_nrrd_type<int>();

template <>
SCISHARE int get_nrrd_type<unsigned int>();

template <>
SCISHARE int get_nrrd_type<long long>();

template <>
SCISHARE int get_nrrd_type<unsigned long long>();

template <>
SCISHARE int get_nrrd_type<float>();

template <>
SCISHARE int get_nrrd_type<Tensor>();

template <class T>
int get_nrrd_type()
{
  return nrrdTypeDouble;
}

SCISHARE void get_nrrd_compile_type( const unsigned int type,
				     std::string & typeStr,
				     std::string & typeName );

SCISHARE double get_nrrd_value( Nrrd* nrrd,
				unsigned int p );


class SCISHARE NrrdGuard {
  public:
    // Constructor
    NrrdGuard();
    
    // Destructor
    ~NrrdGuard(); 
};


} // end namespace SCIRun

#endif // SCI_Teem_NrrdData_h
