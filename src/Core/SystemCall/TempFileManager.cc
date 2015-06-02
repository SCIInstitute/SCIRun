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
 *  TempFileManager.cc
 *
 *  Written by:
 *  Jeroen Stinstra
 *
 */

#include <Core/SystemCall/TempFileManager.h>
#include <boost/filesystem.hpp>
#include <chrono>
#include <iostream>
#include <fstream>
#include <cmath>

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
    r = static_cast<int>((std::fabs(rand_() * 61)));
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
  if (tempdir.empty())
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

    if (!boost::filesystem::exists(newtempdir))
    {
      bool exitcode = boost::filesystem::create_directory(newtempdir);
      if (exitcode) { done = true; break; }
    }
    else
    {
      if (boost::filesystem::is_directory(newtempdir))
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
  dirname += boost::filesystem::path::preferred_separator;
  return(true);
}


bool
TempFileManager::create_tempfile(std::string dir, std::string pattern, std::string &filename)
{
  if (dir.back() != boost::filesystem::path::preferred_separator)
    dir += boost::filesystem::path::preferred_separator;

  bool done = false;
  int attempts = 0;

  std::string newtempfile;

  while (!done)
  {
    std::string ranfilename = create_randname(pattern);
    newtempfile = dir + ranfilename;

    if (boost::filesystem::exists(newtempfile))
    {
      std::ofstream ofs(newtempfile);

      if (ofs.is_open())
      {
        done = true;
        ofs.close();
        break;
      }
      // An error occured: file already existed/ could not be opened etc...
    }
    else
    {
      if (boost::filesystem::is_regular_file(newtempfile))
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

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
bool TempFileManager::create_tempfifo(std::string dir, std::string pattern, std::string &fifoname)
{
  if (dir.back() != boost::filesystem::path::preferred_separator)
    dir += boost::filesystem::path::preferred_separator;

  bool done = false;
  int attempts = 0;

  std::string newfifo;

  while (!done)
  {
    std::string ranfilename = create_randname(pattern);
    newfifo = dir + ranfilename;

    if (boost::filesystem::exists(newfifo))
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
#endif

bool
TempFileManager::delete_tempdir(std::string dirname)
{
  return boost::filesystem::remove(dirname);
}

bool
TempFileManager::delete_tempfile(std::string filename)
{
  return boost::filesystem::remove(filename);
}

#ifdef _WIN32
#define UNLINK _unlink
#else
#define UNLINK unlink
#endif

bool
TempFileManager::delete_tempfifo(std::string fifoname)
{
  UNLINK(fifoname.c_str());
  return(true);
}


std::string
TempFileManager::get_scirun_tmp_dir(std::string subdir)
{
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  const char *TMPDIR = sci_getenv("SCIRUN_SERV_TMP_DIR");
  if (TMPDIR != 0)
  {
    std::string dirname = std::string(TMPDIR);

    if (boost::filesystem::exists(dirname))
    {
      std::cout << "could not locate the directory called '" << dirname << "', using default temp directory" << std::endl;
    }
    else
    {
      if (dirname.back() != boost::filesystem::path::preferred_separator)
        dirname += boost::filesystem::path::preferred_separator;

      if (subdir.size() > 0)
      {

        bool direxists = false;

        std::string subdirname = dirname + subdir + std::string("/");

        if (boost::filesystem::exists(dirname))
        {
          direxists = boost::filesystem::create_directory(subdirname);
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
#endif

  char *HOME = getenv("HOME");
  if (!HOME)
  {
    return(std::string(""));
  }

  boost::filesystem::path homeDir(HOME);

  bool direxists = false;

  auto dirname = homeDir / "SCIRun";

  if (boost::filesystem::exists(dirname))
  {
    direxists = boost::filesystem::create_directory(dirname);
  }
  else
  {
    direxists = true;
  }

  if (!direxists) return(std::string(""));

  direxists = false;
  dirname = homeDir / "SCIRun" / "tmp";

  if (boost::filesystem::exists(dirname))
  {
    direxists = boost::filesystem::create_directory(dirname);
  }
  else
  {
    direxists = true;
  }

  if (!direxists) return(std::string(""));

  if (!subdir.empty())
  {
    direxists = false;
    dirname = homeDir / "SCIRun" / "tmp" / subdir;

    if (boost::filesystem::exists(dirname))
    {
      direxists = boost::filesystem::create_directory(dirname);
    }
    else
    {
      direxists = true;
    }

    if (!direxists) return(std::string(""));

  }

  return dirname.string();
}

std::string
TempFileManager::get_homedirID()
{
  std::string tempdir = get_scirun_tmp_dir();

  auto filename = boost::filesystem::path(tempdir) / "homeid";

  if (boost::filesystem::exists(filename))
  {
    std::ofstream IDfile(filename.string(), std::ios::out);
    std::string ranid = create_randname("homeid=XXXXXX");
    IDfile << ranid;
  }

  std::string homeidstring;
  std::ifstream homeid(filename.string());
  homeid >> homeidstring;
  homeidstring = homeidstring.substr(7);
  return(homeidstring);
}
