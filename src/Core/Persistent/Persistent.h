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

#ifndef CORE_PERSISTENT_H
#define CORE_PERSISTENT_H 1

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <Core/Persistent/PersistentFwd.h>
#include <map>
#include <string>
#include <complex>
#include <boost/shared_ptr.hpp>

// for index and size types
#include <Core/Datatypes/Legacy/Base/Types.h>

#include <Core/Utils/Legacy/Assert.h>
#include <Core/Logging/LoggerFwd.h>

#include <Core/Persistent/share.h>


namespace SCIRun {

  typedef Persistent* (*PersistentMaker0)();

  typedef boost::shared_ptr<Persistent> PersistentHandle;

class SCISHARE PersistentTypeID {
public:
  PersistentTypeID();
  PersistentTypeID(const std::string& type,
                     const std::string& parent,
                     PersistentMaker0 maker,
                     Persistent* (*bc_maker1)() = 0,
                     Persistent* (*bc_maker2)() = 0);
  std::string type;
  std::string parent;
  PersistentMaker0 maker;
  Persistent* (*bc_maker1)();
  Persistent* (*bc_maker2)();
};

typedef boost::shared_ptr<PersistentTypeID> PersistentTypeIDPtr;
typedef boost::shared_ptr<Piostream> PiostreamPtr;

SCISHARE PiostreamPtr auto_istream(const std::string& filename,
                                   Core::Logging::LoggerHandle pr = Core::Logging::LoggerHandle());
SCISHARE PiostreamPtr auto_ostream(const std::string& filename, const std::string& type,
                                 Core::Logging::LoggerHandle pr = Core::Logging::LoggerHandle());


//----------------------------------------------------------------------
class SCISHARE Piostream {

  public:
    typedef std::map<PersistentHandle, int>          MapPersistentInt;
    typedef std::map<int, PersistentHandle>          MapIntPersistent;

    enum Direction {
      Read,
      Write
    };

    enum Endian {
      Big,
      Little
    };

    static const int PERSISTENT_VERSION;
    void flag_error() { err = 1; }

  protected:
    Piostream(Direction, int, const std::string &, Core::Logging::LoggerHandle pr);

    Direction dir;
    int version_;
    bool err;
    int file_endian;

    boost::shared_ptr<MapPersistentInt> outpointers;
    boost::shared_ptr<MapIntPersistent> inpointers;

    int current_pointer_id;

    bool have_peekname_;
    std::string peekname_;

    Core::Logging::LoggerHandle reporter_;
    bool backwards_compat_id_;
    bool disable_pointer_hashing_;
    virtual void emit_pointer(int& have_data, int& pointer_id);
  public:
    static bool readHeader(Core::Logging::LoggerHandle pr,
                           const std::string& filename, char* hdr,
                           const char* type, int& version, int& endian);
  private:
    virtual void reset_post_header() = 0;
  public:
    std::string file_name;

    virtual ~Piostream();

    virtual std::string peek_class();
    virtual int begin_class(const std::string& name, int current_version);
    virtual void end_class();
    virtual void begin_cheap_delim();
    virtual void end_cheap_delim();

    virtual void io(bool&);
    virtual void io(char&) = 0;
    virtual void io(signed char&) = 0;
    virtual void io(unsigned char&) = 0;
    virtual void io(short&) = 0;
    virtual void io(unsigned short&) = 0;
    virtual void io(int&) = 0;
    virtual void io(unsigned int&) = 0;
    virtual void io(long&) = 0;
    virtual void io(unsigned long&) = 0;
    virtual void io(long long&) = 0;
    virtual void io(unsigned long long&) = 0;
    virtual void io(double&) = 0;
    virtual void io(float&) = 0;
    virtual void io(std::string& str) = 0;
    virtual void io(std::complex<double>&) /*= 0;*/
    {
      REPORT_NOT_IMPLEMENTED("Todo later: how to serialize complex directly.");
    }
    virtual bool eof() { return false; }

    void io(PersistentHandle&, const PersistentTypeID&);

    bool reading() const { return dir == Read; }
    bool writing() const { return dir == Write; }
    bool error() const { return err; }

    int version() const { return version_; }
    bool backwards_compat_id() const { return backwards_compat_id_; }
    void set_backwards_compat_id(bool p) { backwards_compat_id_ = p; }
    virtual bool supports_block_io() { return false; } // deprecated, redundant.

    // Returns true if block_io was supported (even on error).
    virtual bool block_io(void*, size_t, size_t) { return false; }

    void disable_pointer_hashing() { disable_pointer_hashing_ = true; }

    SCISHARE friend PiostreamPtr auto_istream(const std::string& filename,
                                   Core::Logging::LoggerHandle pr);
    SCISHARE friend PiostreamPtr auto_ostream(const std::string& filename,
                                   const std::string& type,
                                   Core::Logging::LoggerHandle pr);
};


//----------------------------------------------------------------------
class SCISHARE Persistent {

  public:
    virtual ~Persistent();
    virtual void io(Piostream&);

    //----------------------------------------------------------------------
    // Functions for managing the database of Persistent objects
    // Note all of these functions are static, to allow access from outside
    // the class

    static PersistentTypeIDPtr find_derived( const std::string& classname,
                                        const std::string& basename );
    static bool is_base_of(const std::string& parent, const std::string& type);

    static void add_class(const std::string& type,
                          const std::string& parent,
                          Persistent* (*maker)(),
                          Persistent* (*bc_maker1)() = 0,
                          Persistent* (*bc_maker2)() = 0);

    static void add_mesh_class(const std::string& type,
                          Persistent* (*maker)(),
                          Persistent* (*bc_maker1)() = 0,
                          Persistent* (*bc_maker2)() = 0);

    static void add_field_class(const std::string& type,
                          Persistent* (*maker)(),
                          Persistent* (*bc_maker1)() = 0,
                          Persistent* (*bc_maker2)() = 0);
  private:
    // Mutex protecting the list, these are in the class so they will be
    // initialized before any of the static functions can be called

    typedef std::map<std::string, PersistentTypeIDPtr>	MapStringPersistentID;
    static MapStringPersistentID* persistent_table_;
    static Core::Thread::Mutex* persistent_mutex_;

    static void initialize();
};


//----------------------------------------------------------------------
inline void Pio(Piostream& stream, bool& data) { stream.io(data); }
inline void Pio(Piostream& stream, char& data) { stream.io(data); }
inline void Pio(Piostream& stream, signed char& data) { stream.io(data); }
inline void Pio(Piostream& stream, unsigned char& data) { stream.io(data); }
inline void Pio(Piostream& stream, short& data) { stream.io(data); }
inline void Pio(Piostream& stream, unsigned short& data) { stream.io(data); }
inline void Pio(Piostream& stream, int& data) { stream.io(data); }
inline void Pio(Piostream& stream, unsigned int& data) { stream.io(data); }
inline void Pio(Piostream& stream, long& data) { stream.io(data); }
inline void Pio(Piostream& stream, unsigned long& data) { stream.io(data); }
inline void Pio(Piostream& stream, long long& data) { stream.io(data); }
inline void Pio(Piostream& stream, unsigned long long& data) { stream.io(data); }
inline void Pio(Piostream& stream, double& data) { stream.io(data); }
inline void Pio(Piostream& stream, float& data) { stream.io(data); }
inline void Pio(Piostream& stream, std::complex<double>& data) { stream.io(data); }
inline void Pio(Piostream& stream, std::string& data) { stream.io(data); }
inline void Pio(Piostream& stream, Persistent& data) { data.io(stream); }

SCISHARE void Pio_index(Piostream& stream, index_type* data, size_type sz);

/*
template<class T>
void PioImpl(Piostream& stream, boost::shared_ptr<T>& data, const PersistentTypeID& typeId)
{
  stream.begin_cheap_delim();
  Persistent* trep = data.get();
  stream.io(trep, typeId);
  if (stream.reading())
  {
    data.reset(static_cast<T*>(trep));
  }
  stream.end_cheap_delim();
}
*/

template<class T>
inline void Pio(Piostream& stream, boost::shared_ptr<T>& data, typename boost::enable_if<typename boost::is_base_of<Persistent, T>::type>* = 0)
{
  stream.begin_cheap_delim();
  PersistentHandle h = data;
  stream.io(h, T::type_id);
  if (stream.reading())
  {
    data = boost::static_pointer_cast<T>(h);
  }
  stream.end_cheap_delim();
}

template<class T>
inline void Pio2(Piostream& stream, boost::shared_ptr<T>& data, typename boost::enable_if<typename boost::is_base_of<Persistent, T>::type>* = 0)
{
  stream.begin_cheap_delim();
  PersistentHandle h = data;
  stream.io(h, T::type_id_func());
  if (stream.reading())
  {
    data = boost::static_pointer_cast<T>(h);
  }
  stream.end_cheap_delim();
}

template<class T>
inline void Pio(Piostream& stream, T* data, size_type sz)
{
  if (!stream.block_io(data, sizeof(T), sz))
  {
    for (index_type i=0;i<sz;i++)
      stream.io(data[i]);
  }
}

template <typename Size>
inline void Pio_size(Piostream& stream, Size& size)
{
  long long temp = static_cast<long long>(size);
  stream.io(temp);
  size = static_cast<size_type>(temp);
}

template <typename Index>
inline void Pio_index(Piostream& stream, Index& index)
{
  long long temp = static_cast<long long>(index);
  stream.io(temp);
  index = static_cast<size_type>(temp);
}

} // End namespace SCIRun

#endif
