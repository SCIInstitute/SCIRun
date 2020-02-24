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


#include <sstream>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;

String::String(const std::string& s /* = "" */) : value_(s) {}

static Persistent* maker()
{
  return new String;
}

std::string String::type_name() const { return type_id_obj.type; }

PersistentTypeID String::type_id_obj("String", "Datatype", maker);

PersistentTypeID String::type_id_func() { return type_id_obj;  }
std::string String::dynamic_type_name() const { return type_id_func().type; }

#define STRING_VERSION 1

void String::io(Piostream& stream)
{
  /*int version=*/stream.begin_class("String", STRING_VERSION);

  // Do the base class first.
  PropertyManager().io(stream);

  stream.begin_cheap_delim();
  stream.io(value_);
  stream.end_cheap_delim();
  stream.end_class();
}
