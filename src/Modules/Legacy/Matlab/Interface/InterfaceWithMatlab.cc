/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
 *  InterfaceWithMatlab.cc:
 *
 *  Written by:
 *   Jeroen Stinstra
 *
 */

#include <Modules/Legacy/Matlab/Interface/InterfaceWithMatlab.h>
#include <Core/Logging/Log.h>

#include <Core/Matlab/matlabconverter.h>
#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>

#include <Core/Thread/ConditionVariable.h>

#include <Core/SystemCall/TempFileManager.h>
#include <Core/Utils/Legacy/soloader.h>

#if 0
#include <Core/Services/ServiceClient.h>
#include <Core/Datatypes/MatrixTypeConverter.h>
#include <Packages/MatlabInterface/Services/MatlabEngine.h>
#include <Core/Thread/Runnable.h>

#include <Core/ICom/IComAddress.h>
#include <Core/ICom/IComPacket.h>

#include <Core/Services/Service.h>
#include <Core/Services/ServiceBase.h>
#include <Core/Services/FileTransferClient.h>
#include <Core/ICom/IComSocket.h>
#include <Core/Thread/CleanupManager.h>
#endif

#include <iostream>
#include <fstream>

using namespace SCIRun;
using namespace SCIRun::Modules::Matlab::Interface;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::MatlabIO;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Thread;

//TODO: temporary for compilation.
//#ifdef _WIN32
#define USE_MATLAB_ENGINE_LIBRARY
//#endif

#ifdef USE_MATLAB_ENGINE_LIBRARY
// symbols from the matlab engine library - import from the dll/so manually to not
// explicitly require matlab (it won't work without it, but will compile and SCIRun will run)
typedef struct engine Engine;	/* Incomplete definition for Engine */
typedef Engine* (*ENGOPENPROC)(const char*);
typedef int (*ENGCLOSEPROC)(Engine*);
typedef int (*ENGSETVISIBLEPROC)(Engine*, bool);
typedef int (*ENGEVALSTRINGPROC)(Engine*, const char*);
typedef int (*ENGOUTPUTBUFFERPROC)(Engine*, char*, int);

ENGOPENPROC engOpen = 0;
ENGCLOSEPROC engClose = 0;
ENGSETVISIBLEPROC engSetVisible = 0;
ENGEVALSTRINGPROC engEvalString = 0;
ENGOUTPUTBUFFERPROC engOutputBuffer = 0;
#endif

namespace MatlabImpl
{
  class InterfaceWithMatlabEngineThreadInfo;
  class InterfaceWithMatlabEngineThread;

  typedef boost::shared_ptr<InterfaceWithMatlabEngineThreadInfo> InterfaceWithMatlabEngineThreadInfoHandle;
#if 0
  class InterfaceWithMatlabEngineThread : /*public Runnable,*/ public ServiceBase
  {
  public:
    InterfaceWithMatlabEngineThread(ServiceClientHandle serv_handle, InterfaceWithMatlabEngineThreadInfoHandle info_handle);
    virtual ~InterfaceWithMatlabEngineThread();
    void run();

  private:
    ServiceClientHandle serv_handle_;
    InterfaceWithMatlabEngineThreadInfoHandle info_handle_;
  };
#endif
  class InterfaceWithMatlabEngineThreadInfo
  {
  public:
    InterfaceWithMatlabEngineThreadInfo();
    virtual ~InterfaceWithMatlabEngineThreadInfo();

    void dolock();
    void unlock();

    std::string         output_cmd_;

    ConditionVariable   wait_code_done_;
    bool                code_done_;
    bool                code_success_;
    std::string         code_error_;

    ConditionVariable   wait_exit_;
    bool                exit_;
    bool                passed_test_;

  };
}

  namespace SCIRun {
    namespace Modules {
      namespace Matlab {
        namespace Interface {

          class InterfaceWithMatlabImpl //: public ServiceBase
          {
          public:
            explicit InterfaceWithMatlabImpl(InterfaceWithMatlab* module) : module_(module) {}
#if 0

            // Constructor
            InterfaceWithMatlab(GuiContext* ctx);

            // Destructor
            virtual ~InterfaceWithMatlab();

            // Std functions for each module
            // execute():
            //   Execute the module and put data on the output port

            virtual void execute();
            virtual void presave();
            virtual void tcl_command(GuiArgs& args, void* userdata);


            static matlabarray::mitype	convertdataformat(const std::string& dataformat);
            static std::string totclstring(const std::string& instring);
            std::vector<std::string>	converttcllist(const std::string& str);

#endif
            void	update_status(const std::string& text);

            bool	open_matlab_engine();
            bool	close_matlab_engine();

            bool	create_temp_directory();
            bool	delete_temp_directory();

            bool	save_input_matrices();
            bool	load_output_matrices();

            bool	generate_matlab_code();
            bool	send_matlab_job();
            bool  send_input(const std::string& str);

            bool	synchronise_input();

          private:
            InterfaceWithMatlab* module_;
            // Temp directory for writing files coming from the
            // the matlab engine

            std::string temp_directory_;

            // GUI variables
#if 0
            // Names of matrices
            GuiString   input_matrix_name_;
            GuiString   input_field_name_;
            GuiString   input_nrrd_name_;
            GuiString   input_string_name_;
            GuiString   input_matrix_type_;
            GuiString   input_nrrd_type_;
            GuiString   input_matrix_array_;
            GuiString   input_field_array_;
            GuiString   input_nrrd_array_;
            GuiString   output_matrix_name_;
            GuiString   output_field_name_;
            GuiString   output_nrrd_name_;
            GuiString   output_string_name_;
            GuiString   configfile_;


            // Fields per port
            std::vector<std::string>   input_matrix_name_list_;
            std::vector<std::string>   input_matrix_name_list_old_;
            std::vector<std::string>   input_field_name_list_;
            std::vector<std::string>   input_field_name_list_old_;
            std::vector<std::string>   input_nrrd_name_list_;
            std::vector<std::string>   input_string_name_list_old_;
            std::vector<std::string>   input_string_name_list_;
            std::vector<std::string>   input_nrrd_name_list_old_;
            std::vector<std::string>   input_matrix_type_list_;
            std::vector<std::string>   input_nrrd_type_list_;
            std::vector<std::string>   input_matrix_array_list_;
            std::vector<std::string>   input_field_array_list_;
            std::vector<std::string>   input_nrrd_array_list_;
            std::vector<std::string>   input_string_array_list_;
            std::vector<std::string>   input_matrix_type_list_old_;
            std::vector<std::string>   input_nrrd_type_list_old_;
            std::vector<std::string>   input_matrix_array_list_old_;
            std::vector<std::string>   input_field_array_list_old_;
            std::vector<std::string>   input_nrrd_array_list_old_;
            std::vector<std::string>   output_matrix_name_list_;
            std::vector<std::string>   output_field_name_list_;
            std::vector<std::string>   output_nrrd_name_list_;
            std::vector<std::string>   output_string_name_list_;

            std::vector<int> input_matrix_generation_old_;
            std::vector<int> input_field_generation_old_;
            std::vector<int> input_nrrd_generation_old_;
            std::vector<int> input_string_generation_old_;

            std::string	matlab_code_list_;

            // Ports for input and output
            std::string		input_matrix_matfile_[NUM_MATRIX_PORTS];
            std::string		input_field_matfile_[NUM_FIELD_PORTS];
            std::string		input_nrrd_matfile_[NUM_NRRD_PORTS];
            std::string		input_string_matfile_[NUM_STRING_PORTS];

            std::string		output_matrix_matfile_[NUM_MATRIX_PORTS];
            std::string		output_field_matfile_[NUM_FIELD_PORTS];
            std::string		output_nrrd_matfile_[NUM_NRRD_PORTS];
            std::string		output_string_matfile_[NUM_STRING_PORTS];

            // Internet connectivity stuff
            GuiString   inet_address_;
            GuiString   inet_port_;
            GuiString   inet_passwd_;
            GuiString   inet_session_;

            std::string inet_address_old_;
            std::string inet_port_old_;
            std::string inet_passwd_old_;
            std::string inet_session_old_;
#endif
            // The tempfilemanager
            TempFileManager tfmanager_;
            std::string		mfile_;
#if 0
            GuiString		matlab_code_;
            GuiString   matlab_code_file_;
            GuiString		matlab_var_;

            GuiString   start_matlab_;
            GuiInt      matlab_timeout_;

            std::string start_matlab_old_;
            int         matlab_timeout_old_;
#endif

#ifndef USE_MATLAB_ENGINE_LIBRARY
#if 0
            ServiceClientHandle           matlab_engine_;
            InterfaceWithMatlabEngineThreadInfoHandle	thread_info_
#endif
#else
            Engine* engine_;
            char output_buffer_[51200];
#endif

#if 0
            FileTransferClientHandle      file_transfer_;

            bool            need_file_transfer_;
            std::string     remote_tempdir_;
#endif
            std::string     inputstring_;

          public:
            static void cleanup_callback(void *data);
          };
        }}}}

#if 0
  InterfaceWithMatlabEngineThreadInfo::InterfaceWithMatlabEngineThreadInfo() :
    UsedWithLockingHandle<Mutex>("InterfaceWithMatlabEngineInfo lock"),
    wait_code_done_("InterfaceWithMatlabEngineInfo condition variable code"),
    code_done_(false),
    code_success_(false),
    wait_exit_("InterfaceWithMatlabEngineInfo condition variable exit"),
    exit_(false),
    passed_test_(false)
  {
  }

  inline void InterfaceWithMatlabEngineThreadInfo::dolock()
  {
    lock.lock();
  }

  inline void InterfaceWithMatlabEngineThreadInfo::unlock()
  {
    lock.unlock();
  }

  InterfaceWithMatlabEngineThread::InterfaceWithMatlabEngineThread(ServiceClientHandle serv_handle, InterfaceWithMatlabEngineThreadInfoHandle info_handle) :
    serv_handle_(serv_handle),
    info_handle_(info_handle)
  {
  }

  InterfaceWithMatlabEngineThread::~InterfaceWithMatlabEngineThread()
  {
  }

  void InterfaceWithMatlabEngineThread::run()
  {
    IComPacketHandle	packet;
    bool				done = false;
    while(!done)
    {
      if(!(serv_handle_->recv(packet)))
      {
        info_handle_->dolock();
        if (info_handle_->exit_ == true)
        {
          // It crashed as result of closing of connection
          // Anyway, the module was destroyed so it should not
          // matter anymore
          info_handle_->wait_code_done_.conditionBroadcast();
          info_handle_->wait_exit_.conditionBroadcast();
          info_handle_->unlock();
          return;
        }
        info_handle_->code_done_ = true;
        info_handle_->code_success_ = false;
        info_handle_->wait_code_done_.conditionBroadcast();
        info_handle_->exit_ = true;
        info_handle_->wait_exit_.conditionBroadcast();
        info_handle_->unlock();

        done = true;
        continue;
      }

      info_handle_->dolock();

      if (info_handle_->exit_ == true)
      {
        info_handle_->wait_exit_.conditionBroadcast();
        info_handle_->unlock();
        return;
      }

      switch (packet->gettag())
      {
      case TAG_STDO:
      {
        std::string str;
        if (packet->getparam1() < 0) str = "STDOUT END";
        else str = packet->getstring();
        std::string cmd = info_handle_->output_cmd_ + " \"" + InterfaceWithMatlab::totclstring(str) + "\"";
        info_handle_->unlock();
        TCLInterface::lock();
        TCLInterface::execute(cmd);
        TCLInterface::unlock();
      }
      break;
      case TAG_STDE:
      {
        std::string str;
        if (packet->getparam1() < 0) str = "STDERR END";
        else str = packet->getstring();
        std::string cmd = info_handle_->output_cmd_ + " \"STDERR: " + InterfaceWithMatlab::totclstring(str) + "\"";
        info_handle_->unlock();
        TCLInterface::lock();
        TCLInterface::execute(cmd);
        TCLInterface::unlock();
      }
      break;
      case TAG_END_:
      case TAG_EXIT:
      {
        info_handle_->code_done_ = true;
        info_handle_->code_success_ = false;
        info_handle_->wait_code_done_.conditionBroadcast();
        info_handle_->exit_ = true;
        info_handle_->wait_exit_.conditionBroadcast();
        done = true;
        info_handle_->unlock();
      }
      break;
      case TAG_MCODE_SUCCESS:
      {
        info_handle_->code_done_ = true;
        info_handle_->code_success_ = true;
        info_handle_->wait_code_done_.conditionBroadcast();
        info_handle_->unlock();
      }
      break;
      case TAG_MCODE_ERROR:
      {
        info_handle_->code_done_ = true;
        info_handle_->code_success_ = false;
        info_handle_->code_error_ = packet->getstring();
        info_handle_->wait_code_done_.conditionBroadcast();
        info_handle_->unlock();
      }
      break;
      default:
        info_handle_->unlock();
      }
    }
  }

  DECLARE_MAKER(InterfaceWithMatlab)

  InterfaceWithMatlab::InterfaceWithMatlab(GuiContext *context) :
    Module("InterfaceWithMatlab", context, Filter, "Interface", "MatlabInterface"),
    input_matrix_name_(context->subVar("input-matrix-name")),
    input_field_name_(context->subVar("input-field-name")),
    input_nrrd_name_(context->subVar("input-nrrd-name")),
    input_string_name_(context->subVar("input-string-name")),
    input_matrix_type_(context->subVar("input-matrix-type")),
    input_nrrd_type_(context->subVar("input-nrrd-type")),
    input_matrix_array_(context->subVar("input-matrix-array")),
    input_field_array_(context->subVar("input-field-array")),
    input_nrrd_array_(context->subVar("input-nrrd-array")),
    output_matrix_name_(context->subVar("output-matrix-name")),
    output_field_name_(context->subVar("output-field-name")),
    output_nrrd_name_(context->subVar("output-nrrd-name")),
    output_string_name_(context->subVar("output-string-name")),
    configfile_(context->subVar("configfile")),
    inet_address_(context->subVar("inet-address")),
    inet_port_(context->subVar("inet-port")),
    inet_passwd_(context->subVar("inet-passwd")),
    inet_session_(context->subVar("inet-session")),
    matlab_code_(context->subVar("matlab-code")),
    matlab_code_file_(context->subVar("matlab-code-file")),
    matlab_var_(context->subVar("matlab-var")),
    start_matlab_(context->subVar("start-matlab")),
    matlab_timeout_(context->subVar("matlab-timeout")),
    matlab_timeout_old_(180),
    need_file_transfer_(false)
  {
#ifdef USE_MATLAB_ENGINE_LIBRARY
    engine_ = 0;
#endif
    // find the input and output ports

    input_matrix_name_list_.resize(NUM_MATRIX_PORTS);
    input_matrix_name_list_old_.resize(NUM_MATRIX_PORTS);
    input_field_name_list_.resize(NUM_FIELD_PORTS);
    input_field_name_list_old_.resize(NUM_FIELD_PORTS);
    input_nrrd_name_list_.resize(NUM_NRRD_PORTS);
    input_nrrd_name_list_old_.resize(NUM_NRRD_PORTS);
    input_string_name_list_.resize(NUM_STRING_PORTS);
    input_string_name_list_old_.resize(NUM_STRING_PORTS);

    input_matrix_array_list_old_.resize(NUM_MATRIX_PORTS);
    input_field_array_list_old_.resize(NUM_FIELD_PORTS);
    input_nrrd_array_list_old_.resize(NUM_NRRD_PORTS);

    input_matrix_type_list_old_.resize(NUM_MATRIX_PORTS);
    input_nrrd_type_list_old_.resize(NUM_NRRD_PORTS);

    input_matrix_generation_old_.resize(NUM_MATRIX_PORTS);
    for (int p = 0; p<NUM_MATRIX_PORTS; p++)  input_matrix_generation_old_[p] = -1;
    input_field_generation_old_.resize(NUM_FIELD_PORTS);
    for (int p = 0; p<NUM_FIELD_PORTS; p++)  input_field_generation_old_[p] = -1;
    input_nrrd_generation_old_.resize(NUM_NRRD_PORTS);
    for (int p = 0; p<NUM_NRRD_PORTS; p++)  input_nrrd_generation_old_[p] = -1;
    input_string_generation_old_.resize(NUM_STRING_PORTS);
    for (int p = 0; p<NUM_STRING_PORTS; p++)  input_string_generation_old_[p] = -1;

    CleanupManager::add_callback(InterfaceWithMatlab::cleanup_callback,reinterpret_cast<void *>(this));
  }


  // Function for cleaning up
  // matlab modules
  void InterfaceWithMatlab::cleanup_callback(void *data)
  {
    InterfaceWithMatlab* ptr = reinterpret_cast<InterfaceWithMatlab *>(data);
    // We just want to make sure that the matlab engine is released and
    // any temp dirs are cleaned up
    ptr->close_matlab_engine();
    ptr->delete_temp_directory();
  }

  InterfaceWithMatlab::~InterfaceWithMatlab()
  {
    // Again if we registered a module for destruction and we are removing it
    // we need to unregister
    CleanupManager::invoke_remove_callback(InterfaceWithMatlab::cleanup_callback,reinterpret_cast<void *>(this));
  }
#endif

  void InterfaceWithMatlabImpl::update_status(const std::string& text)
  {
    LOG_DEBUG(module_->get_id().id_ << " UpdateStatus \"" << text << "\"");
  }

#if 0
  matlabarray::mitype InterfaceWithMatlab::convertdataformat(const std::string& dataformat)
  {
    matlabarray::mitype type = matlabarray::miUNKNOWN;
    if (dataformat == "same as data")  { type = matlabarray::miSAMEASDATA; }
    else if (dataformat == "double")   { type = matlabarray::miDOUBLE; }
    else if (dataformat == "single")   { type = matlabarray::miSINGLE; }
    else if (dataformat == "uint64")   { type = matlabarray::miUINT64; }
    else if (dataformat == "int64")    { type = matlabarray::miINT64; }
    else if (dataformat == "uint32")   { type = matlabarray::miUINT32; }
    else if (dataformat == "int32")    { type = matlabarray::miINT32; }
    else if (dataformat == "uint16")   { type = matlabarray::miUINT16; }
    else if (dataformat == "int16")    { type = matlabarray::miINT16; }
    else if (dataformat == "uint8")    { type = matlabarray::miUINT8; }
    else if (dataformat == "int8")     { type = matlabarray::miINT8; }
    return (type);
  }

  // converttcllist:
  // converts a TCL formatted list into a STL array
  // of strings

  std::vector<std::string> InterfaceWithMatlab::converttcllist(const std::string& str)
  {
    std::string result;
    std::vector<std::string> list(0);
    int lengthlist = 0;

    // Yeah, it is TCL dependent:
    // TCL::llength determines the length of the list
    TCLInterface::eval("llength { "+str + " }",result);
    std::istringstream iss(result);
    iss >> lengthlist;
    if (lengthlist < 0) return(list);

    list.resize(lengthlist);
    for (int p = 0;p<lengthlist;p++)
    {
      std::ostringstream oss;
      // TCL dependency:
      // TCL::lindex retrieves the p th element from the list
      oss << "lindex { " << str <<  " } " << p;
      TCLInterface::eval(oss.str(),result);
      list[p] = result;
    }
    return(list);
  }
#endif

bool InterfaceWithMatlabImpl::synchronise_input()
{
#if 0
  TCLInterface::execute(get_id()+" Synchronise");
  get_ctx()->reset();

  std::string str;
  str = input_matrix_name_.get(); input_matrix_name_list_ = converttcllist(str);
  str = input_matrix_type_.get(); input_matrix_type_list_ = converttcllist(str);
  str = input_matrix_array_.get(); input_matrix_array_list_ = converttcllist(str);
  str = output_matrix_name_.get(); output_matrix_name_list_ = converttcllist(str);

  str = input_field_name_.get(); input_field_name_list_ = converttcllist(str);
  str = input_field_array_.get(); input_field_array_list_ = converttcllist(str);
  str = output_field_name_.get(); output_field_name_list_ = converttcllist(str);

  str = input_nrrd_name_.get(); input_nrrd_name_list_ = converttcllist(str);
  str = input_nrrd_type_.get(); input_nrrd_type_list_ = converttcllist(str);
  str = input_nrrd_array_.get(); input_nrrd_array_list_ = converttcllist(str);
  str = output_nrrd_name_.get(); output_nrrd_name_list_ = converttcllist(str);

  str = input_string_name_.get(); input_string_name_list_ = converttcllist(str);
  str = output_string_name_.get(); output_string_name_list_ = converttcllist(str);

  TCLInterface::execute(get_id() + " update_text"); // update matlab_code_ before use.
  matlab_code_list_ = matlab_code_.get();
#endif
  return(true);
}


const ModuleLookupInfo InterfaceWithMatlab::staticInfo_("InterfaceWithMatlab", "Interface", "Matlab");

InterfaceWithMatlab::InterfaceWithMatlab() : Module(staticInfo_), impl_(new InterfaceWithMatlabImpl(this))
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(InputString);
  INITIALIZE_PORT(OutputField0);
  INITIALIZE_PORT(OutputMatrix0);
  INITIALIZE_PORT(OutputString0);
  INITIALIZE_PORT(OutputField1);
  INITIALIZE_PORT(OutputMatrix1);
  INITIALIZE_PORT(OutputString1);
}

void InterfaceWithMatlab::setStateDefaults()
{

}

void InterfaceWithMatlab::execute()
{
  // Synchronise input: translate TCL lists into C++ STL lists
  if (!impl_->synchronise_input())
  {
    error("InterfaceWithMatlab: Could not retreive GUI input");
    return;
  }

  // If we haven't created a temporary directory yet
  // we open one to store all temp files in
  if (!impl_->create_temp_directory())
  {
    error("InterfaceWithMatlab: Could not create temporary directory");
    return;
  }

  if (!impl_->open_matlab_engine())
  {
    error("InterfaceWithMatlab: Could not open matlab engine");
    return;
  }
#if 0
  if (!(save_input_matrices()))
  {
    error("InterfaceWithMatlab: Could not create the input matrices");
    return;
  }

  update_state(Executing);

  if (!(generate_matlab_code()))
  {
    error("InterfaceWithMatlab: Could not create m-file code for matlabengine");
    return;
  }

  if (!send_matlab_job())
  {
    error("InterfaceWithMatlab: Matlab returned an error or Matlab could not be launched");
    return;
  }

  if (!load_output_matrices())
  {
    error("InterfaceWithMatlab: Could not load matrices that matlab generated");
    return;
  }
#endif
}

#if 0
  void InterfaceWithMatlab::presave()
  {
    TCLInterface::execute(get_id() + " update_text");  // update matlab-code before saving.
  }

  bool InterfaceWithMatlab::send_matlab_job()
  {
    std::string mfilename = mfile_.substr(0,mfile_.size()-2); // strip the .m
    std::string remotefile = file_transfer_->remote_file(mfilename);
#ifndef USE_MATLAB_ENGINE_LIBRARY
    IComPacketHandle packet = new IComPacket;

    if (packet.get_rep() == 0)
    {
      error("InterfaceWithMatlab: Could not create packet");
      return(false);
    }

    thread_info_->dolock();
    thread_info_->code_done_ = false;
    thread_info_->unlock();

    packet->settag(TAG_MCODE);
    packet->setstring(remotefile);
    matlab_engine_->send(packet);

    thread_info_->dolock();
    if (!thread_info_->code_done_)
    {
      thread_info_->wait_code_done_.wait(thread_info_->lock);
    }
    bool success = thread_info_->code_success_;
    bool exitcond = thread_info_->exit_;
    if (!success)
    {
      if (exitcond)
      {
        error("InterfaceWithMatlab: the matlab engine crashed or did not start: "+ thread_info_->code_error_);
        error("InterfaceWithMatlab: possible causes are:");
        error("(1) matlab code failed in such a way that the engine was no able to catch the error (e.g. failure mex of code)");
        error("(2) matlab crashed and the matlab engine detected an end of communication of the matlab process");
        error("(3) temparory files could not be created or are corrupted");
        error("(4) improper matlab version, use matlab V5 or higher, currently matlab V5-V7 are supported");
      }
      else
      {
        error("InterfaceWithMatlab: matlab code failed: "+thread_info_->code_error_);
        error("InterfaceWithMatlab: Detected an error in the Matlab code, the matlab engine is still running and caught the exception");
        error("InterfaceWithMatlab: Please check the matlab code in the GUI and try again. The output window in the GUI should contain the reported error message generated by matlab");
      }
      thread_info_->code_done_ = false;
      thread_info_->unlock();
      return(false);
    }
    thread_info_->code_done_ = false;
    thread_info_->unlock();

#else
    std::string command = std::string("addpath('") + remotefile.substr(0,remotefile.size()-12) +"');";
    bool success = (engEvalString(engine_, command.c_str()) == 0);

    command = "scirun_code;";
    success = (engEvalString(engine_, command.c_str()) == 0);

    std::string output(output_buffer_);
    std::string cmd = get_id()+" AddOutput \"" + InterfaceWithMatlab::totclstring(output) + "\"";
    TCLInterface::lock();
    TCLInterface::execute(cmd);
    TCLInterface::unlock();

    std::cout << output_buffer_ << std::endl;

    // without this, matlab will cache the first file as a function and ignore the rest of them.
    engEvalString(engine_, "clear functions");
    // output_buffer_ has output in it - do something with it
#endif
    return(success);
  }

  bool InterfaceWithMatlab::send_input(std::string str)
  {
#ifndef USE_MATLAB_ENGINE_LIBRARY
    IComPacketHandle packet = new IComPacket;

    if (matlab_engine_.get_rep() == 0) return(true);

    if (packet.get_rep() == 0)
    {
      error("InterfaceWithMatlab: Could not create packet");
      return(false);
    }

    packet->settag(TAG_INPUT);
    packet->setstring(str);
    matlab_engine_->send(packet);

    return(true);
#else
    if (engine_ == 0) return true;
    engEvalString(engine_, str.c_str());
    return true;
#endif
  }
#endif

  bool InterfaceWithMatlabImpl::open_matlab_engine()
  {
    std::string matlablibrary;
    const std::string inetaddress;
#if 0
    std::string inetaddress = inet_address_.get();
    std::string inetport = inet_port_.get();
    std::string passwd = inet_passwd_.get();
    std::string session = inet_session_.get();
    std::string startmatlab;

    if ( sci_getenv("SCIRUN_STARTMATLAB") ) startmatlab = sci_getenv("SCIRUN_STARTMATLAB");
    if ( sci_getenv("SCIRUN_MATLABLIBRARY") ) matlablibrary = sci_getenv("SCIRUN_MATLABLIBRARY");

    int timeout = 0;
    std::string timeout_str = sci_getenv("SCIRUN_MATLABTIMEOUT");
    from_string(timeout_str,timeout);

    // Check whether someone touched the engine settings
    // if so close the engine and record the current settings
    if ((timeout != matlab_timeout_old_)||(startmatlab != start_matlab_old_)||(inetaddress != inet_address_old_)||(inetport != inet_port_old_)||(passwd != inet_passwd_old_)||(session != inet_session_old_))
    {
      close_matlab_engine();
      inet_address_old_ = inetaddress;
      inet_port_old_ = inetport;
      inet_passwd_old_ = passwd;
      inet_session_old_ = session;
      matlab_timeout_old_ = timeout;
      start_matlab_old_ = startmatlab;
    }
#endif
#ifndef USE_MATLAB_ENGINE_LIBRARY
    if (!matlab_engine_)
#else
    if (!engine_)
#endif
    {
#if 0
      IComAddress address;
      if (inetaddress == "")
      {
        address.setaddress("internal","servicemanager");
      }
      else
      {
        address.setaddress("scirun",inetaddress,inetport);
      }

      int sessionnum;
      from_string(session,sessionnum);
#endif
      update_status("Please wait while launching matlab, this may take a few minutes ....\n");

#ifndef USE_MATLAB_ENGINE_LIBRARY
#if 0
      matlab_engine_ = new ServiceClient();
      if(!(matlab_engine_->open(address,"matlabengine",sessionnum,passwd,timeout,startmatlab)))
      {
        module_->error(std::string("InterfaceWithMatlab: Could not open matlab engine (error=") + matlab_engine_->geterror() + std::string(")"));
        module_->error(std::string("InterfaceWithMatlab: Make sure the matlab engine has not been disabled in [SCIRUN_DIRECTORY]/services/matlabengine.rc"));
        module_->error(std::string("InterfaceWithMatlab: Press the 'Edit Local Config of Matlab Engine' to change the configuration"));
        module_->error(std::string("InterfaceWithMatlab: Check remote address information, or leave all fields except 'session' blank to connect to local matlab engine"));

        matlab_engine_ = 0;
        return(false);
      }
#endif
#else
      if (engOpen == 0)
      {
        // load functions from dll
        std::string error_msg;

        LIBRARY_HANDLE englib = 0;
        if ( matlablibrary.size() > 10 )
        {
          size_t size = matlablibrary.size();
          if ( matlablibrary.substr( size-10 ) == "libeng.dll" )
          {
            englib = GetLibraryHandle( matlablibrary.c_str(), error_msg );
          }
        }

        if (!englib)
        {
          englib = GetLibraryHandle( "libeng.dll", error_msg );
        }

        if (!englib)
        {
          module_->error(std::string("Could not open matlab library libeng."));
          return false;
        }

        engOpen = (ENGOPENPROC) GetHandleSymbolAddress(englib, "engOpen",error_msg);
        engClose = (ENGCLOSEPROC) GetHandleSymbolAddress(englib, "engClose",error_msg);
        engSetVisible = (ENGSETVISIBLEPROC) GetHandleSymbolAddress(englib, "engSetVisible",error_msg);
        engEvalString = (ENGEVALSTRINGPROC) GetHandleSymbolAddress(englib, "engEvalString",error_msg);
        engOutputBuffer = (ENGOUTPUTBUFFERPROC) GetHandleSymbolAddress(englib, "engOutputBuffer",error_msg);

        if (!engOpen || !engClose || !engSetVisible || !engEvalString || !engOutputBuffer)
        {
          if (!engOpen) module_->error(std::string("Cannot find engOpen"));
          if (!engClose) module_->error(std::string("Cannot find engClose"));
          if (!engSetVisible) module_->error(std::string("Cannot find engSetVisible"));
          if (!engEvalString) module_->error(std::string("Cannot find engEvalString"));
          if (!engOutputBuffer) module_->error(std::string("Cannot find engOutputBuffer"));
          module_->error(std::string("Could not open matlab engine functions from matlab library"));
          return false;
        }
      }


      if (inetaddress == "")
      {
        engine_ = engOpen( NULL );

        if ( !engine_ )
        {
          system("matlab /regserver");
          engine_ = engOpen( NULL );
        }
      }
      else
      {
        engine_ = engOpen(inetaddress.c_str());
      }

      if (!engine_)
      {
        module_->error(std::string("InterfaceWithMatlab: Could not open matlab engine"));
        module_->error(std::string("InterfaceWithMatlab: Check remote address information, or leave all fields except 'session' blank to connect to local matlab engine"));
        return false;
      }

      engOutputBuffer(engine_, output_buffer_, 51200);
      engSetVisible(engine_, true);
#endif

#if 0
      file_transfer_ = new FileTransferClient();
      if(!(file_transfer_->open(address,"matlabenginefiletransfer",sessionnum,passwd)))
      {
        std::string err;
#ifndef USE_MATLAB_ENGINE_LIBRARY
        err = matlab_engine_->geterror();
        matlab_engine_->close();
        matlab_engine_ = 0;
#else
        engClose(engine_);
        engine_ = 0;
#endif
        std::cout << "ERROR" << file_transfer_->geterror() << std::endl;
        module_->error(std::string("InterfaceWithMatlab: Could not open matlab engine file transfer service (error=") + err + std::string(")"));
        module_->error(std::string("InterfaceWithMatlab: Make sure the matlab engine file transfer service has not been disabled in [MATLAB_DIRECTPRY]/services/matlabengine.rc"));
        module_->error(std::string("InterfaceWithMatlab: Press the 'Edit Local Config of Matlab Engine' to change the configuration"));
        module_->error(std::string("InterfaceWithMatlab: Check remote address information, or leave all fields except 'session' blank to connect to local matlab engine"));

        file_transfer_ = 0;
        return(false);
      }

      need_file_transfer_ = false;
      std::string localid;
      std::string remoteid;
      file_transfer_->get_local_homedirid(localid);
      file_transfer_->get_remote_homedirid(remoteid);

      if (localid != remoteid)
      {
        need_file_transfer_ = true;
        if(!(file_transfer_->create_remote_tempdir("matlab-engine.XXXXXX",remote_tempdir_)))
        {
#ifndef USE_MATLAB_ENGINE_LIBRARY
          matlab_engine_->close();
          matlab_engine_ = 0;
#else
          engClose(engine_);
          engine_ = 0;
#endif
          module_->error(std::string("InterfaceWithMatlab: Could not create remote temporary directory"));
          file_transfer_->close();
          file_transfer_ = 0;
          return(false);
        }
        file_transfer_->set_local_dir(temp_directory_);
        file_transfer_->set_remote_dir(remote_tempdir_);
      }
      else
      {
        // Although they might share a home directory
        // This directory can be mounted at different trees
        // Hence we translate between both. InterfaceWithMatlab does not like
        // the use of $HOME
        file_transfer_->set_local_dir(temp_directory_);
        std::string tempdir = temp_directory_;
        file_transfer_->translate_scirun_tempdir(tempdir);
        file_transfer_->set_remote_dir(tempdir);
      }
#endif

#ifndef USE_MATLAB_ENGINE_LIBRARY
      IComPacketHandle packet;
      if(!(matlab_engine_->recv(packet)))
      {
        matlab_engine_->close();
        file_transfer_->close();
        error(std::string("InterfaceWithMatlab: Could not get answer from matlab engine (error=") + matlab_engine_->geterror() + std::string(")"));
        error(std::string("InterfaceWithMatlab: This is an internal communication error, make sure that the portnumber is correct"));
        error(std::string("InterfaceWithMatlab: If address information is correct, this most probably points to a bug in the SCIRun software"));

        matlab_engine_ = 0;
        file_transfer_ = 0;

        return(false);
      }


      if (packet->gettag() == TAG_MERROR)
      {
        matlab_engine_->close();
        file_transfer_->close();

        error(std::string("InterfaceWithMatlab: InterfaceWithMatlab engine returned an error (error=") + packet->getstring() + std::string(")"));
        error(std::string("InterfaceWithMatlab: Please check whether '[MATLAB_DIRECTORY]/services/matlabengine.rc' has been setup properly"));
        error(std::string("InterfaceWithMatlab: Press the 'Edit Local Config of Matlab Engine' to change the configuration"));
        error(std::string("InterfaceWithMatlab: Edit the 'startmatlab=' line to start matlab properly"));
        error(std::string("InterfaceWithMatlab: If you running matlab remotely, this file must be edited on the machine running matlab"));

        matlab_engine_ = 0;
        file_transfer_ = 0;

        return(false);
      }

      thread_info_ = new InterfaceWithMatlabEngineThreadInfo();
      if (thread_info_.get_rep() == 0)
      {
        matlab_engine_->close();
        file_transfer_->close();

        error(std::string("InterfaceWithMatlab: Could not create thread information object"));
        matlab_engine_ = 0;
        file_transfer_ = 0;

        return(false);
      }

      thread_info_->output_cmd_ = get_id()+" AddOutput";

      // By cloning the object, it will have the same fields and sockets, but the socket
      // and error handling will be separate. As the thread will invoke its own instructions
      // it is better to have a separate copy. Besides, the socket obejct will point to the
      // same underlying socket. Hence only the error handling part will be duplicated

      ServiceClientHandle matlab_engine_copy = matlab_engine_->clone();
      InterfaceWithMatlabEngineThread* enginethread = new InterfaceWithMatlabEngineThread(matlab_engine_copy,thread_info_);
      if (enginethread == 0)
      {
        matlab_engine_->close();
        file_transfer_->close();

        matlab_engine_ = 0;
        file_transfer_ = 0;

        error(std::string("InterfaceWithMatlab: Could not create thread object"));
        return(false);
      }

      Thread* thread = new Thread(enginethread,"InterfaceWithMatlab module thread");
      if (thread == 0)
      {
        delete enginethread;
        matlab_engine_->close();
        file_transfer_->close();

        matlab_engine_ = 0;
        file_transfer_ = 0;

        error(std::string("InterfaceWithMatlab: Could not create thread"));
        return(false);
      }
      thread->detach();

      int sessionn = packet->getparam1();
      matlab_engine_->setsession(sessionn);

      std::string sharehomedir = "yes";
      if (need_file_transfer_) sharehomedir = "no";

      // TODO: stream
      std::string status = "Matlab engine running\n\nmatlabengine version: " + matlab_engine_->getversion() + "\nmatlabengine address: " +
        matlab_engine_->getremoteaddress() + "\nmatlabengine session: " + matlab_engine_->getsession() + "\nmatlabengine filetransfer version :" +
        file_transfer_->getversion() + "\nshared home directory: " + sharehomedir + "\nlocal temp directory: " + file_transfer_->local_file("") +
        "\nremote temp directory: " + file_transfer_->remote_file("") + "\n";
#else
      std::string status = "InterfaceWithMatlab engine running\n";
#endif
      update_status(status);
    }

    return(true);
  }

#if 0

  bool InterfaceWithMatlab::close_matlab_engine()
  {
#ifndef USE_MATLAB_ENGINE_LIBRARY
    if (matlab_engine_.get_rep())
    {
      matlab_engine_->close();
      matlab_engine_ = 0;
    }
    if (file_transfer_.get_rep())
    {
      file_transfer_->close();
      file_transfer_ = 0;
    }

    if (thread_info_.get_rep())
    {
      thread_info_->dolock();
      if (thread_info_->exit_ == false)
      {
        thread_info_->exit_ = true;
        thread_info_->wait_exit_.conditionBroadcast();
      }
      thread_info_->unlock();
      thread_info_ = 0;
    }
#else
    if (engine_) {
      engClose(engine_);
      engine_ = 0;
    }
#endif

    return(true);
  }


  bool InterfaceWithMatlab::load_output_matrices()
  {
    int port = 0;
    try
    {
      for (int p = 0; p < NUM_MATRIX_PORTS; p++)
      {
        if (!oport_connected(port)) { port++; continue; }

        // Test whether the matrix port exists
        if (output_matrix_name_list_[p] == "") continue;
        if (output_matrix_matfile_[p] == "") continue;

        matlabfile mf;
        matlabarray ma;

        try
        {
          if (need_file_transfer_) file_transfer_->get_file(file_transfer_->remote_file(output_matrix_matfile_[p]),file_transfer_->local_file(output_matrix_matfile_[p]));
          mf.open(file_transfer_->local_file(output_matrix_matfile_[p]),"r");
          ma = mf.getmatlabarray(output_matrix_name_list_[p]);
          mf.close();
        }
        catch(...)
        {
          error("InterfaceWithMatlab: Could not read output matrix");
          continue;
        }

        if (ma.isempty())
        {
          error("InterfaceWithMatlab: Could not read output matrix");
          continue;
        }

        MatrixHandle handle;
        std::string info;
        matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter *>(this));
        if (translate.sciMatrixCompatible(ma,info)) translate.mlArrayTOsciMatrix(ma,handle);
        send_output_handle(port,handle,true); port++;
      }


      for (int p = 0; p < NUM_FIELD_PORTS; p++)
      {
        if (!oport_connected(port)) { port++; continue; }

        // Test whether the field port exists
        if (output_field_name_list_[p] == "") continue;
        if (output_field_matfile_[p] == "") continue;

        matlabfile mf;
        matlabarray ma;
        try
        {
          if (need_file_transfer_) file_transfer_->get_file(file_transfer_->remote_file(output_field_matfile_[p]),file_transfer_->local_file(output_field_matfile_[p]));

          mf.open(file_transfer_->local_file(output_field_matfile_[p]),"r");
          ma = mf.getmatlabarray(output_field_name_list_[p]);
          mf.close();
        }
        catch(...)
        {
          error("InterfaceWithMatlab: Could not read output matrix");
          continue;
        }

        if (ma.isempty())
        {
          error("InterfaceWithMatlab: Could not read output matrix");
          continue;
        }

        FieldHandle handle;
        std::string info;
        matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter *>(this));
        if (translate.sciFieldCompatible(ma,info)) translate.mlArrayTOsciField(ma,handle);
        send_output_handle(port,handle,true); port++;
      }


      for (int p = 0; p < NUM_NRRD_PORTS; p++)
      {
        if (!oport_connected(port)) { port++; continue; }

        // Test whether the nrrd port exists
        if (output_nrrd_name_list_[p] == "") continue;
        if (output_nrrd_matfile_[p] == "") continue;

        matlabfile mf;
        matlabarray ma;
        try
        {
          if (need_file_transfer_) file_transfer_->get_file(file_transfer_->remote_file(output_nrrd_matfile_[p]),file_transfer_->local_file(output_nrrd_matfile_[p]));
          mf.open(file_transfer_->local_file(output_nrrd_matfile_[p]),"r");
          ma = mf.getmatlabarray(output_nrrd_name_list_[p]);
          mf.close();
        }
        catch(...)
        {
          error("InterfaceWithMatlab: Could not read output matrix");
          continue;
        }

        if (ma.isempty())
        {
          error("InterfaceWithMatlab: Could not read output matrix");
          continue;
        }

        NrrdDataHandle handle;
        std::string info;
        matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter *>(this));
        if (translate.sciNrrdDataCompatible(ma,info)) translate.mlArrayTOsciNrrdData(ma,handle);
        send_output_handle(port,handle,true); port++;
      }


      for (int p = 0; p < NUM_STRING_PORTS; p++)
      {
        if (!oport_connected(port)) { port++; continue; }

        // Test whether the nrrd port exists
        if (output_string_name_list_[p] == "") continue;
        if (output_string_matfile_[p] == "") continue;

        matlabfile mf;
        matlabarray ma;
        try
        {
          if (need_file_transfer_) file_transfer_->get_file(file_transfer_->remote_file(output_string_matfile_[p]),file_transfer_->local_file(output_string_matfile_[p]));
          mf.open(file_transfer_->local_file(output_string_matfile_[p]),"r");
          ma = mf.getmatlabarray(output_string_name_list_[p]);
          mf.close();
        }
        catch(...)
        {
          error("InterfaceWithMatlab: Could not read output matrix");
          continue;
        }

        if (ma.isempty())
        {
          error("InterfaceWithMatlab: Could not read output matrix");
          continue;
        }

        StringHandle handle;
        std::string info;
        matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter *>(this));
        if (translate.sciStringCompatible(ma,info)) translate.mlArrayTOsciString(ma,handle);
        send_output_handle(port,handle,true); port++;
      }
    }
    catch(...)
    {
      return(false);
    }
    return(true);
  }

  bool InterfaceWithMatlab::generate_matlab_code()
  {
    try
    {
      std::ofstream m_file;

      mfile_ = std::string("scirun_code.m");
      std::string filename = file_transfer_->local_file(mfile_);
      m_file.open(filename.c_str(),std::ios::app);

      int port = 0;
      m_file << matlab_code_list_ << "\n";
      for (int p = 0; p < NUM_MATRIX_PORTS; p++)
      {
        // Test whether the matrix port exists
        if (!oport_connected(port)) { port++; continue; }
        port++;
        if (output_matrix_name_list_[p] == "") continue;

        std::ostringstream oss;
        oss << "output_matrix" << p << ".mat";
        output_matrix_matfile_[p] = oss.str();
        std::string cmd;
        cmd = "if exist('" + output_matrix_name_list_[p] + "','var'), save " + file_transfer_->remote_file(output_matrix_matfile_[p]) + " " + output_matrix_name_list_[p] + "; end\n";
        m_file << cmd;
      }

      for (int p = 0; p < NUM_FIELD_PORTS; p++)
      {
        // Test whether the matrix port exists
        if (!oport_connected(port)) { port++; continue; }
        port++;
        if (output_field_name_list_[p] == "") continue;

        std::ostringstream oss;
        oss << "output_field" << p << ".mat";
        output_field_matfile_[p] = oss.str();
        std::string cmd;
        cmd = "if exist('" + output_field_name_list_[p] + "','var'), save " + file_transfer_->remote_file(output_field_matfile_[p]) + " " + output_field_name_list_[p] + "; end\n";
        m_file << cmd;
      }

      for (int p = 0; p < NUM_NRRD_PORTS; p++)
      {
        // Test whether the matrix port exists
        if (!oport_connected(port)) { port++; continue; }
        port++;
        if (output_nrrd_name_list_[p] == "") continue;

        std::ostringstream oss;

        oss << "output_nrrd" << p << ".mat";
        output_nrrd_matfile_[p] = oss.str();
        std::string cmd;
        cmd = "if exist('" + output_nrrd_name_list_[p] + "','var'), save " + file_transfer_->remote_file(output_nrrd_matfile_[p]) + " " + output_nrrd_name_list_[p] + "; end\n";
        m_file << cmd;
      }

      for (int p = 0; p < NUM_STRING_PORTS; p++)
      {
        // Test whether the matrix port exists
        if (!oport_connected(port)) { port++; continue; }
        port++;
        if (output_string_name_list_[p] == "") continue;

        std::ostringstream oss;

        oss << "output_string" << p << ".mat";
        output_string_matfile_[p] = oss.str();
        std::string cmd;
        cmd = "if exist('" + output_string_name_list_[p] + "','var'), save " + file_transfer_->remote_file(output_string_matfile_[p]) + " " + output_string_name_list_[p] + "; end\n";
        m_file << cmd;
      }

      m_file.close();

      if (need_file_transfer_) file_transfer_->put_file(file_transfer_->local_file(mfile_),file_transfer_->remote_file(mfile_));
    }
    catch(...)
    {
      return(false);
    }

    return(true);
  }


  bool InterfaceWithMatlab::save_input_matrices()
  {
    try
    {
      int port = 0;

      std::ofstream m_file;
      std::string loadcmd;

      mfile_ = std::string("scirun_code.m");
      std::string filename = file_transfer_->local_file(mfile_);

      m_file.open(filename.c_str(),std::ios::out);

      for (int p = 0; p < NUM_MATRIX_PORTS; p++)
      {
        MatrixHandle	handle;
        if(!(get_input_handle(port,handle,false))) { port++; continue; }
        port++;

        // if there is no data
        if (handle.get_rep() == 0)
        {
          // we do not need the old file any more so delete it
          input_matrix_matfile_[p] = "";
          continue;
        }
        // if the data as the same before
        // do nothing
        if ((input_matrix_name_list_[p]==input_matrix_name_list_old_[p])&&
          (input_matrix_type_list_[p]==input_matrix_type_list_old_[p])&&
          (input_matrix_array_list_[p]==input_matrix_array_list_old_[p])&&
          (handle->generation == input_matrix_generation_old_[p]))
        {
          // this one was not created again
          // hence we do not need to translate it again
          // with big datasets this should improve performance
          loadcmd = "load " + file_transfer_->remote_file(input_matrix_matfile_[p]) + ";\n";
          m_file << loadcmd;
          continue;
        }

        // Create a new filename for the input matrix
        std::ostringstream oss;
        oss << "input_matrix" << p << ".mat";
        input_matrix_matfile_[p] = oss.str();

        matlabfile mf;
        matlabarray ma;

        mf.open(file_transfer_->local_file(input_matrix_matfile_[p]),"w");
        mf.setheadertext("InterfaceWithMatlab V5 compatible file generated by SCIRun [module InterfaceWithMatlab version 1.3]");

        matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter *>(this));
        translate.converttostructmatrix();
        if (input_matrix_array_list_[p] == "numeric array")
          translate.converttonumericmatrix();
        translate.setdatatype(convertdataformat(input_matrix_type_list_[p]));
        translate.sciMatrixTOmlArray(handle,ma);

        mf.putmatlabarray(ma,input_matrix_name_list_[p]);
        mf.close();

        loadcmd = "load " + file_transfer_->remote_file(input_matrix_matfile_[p]) + ";\n";
        m_file << loadcmd;

        if (need_file_transfer_)
        {
          if(!(file_transfer_->put_file(
            file_transfer_->local_file(input_matrix_matfile_[p]),
            file_transfer_->remote_file(input_matrix_matfile_[p]))))
          {
            error("InterfaceWithMatlab: Could not transfer file");
            std::string err = "Error :" + file_transfer_->geterror();
            error(err);
            return(false);
          }
        }

        input_matrix_type_list_old_[p] = input_matrix_type_list_[p];
        input_matrix_array_list_old_[p] = input_matrix_array_list_[p];
        input_matrix_name_list_old_[p] = input_matrix_name_list_[p];
        input_matrix_generation_old_[p] = handle->generation;

      }

      for (int p = 0; p < NUM_FIELD_PORTS; p++)
      {
        FieldHandle	handle;
        if(!(get_input_handle(port,handle,false))) { port++; continue; }
        port++;

        // if there is no data
        if (handle.get_rep() == 0)
        {
          // we do not need the old file any more so delete it
          input_field_matfile_[p] = "";
          continue;
        }
        // if the data as the same before
        // do nothing
        if ((input_field_name_list_[p]==input_field_name_list_old_[p])&&
          (input_field_array_list_[p]==input_field_array_list_old_[p])&&
          (handle->generation == input_field_generation_old_[p]))
        {
          // this one was not created again
          // hence we do not need to translate it again
          // with big datasets this should improve performance
          loadcmd = "load " + file_transfer_->remote_file(input_field_matfile_[p]) + ";\n";
          m_file << loadcmd;

          continue;
        }

        // Create a new filename for the input matrix
        std::ostringstream oss;
        oss << "input_field" << p << ".mat";
        input_field_matfile_[p] = oss.str();

        matlabfile mf;
        matlabarray ma;

        mf.open(file_transfer_->local_file(input_field_matfile_[p]),"w");
        mf.setheadertext("InterfaceWithMatlab V5 compatible file generated by SCIRun [module InterfaceWithMatlab version 1.3]");

        matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter *>(this));
        translate.converttostructmatrix();

        if (input_field_array_list_[p] == "numeric array")
        {
          translate.converttonumericmatrix();
        }
        translate.sciFieldTOmlArray(handle,ma);

        mf.putmatlabarray(ma,input_field_name_list_[p]);
        mf.close();

        loadcmd = "load " + file_transfer_->remote_file(input_field_matfile_[p]) + ";\n";
        m_file << loadcmd;

        if (need_file_transfer_)
        {
          if(!(file_transfer_->put_file(
            file_transfer_->local_file(input_field_matfile_[p]),
            file_transfer_->remote_file(input_field_matfile_[p]))))
          {
            error("InterfaceWithMatlab: Could not transfer file");
            std::string err = "Error :" + file_transfer_->geterror();
            error(err);
            return(false);
          }
        }
        input_field_array_list_old_[p] = input_field_array_list_[p];
        input_field_name_list_old_[p] = input_field_name_list_[p];
        input_field_generation_old_[p] = handle->generation;
      }

      for (int p = 0; p < NUM_NRRD_PORTS; p++)
      {
        NrrdDataHandle	handle;
        if(!(get_input_handle(port,handle,false))) { port++; continue; }
        port++;

        // if there is no data
        if (handle.get_rep() == 0)
        {
          // we do not need the old file any more so delete it
          input_nrrd_matfile_[p] = "";
          continue;
        }
        // if the data as the same before
        // do nothing
        if ((input_nrrd_name_list_[p]==input_nrrd_name_list_old_[p])&&
          (input_nrrd_type_list_[p]==input_nrrd_type_list_old_[p])&&
          (input_nrrd_array_list_[p]==input_nrrd_array_list_old_[p])&&
          (handle->generation == input_nrrd_generation_old_[p]))
        {
          // this one was not created again
          // hence we do not need to translate it again
          // with big datasets this should improve performance
          loadcmd = "load " + file_transfer_->remote_file(input_nrrd_matfile_[p]) + ";\n";
          m_file << loadcmd;

          continue;
        }

        // Create a new filename for the input matrix
        std::ostringstream oss;
        oss << "input_nrrd" << p << ".mat";
        input_nrrd_matfile_[p] = oss.str();

        matlabfile mf;
        matlabarray ma;

        mf.open(file_transfer_->local_file(input_nrrd_matfile_[p]),"w");
        mf.setheadertext("InterfaceWithMatlab V5 compatible file generated by SCIRun [module InterfaceWithMatlab version 1.3]");

        matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter *>(this));
        translate.converttostructmatrix();
        if (input_nrrd_array_list_[p] == "numeric array") translate.converttonumericmatrix();
        translate.setdatatype(convertdataformat(input_nrrd_type_list_[p]));
        translate.sciNrrdDataTOmlArray(handle,ma);
        mf.putmatlabarray(ma,input_nrrd_name_list_[p]);
        mf.close();

        loadcmd = "load " + file_transfer_->remote_file(input_nrrd_matfile_[p]) + ";\n";
        m_file << loadcmd;

        if (need_file_transfer_)
        {
          if(!(file_transfer_->put_file(
            file_transfer_->local_file(input_nrrd_matfile_[p]),
            file_transfer_->remote_file(input_nrrd_matfile_[p]))))
          {
            error("InterfaceWithMatlab: Could not transfer file");
            std::string err = "Error :" + file_transfer_->geterror();
            error(err);
            return(false);
          }
        }
        input_nrrd_type_list_old_[p] = input_nrrd_type_list_[p];
        input_nrrd_array_list_old_[p] = input_nrrd_array_list_[p];
        input_nrrd_name_list_old_[p] = input_nrrd_name_list_[p];
        input_nrrd_generation_old_[p] = handle->generation;
      }

      for (int p = 0; p < NUM_STRING_PORTS; p++)
      {
        StringHandle	handle;
        if(!(get_input_handle(port,handle,false))) { port++; continue; }
        port++;

        // if there is no data
        if (handle.get_rep() == 0)
        {
          // we do not need the old file any more so delete it
          input_string_matfile_[p].clear();
          continue;
        }
        // if the data as the same before
        // do nothing
        if ((input_string_name_list_[p]==input_string_name_list_old_[p])&&
          (handle->generation == input_string_generation_old_[p]))
        {
          // this one was not created again
          // hence we do not need to translate it again
          // with big datasets this should improve performance
          loadcmd = "load " + file_transfer_->remote_file(input_string_matfile_[p]) + ";\n";
          m_file << loadcmd;

          continue;
        }

        // Create a new filename for the input matrix
        std::ostringstream oss;
        oss << "input_string" << p << ".mat";
        input_string_matfile_[p] = oss.str();

        matlabfile mf;
        matlabarray ma;

        mf.open(file_transfer_->local_file(input_string_matfile_[p]),"w");
        mf.setheadertext("InterfaceWithMatlab V5 compatible file generated by SCIRun [module InterfaceWithMatlab version 1.3]");

        matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter *>(this));
        translate.sciStringTOmlArray(handle,ma);
        mf.putmatlabarray(ma,input_string_name_list_[p]);
        mf.close();

        loadcmd = "load " + file_transfer_->remote_file(input_string_matfile_[p]) + ";\n";
        m_file << loadcmd;

        if (need_file_transfer_)
        {
          if(!(file_transfer_->put_file(
            file_transfer_->local_file(input_string_matfile_[p]),
            file_transfer_->remote_file(input_string_matfile_[p]))))
          {
            error("InterfaceWithMatlab: Could not transfer file");
            std::string err = "Error :" + file_transfer_->geterror();
            error(err);
            return(false);
          }

        }
        input_string_name_list_old_[p] = input_string_name_list_[p];
        input_string_generation_old_[p] = handle->generation;
      }

    }
    catch (matlabfile::could_not_open_file)
    {   // Could not open the temporary file
      error("InterfaceWithMatlab: Could not open temporary matlab file");
      return(false);
    }
    catch (matlabfile::io_error)
    {   // IO error from ferror
      error("InterfaceWithMatlab: IO error");
      return(false);
    }
    catch (matlabfile::matfileerror)
    {   // All other errors are classified as internal
      // matfileerrror is the base class on which all
      // other exceptions are based.
      error("InterfaceWithMatlab: Internal error in writer");
      return(false);
    }

    return(true);
  }
#endif

  bool InterfaceWithMatlabImpl::create_temp_directory()
  {
    if (temp_directory_.empty())
    {
      return (tfmanager_.create_tempdir("matlab-engine.XXXXXX",temp_directory_));
    }
    return(true);
  }

#if 0
  bool InterfaceWithMatlab::delete_temp_directory()
  {
    if(temp_directory_ != "") tfmanager_.delete_tempdir(temp_directory_);
    temp_directory_ = "";
    return(true);
  }

  std::string InterfaceWithMatlab::totclstring(std::string &instring)
  {
    int strsize = instring.size();
    int specchar = 0;
    for (int p = 0; p < strsize; p++)
      if ((instring[p]=='\n')||(instring[p]=='\t')||(instring[p]=='\b')||(instring[p]=='\r')||(instring[p]=='{')||(instring[p]=='}')
        ||(instring[p]=='[')||(instring[p]==']')||(instring[p]=='\\')||(instring[p]=='$')||(instring[p]=='"')) specchar++;

    std::string newstring;
    newstring.resize(strsize+specchar);
    int q = 0;
    for (int p = 0; p < strsize; p++)
    {
      if (instring[p]=='\n') { newstring[q++] = '\\'; newstring[q++] = 'n'; continue; }
      if (instring[p]=='\t') { newstring[q++] = '\\'; newstring[q++] = 't'; continue; }
      if (instring[p]=='\b') { newstring[q++] = '\\'; newstring[q++] = 'b'; continue; }
      if (instring[p]=='\r') { newstring[q++] = '\\'; newstring[q++] = 'r'; continue; }
      if (instring[p]=='{')  { newstring[q++] = '\\'; newstring[q++] = '{'; continue; }
      if (instring[p]=='}')  { newstring[q++] = '\\'; newstring[q++] = '}'; continue; }
      if (instring[p]=='[')  { newstring[q++] = '\\'; newstring[q++] = '['; continue; }
      if (instring[p]==']')  { newstring[q++] = '\\'; newstring[q++] = ']'; continue; }
      if (instring[p]=='\\') { newstring[q++] = '\\'; newstring[q++] = '\\'; continue; }
      if (instring[p]=='$')  { newstring[q++] = '\\'; newstring[q++] = '$'; continue; }
      if (instring[p]=='"')  { newstring[q++] = '\\'; newstring[q++] = '"'; continue; }
      newstring[q++] = instring[p];
    }

    return(newstring);
  }

  void InterfaceWithMatlab::tcl_command(GuiArgs& args, void* userdata)
  {
    if (args.count() > 1)
    {
      if (args[1] == "keystroke")
      {
        std::string str = args[2];
        if (str.size() == 1)
        {
          if (str[0] == '\r') str[0] = '\n';

          if (str[0] == '\b')
          {
            inputstring_ = inputstring_.substr(0,(inputstring_.size()-1));
          }
          else
          {
            inputstring_ += str;
          }

          if (str[0] == '\n')
          {
            if(!(send_input(inputstring_)))
            {
              error("InterfaceWithMatlab: Could not close matlab engine");
              return;
            }
            inputstring_ = "";
          }
        }
        else
        {
          std::string key = args[3];
          if (key == "Enter")
          {
            str = "\n";
            inputstring_ += str;
          }
          else if (key == "BackSpace")
          {
            inputstring_ = inputstring_.substr(0,(inputstring_.size()-1));
          }
          else if (key == "Tab") str = "\t";
          else if (key == "Return") str ="\r";

          if (str.size() == 1)
          {
            if (str[0] == '\n')
            {
              if(!(send_input(inputstring_)))
              {
                error("InterfaceWithMatlab: Could not close matlab engine");
                return;
              }
              inputstring_ = "";
            }
          }

        }
        return;
      }
      if (args[1] == "disconnect")
      {
        get_ctx()->reset();
        if(!(close_matlab_engine()))
        {
          error("InterfaceWithMatlab: Could not close matlab engine");
          return;
        }
        update_status("InterfaceWithMatlab engine not running\n");
        return;
      }


      if (args[1] == "connect")
      {

        if(!(close_matlab_engine()))
        {
          error("InterfaceWithMatlab: Could not close matlab engine");
          return;
        }

        update_status("InterfaceWithMatlab engine not running\n");

        // Synchronise input: translate TCL lists into C++ STL lists
        if (!(synchronise_input()))
        {
          error("InterfaceWithMatlab: Could not retreive GUI input");
          return;
        }

        // If we haven't created a temporary directory yet
        // we open one to store all temp files in
        if (!(create_temp_directory()))
        {
          error("InterfaceWithMatlab: Could not create temporary directory");
          return;
        }

        if (!(open_matlab_engine()))
        {
          error("InterfaceWithMatlab: Could not open matlab engine");
          return;
        }
        return;
      }

      if (args[1] == "configfile")
      {
        ServiceDBHandle servicedb = new ServiceDB;
        // load all services and find all makers
        servicedb->loadpackages();

        ServiceInfo *si = servicedb->getserviceinfo("matlabengine");
        configfile_.set(si->rcfile);
        reset_vars();
        return;
      }
    }

    Module::tcl_command(args, userdata);
  }
#endif
