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


/// @todo Documentation Core/Utils/Legacy/Debug.h

#ifndef CORE_UTIL_DEBUG_H
#define CORE_UTIL_DEBUG_H 1

#include <string>
#include <sci_debug.h>
#include <Core/Utils/Legacy/share.h>

namespace SCIRun {

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
// Count objects
SCISHARE void   debug_incr_object_count(const std::string& type);
SCISHARE void   debug_decr_object_count(const std::string& type);
SCISHARE void   debug_print_objects();
SCISHARE size_t debug_number_of_objects();
SCISHARE void   debug_tag_default_number_of_objects();
#endif

// MACROS THAT ARE INSERTED INTO OBJECTS
// These macros are there so we can selectively insert the system

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#if DEBUG
#define DEBUG_CONSTRUCTOR(type) \
debug_incr_object_count(type);

#define DEBUG_DESTRUCTOR(type) \
debug_decr_object_count(type);

#else
#endif
#endif


#define DEBUG_CONSTRUCTOR(type)
#define DEBUG_DESTRUCTOR(type)



} // End namespace SCIRun

#endif
