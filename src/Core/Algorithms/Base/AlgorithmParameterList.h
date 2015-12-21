/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#ifndef ALGORITHMS_BASE_ALGORITHMPARAMETERLIST_H
#define ALGORITHMS_BASE_ALGORITHMPARAMETERLIST_H

#include <string>
#include <set>
#include <stdexcept>
#include <iosfwd>
#include <boost/variant.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>
#include <Core/Logging/LoggerInterface.h>
#include <Core/Utils/Exception.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Datatypes/HasId.h>
#include <Core/Utils/ProgressReporter.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Thread/Mutex.h>
#include <Core/Algorithms/Base/AlgorithmInterfaces.h>
#include <Core/Algorithms/Base/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {

  /// @todo: link this to ModuleState via meeting discussion
  class SCISHARE AlgorithmParameterList : public AlgorithmParameterInterface
  {
  public:
    AlgorithmParameterList();
    bool set(const AlgorithmParameterName& key, const AlgorithmParameter::Value& value) override;
    const AlgorithmParameter& get(const AlgorithmParameterName& key) const override;

    bool set_option(const AlgorithmParameterName& key, const std::string& value);
    bool get_option(const AlgorithmParameterName& key, std::string& value) const;
    std::string get_option(const AlgorithmParameterName& key) const;
    bool check_option(const AlgorithmParameterName& key, const std::string& value) const;

    virtual bool keyNotFoundPolicy(const AlgorithmParameterName& key) const;

  protected:
    void addParameter(const AlgorithmParameterName& key, const AlgorithmParameter::Value& defaultValue);
    void add_option(const AlgorithmParameterName& key, const std::string& defval, const std::string& options);
    typedef std::map<AlgorithmParameterName, AlgorithmParameter> ParameterMap;
    ParameterMap::const_iterator paramsBegin() const { return parameters_.begin(); }
    ParameterMap::const_iterator paramsEnd() const { return parameters_.end(); }
  private:
    ParameterMap parameters_;
  };
  
}}}

#endif
