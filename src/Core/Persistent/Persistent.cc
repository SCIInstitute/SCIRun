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


///
///@file  Persistent.h
///@brief Base class for persistent objects...
///
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  April 1994
///

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Persistent/Persistent.h>
#include <Core/Persistent/Pstreams.h>
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#include <Core/Persistent/GZstream.h>
#endif

#include <Core/Logging/ConsoleLogger.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Thread/Mutex.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include <sci_debug.h>

using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Thread;

namespace SCIRun {

const int Piostream::PERSISTENT_VERSION = 2;

//----------------------------------------------------------------------
PersistentTypeID::PersistentTypeID(const std::string& type,
                                   const std::string& parent,
                                   PersistentMaker0 maker,
                                   Persistent* (*bc_maker1)(),
                                   Persistent* (*bc_maker2)()) :
type(type),
parent(parent),
maker(maker),
bc_maker1(bc_maker1),
bc_maker2(bc_maker2)
{
  Persistent::add_class(type, parent, maker, bc_maker1, bc_maker2);
}

PersistentTypeID::PersistentTypeID() : maker(0), bc_maker1(0), bc_maker2(0) {}

//----------------------------------------------------------------------
Persistent::~Persistent()
{
}

///@group Piostream class implementation
//////////
//

//----------------------------------------------------------------------
Piostream::Piostream(Direction dir, int version, const std::string &name,
                     LoggerHandle pr)
  : dir(dir),
    version_(version),
    err(false),
    outpointers(0),
    inpointers(0),
    current_pointer_id(1),
    have_peekname_(false),
    reporter_(pr),
    backwards_compat_id_(false),
    disable_pointer_hashing_(false),
    file_name(name)
{
  if (!reporter_)
  {
    reporter_.reset(new ConsoleLogger());
  }
}

//----------------------------------------------------------------------
Piostream::~Piostream()
{
}

//----------------------------------------------------------------------
void
Piostream::emit_pointer(int& have_data, int& pointer_id)
{
  io(have_data);
  io(pointer_id);
}

//----------------------------------------------------------------------
std::string
Piostream::peek_class()
{
  if (!have_peekname_)
  {
    have_peekname_ = true;
    io(peekname_);
  }
  return peekname_;
}

//----------------------------------------------------------------------
int
Piostream::begin_class(const std::string& classname, int current_version)
{
  if (err) return -1;
  int version = current_version;
  std::string gname;
  if (dir == Write)
  {
    gname = classname;
    io(gname);
  }
  else if (dir == Read && have_peekname_)
  {
    gname = peekname_;
  }
  else
  {
    io(gname);
  }
  have_peekname_ = false;

  io(version);

  if (dir == Read && version > current_version)
  {
    err = true;
    reporter_->error("File too new.  " + classname + " has version " +
                     to_string(version) +
                     ", but this scirun build is at version " +
                     to_string(current_version) + ".");
  }

  return version;
}

void
Piostream::end_class()
{
}

void
Piostream::begin_cheap_delim()
{
}

void
Piostream::end_cheap_delim()
{
}

void
Piostream::io(bool& data)
{
  if (err) return;
  unsigned char tmp = data;
  io(tmp);
  if (dir == Read)
  {
    data = tmp;
  }
}

void
Persistent::io(Piostream&)
{
}

void
Piostream::io(PersistentHandle& data, const PersistentTypeID& pid)
{
#if DEBUG
//  std::cerr << "looking for pid: "<<pid.type.c_str()<<" "<<pid.parent.c_str()<<std::endl;
#endif
  if (err) return;
  if (dir == Read)
  {
    int have_data;
    int pointer_id;
    data = 0;
    emit_pointer(have_data, pointer_id);

#if DEBUG
//    std::cerr << "after emit: " << have_data << ", "<< pointer_id << std::endl;
#endif

    if (have_data)
    {
      // See what type comes next in the stream.  If it is a type
      // derived from pid->type, then read it in, otherwise it is an
      // error.
      const std::string in_name(peek_class());
      const std::string want_name(pid.type);

#if DEBUG
//      std::cerr << "in here: "<< in_name<<", "<< want_name<<std::endl;
#endif

      Persistent* (*maker)() = 0;
      Persistent* (*bc_maker1)() = 0;
      Persistent* (*bc_maker2)() = 0;

      /// @todo ULTRA HACKY CODE
      if (in_name == "Manager")
        return;

      if (in_name == want_name || backwards_compat_id_)
      {
        maker = pid.maker;
      }
      else
      {
        PersistentTypeIDPtr found_pid = Persistent::find_derived(in_name, want_name);

        if (found_pid)
        {
          maker =     found_pid->maker;
          bc_maker1 = found_pid->bc_maker1;
          bc_maker2 = found_pid->bc_maker2;
          if (bc_maker1) set_backwards_compat_id(true);
        }
        else
        {
          std::ostringstream ostr;
          ostr << "Did not find a PersistentTypeID with class name [" << in_name << "], base name [" << want_name << "]";
          reporter_->error(ostr.str());
        }
      }
      if (!maker)
      {
        reporter_->error("Maker not found? (class=" + in_name + ").");
        reporter_->error("want_name: " + want_name + ".");
        err = true;
        BOOST_THROW_EXCEPTION(SCIRun::Core::Algorithms::AlgorithmProcessingException() << SCIRun::Core::ErrorMessage("Could not find persistent maker for: " + in_name));
        return;
      }

      // Make it.
      data.reset((*maker)());

      // Read it in.
      data->io(*this);
      if (err && backwards_compat_id_)
      {
        err = 0;
        reset_post_header();
        // replicate the io that has gone before this point.
        begin_cheap_delim();
        int hd;
        int p_id;
        data.reset();
        emit_pointer(hd, p_id);
        if (hd) peek_class();
        data.reset((*bc_maker1)());
        // Read it in.
        data->io(*this);
        if (err && bc_maker2)
        {
          err = 0;
          reset_post_header();
          // replicate the io that has gone before this point.
          begin_cheap_delim();
          int hd;
          int p_id;
          data.reset();
          emit_pointer(hd, p_id);
          if (hd) peek_class();
          data.reset((*bc_maker2)());
          // Read it in.
          data->io(*this);
        }
      }

      // Insert this pointer in the database.
      if (!inpointers)
      {
        inpointers.reset(new MapIntPersistent);
      }
      (*inpointers)[pointer_id] = data;
    }
    else
    {
      // Look it up.
      if (pointer_id == 0)
      {
        data.reset();
      }
      else
      {
        MapIntPersistent::iterator initer;
        if (inpointers) initer = inpointers->find(pointer_id);
        if (!inpointers || initer == inpointers->end())
        {
          reporter_->error("Pointer not in file, but should be!.");
          err = true;
          return;
        }
        data = (*initer).second;
      }
    }
  }
  else // dir == Write
  {
    int have_data;
    int pointer_id;

    MapPersistentInt::iterator outiter;
    if (outpointers)
    {
      outiter = outpointers->find(data);
      if (outiter != outpointers->end())
        pointer_id = (*outiter).second;
      else
        pointer_id = 0;
    }

    if (!data)
    {
      have_data = 0;
      pointer_id = 0;
    }
    else if (outpointers && outiter != outpointers->end() &&
             !disable_pointer_hashing_)
    {
      // Already emitted, pointer id fetched from hashtable.
      have_data = 0;
    }
    else
    {
      // Emit it.
      have_data = 1;
      pointer_id = current_pointer_id++;
      if (!outpointers)
      {
        outpointers.reset(new MapPersistentInt);
      }
      (*outpointers)[data] = pointer_id;
    }

    emit_pointer(have_data, pointer_id);

    if (have_data)
    {
      data->io(*this);
    }
  }
}


//----------------------------------------------------------------------
PiostreamPtr
auto_istream(const std::string& filename, LoggerHandle pr)
{
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (filename.find(".gz") != std::string::npos)
  {
    return auto_gzistream(filename, pr);
  }
#endif

  std::ifstream in(filename.c_str());
  if (!in)
  {
    if (pr) pr->error("File not found: " + filename);
    else std::cerr << "ERROR - File not found: " << filename << std::endl;
    return PiostreamPtr();
  }

  // Create a header of size 16 to account for new endianness
  // flag in binary headers when the version > 1.
  char hdr[16];
  in.read(hdr, 16);

  if (!in)
  {
    if (pr) pr->error("Unable to open file: " + filename);
    else std::cerr << "ERROR - Unable to open file: " << filename << std::endl;
    return PiostreamPtr();
  }

  // Close the file.
  in.close();

  // Determine endianness of file.
  int file_endian, version;

  if (!Piostream::readHeader(pr, filename, hdr, 0, version, file_endian))
  {
    if (pr) pr->error("Cannot parse header of file: " + filename);
    else std::cerr << "ERROR - Cannot parse header of file: " << filename << std::endl;
    return PiostreamPtr();
  }
  if (version > Piostream::PERSISTENT_VERSION)
  {
    const std::string errmsg = "File '" + filename + "' has version " +
      to_string(version) + ", this build only supports up to version " +
      to_string(Piostream::PERSISTENT_VERSION) + ".";
    if (pr) pr->error(errmsg);
    else std::cerr << "ERROR - " + errmsg;
    return PiostreamPtr();
  }

  const char m1 = hdr[4];
  const char m2 = hdr[5];
  const char m3 = hdr[6];
  if (m1 == 'B' && m2 == 'I' && m3 == 'N')
  {
    // Old versions of Pio used XDR which always wrote big endian so if
    // the version = 1, readHeader would return BIG, otherwise it will
    // read it from the header.
    int machine_endian = Piostream::Little;

    if (file_endian == machine_endian)
      return PiostreamPtr(new BinaryPiostream(filename, Piostream::Read, version, pr));
    else
      return PiostreamPtr(new BinarySwapPiostream(filename, Piostream::Read, version,pr));
  }
  else if (m1 == 'A' && m2 == 'S' && m3 == 'C')
  {
    return PiostreamPtr(new TextPiostream(filename, Piostream::Read, pr));
  }

  if (pr) pr->error(filename + " is an unknown type!");
  else std::cerr << filename << " is an unknown type!" << std::endl;
  return PiostreamPtr();
}


//----------------------------------------------------------------------
PiostreamPtr
auto_ostream(const std::string& filename, const std::string& type, LoggerHandle pr)
{
  // Based on the type string do the following
  //     Binary:  Return a BinaryPiostream
  //     Fast:    Return FastPiostream
  //     Text:    Return a TextPiostream
  //     Default: Return BinaryPiostream
  // NOTE: Binary will never return BinarySwap so we always write
  //       out the endianness of the machine we are on
  Piostream* stream;
  if (type == "Binary")
  {
    stream = new BinaryPiostream(filename, Piostream::Write, -1, pr);
  }
  else if (type == "Text")
  {
    stream = new TextPiostream(filename, Piostream::Write, pr);
  }
  else if (type == "Fast")
  {
    stream = new FastPiostream(filename, Piostream::Write, pr);
  }
  else
  {
    stream = new BinaryPiostream(filename, Piostream::Write, -1, pr);
  }
  return PiostreamPtr(stream);
}


//----------------------------------------------------------------------
bool
Piostream::readHeader( LoggerHandle pr,
                       const std::string& filename, char* hdr,
                       const char* filetype, int& version,
                       int& endian)
{
  char m1=hdr[0];
  char m2=hdr[1];
  char m3=hdr[2];
  char m4=hdr[3];

  if (m1 != 'S' || m2 != 'C' || m3 != 'I' || m4 != '\n')
  {
    if (pr)
    {
      pr->error( filename + " is not a valid SCI file! (magic=" +
                 m1 + m2 + m3 + m4 + ").");
    }
    else
    {
      std::cerr << filename << " is not a valid SCI file! (magic=" <<
        m1 << m2 << m3 << m4 << ")." << std::endl;
    }
    return false;
  }
  char v[5];
  v[0]=hdr[8];
  v[1]=hdr[9];
  v[2]=hdr[10];
  v[3]=hdr[11];
  v[4]=0;
  std::istringstream in(v);
  in >> version;
  if (!in)
  {
    if (pr)
    {
      pr->error("Error reading file: " + filename +
                " (while reading version).");
    }
    else
    {
      std::cerr << "Error reading file: " << filename <<
                   " (while reading version)." << std::endl;
    }
    return false;
  }
  if (filetype)
  {
    if (hdr[4] != filetype[0] || hdr[5] != filetype[1] ||
        hdr[6] != filetype[2])
    {
      if (pr) pr->error("Wrong filetype: " + filename);
      else std::cerr << "Wrong filetype: " << filename << std::endl;
      return false;
    }
  }

  bool is_binary = false;
  if (hdr[4] == 'B' && hdr[5] == 'I' && hdr[6] == 'N' && hdr[7] == '\n')
    is_binary = true;
  if(version > 1 && is_binary)
  {
    // can only be BIG or LIT
    if (hdr[12] == 'B' && hdr[13] == 'I' &&
      hdr[14] == 'G' && hdr[15] == '\n')
    {
      endian = Big;
    }
    else if (hdr[12] == 'L' && hdr[13] == 'I' &&
	       hdr[14] == 'T' && hdr[15] == '\n')
    {
      endian = Little;
    }
    else
    {
      if (pr)
      {
        pr->error(std::string("Unknown endianness: ") +
                  hdr[12] + hdr[13] + hdr[14]);
      }
      else
      {
        std::cerr << "Unknown endianness: " <<
                     hdr[12] << hdr[13] << hdr[14] << std::endl;
      }
      return false;
    }
  }
  else
  {
    endian = Big; // old system using XDR always read/wrote big endian
  }
  return true;
}


void Pio_index(Piostream& stream, index_type* data,
               size_type size)
{
  int data_size = sizeof(index_type);
  stream.io(data_size);

  if ((data_size == 4)&&(sizeof(index_type)==4))
  {
    if (!stream.block_io(data,sizeof(index_type),size))
    {
      for (index_type i=0;i < size; i++) stream.io(data[i]);
    }
  }
  else if ((data_size == 4)&&(sizeof(index_type)==8))
  {
    // only for reading
    std::vector<int> temp(size);
    if (!stream.block_io(&(temp[0]),sizeof(index_type),size))
    {
      for (index_type i=0;i < size; i++) stream.io(temp[i]);
    }
    for (index_type i=0; i <size; i++)
      data[i] = static_cast<index_type>(temp[i]);
  }
  else if ((data_size == 8)&&(sizeof(index_type)==4))
  {
    // only for reading
    std::vector<long long> temp(size);
    if (!stream.block_io(&(temp[0]),sizeof(index_type),size))
    {
      for (index_type i=0;i < size; i++) stream.io(temp[i]);
    }
    for (index_type i=0; i <size; i++)
      data[i] = static_cast<index_type>(temp[i]);
  }
  else if ((data_size == 8)&&(sizeof(index_type)==8))
  {
    if (!stream.block_io(data,sizeof(index_type),size))
    {
      for (index_type i=0;i < size; i++) stream.io(data[i]);
    }
  }
  else
  {
    std::cerr << "data_size = "<<data_size<< " and index_type size = "<< sizeof(index_type) << "\n" << "FILE FORMAT ERROR\n";
  }
}



//----------------------------------------------------------------------

Mutex* Persistent::persistent_mutex_ = 0;
Persistent::MapStringPersistentID* Persistent::persistent_table_ = 0;

void
Persistent::initialize()
{
  if (0 == persistent_mutex_)
  {
    persistent_mutex_ = new Mutex("Persistent Mutex");
  }

  if (0 == persistent_table_)
  {
    persistent_table_ = new MapStringPersistentID;
  }
}


PersistentTypeIDPtr
Persistent::find_derived( const std::string& classname,
                          const std::string& basename )
{
  PersistentTypeIDPtr pid;
  MapStringPersistentID::iterator iter;

  {
    initialize();
    Guard g(persistent_mutex_->get());

    iter = persistent_table_->find(classname);
    if (iter == persistent_table_->end())
    {
      return PersistentTypeIDPtr();
    }
  }

  pid = iter->second;
  if (pid->parent.empty())
  {
    return PersistentTypeIDPtr();
  }

  if (basename == pid->parent)
    return pid;
  if (find_derived(pid->parent, basename))
    return pid;

  return PersistentTypeIDPtr();
}

void
Persistent::add_class(const std::string& type,
                      const std::string& parent,
                      PersistentMaker0 maker,
                      Persistent* (*bc_maker1)(),
                      Persistent* (*bc_maker2)())
{
  initialize();
  Guard g(persistent_mutex_->get());

  MapStringPersistentID::iterator iter = persistent_table_->find(type);

  if (iter != persistent_table_->end())
  {
    if (iter->second->maker != maker)
      return;
    if (iter->second->parent != parent)
    {
      std::cerr << "WARNING: duplicate type in Persistent Object Type Database: " << type << std::endl;
      std::cerr << "\tParent is different. New parent: " << parent << " Existing parent: " << iter->second->parent << std::endl;
      return;
    }
  }

  (*persistent_table_)[type].reset(new PersistentTypeID);
  (*persistent_table_)[type]->type = type;
  (*persistent_table_)[type]->parent = parent;
  (*persistent_table_)[type]->maker = maker;
  (*persistent_table_)[type]->bc_maker1 = bc_maker1;
  (*persistent_table_)[type]->bc_maker2 = bc_maker2;
}


void
Persistent::add_mesh_class(const std::string& type,
                      PersistentMaker0 maker,
                      Persistent* (*bc_maker1)(),
                      Persistent* (*bc_maker2)())
{
  add_class(type,"Mesh",maker,bc_maker1,bc_maker2);
}


void
Persistent::add_field_class(const std::string& type,
                      PersistentMaker0 maker,
                      Persistent* (*bc_maker1)(),
                      Persistent* (*bc_maker2)())
{
  add_class(type,"Field",maker,bc_maker1,bc_maker2);
}

bool
Persistent::is_base_of(const std::string& parent, const std::string& type)
{
  if (parent == type)
    return true;

  PersistentTypeIDPtr found_pid = find_derived(type, parent);
  return found_pid.get() != nullptr;
}


} // End namespace SCIRun
