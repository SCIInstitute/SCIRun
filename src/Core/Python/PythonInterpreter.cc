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

/// @todo Documentation Core/Python/PythonInterpreter.cc

#ifdef BUILD_WITH_PYTHON
#ifdef _MSC_VER
//#pragma warning( push )
#pragma warning( disable: 4244 )
#endif

#include <Python.h>
#include <boost/python.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/preprocessor.hpp>
#include <string>
#include <vector>
#include <boost/thread/condition_variable.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>

#include <Core/Application/Application.h>
#include <Core/Utils/StringContainer.h>
#include <Core/Utils/Lockable.h>
#include <Core/Utils/Legacy/StringUtil.h>

#include <Core/Python/PythonInterpreter.h>

#include <Dataflow/Engine/Python/SCIRunPythonModule.h>


//#ifdef _MSC_VER
//#pragma warning( pop )
//#endif

//#include <Interface/PythonTestGui/Python/ToPythonConverters.h>

using namespace SCIRun::Core;

namespace SCIRun 
{
namespace Core
{

class PythonInterpreterPrivate : public Lockable
{
public:
  typedef std::pair< std::string, PyObject* ( * )( void ) > module_entry_type;
  typedef std::list< module_entry_type > module_list_type;
	
  std::string read_from_console( const int bytes = -1 );

  const wchar_t* programName() const { return !program_name_.empty() ? &program_name_[0] : L"<unset>"; }
  void setProgramName(const std::vector<wchar_t>& name) 
  { 
    program_name_ = name; 
    //std::wcout << "PROGRAM NAME SET TO: " << programName() << std::endl;
  }

	// A list of Python extension modules that need to be initialized
	module_list_type modules_;
	// An instance of python CommandCompiler object (defined in codeop.py)
	boost::python::object compiler_;
	// The context of the Python main module.
	boost::python::object globals_;
	// Whether the Python interpreter has been initialized.
	bool initialized_;
	bool terminal_running_;
	// The input buffer
	std::string input_buffer_;
	// Whether the interpreter is waiting for input
	bool waiting_for_input_;
	// The command buffer (for Python statements that span over multiple lines)
	std::string command_buffer_;
	// Python sys.ps1
	std::string prompt1_;
	// Python sys.ps2
	std::string prompt2_;

	// Condition variable to make sure the PythonInterpreter thread has 
	// completed initialization before continuing the main thread.
	boost::condition_variable thread_condition_variable_;
private:
  // The name of the executable
  std::vector< wchar_t > program_name_;
};

std::string PythonInterpreterPrivate::read_from_console( const int bytes /*= -1 */ )
{
	lock_type lock( this->get_mutex() );

	std::string result;
	if ( !input_buffer_.empty() )
	{
		if ( bytes <= 0 )
		{
			result = input_buffer_;
			input_buffer_.clear();
		}
		else
		{
			result = input_buffer_.substr( 0, bytes );
			if ( bytes < static_cast< int >( input_buffer_.size() ) )
			{
				input_buffer_ = input_buffer_.substr( bytes );
			}
			else
			{
				input_buffer_.clear();
			}
		}
	}

	int more_bytes = bytes - static_cast< int >( result.size() );
	while ( ( bytes <= 0 && result.empty() ) ||
		( bytes > 0 && more_bytes > 0 ) )
	{
		this->waiting_for_input_ = true;
		this->thread_condition_variable_.wait( lock );

		// Abort reading if an interrupt signal has been received.
		if ( PyErr_CheckSignals() != 0 ) break;

		if ( bytes <= 0 )
		{
			result = input_buffer_;
			input_buffer_.clear();
		}
		else
		{
			result += input_buffer_.substr( 0, more_bytes );
			if ( more_bytes < static_cast< int >( input_buffer_.size() ) )
			{
				input_buffer_ = input_buffer_.substr( more_bytes );
			}
			else
			{
				input_buffer_.clear();
			}
		}

		more_bytes = bytes - static_cast< int >( result.size() );
	}

	this->waiting_for_input_ = false;
	return result;
}

}}


//////////////////////////////////////////////////////////////////////////
// Class PythonTerminal
//////////////////////////////////////////////////////////////////////////
class PythonStdIO
{
public:
	boost::python::object read( int n )
	{
		std::string data = PythonInterpreter::Instance().private_->read_from_console( n );
		boost::python::str pystr( data.c_str() );
		return pystr.encode();
	}

	std::string readline()
	{
		return PythonInterpreter::Instance().private_->read_from_console();
	}

	int write( const std::string& data )
	{
		PythonInterpreter::Instance().output_signal_( data );
		return static_cast< int >( data.size() );
	}
};

class PythonStdErr
{
public:
	int write( const std::string& data )
	{
		PythonInterpreter::Instance().error_signal_( data );
		return static_cast< int >( data.size() );
	}
};

BOOST_PYTHON_MODULE( interpreter )
{
	boost::python::class_< PythonStdIO >( "terminalio" )
		.def( "read", &PythonStdIO::read )
		.def( "readline", &PythonStdIO::readline )
		.def( "write", &PythonStdIO::write );

	boost::python::class_< PythonStdErr >( "terminalerr" )
		.def( "write", &PythonStdErr::write );
}

CORE_SINGLETON_IMPLEMENTATION( PythonInterpreter );
	
PythonInterpreter::PythonInterpreter() :
	//Core::EventHandler(),
	private_( new PythonInterpreterPrivate )
{
	this->private_->initialized_ = false;
	this->private_->terminal_running_ = false;
	this->private_->waiting_for_input_ = false;
	//this->private_->action_context_.reset( new PythonActionContext );
//  initialize_eventhandler();
}

PythonInterpreter::~PythonInterpreter()
{
	// NOTE: Boost.Python requires that we don't call Py_Finalize
	//Py_Finalize();
}

//#define PRINT_PY_INIT_DEBUG(n) std::cout << "ev_" << (n) << std::endl;
#define PRINT_PY_INIT_DEBUG(n)


void PythonInterpreter::initialize_eventhandler()
{
  PRINT_PY_INIT_DEBUG(1);
	using namespace boost::python;

	PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
  PRINT_PY_INIT_DEBUG(2);
	// Register C++ to Python type converters
	//RegisterToPythonConverters();

  // Add the extension modules
  PyImport_AppendInittab( "interpreter", PyInit_interpreter );
  PyImport_AppendInittab( "SCIRunPythonAPI", PyInit_SCIRunPythonAPI );
  for ( module_list_type::iterator it = this->private_->modules_.begin();
       it != this->private_->modules_.end(); ++it )
  {
    PyImport_AppendInittab( ( *it ).first.c_str(), ( *it ).second );
  }
  PRINT_PY_INIT_DEBUG(3);
  std::wcerr << "initialize_eventhandler: program name=" << this->private_->programName() << std::endl;
  Py_SetProgramName(const_cast< wchar_t* >(this->private_->programName()));

  PRINT_PY_INIT_DEBUG(4);
  //std::wcout << "lib_path: " << lib_path.wstring() << std::endl;
  std::wstringstream lib_paths;
#if defined( _WIN32 )
  const std::wstring PATH_SEP(L";");
#else
  const std::wstring PATH_SEP(L":");
#endif
  PRINT_PY_INIT_DEBUG(5);
#if defined( __APPLE__ )
  boost::filesystem::path lib_path = Application::Instance().executablePath().parent_path();
  std::vector<boost::filesystem::path> lib_path_list;
  // relative paths
  lib_path_list.push_back(lib_path.parent_path() / boost::filesystem::path("Frameworks") / PYTHONPATH);
  lib_path_list.push_back(lib_path / PYTHONPATH);

  // for test executable
  if ( boost::filesystem::path(Application::Instance().parameters()->entireCommandLine()).stem() == "SCIRun_test" )
  {
    boost::filesystem::path full_lib_path(PYTHONLIBDIR);
    full_lib_path /= PYTHONLIB;
    lib_path_list.push_back(full_lib_path);
  }

  for ( size_t i = 0; i < lib_path_list.size(); ++i )
  {
    auto path = lib_path_list[i];
    boost::filesystem::path plat_lib_path = path / "plat-darwin";
    boost::filesystem::path dynload_lib_path = path / "lib-dynload";
    boost::filesystem::path site_lib_path = path / "site-packages";
    if (i > 0)
    {
      lib_paths << PATH_SEP;
    }
    lib_paths << path.wstring() << PATH_SEP
              << plat_lib_path.wstring() << PATH_SEP
              << dynload_lib_path.wstring() << PATH_SEP
              << site_lib_path.wstring();
  }

  Py_SetPath( lib_paths.str().c_str() );
#elif defined (_WIN32)
  boost::filesystem::path lib_path = Application::Instance().executablePath();
  boost::filesystem::path top_lib_path = lib_path / PYTHONPATH / PYTHONNAME;
  //std::cout << "top_lib_path: " << top_lib_path.string() << std::endl;
  boost::filesystem::path dynload_lib_path = top_lib_path / "lib-dynload";
  //std::cout << "dynload_lib_path: " << dynload_lib_path.string() << std::endl;
  boost::filesystem::path site_lib_path = top_lib_path / "site-packages";
  //std::cout << "site_lib_path: " << site_lib_path.string() << std::endl;
  lib_paths << top_lib_path.wstring() << PATH_SEP
            << site_lib_path.wstring();
  //std::wcout << "lib_paths final: " << lib_paths.str() << std::endl;
  Py_SetPath( lib_paths.str().c_str() );
  PRINT_PY_INIT_DEBUG(6);
#else
  // linux...
  boost::filesystem::path lib_path = Application::Instance().executablePath();
  boost::filesystem::path top_lib_path = lib_path / PYTHONPATH;
  boost::filesystem::path dynload_lib_path = top_lib_path / "lib-dynload";
  boost::filesystem::path site_lib_path = top_lib_path / "site-packages";
  boost::filesystem::path plat_lib_path = top_lib_path / "plat-linux";
  lib_paths << top_lib_path.wstring() << PATH_SEP
            << plat_lib_path.wstring() << PATH_SEP
            << dynload_lib_path.wstring() << PATH_SEP
            << site_lib_path.wstring();
  Py_SetPath( lib_paths.str().c_str() );
#endif

  // TODO: remove debug print when confident python initialization is stable
  std::wcerr << lib_paths.str() << std::endl;
  PRINT_PY_INIT_DEBUG(7);
  Py_IgnoreEnvironmentFlag = 1;
  Py_InspectFlag = 1;
  Py_OptimizeFlag = 2;
#if !defined( _WIN32 )
  Py_NoSiteFlag = 1;
#endif
  Py_Initialize();
  PRINT_PY_INIT_DEBUG(8);
	// Create the compiler object
	PyRun_SimpleString( "from codeop import CommandCompiler\n"
		"__internal_compiler = CommandCompiler()\n" );
 	boost::python::object main_module = boost::python::import( "__main__" );
 	boost::python::object main_namespace = main_module.attr( "__dict__" );
	this->private_->compiler_ = main_namespace[ "__internal_compiler" ];
	this->private_->globals_ = main_namespace;
  PRINT_PY_INIT_DEBUG(9);
	// Set up the prompt strings
	PyRun_SimpleString( "import sys\n"
		"try:\n"
		"\tsys.ps1\n"
		"except AttributeError:\n"
		"\tsys.ps1 = \">>> \"\n"
		"try:\n"
		"\tsys.ps2\n"
		"except AttributeError:\n"
		"\tsys.ps2 = \"... \"\n" );
	boost::python::object sys_module = main_namespace[ "sys" ];
	boost::python::object sys_namespace = sys_module.attr( "__dict__" );
	this->private_->prompt1_ = boost::python::extract< std::string >( sys_namespace[ "ps1" ] );
	this->private_->prompt2_ = boost::python::extract< std::string >( sys_namespace[ "ps2" ] );
  PRINT_PY_INIT_DEBUG(10);
	// Hook up the I/O
	PyRun_SimpleString( "import interpreter\n"
		"__term_io = interpreter.terminalio()\n"
		"__term_err = interpreter.terminalerr()\n" );
	PyRun_SimpleString( "import sys\n" 
		"sys.stdin = __term_io\n"
		"sys.stdout = __term_io\n"
		"sys.stderr = __term_err\n" );
  PRINT_PY_INIT_DEBUG(11);
	// Remove intermediate python variables
	PyRun_SimpleString( "del (interpreter, __internal_compiler, __term_io, __term_err)\n" );
  PRINT_PY_INIT_DEBUG(12);
	//this->private_->thread_condition_variable_.notify_one();

  this->private_->initialized_ = true;
  PRINT_PY_INIT_DEBUG(999);
}

void PythonInterpreter::initialize( bool needProgramName /*const wchar_t* program_name, const module_list_type& init_list*/ )
{
  if (needProgramName)
  {
    using namespace boost::algorithm;
    std::string cmdline = Application::Instance().parameters()->entireCommandLine();
    trim_all(cmdline);
    std::vector< std::string > argv;
    split(argv, cmdline, is_any_of(" "));

    size_t name_len = strlen(argv[0].c_str());
    std::vector< wchar_t > program_name(name_len + 1);
    mbstowcs(&program_name[0], argv[0].c_str(), name_len + 1);

    //  SCIRun::Core::PythonInterpreter::Instance().run_string( "import " + module_name + "\n" );
    //  SCIRun::Core::PythonInterpreter::Instance().run_string( "from " + module_name + " import *\n" );

    std::cerr << "Initializing Python ..." << std::endl;
    this->private_->setProgramName(program_name);
    // TODO: remove debug print when confident python initialization is stable
    std::wcerr << "initialize program name=" << this->private_->programName() << std::endl;
  }
//  PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
//  this->start_eventhandler();

  initialize_eventhandler();

  {
    auto out = [](const std::string& s) { std::cout << "[PYTHON] " << s << std::endl; };
    auto error = [](const std::string& s) { std::cerr << "[PYTHON ERROR] " << s << std::endl; };
    output_signal_.connect(out);
    error_signal_.connect(error);
  }

  //this->private_->thread_condition_variable_.wait( lock );
  this->private_->initialized_ = true;

  std::cerr << "Python initialized." << std::endl;
}

void PythonInterpreter::print_banner()
{
	//if ( !this->is_eventhandler_thread() )
	//{
	//	this->post_event( boost::bind( &PythonInterpreter::print_banner, this ) );
	//	return;
	//}
	PyRun_SimpleString( "print('Python %s on %s' % (sys.version, sys.platform))\n" );
	this->prompt_signal_( this->private_->prompt1_ );
}

void PythonInterpreter::run_string( const std::string& command )
{
	{
		PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
		if ( !this->private_->initialized_ )
		{
			throw std::invalid_argument( "The python interpreter hasn't been initialized!" );
		}
	}
	
	//if ( !this->is_eventhandler_thread() )
	//{
	//	{
	//		PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
	//		// If the Python thread is currently waiting for input, feed the string
	//		// to the input buffer directly and return.
	//		if ( this->private_->waiting_for_input_ )
	//		{
	//			this->private_->input_buffer_ = command + "\n";
	//			this->private_->thread_condition_variable_.notify_one();
	//			return;
	//		}
	//	}

	//	this->post_event( boost::bind( &PythonInterpreter::run_string, this, command ) );
	//	return;
	//}

	// Clear any previous Python errors.
	PyErr_Clear();

	// Append the command to the current command buffer
	if ( this->private_->command_buffer_.empty() )
	{
		this->private_->command_buffer_ = command;
	}
	else
	{
		if ( *this->private_->command_buffer_.rbegin() != '\n' )
		{
			this->private_->command_buffer_ += "\n";
		}
		this->private_->command_buffer_ += command;
	}

	// Compile the statement in the buffer
	boost::python::object code_obj;
	try
	{
		code_obj = this->private_->compiler_( this->private_->command_buffer_ );
	}
	catch ( ... ) {}

	// If an error happened during compilation, print the error message
	if ( PyErr_Occurred() != NULL )
	{
		PyErr_Print();
	}
	// If compilation succeeded and the code object is not Py_None
	else if ( code_obj )
	{
		//this->private_->action_context_->set_action_mode( PythonActionMode::INTERACTIVE_E );
		try
		{
			PyObject* result = PyEval_EvalCode( code_obj.ptr(), this->private_->globals_.ptr(), NULL );
			Py_XDECREF( result );
		}
		catch ( ... ) {}

		if ( PyErr_Occurred() != NULL )
		{
			if ( PyErr_ExceptionMatches( PyExc_EOFError ) )
			{
				this->error_signal_( "\nKeyboardInterrupt\n" );
				PyErr_Clear();
			}
			else
			{
				PyErr_Print();
			}
		}
	}
	// If the code object is Py_None, prompt for more input
	else
	{
		this->prompt_signal_( this->private_->prompt2_ );
		return;
	}

	this->private_->command_buffer_.clear();
	this->prompt_signal_( this->private_->prompt1_ );
}

void PythonInterpreter::run_script( const std::string& script )
{
	{
		PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
		if ( !this->private_->initialized_ )
		{
			throw std::logic_error( "The python interpreter hasn't been initialized!" );
		}
	}

	//if ( !this->is_eventhandler_thread() )
	//{
	//	this->post_event( boost::bind( static_cast< void ( PythonInterpreter::* ) ( std::string ) >( 
	//		&PythonInterpreter::run_script ), this, script ) );
	//	return;
	//}

	// Output the script to the console
	this->output_signal_( "Running script ...\n" );

	// Clear any previous Python errors.
	PyErr_Clear();

	// Compile the script
	boost::python::object code_obj;
	try
	{
		code_obj = this->private_->compiler_( script, "<script>", "exec" );
	}
	catch ( ... ) {}

	// If an error happened during compilation, print the error message
	if ( PyErr_Occurred() != NULL )
	{
		PyErr_Print();
	}
	// If compilation succeeded and the code object is not Py_None
	else if ( code_obj )
	{
		//this->private_->action_context_->set_action_mode( PythonActionMode::REPLAY_E );
		boost::python::dict local_var;
		PyObject* result = PyEval_EvalCode( code_obj.ptr(), this->private_->globals_.ptr(), local_var.ptr() );
		Py_XDECREF( result );
		if ( PyErr_Occurred() != NULL )
		{
			PyErr_Print();
		}
	}

	this->private_->command_buffer_.clear();
	this->prompt_signal_( this->private_->prompt1_ );
}

//void PythonInterpreter::run_script( StringVectorConstHandle script )
//{
//	{
//		PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
//		if ( !this->private_->initialized_ )
//		{
//			throw std::logic_error( "The python interpreter hasn't been initialized!" );
//		}
//	}
//
//	//if ( !this->is_eventhandler_thread() )
//	//{
//	//	this->post_event( boost::bind( static_cast< void ( PythonInterpreter::* ) ( StringVectorConstHandle ) >(
//	//		&PythonInterpreter::run_script ), this, script ) );
//	//	return;
//	//}
//
//	// Concatenate the strings into one single string
//	std::string str;
//	for ( size_t i = 0; i < script->size(); ++i )
//	{
//		str += script->at( i );
//	}
//	this->run_script( str );
//}

void PythonInterpreter::run_file( const std::string& file_name )
{
	{
		PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
		if ( !this->private_->initialized_ )
		{
			throw std::logic_error( "The python interpreter hasn't been initialized!" );
		}
	}
  const char* file = file_name.c_str();
  PyObject *obj = Py_BuildValue("s", file);
  FILE *fp2 = _Py_fopen_obj(obj, "r+");
  if (fp2) 
  {
    PyRun_SimpleFile(fp2, file);
  }
}

void PythonInterpreter::interrupt()
{
	//if ( !this->is_eventhandler_thread() )
	//{
	//	PyErr_SetInterrupt();
	//	this->private_->thread_condition_variable_.notify_all();
	//	this->post_event( boost::bind( &PythonInterpreter::interrupt, this ) );
	//}
	//else
	//{
		if ( PyErr_CheckSignals() != 0 )
		{
			this->error_signal_( "\nKeyboardInterrupt\n" );
			this->private_->command_buffer_.clear();
			this->prompt_signal_( this->private_->prompt1_ );
		}
	//}
}

void PythonInterpreter::start_terminal()
{
	{
		PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
		if ( !this->private_->initialized_ )
		{
			throw std::logic_error( "The python interpreter hasn't been initialized!" );
		}

		if ( this->private_->terminal_running_ )
		{
			return;
		}
	}

	//if ( !this->is_eventhandler_thread() )
	//{
	//	this->post_event( boost::bind( &PythonInterpreter::start_terminal, this ) );
	//	return;
	//}
	
	PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
	this->private_->terminal_running_ = true;
	lock.unlock();

	//PyRun_SimpleString("import sys\n");
	//PyRun_SimpleString("import io\n");
	//PyRun_SimpleString("sys.stdout=io.open('CONOUT$', 'wt')\n");
	//PyObject* sys = PyImport_ImportModule("sys");
	//PyObject* io = PyImport_ImportModule("io");
	//PyObject* pystdout = PyObject_CallMethod(io, "open", "ss", "CONOUT$", "wt");
	//if (-1 == PyObject_SetAttrString(sys, "stdout", pystdout)) {
	//	/* Announce your error to the world */
	//}
	//Py_DECREF(sys);
	//Py_DECREF(io);
	//Py_DECREF(pystdout);
	wchar_t** argv = new wchar_t*[ 2 ];
  argv[0] = const_cast< wchar_t* >(this->private_->programName());
	argv[ 1 ] = 0;
	Py_Main( 1, argv );
	delete[] argv;
	
	lock.lock();
	this->private_->terminal_running_ = false;
}

//PythonActionContextHandle PythonInterpreter::get_action_context()
//{
//	return this->private_->action_context_;
//}
//
//PythonActionContextHandle PythonInterpreter::GetActionContext()
//{
//	return Instance()->get_action_context();
//}

std::string PythonInterpreter::EscapeSingleQuotedString( const std::string& str )
{
	static const boost::regex reg( "[\\\\']" );
	return boost::regex_replace( str, reg, "\\\\$&", boost::regex_constants::format_default );
}
#endif