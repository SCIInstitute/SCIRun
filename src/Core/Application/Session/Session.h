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


#ifndef CORE_APPLICATION_Session_H
#define CORE_APPLICATION_Session_H

#include <boost/filesystem/path.hpp>
#include <Core/Utils/Singleton.h>
#include <Core/Application/Session/share.h>

namespace SCIRun
{
  namespace Core
  {
    // Idea phase now. Just mapping out potential functions and dependencies.
    class SCISHARE SessionInterface
    {
    public:
      virtual ~SessionInterface() {}

      virtual void beginSession() = 0;
      virtual void endSession() = 0;

      virtual void recordSystemDetails() = 0; // hardware/OS

      virtual void networkFileLoaded() = 0;
      virtual void networkFileSaved() = 0;

      // lump all edits together for now.
      virtual void networkEdited(/*EditDescription*/) = 0;

      virtual void executionStarted() = 0;
      virtual void executionFinished() = 0;

      virtual void moduleExecutionStarted(/*ModuleId*/) = 0;
      virtual void moduleExecutionFinished(/*ModuleId*/) = 0;

      virtual void recordStateVariable(/*ModuleId, Variable*/) = 0;
      virtual void fileRead(/*ModuleId, filename*/) = 0;
      virtual void fileWritten(/*ModuleId, filename*/) = 0;

      virtual void externalProgramAccessed() = 0;
    };

    typedef boost::shared_ptr<SessionInterface> SessionHandle;

    // File or db.
    class SCISHARE SessionBackEnd
    {
    public:
      virtual ~SessionBackEnd() {}
      virtual void consume(const std::string& statement, const std::string& message) = 0;
    };

    typedef boost::shared_ptr<SessionBackEnd> SessionBackEndHandle;

    class SCISHARE SessionBuilder
    {
    public:
      SessionHandle build(const boost::filesystem::path& file);
    };

    class SCISHARE SessionManager : boost::noncopyable
    {
	    CORE_SINGLETON( SessionManager );

    private:
	    SessionManager();

    public:
      void initialize(const boost::filesystem::path& dir);
      SessionHandle session();
    private:
	    //boost::filesystem::path sessionDir_;
      SessionHandle session_;
    };

    class SCISHARE SessionUser
    {
    public:
      SessionHandle session();
    private:
      friend class SessionManager;
      static SessionHandle session_;
    };

}}

#endif
