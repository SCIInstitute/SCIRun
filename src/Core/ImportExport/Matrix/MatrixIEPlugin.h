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
 *  MatrixIEPlugin:  Data structure needed to make a SCIRun MatrixIE Plugin
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   May 2004
 *
 */

#ifndef SCI_project_MatrixIEPlugin_h
#define SCI_project_MatrixIEPlugin_h 1

#include <Core/Logging/LoggerFwd.h>
#include <Core/Datatypes/MatrixFwd.h>

#include <map>

#include <Core/ImportExport/share.h>

namespace SCIRun {

//----------------------------------------------------------------------
class SCISHARE MatrixIEPlugin {
public:
  const std::string pluginname_;

  const std::string fileExtension_;
  const std::string fileMagic_;

  Core::Datatypes::MatrixHandle (*fileReader_)(Core::Logging::Log& pr, const char *filename);
  bool (*fileWriter_)(Core::Logging::Log& pr,
		     Core::Datatypes::MatrixHandle f, const char *filename);

  MatrixIEPlugin(const std::string &name,
		 const std::string &fileextension,
		 const std::string &filemagic,
		 Core::Datatypes::MatrixHandle (*freader)(Core::Logging::Log& pr,
					 const char *filename) = 0,
		 bool (*fwriter)(Core::Logging::Log& pr, Core::Datatypes::MatrixHandle f,
				 const char *filename) = 0);

  ~MatrixIEPlugin();

  bool operator==(const MatrixIEPlugin &other) const;
};



class SCISHARE MatrixIEPluginManager {
public:
  void get_importer_list(std::vector<std::string> &results);
  void get_exporter_list(std::vector<std::string> &results);
  MatrixIEPlugin *get_plugin(const std::string &name);
};


} // End namespace SCIRun

#endif
