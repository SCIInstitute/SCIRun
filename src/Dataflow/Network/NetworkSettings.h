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


/// @todo Documentation Dataflow/Network/NetworkSettings.h

#ifndef DATAFLOW_NETWORK_NETWORK_SETTINGS_H
#define DATAFLOW_NETWORK_NETWORK_SETTINGS_H

#include <map>
#include <boost/noncopyable.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE NetworkGlobalSettings : boost::noncopyable
  {
  public:
    //mirror QSettings interface.
    std::string value(const std::string& key) const;
    void setValue(const std::string& key, const std::string& value);
  private:
    std::map<std::string,std::string> settings_;
  };

  //TODO: class may be useful later--keep as a reminder to extract this functionality when we add more path settings.
  //class SCISHARE NetworkPathSettings
  //{
  //public:
  //  explicit NetworkPathSettings(const NetworkGlobalSettings& globalSettings);
  //  boost::filesystem::path regressionTestDataDirectory() const;
  //private:
  //  const NetworkGlobalSettings& settings_;
  //};

}}}

#endif
