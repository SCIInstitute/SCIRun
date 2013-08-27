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
#include <set>
#include <stdexcept>
#include <iosfwd>
#include <boost/variant.hpp>
#include <boost/function.hpp>
#include <boost/assign/list_of.hpp>
#include <Core/Logging/LoggerInterface.h>
#include <Core/Utils/Exception.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Utils/ProgressReporter.h>
#include <Core/Algorithms/Base/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {

  struct SCISHARE Name
  {
    Name() : name_("_unspecified_") {}
    explicit Name(const std::string& name);

    std::string name1() const { return name_; }
    
    bool operator<(const Name& rhs) const
    {
      return name_ < rhs.name_;
    }

    std::string name_;
  };
    
  typedef Name AlgorithmParameterName;
  typedef Name AlgorithmInputName;
  typedef Name AlgorithmOutputName;

  class SCISHARE AlgoOption 
  {
  public:
    AlgoOption() {}
    AlgoOption(const std::string& option, const std::set<std::string>& options) 
      : option_(option), options_(options) {}

    std::string option_;
    std::set<std::string> options_;
  };

  SCISHARE bool operator==(const AlgoOption& lhs, const AlgoOption& rhs);
  SCISHARE std::ostream& operator<<(std::ostream& out, const AlgoOption& op);

  class SCISHARE Variable
  {
  public:
    //TODO: expand this 
    typedef boost::variant<
      int,
      double,
      std::string,
      bool,
      AlgoOption
    > Value;

    Variable() {}
    Variable(const Name& name, const Value& value) : name_(name), value_(value) {}
    Variable(const Name& name, const Datatypes::DatatypeHandle& value) : name_(name), data_(value) {}

    Name name_;
    Value value_;
    Datatypes::DatatypeHandle data_;

    int getInt() const;
    double getDouble() const;
    std::string getString() const;
    bool getBool() const;
    Datatypes::DatatypeHandle getDatatype() const;
    AlgoOption getOption() const;
  };
  
  typedef Variable AlgorithmParameter;

  class SCISHARE AlgorithmLogger : public Core::Logging::LoggerInterface
  {
  public:
    AlgorithmLogger();
    ~AlgorithmLogger();
    void setLogger(Core::Logging::LoggerHandle logger);

    //! functions for the algorithm, so it can forward errors if needed
    virtual void error(const std::string& error) const;
    virtual void warning(const std::string& warning) const;
    virtual void remark(const std::string& remark) const;
    virtual void status(const std::string& status) const;
  private:
    Core::Logging::LoggerHandle logger_;
    Core::Logging::LoggerHandle defaultLogger_;
  };

  //TODO: integrate with logger type above
  class SCISHARE AlgorithmStatusReporter : public Core::Utility::ProgressReporter
  {
  public:
    AlgorithmStatusReporter() 
    {
      setUpdaterFunc(defaultUpdaterFunc_);
    }
    ~AlgorithmStatusReporter() {}
        
    virtual void report_start(const std::string& tag) const {}
    virtual void report_end() const {}

    void update_progress(double percent) const { updaterFunc_(percent); }

    typedef boost::function<void(double)> UpdaterFunc;
    void setUpdaterFunc(UpdaterFunc func) { updaterFunc_ = func; }
    UpdaterFunc getUpdaterFunc() const { return updaterFunc_; }
  private:
    UpdaterFunc updaterFunc_;
    static UpdaterFunc defaultUpdaterFunc_;
  };

  class SCISHARE ScopedAlgorithmStatusReporter : boost::noncopyable
  {
  public:
    ScopedAlgorithmStatusReporter(const AlgorithmStatusReporter* asr, const std::string& tag);
    ~ScopedAlgorithmStatusReporter();
  private:
    const AlgorithmStatusReporter* asr_;
  };

  //TODO: link this to ModuleState via meeting discussion
  class SCISHARE AlgorithmParameterList
  {
  public:
    AlgorithmParameterList();
    void set(const AlgorithmParameterName& key, const AlgorithmParameter::Value& value);
    const AlgorithmParameter& get(const AlgorithmParameterName& key) const;

    bool set_option(const AlgorithmParameterName& key, const std::string& value);
    bool get_option(const AlgorithmParameterName& key, std::string& value) const;
    std::string get_option(const AlgorithmParameterName& key) const;

  protected:
    void addParameter(const AlgorithmParameterName& key, const AlgorithmParameter::Value& defaultValue);
    void add_option(const AlgorithmParameterName& key, const std::string& defval, const std::string& options);
  private:
    std::map<AlgorithmParameterName, AlgorithmParameter> parameters_;
  };

  class SCISHARE AlgorithmData
  {
  public:
    typedef std::map<Name, Datatypes::DatatypeHandle> Map;
    AlgorithmData() {}
    explicit AlgorithmData(const Map& m) : data_(m) {}

    Datatypes::DatatypeHandle& operator[](const Name& name);

    template <typename T>
    boost::shared_ptr<T> get(const Name& name) const
    {
      auto it = data_.find(name);
      return it == data_.end() ? boost::shared_ptr<T>() : boost::dynamic_pointer_cast<T>(it->second);
    }

  private:
    Map data_;
  };
  
  class SCISHARE AlgorithmInput : public AlgorithmData 
  {
  public:
    AlgorithmInput() {}
    AlgorithmInput(const Map& m) : AlgorithmData(m) {}
  };
  
  class SCISHARE AlgorithmOutput : public AlgorithmData {};
  
  typedef boost::shared_ptr<AlgorithmInput> AlgorithmInputHandle;
  typedef boost::shared_ptr<AlgorithmOutput> AlgorithmOutputHandle;

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

    virtual AlgorithmOutput run_generic(const AlgorithmInput& input) const = 0;

  };
  
  class SCISHARE AlgorithmCollaborator
  {
  public:
    virtual ~AlgorithmCollaborator() {}
    virtual Logging::LoggerHandle getLogger() const = 0;
    virtual AlgorithmStatusReporter::UpdaterFunc getUpdaterFunc() const = 0;
  };

  class SCISHARE AlgorithmFactory
  {
  public:
    virtual ~AlgorithmFactory() {}
    virtual AlgorithmHandle create(const std::string& name, const AlgorithmCollaborator* algoCollaborator) const = 0;
  };

}}}

#define make_input(list) SCIRun::Core::Algorithms::AlgorithmInput(SCIRun::Core::Algorithms::AlgorithmData::Map(boost::assign::map_list_of list))
#define make_output(portName) SCIRun::Core::Algorithms::AlgorithmParameterName(#portName)
#define get_output(outputObj, portName, type) boost::dynamic_pointer_cast<type>(outputObj[make_output(portName)]);

#endif