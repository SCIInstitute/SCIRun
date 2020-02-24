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


///
///@file  String.h
///@brief String Object
///
///@author
///       Jeroen Stinstra
///       Department of Computer Science
///       University of Utah
///@date  October 2005
///

#ifndef CORE_DATATYPES_STRING_H
#define CORE_DATATYPES_STRING_H 1

#include <Core/Containers/LockingHandle.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/PropertyManager.h>


#include <string>


#include <Core/Datatypes/share.h>

namespace SCIRun {

class String;
typedef LockingHandle<String> StringHandle;

class SCISHARE String : public PropertyManager {

  std::string str_;

public:
  /// Constructors
  String();
  explicit String(const std::string& str);
  String(const String& str);
  explicit String(const char* str);

  /// Destructor
  virtual ~String();

  /// Public member functions
  String* clone();
  inline void        set(const std::string& str);
  inline std::string get();

  /// Persistent representation...
  virtual void io(Piostream&);
  static PersistentTypeID type_id;
  virtual std::string dynamic_type_name() const { return type_id.type; }
};

inline void String::set(const std::string& str)
{
  str_ = str;
}

inline std::string String::get()
{
  return str_;
}

} // End namespace SCIRun

#endif
