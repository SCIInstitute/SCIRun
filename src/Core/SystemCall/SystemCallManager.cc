#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
 *  SystemCallManager.cc
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */
 

#include <Core/SystemCall/SystemCallManager.h> 
#include <Core/Util/Dir.h>
#include <signal.h>

#include <Core/SystemCall/share.h>

namespace SCIRun {

SystemCallProcess::SystemCallProcess(int processid) :
    processid_(processid),    
    pid_(0),
    fd_stdin_(-1),
    fd_stdout_(-1),
    fd_stderr_(-1),
    fd_exit_(-1),
    lock_("system_call_process")
{
}


SystemCallProcess::~SystemCallProcess()
{
    close();
}


void SystemCallProcess::kill(int secs, bool processexit)
{
#ifndef _WIN32

    // Make the critical zone as tiny as possible
    // We do not want the exit routine to get stuck in here
    // The latter will crash the exit mechanism as a lockinghandle
    // cannot be freed
    dolock();
    
    if ((pid_ > 0)&&(processexit == false))
    {
        bool killprocess = true; // Do we need to kill the child process

        struct timeval tv;
        tv.tv_sec = secs;
        tv.tv_usec = 0;
        int ret = 0;
        
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(fd_exit_,&fdset);
        ret = ::select(fd_exit_+1,&fdset,0,0,&tv);
        if (ret > 0) killprocess = false;        
    
        if (killprocess) 
        {
            killpg(pid_,SIGKILL);
        }
    }
    pid_ = 0;
    unlock();

#endif
}


void    SystemCallProcess::close()
{
#ifndef _WIN32

    if (pid_ > 0) kill();    // if the process is still running lets end it

    dolock();
    pid_ = 0;
    // close all the pipelines
    if(fd_exit_ > -1) ::close(fd_exit_);
    if(fd_stdin_ > -1) ::close(fd_stdin_);
    if(fd_stdout_ > -1) ::close(fd_stdout_);
    if(fd_stderr_ > -1) ::close(fd_stderr_);
    
    fd_stdin_  = -1;
    fd_stdout_ = -1;
    fd_stderr_ = -1;
    fd_exit_   = -1;
    
    // unlink all the fifos
    if (fifo_exit_ != "")   ::unlink(fifo_exit_.c_str());
    if (fifo_stdin_ != "")  ::unlink(fifo_stdin_.c_str());
    if (fifo_stdout_ != "") ::unlink(fifo_stdout_.c_str());
    if (fifo_stderr_ != "") ::unlink(fifo_stderr_.c_str());
    
    fifo_exit_ = "";
    fifo_stdin_ = "";
    fifo_stdout_ = "";
    fifo_stderr_ = "";
    
    unlock();
#endif    
}

void SystemCallProcess::wait()
{
#ifndef _WIN32

    if (pid_ == 0) return;
    // As soon as data is put in the exit fifo, it is a
    // signal that the external program was terminated
    //
    // We use this construction as waitpid only waits for
    // pids of child processes. Since the shell is a grandchild
    // this method should be more secure.
    
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fd_exit_,&fdset);
    ::select(fd_exit_+1,&fdset,0,0,0);

    pid_ = 0;
    return;
    
#endif    
}




SystemCallManager::SystemCallManager() :
    lock("system_call_manager_lock"),
    ref_cnt(0),
    exit_(false),
    childpid_(0),
    child_in_(0),
    child_out_(0),
    processidcnt_(1)
{
}

SystemCallManager::~SystemCallManager()
{
#ifndef _WIN32
#endif    
}

void SystemCallManager::create()
{
    // First create pipes, because both child and parent need to have access to these
    // Hence we need to do this before we split.
#ifndef _WIN32

    tfm_.create_tempdir("systemcall.XXXXXX",tempdir_);
        
    int    fd_tochild[2];
    int    fd_fromchild[2];
    
    if (::pipe(fd_tochild) < 0) throw (SystemCallError("Could not open pipe",errno,SCE_IOERROR));
    if (::pipe(fd_fromchild) < 0)
    {
        ::close(fd_tochild[0]); 
        ::close(fd_tochild[1]);
        throw (SystemCallError("Could not open pipe",errno,SCE_IOERROR));
    }
    // We open another pipe which collects which processes have ended. This pipe functions as a 
    
    // Now we need to split the process in two
    childpid_ = ::fork();
    
    if (childpid_ < 0) 
    {
        // close the just created pipes
        
        ::close(fd_tochild[0]);
        ::close(fd_tochild[1]);
        ::close(fd_fromchild[0]);
        ::close(fd_fromchild[1]);
        
        // Throw an exception as we cannot continue from here
        throw (SystemCallError("Could not fork process",errno,SCE_NOSYSMANAGER));
    }
    
    if (childpid_ == 0)
    {    // Child process
        Thread::initialized = false;
    
        // Close the parent's pipes
        ::close(fd_tochild[1]);
        ::close(fd_fromchild[0]);

        child_in_ = fd_tochild[0];
        child_out_ = fd_fromchild[1];
    
        // Jump to the main function for launching 
        // shells and pipes for stdin/stdout
        childmain();
        
        // Kill this process without exit
        ::exit(0);
    }
    else
    {   
        // Close the two descriptors we do not need any more
        // We do not want to have too many open files
        ::close(fd_tochild[0]);
        ::close(fd_fromchild[1]);

        child_in_ = fd_tochild[1];
        child_out_ = fd_fromchild[0];
   
        CleanupManager::add_callback(SystemCallManager::cleanup,reinterpret_cast<void *>(this));
                  
        // We forked a child so we can return to SCIRun
        // To communicate with the new child we will use
        // the newly created pipes.
        return;
    }
#else
    // Give the user notice of not being able to start external processes
    std::cout << "External processes manager is not being forked... we are running Windows" << std::endl;
#endif
}

void SystemCallManager::cleanup(void *data)
{
    SystemCallManager* syscall = reinterpret_cast<SystemCallManager*>(data);
    syscall->killall();
}


int SystemCallManager::exec(std::string command)
{
#ifndef _WIN32
    dolock();
    if (exit_)
    {
        unlock();
        return(0);
    }

    
    if (childpid_ == 0) throw (SystemCallError("SystemManager has not been opened",0,SCE_NOSYSMANAGER));
    if (child_in_ < 0) throw (SystemCallError("SystemManager has not been opened",0,SCE_NOSYSMANAGER));
    if (child_out_ < 0) throw (SystemCallError("SystemManager has not been opened",0,SCE_NOSYSMANAGER));
    
    SystemCallProcess* proc = new SystemCallProcess(processidcnt_++);    
    
    // The following communication protocol assumes that the communication with the child over 
    // internal pipes is pretty robust. If this communication fails, the child is not properly
    // informed an the communication may stall
    
    try
    {
        // First try to make new unique fifo for communicating with the shell
        // The stdin/stdou/stderr will be rerouted over these channels
        
        if (!(tfm_.create_tempfifo(tempdir_,"SCIRun_systemcall.XXXXXXXX",proc->fifo_stdin_))) throw(SystemCallError("Could not create FIFO for stdin",0,SCE_NOFIFO));
        if (!(tfm_.create_tempfifo(tempdir_,"SCIRun_systemcall.XXXXXXXX",proc->fifo_stdout_))) throw(SystemCallError("Could not create FIFO for stdout",0,SCE_NOFIFO));
        if (!(tfm_.create_tempfifo(tempdir_,"SCIRun_systemcall.XXXXXXXX",proc->fifo_stderr_))) throw(SystemCallError("Could not create FIFO for stderr",0,SCE_NOFIFO));
        if (!(tfm_.create_tempfifo(tempdir_,"SCIRun_systemcall.XXXXXXXX",proc->fifo_exit_))) throw(SystemCallError("Could not create FIFO for exit",0,SCE_NOFIFO));
                
        // Make clear to the SystemCallManager process that we want to initiate a new process        
        writeline(child_in_,"execute");

        // First open the READONLY pipes as we can do that non-blocking
        // We do that non-blocking otherwise we have to wait for the child process
        // to do the same. We cannot do this for the WRITEONLY port as it will
        // result in an error
        if ((proc->fd_stdout_ = ::open(proc->fifo_stdout_.c_str(),O_RDONLY|O_NONBLOCK,0)) < 0) 
        {
            writeline(child_in_,"error=Could not open stdout fifo");
            throw (SystemCallError("Could not open fifo for stdout",errno,SCE_IOERROR));
        }
        if ((proc->fd_stderr_ = ::open(proc->fifo_stderr_.c_str(),O_RDONLY|O_NONBLOCK,0)) < 0) 
        {
            writeline(child_in_,"error-Could not open stderr fifo");
            throw (SystemCallError("Could not open fifo for stderr",errno,SCE_IOERROR));
        }
        if ((proc->fd_exit_ = ::open(proc->fifo_exit_.c_str(),O_RDONLY|O_NONBLOCK,0)) < 0) 
        {
            writeline(child_in_,"error-Could not open exit fifo");
            throw (SystemCallError("Could not open fifo for exit",errno,SCE_IOERROR));
        }
        
        
        // Now inform the child on the names of all the fifos
        // If an error occured this slot was used to transmit an error
        writeline(child_in_,proc->fifo_stdin_);
        writeline(child_in_,proc->fifo_stdout_);
        writeline(child_in_,proc->fifo_stderr_);
        writeline(child_in_,proc->fifo_exit_);

        // Now the child opens all its fifos, of which the stdin will be non-blocking

        std::string result;
        if (!(readline(child_out_,result))) throw (SystemCallError("SystemManager process did not respond",errno, SCE_IOERROR));
        if (result.substr(0,6)=="error=") throw (SystemCallError("SystemManager process returned error : "+result.substr(6),0,SCE_SYSMANAGER));
        
        if ((proc->fd_stdin_ = ::open(proc->fifo_stdin_.c_str(),O_WRONLY,0)) < 0) 
        {
            writeline(child_in_,"error=Could not open stdin fifo");
            throw (SystemCallError("Could not open fifo for stdin",errno,SCE_FIFOOPEN));
        }
        
        int flags;
        if ( (flags = ::fcntl(proc->fd_stdout_,F_GETFL,0)) < 0 )
        {
            writeline(child_in_,"error=Could not change stdout-fifo non-blocking flag");
            throw (SystemCallError("error=Could not change stdout-fifo non-blocking flag",errno,SCE_FIFOFLAG));            
        }
        flags &= ~O_NONBLOCK;
        if ( ::fcntl(proc->fd_stdout_,F_SETFL,flags) < 0)
        {
            writeline(child_in_,"error=Could not change stdout-fifo non-blocking flag");
            throw (SystemCallError("error=Could not change stdout-fifo non-blocking flag",errno,SCE_FIFOFLAG));            
        }
        
        if ( (flags = ::fcntl(proc->fd_stderr_,F_GETFL,0)) < 0 )
        {
            writeline(child_in_,"error=Could not change stderr-fifo non-blocking flag");
            throw (SystemCallError("error=Could not change stderr-fifo non-blocking flag",errno,SCE_FIFOFLAG));            
        }
        flags &= ~O_NONBLOCK;
        if ( ::fcntl(proc->fd_stderr_,F_SETFL,flags) < 0)
        {
            writeline(child_in_,"error=Could not change stderr-fifo non-blocking flag");
            throw (SystemCallError("error=Could not change stderr-fifo non-blocking flag",errno,SCE_FIFOFLAG));            
        }

        if ( (flags = ::fcntl(proc->fd_exit_,F_GETFL,0)) < 0 )
        {
            writeline(child_in_,"error=Could not change exit-fifo non-blocking flag");
            throw (SystemCallError("error=Could not change exit-fifo non-blocking flag",errno,SCE_FIFOFLAG));            
        }
        flags &= ~O_NONBLOCK;
        if ( ::fcntl(proc->fd_exit_,F_SETFL,flags) < 0)
        {
            writeline(child_in_,"error=Could not change exit-fifo non-blocking flag");
            throw (SystemCallError("error=Could not change exit-fifo non-blocking flag",errno,SCE_FIFOFLAG));            
        }
        
        // All pipes from side of the world seem to be working so tell
        // the child to fork a new process and run this function in a new shell
        std::ostringstream oss;
        oss << "processid=" << proc->processid_;
        writeline(child_in_,oss.str());                                
        writeline(child_in_,command);
        
        // Wait until the systemcallmanager child has forked a new process
        if(!(readline(child_out_,result))) throw (SystemCallError("SystemManager process did not respond",errno,SCE_IOERROR));
        if (result.substr(0,6)=="error=") throw (SystemCallError("SystemManager process returned error : "+result.substr(6),0,SCE_SYSMANAGER));
        if (result.substr(0,4)!="pid=") throw (SystemCallError("SystemManager process returned improper pid information",0,SCE_SYSMANAGER));

        int pidnum;
        std::istringstream(result.substr(4)) >> pidnum;
        proc->pid_ = static_cast<pid_t>(pidnum);
    
        // process is running
    }
    catch (...)
    {
        // Close pipes that were opened
        if (proc->fd_stdin_ > -1) ::close(proc->fd_stdin_);
        proc->fd_stdin_ = -1;
        if (proc->fd_stdout_ > -1) ::close(proc->fd_stdout_);
        proc->fd_stdout_ = -1;
        if (proc->fd_stderr_ > -1) ::close(proc->fd_stderr_);
        proc->fd_stderr_ = -1;
        if (proc->fd_exit_ > -1) ::close(proc->fd_exit_);
        proc->fd_exit_ = -1;
    
        // Close fifos that were still open
        if (proc->fifo_stdin_ != "") ::unlink(proc->fifo_stdin_.c_str());
        proc->fifo_stdin_ = "";
        if (proc->fifo_stdout_ != "") ::unlink(proc->fifo_stdout_.c_str());
        proc->fifo_stdout_ = "";
        if (proc->fifo_stderr_ != "") ::unlink(proc->fifo_stderr_.c_str());
        proc->fifo_stderr_ = "";
        if (proc->fifo_exit_ != "") ::unlink(proc->fifo_exit_.c_str());
        proc->fifo_exit_ = "";
        
        unlock();

        throw;
    }

    // Check this process in to our database so we can destroy it when SCIRun
    // exits
    
    processlist_.push_back(proc);
    unlock();
    return(proc->processid_);
#else
    // If somebody tries to launch an external application just throw an exception
    throw (SystemCallError("SystemManager has not been opened",0,SCE_NOSYSMANAGER));
    return(-1);
#endif    
}


void SystemCallManager::childmain()
{
#ifndef _WIN32

    // We still have access to STDIN, STDOUT and STDERR of SCIRun
    
    // Create a stream interface for the pipes we created
    
    int    fd_stdin = -1;
    int    fd_stdout = -1;
    int    fd_stderr = -1;
    int    fd_exit = -1;
    
    std::string    instruction;
    std::string    stdinfile;
    std::string    stdoutfile;
    std::string    stderrfile;
    std::string    exitfile;
    
    
    for (;;)
    {    // Loop forever
    
        // Close any remains from a previous run 
        // If any of the fifos was left open, we close them here
        if (fd_stdin > -1)  ::close(fd_stdin);
        fd_stdin = -1;
        if (fd_stdout > -1) ::close(fd_stdout);
        fd_stdout = -1;
        if (fd_stderr > -1) ::close(fd_stderr);
        fd_stderr = -1;        
        if (fd_exit > -1)   ::close(fd_exit);
        fd_exit = -1;        
        
        if(!(readline(child_in_,instruction)))
        {
            // We lost communication with the SCIRun process
            // Hence just exit... there is no way to restore
            // communication
            return;
        }
        
        if (instruction == "execute")
        {
            // First read the fifo filenames
            if(!(readline(child_in_,stdinfile)))
            {
                // We lost communication with the SCIRun process
                // Hence just exit... there is no way to restore
                // communication
                return;
            }
            
            // in case opening of fifos did not succeed
            if (stdinfile.substr(0,6) == "error=") continue;
            
            if(!(readline(child_in_,stdoutfile))) return;
            if(!(readline(child_in_,stderrfile))) return;
            if(!(readline(child_in_,exitfile))) return;
                        
            // We now have to open the stdout and stderr fifo
            
            fd_stdout = ::open(stdoutfile.c_str(),O_WRONLY,0);
            if (fd_stdout < 0)
            {
                writeline(child_out_,"error=Could not open STDOUT pipe with name '"+ stdoutfile + "'");
                continue;
            }

            fd_stderr = ::open(stderrfile.c_str(),O_WRONLY,0);
            if (fd_stderr < 0)
            {
                writeline(child_out_,"error=Could not open STDERR pipe with name '"+ stderrfile + "'");
                continue;
            }        

            fd_exit = ::open(exitfile.c_str(),O_WRONLY,0);
            if (fd_exit < 0)
            {
                writeline(child_out_,"error=Could not open EXIT pipe with name '"+ exitfile + "'");
                continue;
            }        

            
            // Open the stdin fifo non-blocking
            
            fd_stdin = ::open(stdinfile.c_str(),O_RDONLY|O_NONBLOCK,0);
            if (fd_stdin < 0)
            {
                writeline(child_out_,"error=Could not open STDERR pipe with name '"+ stdinfile +  "'");
                continue;        
            }
    
            writeline(child_out_,"fifos are open");
            
            // We are ready to fork once more
            
            std::string    processid;
            if(!(readline(child_in_,processid)))
            {
                // We lost communication with the SCIRun process
                // Hence just exit... there is no way to restore
                // communication
                return;
            }
            if (processid.substr(0,6) == "error=") continue;
            
            std::string    command;
            if(!(readline(child_in_,command)))
            {
                // We lost communication with the SCIRun process
                // Hence just exit... there is no way to restore
                // communication
                return;
            }
                                    
            int flags;
            if ( (flags = ::fcntl(fd_stdin,F_GETFL,0)) < 0 )
            {
                writeline(child_out_,"error=Could not change stdout-fifo non-blocking flag");
                continue;
            }
            flags &= ~O_NONBLOCK;
            if ( ::fcntl(fd_stdin,F_SETFL,flags) < 0)
            {
                writeline(child_out_,"error=Could not change stdout-fifo non-blocking flag");
            }
            
            // We are now splitting the process
            pid_t    shellpid = ::fork();

            if (shellpid == 0)
            {    
                // Create a new group
                pid_t localpid = ::getpid();
                ::setpgid(localpid,localpid);
                // Shell process
                // Reroute STDIN,STDOUT and STDERR
                
                if (fd_stdin != STDIN_FILENO)
                {    // If they are already equal, copying and closing
                    // would result in closing the STDIN
                    ::dup2(fd_stdin,STDIN_FILENO);
                    ::close(fd_stdin);
                }

                if (fd_stdout != STDOUT_FILENO)
                {    // If they are already equal, copying and closing
                    // would result in closing the STDOUT
                    ::dup2(fd_stdout,STDOUT_FILENO);
                    ::close(fd_stdout);
                }
                
                if (fd_stderr != STDERR_FILENO)
                {    // If they are already equal, copying and closing
                    // would result in closing the STDERR
                    ::dup2(fd_stderr,STDERR_FILENO);
                    ::close(fd_stderr);
                }
                // Now all std IO communication is diverted into SCIRun
                // So call the program to execute
    
                // Close the pipeline to SCIRun for launching new shells
                ::close(child_in_);
                ::close(child_out_);

                ::fcntl(STDIN_FILENO,F_SETFD,0);
                ::fcntl(STDOUT_FILENO,F_SETFD,0);
                ::fcntl(STDERR_FILENO,F_SETFD,0);
                ::fcntl(fd_exit,F_SETFD,0);
            
                int retcode = ::system(command.c_str());
                
                // Write the word exit on this channel to indicate that the process
                // is terminated
                std::ostringstream oss;
                oss << retcode << "\n";
                std::string exitstr = oss.str();
                ::write(fd_exit,&(exitstr[0]),exitstr.size());

                ::close(fd_stdin);
                ::close(fd_stdout);
                ::close(fd_stderr);
                ::close(fd_exit);
                ::_exit(0);
                
                // The latter instruction should close all remaining 
                // fifos from this side.
            }
            
            if (shellpid > 0)
            {    // Parent process
            
                std::ostringstream oss;
                oss << "pid=" << shellpid;
                writeline(child_out_,oss.str());
                continue; 
                // The latter instruction will close any open fifos, as it is done at the
                // start of the loop. Hence the parent only has to signal the pid to SCIRun
            }
            
        }
        
        if (instruction == "exit") return;
        
        // Get the next instruction
    }
    
#endif    
}


int    SystemCallManager::getstdin(int processid)
{
#ifndef _WIN32
    dolock();
    if (exit_)
    {
        unlock();
        return -1;
    }

    std::list<SystemCallProcess*>::iterator it = processlist_.begin();
    for (;it != processlist_.end(); it++)
    {
        if ((*it)->processid_ == processid) 
        {
            int fd = (*it)->fd_stdin_;
            unlock();
            return(fd);
        }
    }
    unlock();
    return(-1);
#else
    return(-1);
#endif
}

int    SystemCallManager::getstdout(int processid)
{
#ifndef _WIN32
    dolock();
    if (exit_)
    {
        unlock();
        return(-1);
    }

    std::list<SystemCallProcess*>::iterator it = processlist_.begin();
    for (;it != processlist_.end(); it++)
    {
        if ((*it)->processid_ == processid) 
        {
            int fd = (*it)->fd_stdout_;
            unlock();
            return(fd);
        }
    }
    unlock();
    return(-1);
#else
    return(-1);
#endif
}

int    SystemCallManager::getstderr(int processid)
{
#ifndef _WIN32
    dolock();
    if (exit_)
    {
        unlock();
        return(-1);
    }

    std::list<SystemCallProcess*>::iterator it = processlist_.begin();
    for (;it != processlist_.end(); it++)
    {
        if ((*it)->processid_ == processid) 
        {
            int fd = (*it)->fd_stderr_;
            unlock();
            return(fd);
        }
    }
    unlock();
    return(-1);
#else
    return(-1);
#endif    
}

int    SystemCallManager::getexit(int processid)
{
#ifndef _WIN32
    dolock();
    if (exit_)
    {
        unlock();
        return(-1);
    }


    std::list<SystemCallProcess*>::iterator it = processlist_.begin();
    for (;it != processlist_.end(); it++)
    {
        if ((*it)->processid_ == processid) 
        {
            int fd = (*it)->fd_exit_;
            unlock();
            return(fd);
        }
    }
    unlock();
    return(-1);
#else
    return(-1);
#endif
}




void SystemCallManager::wait(int  processid)
{
#ifndef _WIN32
    dolock();
    if (exit_)
    {
        unlock();
        return;
    }


    std::list<SystemCallProcess*>::iterator it = processlist_.begin();
    for (;it != processlist_.end(); it++)
    {
        if ((*it)->processid_ == processid) 
        {
            SystemCallProcess *proc = (*it);
            unlock();
            proc->wait();
            return;
        }
    }
    unlock();
    return;
#endif    
}

void SystemCallManager::kill(int processid, int secs, bool processexit)
{
#ifndef _WIN32
    dolock();
    if (exit_)
    {
        unlock();
        return;
    }

    std::list<SystemCallProcess*>::iterator it = processlist_.begin();
    for (;it != processlist_.end(); it++)
    {
        if ((*it)->processid_ == processid) 
        {
            SystemCallProcess* ptr = (*it);
            unlock();            
            ptr->kill(secs,processexit);
            return;
        }
    } 
    unlock();
#endif    
}


void SystemCallManager::close(int processid)
{
#ifndef _WIN32
    dolock();
    if (exit_)
    {
        unlock();
        return;
    }
    
    std::list<SystemCallProcess*>::iterator it = processlist_.begin();
    for (;it != processlist_.end(); it++)
    {
        if ((*it)->processid_ == processid) 
        {
            SystemCallProcess *proc = (*it);
            processlist_.erase(it);
            unlock();
            proc->close();
            delete proc;
            
            return;
        }
    }
    unlock();
#endif
}



bool SystemCallManager::readline(int fd,std::string& str)
{
    int        bytestoread;
    int        bytesread;
    size_t     len;
    char       *buffer;
    
    int   length;

    bytestoread = sizeof(int);
    bytesread = 0;
    buffer = reinterpret_cast<char *>(&length);
    while(bytesread < bytestoread)
    {
        len = ::read(fd,&(buffer[bytesread]),bytestoread-bytesread);
        if (len == 0) { str = ""; return(false); } // End of file
        if (len == -1) { if ((errno == EINTR)||(errno == EAGAIN)) { continue; } else { str = ""; return(false); } } // IO Error
        bytesread += len;
    }
    
    str.resize(length);
    
    bytestoread = length;
    bytesread = 0;
    buffer = &(str[0]);
    while(bytesread < bytestoread)
    {
        len = ::read(fd,&(buffer[bytesread]),bytestoread-bytesread);
        if (len == 0) { str = ""; return(false); } // End of file
        if (len == -1) { if ((errno == EINTR)||(errno == EAGAIN)) { continue; } else { str = ""; return(false); } } // IO Error
        bytesread += len;
    }

    return(true);
}


bool SystemCallManager::writeline(int fd, std::string str)
{
    int        bytestowrite;
    int        byteswritten;
    size_t     len;
    char*      buffer;
    
    int length = static_cast<int>(str.size());
    
    bytestowrite = sizeof(int);
    byteswritten = 0;
    buffer = reinterpret_cast<char *>(&length);
    while (byteswritten  < bytestowrite)
    {
        len = ::write(fd,&(buffer[byteswritten]),bytestowrite-byteswritten);
        if (len == -1) { if ((errno == EINTR)||(errno == EAGAIN)) { continue; } else { return(false); } } // IO Error
        byteswritten += len;
    }
    
    bytestowrite = length;
    byteswritten = 0;
    buffer = reinterpret_cast<char *>(&(str[0]));
    while (byteswritten  < bytestowrite)
    {
        len = ::write(fd,&(buffer[byteswritten]),bytestowrite-byteswritten);
        if (len == -1) { if ((errno == EINTR)||(errno == EAGAIN)) { continue; } else { return(false); } } // IO Error
        byteswritten += len;
    }
    
    return(true);
}

void SystemCallManager::killall()
{
#ifndef WIN32_

    dolock();
    exit_ = true;

    // If main child is already gone, this function was called before
    // so bail out as there should be nothing left to do
    if (childpid_ == 0)
    {
        unlock();
        return;
    }

    childpid_ = 0;
    writeline(child_in_,"exit");

    // Destroy the process that creates the new child processes
    // If this one is gone, no new processses can be launched

    std::list<SystemCallProcess*>::iterator it = processlist_.begin();
    for (;it != processlist_.end(); it++)
    {
        delete (*it);
        (*it) = 0;
    }

    unlock();
#endif    
}

SystemCallManager*    systemcallmanager_;

} // end namespace

#endif