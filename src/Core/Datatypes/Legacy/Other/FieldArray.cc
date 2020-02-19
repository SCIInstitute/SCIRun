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


/// @todo Documentation Core/Datatypes/Legacy/Other/FieldArray.cc

#include <Core/Datatypes/FieldArray.h>

namespace SCIRun {

FieldArray::FieldArray()
{
}

FieldArray::~FieldArray()
{
}

FieldArray*
FieldArray::clone() const
{
  FieldArray* fa = new FieldArray;
  fa->array() = array();
  return fa;
}

const int FIELD_ARRAY_VERSION = 1;

void
FieldArray::io(Piostream& stream)
{
  /*int version =*/ stream.begin_class("FieldArray",FIELD_ARRAY_VERSION);
  PropertyManager::io(stream);

  if (stream.reading())
  {
    long long nfields = 0;
    stream.io(nfields);
    array_.resize(static_cast<size_t>(nfields));

    stream.begin_cheap_delim();

    for (size_type j=0; j<nfields;j++)
    {
      Pio(stream,array_[j]);
    }

    stream.end_cheap_delim();
  }
  else
  {
    long long nfields = static_cast<long long>(array_.size());
    stream.io(nfields);

    stream.begin_cheap_delim();

    for (size_type j=0; j<nfields;j++)
    {
      Pio(stream,array_[j]);
    }

    stream.end_cheap_delim();
  }

  stream.end_class();
}

static Persistent* make_FieldArray() {
  return new FieldArray;
}

// initialize the static member type_id
PersistentTypeID FieldArray::type_id("FieldArray", "PropertyManager", make_FieldArray);

}
