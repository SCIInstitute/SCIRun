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
 *  NrrdIEPlugin:  Data structure needed to make a SCIRun NrrdIE Plugin
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   May 2004
 *
 */

#ifndef SCI_project_NrrdIEPlugin_h
#define SCI_project_NrrdIEPlugin_h 1

#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>
#include <Core/ImportExport/GenericIEPlugin.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/ImportExport/share.h>

namespace SCIRun
{
  typedef GenericIEPluginInterface<NrrdData> NrrdIEPlugin;

  typedef IEPluginLegacyAdapter<NrrdData> NrrdIEPluginLegacyAdapter;

  class SCISHARE NrrdIEPluginManager : public GenericIEPluginManager<NrrdData>
  {

  };


  template <class Data>
  struct StaticIEPluginGetter
  {
    using Manager = void;
  };

  template <>
  struct StaticIEPluginGetter<Field>
  {
    using Manager = FieldIEPluginManager;
  };

  template <>
  struct StaticIEPluginGetter<Core::Datatypes::Matrix>
  {
    using Manager = MatrixIEPluginManager;
  };


//
////----------------------------------------------------------------------
//class SCISHARE NrrdIEPlugin {
//public:
//  const std::string pluginname;
//
//  const std::string fileextension;
//  const std::string filemagic;
//
//  NrrdDataHandle (*fileReader_)(Core::Logging::Log& pr, const char *filename);
//  bool (*fileWriter_)(Core::Logging::Log& pr,
//		     NrrdDataHandle f, const char *filename);
//
//  NrrdIEPlugin(const std::string &name,
//		 const std::string &fileextension,
//		 const std::string &filemagic,
//		 NrrdDataHandle (*freader)(Core::Logging::Log& pr,
//					 const char *filename) = 0,
//		 bool (*fwriter)(Core::Logging::Log& pr, NrrdDataHandle f,
//				 const char *filename) = 0);
//
//  ~NrrdIEPlugin();
//
//  bool operator==(const NrrdIEPlugin &other) const;
//};
//
//
//
//class SCISHARE NrrdIEPluginManager {
//public:
//  void get_importer_list(std::vector<std::string> &results);
//  void get_exporter_list(std::vector<std::string> &results);
//  NrrdIEPlugin *get_plugin(const std::string &name);
//};


} // End namespace SCIRun

#endif
