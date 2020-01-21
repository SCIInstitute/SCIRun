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


/// @todo Documentation Core/Utils/Legacy/FullFileName.cc

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

#include <Core/Utils/Legacy/FullFileName.h>
#include <iostream>

#include <stdio.h>

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

namespace bfs=boost::filesystem;
namespace bsys=boost::system;

namespace SCIRun {

FullFileName::FullFileName(std::string filename)
{
  name_ = make_absolute_filename(filename);
}


bool
FullFileName::create_file_path()
{
  bool result = false;

  bfs::path filepath(name_);
  if (filepath.is_relative())
  {
    std::cerr << "FullFileName: internal path " << name_ << " is not absolute" << std::endl;
    return result;
  }

  bfs::path parent = filepath.parent_path();
  if ( exists(parent) )
  {
    result = true;
  }
  else
  {
    try
    {
      bsys::error_code ec;
      result = bfs::create_directories(parent, ec);
#if DEBUG
      if (! result)
      {
        std::cerr << name_ << " was not found" << std::endl;
      }
#endif
      if (ec.value() == bsys::errc::success)
      {
        result = true;
      }
      else
      {
        std::cerr << "error " << ec.value() << ": " << ec.message() << std::endl;
      }
    }
    catch (const bfs::filesystem_error& ex)
    {
      std::cerr << "Error creating " << name_ << ": " << ex.what() << std::endl;
    }
  }

  return result;
}


std::string
FullFileName::get_filename()
{
  std::string::size_type pos = name_.find_last_of("/");
  if (pos == std::string::npos) return(name_);
  return(name_.substr(pos+1));
}

std::string
FullFileName::get_ext()
{
  std::string name = get_filename();
  std::string::size_type pos = name.find_last_of(".");
  if (pos == std::string::npos) return("");
  return (name.substr(pos));
}

std::string
FullFileName::get_basename()
{
  std::string name = get_filename();
  std::string::size_type pos = name.find_last_of(".");
  if (pos == std::string::npos) return(name);
  return (name.substr(0,pos));
}

std::string
FullFileName::get_abs_path()
{
  std::string::size_type pos = name_.find_last_of("/");
  if (pos == std::string::npos) return("");
  return(name_.substr(0,pos+1));
}

std::string
FullFileName::get_rel_path()
{
  Dir CWD = Dir::current_directory();
  std::string relfile = make_relative_filename(name_,CWD.getName());

  std::string::size_type pos = relfile.find_last_of("/");
  if (pos == std::string::npos) return(relfile);
  return(relfile.substr(pos+1));
}

std::string
FullFileName::get_rel_path(std::string path)
{
  std::string relfile = make_relative_filename(name_,path);

  std::string::size_type pos = relfile.find_last_of("/");
  if (pos == std::string::npos) return(relfile);
  return(relfile.substr(pos+1));
}

std::string
FullFileName::get_rel_path(Dir path)
{
  std::string relfile = make_relative_filename(name_,path.getName());

  std::string::size_type pos = relfile.find_last_of("/");
  if (pos == std::string::npos) return(relfile);
  return(relfile.substr(pos+1));
}

std::string
FullFileName::get_rel_filename()
{
  Dir CWD = Dir::current_directory();
  return(make_relative_filename(name_,CWD.getName()));
}

std::string
FullFileName::get_rel_filename(std::string path)
{
  return(make_relative_filename(name_,path));
}

std::string
FullFileName::get_rel_filename(Dir path)
{
  return(make_relative_filename(name_,path.getName()));
}


std::string
FullFileName::get_abs_filename()
{
  return (name_);
}

std::string
FullFileName::make_absolute_filename(std::string name)
{
  // Remove tcl quoting for deferred substitution
  while (name.size() > 0 && name[0] == '{') name = name.substr(1);
  while (name.size() > 0 && (name[name.size()-1] == '}')) name = name.substr(0,name.size()-1);

	// Remove blanks and tabs from the input (Some one could have editted the XML file manually and may have left spaces)
  while (name.size() > 0 && ((name[0] == ' ')||(name[0] == '\t'))) name = name.substr(1);
  while (name.size() > 0 && ((name[name.size()-1] == ' ')||(name[name.size()-1] == '\t'))) name = name.substr(0,name.size()-1);

  // Remove backward slashes
  for (size_t i=0; i<name.size();i++) if (name[i] == '\\') name[i] = '/';

  // Remove double slashes which confuses the clean up
  if (name.size() > 1) for (size_t i=0; i<(name.size()-1);i++) if (name[i] == '/' && name[i+1] == '/') { name = name.substr(0,i)+name.substr(i+1); i--; }

	// Check whether filename is absolute:

	if (( name.size() > 0 && name[0] == '/') ||
      ( name.size() > 2 && name[1] == ':' && ((name[2] == '\\')||(name[2] == '/'))))
  {
    // Unix name
    if(name[0] == '/')
    {
      // collapse name further
      std::string::size_type ddpos = name.find("../");

      while (ddpos != std::string::npos)
      {
        if (ddpos > 1 && name[ddpos-1] == '/')
        {
          std::string::size_type slashpos = name.find_last_of("/",ddpos-2);
          if (slashpos == std::string::npos)
          {
            if ((name.substr(0,ddpos-1) != "..")&&(name.substr(0,ddpos-1) != "."))
            {
              name = name.substr(ddpos+3);
              ddpos = name.find("../");
            }
            else
            {
              ddpos = name.find("../",ddpos+3);
            }
          }
          else
          {
            if ((name.substr(slashpos+1,ddpos-slashpos-2)!="..")&&(name.substr(slashpos+1,ddpos-slashpos-2)!="."))
            {
              name = name.substr(0,slashpos+1)+name.substr(ddpos+3);
              ddpos = name.find("../");
            }
            else
            {
              ddpos = name.find("../",ddpos+3);
            }
          }

        }
        else
        {
          ddpos = name.find("../",ddpos+3);
        }
      }
    }
    else
    {
      // Windows filename

      // collapse name further
      std::string::size_type ddpos = name.find("../");

      while (ddpos != std::string::npos)
      {
        if (ddpos > 1 && name[ddpos-1] == '/')
        {
          std::string::size_type slashpos = name.find_last_of("/",ddpos-2);
          if (slashpos == std::string::npos)
          {
            if ((name.substr(0,ddpos-1) != "..")&&(name.substr(0,ddpos-1) != "."))
            {
              name = name.substr(ddpos+3);
              ddpos = name.find("../");
            }
            else
            {
              ddpos = name.find("../",ddpos+3);
            }
          }
          else
          {
            if ((name.substr(slashpos+1,ddpos-slashpos-2)!="..")&&(name.substr(slashpos+1,ddpos-slashpos-2)!="."))
            {
              name = name.substr(0,slashpos+1)+name.substr(ddpos+3);
              ddpos = name.find("../");
            }
            else
            {
              ddpos = name.find("../",ddpos+3);
            }
          }

        }
        else
        {
          ddpos = name.find("../",ddpos+3);
        }
      }
    }

    return (name);
	}

  Dir CWD = Dir::current_directory();
  std::string cwd = CWD.getName();

  for (size_t i=0; i<cwd.size();i++) if (cwd[i] == '\\') cwd[i] = '/';
  if (cwd.size() > 1) for (size_t i=0; i<(cwd.size()-1);i++)
    if (cwd[i] == '/' && cwd[i+1] == '/') if (i+1 < cwd.size()) cwd = cwd.substr(0,i)+cwd.substr(i+1);

  if (cwd.size() > 0)
	{
		if(cwd[0] == '/')
		{
      // add separator at end of path
			if (cwd[cwd.size()-1]!='/') cwd +='/';
			name = cwd+name;
      // collapse name further

			std::string::size_type ddpos = name.find("../");

			while (ddpos != std::string::npos)
			{
				if (ddpos > 1 && name[ddpos-1] == '/')
				{
					std::string::size_type slashpos = name.find_last_of("/",ddpos-2);
					if (slashpos == std::string::npos)
					{
						if ((name.substr(0,ddpos-1) != "..")&&(name.substr(0,ddpos-1) != "."))
						{
							name = name.substr(ddpos+3);
							ddpos = name.find("../");
						}
						else
						{
							ddpos = name.find("../",ddpos+3);
						}
					}
					else
					{
						if ((name.substr(slashpos+1,ddpos-slashpos-2)!="..")&&(name.substr(slashpos+1,ddpos-slashpos-2)!="."))
						{
							name = name.substr(0,slashpos+1)+name.substr(ddpos+3);
							ddpos = name.find("../");
						}
						else
						{
							ddpos = name.find("../",ddpos+3);
						}
					}

				}
				else
				{
					ddpos = name.find("../",ddpos+3);
				}
			}
		}
		else
		{
      // Windows filename

			if (cwd[cwd.size()-1]!='/') cwd +='/';

			name = cwd+name;

					// collapse name further

			std::string::size_type ddpos = name.find("../");

			while (ddpos != std::string::npos)
			{
				if (ddpos > 1 && name[ddpos-1] == '/')
				{
					std::string::size_type slashpos = name.find_last_of("/",ddpos-2);
					if (slashpos == std::string::npos)
					{
						if ((name.substr(0,ddpos-1) != "..")&&(name.substr(0,ddpos-1) != "."))
						{
							name = name.substr(ddpos+3);
							ddpos = name.find("../");
						}
						else
						{
							ddpos = name.find("../",ddpos+3);
						}
					}
					else
					{
						if ((name.substr(slashpos+1,ddpos-slashpos-2)!="..")&&(name.substr(slashpos+1,ddpos-slashpos-2)!="."))
						{
							name = name.substr(0,slashpos+1)+name.substr(ddpos+3);
							ddpos = name.find("../");
						}
						else
						{
							ddpos = name.find("../",ddpos+3);
						}
					}

        }
				else
				{
					ddpos = name.find("../",ddpos+3);
				}
			}

		}
	}

	return (name);
}


std::string
FullFileName::make_relative_filename(std::string name, std::string path)
{
	// if it is not absolute assume it is relative to current directory
	path = make_absolute_filename(path);

	// Remove blanks and tabs from the input (Some could have editted the XML file manually and may have left spaces)
	while (name.size() > 0 && ((name[0] == ' ')||(name[0] == '\t'))) name = name.substr(1);
	while (name.size() > 0 && ((name[name.size()-1] == ' ')||(name[name.size()-1] == '\t'))) name = name.substr(1,name.size()-1);

	// Check whether filename is absolute:

	bool abspath = false;
	if ( name.size() > 0 && name[0] == '/') abspath = true; // Unix absolute path
	if ( name.size() > 2 && name[1] == ':' && ((name[2] == '\\') ||(name[2] == '/'))) abspath = true; // Windows absolute path

	if (abspath == false) return (name); // We could not make it relative as it is already relative

	if ( name.size() > 0 && name[0] == '/')
	{
		std::string npath = path;
	  std::string nname = name;
		std::string::size_type slashpos = path.find("/");
		bool backtrack = false;
		while(slashpos != std::string::npos)
		{
			if (npath.substr(0,slashpos) == nname.substr(0,slashpos) && backtrack == false)
			{
				npath = npath.substr(slashpos+1);
				nname = nname.substr(slashpos+1);
			}
			else
			{
				backtrack = true;
				npath = npath.substr(slashpos+1);
				nname = "../" + nname;
			}
			slashpos = npath.find("/");
		}

		// collapse name further

		std::string::size_type ddpos = nname.find("../");

		while (ddpos != std::string::npos)
		{
			if (ddpos > 2 && nname[ddpos-1] == '/')
			{
				std::string::size_type slashpos = nname.find_last_of("/",ddpos-2);
				if (slashpos == std::string::npos)
				{
					if ((nname.substr(0,ddpos-1) != "..")&&(nname.substr(0,ddpos-1) != "."))
					{
						nname = nname.substr(ddpos+3);
						ddpos = nname.find("../");
					}
					else
					{
						ddpos = nname.find("../",ddpos+3);
					}
				}
				else
				{
					if ((nname.substr(slashpos+1,ddpos-slashpos-2)!="..")&&(nname.substr(slashpos+1,ddpos-slashpos-2)!="."))
					{
						nname = nname.substr(0,slashpos+1)+nname.substr(ddpos+3);
						ddpos = nname.find("../");
					}
					else
					{
						ddpos = nname.find("../",ddpos+3);
					}
				}

			}
			else
			{
				ddpos = nname.find("../",ddpos+3);
			}
		}

		nname = "scisub_networkdir/"+nname;
		return (nname);
	}
	else if ( name.size() > 2 && name[1] == ':' && ((name[2] == '\\')||(name[2] == '/' )))
	{
    // Convert everything to forward slash
    for (size_t i=0; i< name.size(); i++) if (name[i] == '\\') name[i] = '/';

		if (path.size() > 2)
		{
			if (path.substr(0,3) != name.substr(0,3))
			{
				std::cerr << "WARNING: Could not make pathname relative as it is on another drive\n";
				return (name);
			}
		}
		else
		{
			std::cerr << "WARNING: Failed to convert network pathname to an absolute path name\n";
			return (name);
		}

		std::string npath = path;
	  std::string nname = name;
		std::string::size_type slashpos = path.find("/");
		bool backtrack = false;
		while(slashpos != std::string::npos)
		{
			if (npath.substr(0,slashpos) == nname.substr(0,slashpos) && backtrack == false)
			{
				npath = npath.substr(slashpos+1);
				nname = nname.substr(slashpos+1);
			}
			else
			{
				backtrack = true;
				npath = npath.substr(slashpos+1);
				nname = "../" + nname;
			}
			slashpos = npath.find("/");
		}

		// collapse name further

		std::string::size_type ddpos = nname.find("../");


		while (ddpos != std::string::npos)
		{
			if (ddpos > 2 && nname[ddpos-1] == '/')
			{
				std::string::size_type slashpos = nname.find_last_of("/",ddpos-2);
				if (slashpos == std::string::npos)
				{
					if ((nname.substr(0,ddpos-1) != "..")&&(nname.substr(0,ddpos-1) != "."))
					{
						nname = nname.substr(ddpos+3);
						ddpos = nname.find("../");
					}
					else
					{
						ddpos = nname.find("../",ddpos+3);
					}
				}
				else
				{
					if ((nname.substr(slashpos+1,ddpos-slashpos-2)!="..")&&(nname.substr(slashpos+1,ddpos-slashpos-2)!="."))
					{
						nname = nname.substr(0,slashpos+1)+nname.substr(ddpos+3);
						ddpos = nname.find("../");
					}
					else
					{
						ddpos = nname.find("../",ddpos+3);
					}
				}
			}
			else
			{
				ddpos = nname.find("../",ddpos+3);
			}
		}
    nname = "scisub_networkdir/"+nname;
		return (nname);
	}

	std::cerr << "WARNING: Could not convert filename into a relative filename\n";
	return (name);
}




} // end name space

#endif
