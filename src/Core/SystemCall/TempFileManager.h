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
 *  TempFileManager.h
 *
 *  This class should make it easier to create temporary directories and files
 *  Currently the class is based on a set of unix commands. These are not absolute
 *  necessary, an alternative implementation may be create for the windows port.
 *  This is still on the TODO list, but is definitely feasible
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */


#ifndef CORE_SYSTEMCALL_TEMPFILE_H
#define CORE_SYSTEMCALL_TEMPFILE_H 1

#include <string>
#include <Core/Math/MusilRNG.h>
#include <Core/SystemCall/share.h>

namespace SCIRun {

class SCISHARE TempFileManager {

  public:

    TempFileManager();

    // create_tempdir():
    // This function creates a new temporary directory in $HOME/SCIRun/tmp/
    // The function needs a pattern for the filename such as 'matlabengine.XXXXXX', where
    // the capital X's will be replaced with a random code, to ensure the directory name is
    // unique. The directoryname returned does have the full path included to the temporary
    // directory. The Xs have to be at the end of the name.

    // create_tempfile():
    // After making a temp directory in the $HOME/SCIRun/tmp directory, you can create temp
    // files in this directory. A new file is generated with a similar filename pattern as is
    // used by create_tempdir(). The tempfile will be created but will not have any data in it.
    // This is done to prevent other SCIRun threads are other instances of SCIRun to use the
    // same filename

    bool    create_tempdir(std::string dirname_pattern, std::string &dirname);
    bool    create_tempfile(std::string dir, std::string filename_pattern, std::string &filename);
    bool    create_tempfilename(std::string dir, std::string filename_pattern, std::string &filename);
    #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    bool    create_tempfifo(std::string dir, std::string filename_pattern, std::string &fifoname);
    #endif

    bool    delete_tempdir(std::string dirname);
    bool    delete_tempfile(std::string filename);
    bool    delete_tempfifo(std::string fifoname);

    // some other useful functions

    // Get the local of the SCIRun temp directory
    std::string get_scirun_tmp_dir(std::string subdir = "");

    // This writes an unique file in SCIRun's temp directory in the current homedirectory
    // It will use this to ID this directory.
    // Currently the implementation is a seed from the system clock feed through a random
    // number generator. In the future I'd like to replace this with a call to UUID as
    // being implemented in SCIRun2
    std::string get_homedirID();

  private:
    MusilRNG    rand_;
    std::string create_randname(std::string str);
};


} // end namespace


#endif
