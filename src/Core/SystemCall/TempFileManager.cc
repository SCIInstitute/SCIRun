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
 *  TempFileManager.cc
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */

#include <Core/SystemCall/TempFileManager.h>

//#include <Core/Util/Environment.h>
//#include <Core/Util/sci_system.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#else
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h> 
//#include <Core/Util/Dir.h> // for LSTAT, MKDIR
#include <chrono>
//#include <Core/Util/FileUtils.h>


#include <iostream>
#include <fstream>


using namespace SCIRun;


TempFileManager::TempFileManager()
{
  rand_ = MusilRNG(static_cast<int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())));
}


std::string
TempFileManager::create_randname(std::string str)
{
  int size = static_cast<int>(str.size());
  int p = size - 1;
  for (; p >= 0; p--)
    if (str[p] != 'X')
      break;
  p++;
  if (p == size) {
    // Names does not require any randomness
    return str;
  }

  int r;
  char c;
  std::string result = str;
  for (; p < size; p++) {
    // Use only A-Z, a-z, and 0-9 for naming random filenames
    r = static_cast<int>((fabs(rand_() * 61)));
    if (r < 10)
      c = static_cast<char>(r + 48);
    else if ((10 <= r) && (r < 36))
      c = static_cast<char>(r + 55);
    else if ((36 <= r) && (r <= 61))
      c = static_cast<char>(r + 61);
    else
      c = 'z';

    str[p] = c;
  }
  return str;
}


bool
TempFileManager::create_tempdir(std::string pattern, std::string &dirname)
{

  // Try to create or obtain the SCIRun/tmp directory in the HOME directory
  // If we cannot create this one or not obtain it fail and return to user code
  std::string tempdir = get_scirun_tmp_dir();
  if (tempdir == "")
  {
    std::cerr << "Could not find/create $HOME/SCIRun/tmp directory" << std::endl;
    dirname = "";
    return(false);
  }

  bool done = false;
  int attempts = 0;
  std::string newtempdir;

  while (!done)
  {
    std::string ranfilename = create_randname(pattern);
    newtempdir = tempdir + ranfilename;

    struct stat buf;
    if (::LSTAT(newtempdir.c_str(), &buf) < 0)
    {
      int exitcode = MKDIR(newtempdir.c_str(), 0777);
      if (exitcode == 0) { done = true; break; }
    }
    else
    {
      if (S_ISDIR(buf.st_mode))
      {
        if (pattern == ranfilename)
        {
          // User did not want an unique file name
          // so if it exists there is no problem
          done = true;
          break;
        }
      }
    }

    attempts++;
    if (attempts == 50)
    {
      std::cerr << "Error could not create temporary directory (50 attempts failed)" << std::endl;
      std::cerr << "The directory name that failed was: " << newtempdir << std::endl;
      dirname = "";
      return(false);
    }
  }

  // Make the dirname usable for adding other names to the end of it.
  dirname = newtempdir;
  dirname += std::string("/");
  return(true);
}


bool
TempFileManager::create_tempfile(std::string dir, std::string pattern, std::string &filename)
{
  if (dir[dir.size() - 1] != '/') dir += '/';

  bool done = false;
  int attempts = 0;

  std::string newtempfile;

  while (!done)
  {
    std::string ranfilename = create_randname(pattern);
    newtempfile = dir + ranfilename;

    struct stat buf;
    if (LSTAT(newtempfile.c_str(), &buf) < 0)
    {
      int fd = open(newtempfile.c_str(), O_EXCL | O_CREAT, 0700);
      if (fd > -1)
      {
        done = true;
        close(fd);
        break;
      }
      // An error occured: file already existed/ could not be opened etc...
    }
    else
    {
      if (S_ISREG(buf.st_mode))
      {
        if (pattern == ranfilename)
        {
          // User did not want an unique file name
          // so if it exists there is no problem
          done = true;
          break;
        }
      }
    }


    attempts++;
    if (attempts == 50)
    {
      std::cerr << "Error could not create temporary file (50 attempts failed)" << std::endl;
      std::cerr << "The filename that failed was: " << newtempfile << std::endl;
      filename = "";
      return(false);
    }
  }

  // We successfully open and created the file
  // We closed it again so the program can open it with the proper interface
  filename = newtempfile;
  return(true);
}


bool
TempFileManager::create_tempfilename(std::string dir, std::string pattern, std::string &filename)
{
  // This function has become obsolete
  return(create_tempfile(dir, pattern, filename));
}

bool TempFileManager::create_tempfifo(std::string dir, std::string pattern, std::string &fifoname)
{

  if (dir[dir.size() - 1] != '/') dir += '/';

  bool done = false;
  int attempts = 0;

  std::string newfifo;

  while (!done)
  {
    std::string ranfilename = create_randname(pattern);
    newfifo = dir + ranfilename;
    struct stat buf;

    if (LSTAT(newfifo.c_str(), &buf) < 0)
    {
#ifndef _WIN32
      // this is not supported on win32
      if ( mkfifo(newfifo.c_str(),0600) == 0 )
      {
        // We were successful in creating a new fifo
        // with a unique filename
        done = true;

      } 
#endif
      // An error occured: file already existed/ could not be opened etc...
    }
    attempts++;
    if (attempts == 50)
    {
      std::cerr << "Error could not create temporary file (50 attempts failed)" << std::endl;
      std::cerr << "The filename that failed was: " << newfifo << std::endl;
      fifoname = "";
      return(false);
    }
  }

  fifoname = newfifo;
  return(true);
}


bool
TempFileManager::delete_tempdir(std::string dirname)
{
  struct stat buf;
  if (LSTAT(dirname.c_str(), &buf) < 0)
  {
    return(false);
  }
  if (S_ISDIR(buf.st_mode))
  {
    deleteDir(dirname);
    return(true);
  }
  return(false);
}

bool
TempFileManager::delete_tempfile(std::string filename)
{
  struct stat buf;
  if (LSTAT(filename.c_str(), &buf) < 0)
  {
    return(false);
  }
  if (S_ISREG(buf.st_mode))
  {
    deleteFile(filename);
    return(true);
  }
  return(false);
}

bool
TempFileManager::delete_tempfifo(std::string fifoname)
{
  unlink(fifoname.c_str());
  return(true);
}


std::string
TempFileManager::get_scirun_tmp_dir(std::string subdir)
{
  struct stat buf;

  const char *TMPDIR = sci_getenv("SCIRUN_SERV_TMP_DIR");
  if (TMPDIR != 0)
  {
    std::string dirname = std::string(TMPDIR);

    if (LSTAT(dirname.c_str(), &buf) < 0)
    {
      std::cout << "could not locate the directory called '" << dirname << "', using default temp directory" << std::endl;
    }
    else
    {
      if (dirname[dirname.size() - 1] != '/') dirname += '/';

      if (subdir.size() > 0)
      {

        bool direxists = false;

        std::string subdirname = dirname + subdir + std::string("/");

        if (LSTAT(dirname.c_str(), &buf) < 0)
        {
          MKDIR(subdirname.c_str(), 0700);
          direxists = true;
        }
        else
        {
          direxists = true;
        }

        if (!direxists) return(std::string(""));

        dirname = subdirname;
      }

      return(dirname);
    }

  }

  char *HOME = getenv("HOME");
  if (HOME == 0)
  {
    return(std::string(""));
  }

  bool direxists = false;

  std::string dirname = HOME + std::string("/SCIRun");

  if (LSTAT(dirname.c_str(), &buf) < 0)
  {
    MKDIR(dirname.c_str(), 0700);
    direxists = true;
  }
  else
  {
    direxists = true;
  }

  if (!direxists) return(std::string(""));

  direxists = false;
  dirname = HOME + std::string("/SCIRun/tmp/");

  if (LSTAT(dirname.c_str(), &buf) < 0)
  {
    MKDIR(dirname.c_str(), 0700);
    direxists = true;
  }
  else
  {
    direxists = true;
  }

  if (!direxists) return(std::string(""));

  if (subdir.size() > 0)
  {

    direxists = false;
    dirname = HOME + std::string("/SCIRun/tmp/") + subdir + std::string("/");

    if (LSTAT(dirname.c_str(), &buf) < 0)
    {
      MKDIR(dirname.c_str(), 0700);
      direxists = true;
    }
    else
    {
      direxists = true;
    }

    if (!direxists) return(std::string(""));

  }

  return(dirname);
}

std::string
TempFileManager::get_homedirID()
{
  std::string tempdir = get_scirun_tmp_dir("");

  struct stat buf;
  std::string filename = tempdir + std::string("homeid");

  if (LSTAT(filename.c_str(), &buf) < 0)
  {
    std::ofstream IDfile(filename.c_str(), std::ios::out);
    std::string ranid = create_randname("homeid=XXXXXX");
    IDfile << ranid;
  }

  std::string homeidstring;
  std::ifstream homeid(filename.c_str());
  homeid >> homeidstring;
  homeidstring = homeidstring.substr(7);
  return(homeidstring);
}

