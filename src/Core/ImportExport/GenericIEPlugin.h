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

#ifndef SCI_project_GenericIEPlugin_h
#define SCI_project_GenericIEPlugin_h 1

#include <Core/Util/ProgressReporter.h>

#include <map>

#include <Core/ImportExport/share.h>

namespace SCIRun {

//----------------------------------------------------------------------
template <class Data>
class SCISHARE GenericIEPluginInterface
{
public:
  virtual ~GenericIEPluginInterface() {}
  virtual std::string pluginname() const = 0;

  virtual std::string fileExtension() const = 0;
  virtual std::string fileMagic() const = 0;

  virtual boost::shared_ptr<Data> readFile(ProgressReporter *pr, const char *filename) const = 0;
  virtual bool writeFile(ProgressReporter *pr, boost::shared_ptr<Data> f, const char *filename) const = 0;
  virtual bool equals(const GenericIEPluginInterface<Data>& other) const = 0;
};


template <class Data>
class SCISHARE GenericIEPluginManager {
public:
  void get_importer_list(std::vector<std::string> &results);
  void get_exporter_list(std::vector<std::string> &results);
  GenericIEPluginInterface<Data>* get_plugin(const std::string &name);
};


} // End namespace SCIRun

#endif
