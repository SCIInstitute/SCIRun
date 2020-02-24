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
///@file  WriteHDF5DumpFile.h
///
///@author
///   Allen Sanderson
///   SCI Institute
///   University of Utah
///@date  May 2003
///


#ifndef HDF5_DUMP_API
#define HDF5_DUMP_API

#include <sci_defs/hdf5_defs.h>

#include <iostream>
#include <vector>

#ifdef HAVE_HDF5

#include "hdf5.h"

namespace SCIRun {

herr_t hdf5_attr_iter (hid_t obj_id, const char *name, void *op_data);
herr_t hdf5_group_iter(hid_t obj_id, const char *name, void *op_data);
herr_t hdf5_link_iter (hid_t obj_id, const char *name, const H5L_info_t *linfo, void *op_data);

class WriteHDF5DumpFile {
public:
  WriteHDF5DumpFile( ostream *iostr );

  void tab( );

  herr_t file(const std::string fname);
  herr_t group    (hid_t group_id,   const char *name);
  herr_t all      (hid_t obj_id,     const char *name);
  herr_t link     (hid_t link_id,    const char *name);
  herr_t attr     (hid_t group_id,   const char *name);
  herr_t dataset  (hid_t dataset_id, const char *name);
  herr_t datatype (hid_t obj_id,     hid_t type);
  herr_t data     (hid_t obj_id,     hid_t type);
  herr_t dataspace(hid_t file_space_id);

  std::string error() { return error_msg_; };

private:
  vector<hid_t> group_ids, dataset_ids;

  unsigned int indent_;
  unsigned int max_indent_;

  vector<string> path_;

  ostream *iostr_;
  std::string error_msg_;

  template< class C >
  typename C::iterator find( C& c, typename C::value_type v )
  {
    typename C::iterator p = c.begin();

    while( p != c.end() )
      if( *p == v ) return p;
      else p++;
    return p;
  }

};


} // end namespace SCIRun

#endif  // HAVE_HDF5

#endif  // HDF5_DUMP_API
