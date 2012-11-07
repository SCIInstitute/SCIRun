/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#ifndef ALGORITHMS_BASE_ALGORITHMBASE_H
#define ALGORITHMS_BASE_ALGORITHMBASE_H

#include <string>
#include <stdexcept>
#include <boost/variant.hpp>
#include <boost/function.hpp>
#include <Core/Logging/LoggerFwd.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/ProgressReporter.h>
#include <Core/Algorithms/Base/Share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {

  struct SCISHARE AlgorithmParameterName
  {
    AlgorithmParameterName() : name_("<unspecified>") {}
    explicit AlgorithmParameterName(const std::string& name) : name_(name) {}
    std::string name_;
    bool operator<(const AlgorithmParameterName& rhs) const
    {
      return name_ < rhs.name_;
    }
  };

  class SCISHARE AlgorithmParameter
  {
  public:
    typedef boost::variant<int,double,std::string,bool> Value;

    AlgorithmParameter() {}
    AlgorithmParameter(const AlgorithmParameterName& name, const Value& value) : name_(name), value_(value) {}

    AlgorithmParameterName name_;
    Value value_;

    int getInt() const;
    double getDouble() const;
    std::string getString() const;
    bool getBool() const;
    //etc
  };

  class SCISHARE AlgorithmLogger
  {
  public:
    AlgorithmLogger();
    ~AlgorithmLogger();
    void setLogger(Core::Logging::LoggerHandle logger);

    //! functions for the algorithm, so it can forward errors if needed
    void error(const std::string& error) const;
    void warning(const std::string& warning) const;
    void remark(const std::string& remark) const;
    void status(const std::string& status) const;
  private:
    Core::Logging::LoggerHandle logger_;
    Core::Logging::LoggerHandle defaultLogger_;
  };

  //TODO
  class SCISHARE AlgorithmStatusReporter : public Core::Utility::ProgressReporter
  {
  public:
    AlgorithmStatusReporter() {}
    ~AlgorithmStatusReporter() {}
        
    virtual void report_start(const std::string& tag) const {}
//     {
//       std::cout << "Algorithm starting: " << tag << std::endl;
//     }

   virtual void report_end() const {}
//     {
//       std::cout << "Algorithm ending." << std::endl;
//     }

    void update_progress(double percent) const { updaterFunc_(percent); }

    typedef boost::function<void(double)> UpdaterFunc;
    static void setUpdaterFunc(UpdaterFunc func) { updaterFunc_ = func; }
    static UpdaterFunc getUpdaterFunc() { return updaterFunc_; }
  private:
    static UpdaterFunc updaterFunc_;
  };

  //TODO: link this to ModuleState via meeting discussion
  class SCISHARE AlgorithmParameterList
  {
  public:
    AlgorithmParameterList();
    void set(const AlgorithmParameterName& key, const AlgorithmParameter::Value& value);
    const AlgorithmParameter& get(const AlgorithmParameterName& key) const;
  protected:
    void addParameter(const AlgorithmParameterName& key, const AlgorithmParameter::Value& defaultValue);
  private:
    std::map<AlgorithmParameterName, AlgorithmParameter> parameters_;
  };

  

  //template <class Input, class Output, class Parameters>
  class SCISHARE AlgorithmBase : public AlgorithmLogger, public AlgorithmStatusReporter, public AlgorithmParameterList
  {
  public:
    virtual ~AlgorithmBase();

    /*
      TODO idea: make it mockable
  
    virtual OutputDatatypeHandleOptions run(InputDatatypeHandleOptions, ModuleParameterState) = 0;

      ModuleParameterState: essentially a map of GuiVars. but need hooks for undo/redo and serialization
      Input: tuple/heterogeneous vector of Datatypes
      Output: tuple of Datatypes, possibly delay-executed
    */

    //virtual Output run(const Input& input, const Parameters& params) const = 0;

  };
  
}}}

#endif