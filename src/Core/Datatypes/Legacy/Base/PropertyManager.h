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
///@file  PropertyManager.h
///
///@author
///       Yarden Livnat
///       Department of Computer Science
///       University of Utah
///@date  March 2001
///
///@brief Manage dynamic properties of persistent objects.
///

#ifndef CORE_DATATYPES_PROPERTYMANAGER_H
#define CORE_DATATYPES_PROPERTYMANAGER_H 1

#include <Core/Utils/Legacy/Assert.h>
#include <Core/Datatypes/Legacy/Base/TypeName.h>
#include <Core/Persistent/PersistentSTL.h>
#include <Core/Thread/Mutex.h>
#include <Core/Containers/Array1.h>

#include <iostream>
#include <map>


#include <Core/Datatypes/Legacy/Base/share.h>

namespace SCIRun {

class SCISHARE PropertyBase : public Persistent
{
public:
  explicit PropertyBase(bool trans) : transient_(trans) {}
  virtual PropertyBase* clone() const {
    ASSERTFAIL("PropertyBase clone called");
  }

  virtual void io(Piostream &) {}
  static  PersistentTypeID type_id;
  virtual std::string dynamic_type_name() const;

  bool transient() const { return transient_; }
  void set_transient(bool t) { transient_ = t; }

  virtual bool operator==(PropertyBase &/*pb*/) const {
    ASSERTFAIL( "Not defined."); }
  virtual bool operator!=(PropertyBase &/*pb*/) const {
    ASSERTFAIL( "Not defined."); }

protected:
  /// Transient properties are deleted when the PropertyManager that this
  /// Property belongs to is thawed.
  bool transient_;
  static Persistent *maker();
};

typedef boost::shared_ptr<PropertyBase> PropertyBaseHandle;

class SCISHARE PropertyManager;

template<class T>
class Property : public PropertyBase {
public:
  friend class PropertyManager;

  Property(const T &o, bool trans) :  PropertyBase(trans), obj_(o)
  {
  }
  virtual ~Property() {}
  virtual PropertyBase *clone() const
  { return new Property(obj_, transient()); }

  static const std::string type_name(int n = -1);
  virtual void io(Piostream &stream);
  static PersistentTypeID type_id;
  virtual std::string dynamic_type_name() const { return type_id.type; }

  virtual bool operator==(PropertyBase &pb) const {
    const Property<T> *prop = dynamic_cast<Property<T> *>(&pb);

    if (prop && obj_ == prop->obj_ )
      return true;

    return false;
  }

  virtual bool operator!=(PropertyBase &pb) const {
    const Property<T> *prop = dynamic_cast<Property<T> *>(&pb);

    if (prop && obj_ == prop->obj_ )
      return false;

    return true;
  }

protected:
  // Only Pio should use this constructor.
  // Default is for objects read in to be non-transient.
  Property() : PropertyBase(false) {}

private:
  T obj_;

  static Persistent *maker();
};



/*
 * Persistent Io
 */

const int PROPERTY_VERSION = 2;

/*
 * Property<T>
 */

template<class T>
const std::string Property<T>::type_name(int n)
{
  if ( n == -1 ) {
    const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if ( n == 0 ) {
    const std::string nm("Property");
    return nm;
  }
  else
    return find_type_name( static_cast<T*>(0));
}

template <class T>
PersistentTypeID
Property<T>::type_id(type_name(-1), "PropertyBase", maker);

template <class T>
Persistent*
Property<T>::maker()
{
  // Properties in a file start out to be non-transient.
  return new Property<T>();
}

template<class T>
void
Property<T>::io( Piostream &stream)
{
  const int version = stream.begin_class( type_name(-1), PROPERTY_VERSION);
  if (version > 1)
  {
    Pio(stream, transient_);
  }
  Pio(stream, obj_);
  stream.end_class();
}


/*
 * PropertyManager
 */

class PropertyManager : public Persistent
{
public:
  PropertyManager();
  PropertyManager(const PropertyManager &copy);
  virtual ~PropertyManager();

  PropertyManager & operator=(const PropertyManager &pm);
  void copy_properties(const PropertyManager* src);

  bool operator==(const PropertyManager &pm);
  bool operator!=(const PropertyManager &pm);

  template<class T> void set_property(const std::string &, const T &,
				      bool is_transient);
  template<class T> bool get_property( const std::string &, T &);
  bool is_property( const std::string & );
  std::string get_property_name( size_t index );

  /// -- mutability --

  /// Transient data may only be stored in a frozen PropertyManager.
  virtual void freeze();

  /// thaw will remove all transient properties from the PropertyManager.
  virtual void thaw();

  /// query frozen state of a PropertyManager.
  bool is_frozen() const { return frozen_; }

  void remove_property( const std::string & );

  //NOTE: do NOT change this type to size_t to avoid casting below! it will break reading all old matrix/field types.
  typedef unsigned int PropertyManagerSize;
  PropertyManagerSize nproperties() const { return static_cast<PropertyManagerSize>(properties_.size()); }
  const std::map<std::string, PropertyBaseHandle>& properties() const { return properties_; }

  void    io(Piostream &stream);
  static  PersistentTypeID type_id;
  virtual std::string dynamic_type_name() const;

  void set_name(const std::string& name)
  {
    set_property("name", name, false);
  }
  std::string get_name()
  {
    std::string name;
    return get_property("name", name) ? name : std::string();
  }

private:

  typedef std::map<std::string, PropertyBaseHandle> map_type;
  map_type properties_;

protected:
/// A frozen PropertyManager may store transient data.
  void clear_transient();

  bool frozen_;

  Core::Thread::Mutex lock;
};


template<class T>
void
PropertyManager::set_property(const std::string &name,  const T& obj,
			      bool is_transient)
{
  if (is_transient && (! is_frozen())) {
    std::cerr << "WARNING::PropertyManager must be frozen to store transient data"
              << " freezing now!" << std::endl;
    freeze();
  }
  Core::Thread::Guard g(lock.get());
  properties_[name].reset(new Property<T>(obj, is_transient));
}


template<class T>
bool
PropertyManager::get_property(const std::string &name, T &ref)
{
  Core::Thread::Guard g(lock.get());

  bool ans = false;
  map_type::iterator loc = properties_.find(name);
  if (loc != properties_.end()) {
    auto prop = boost::dynamic_pointer_cast<const Property<T>>(loc->second);
    if (prop)
    {
      ref = prop->obj_;
      ans = true;
    }
  }
  return ans;
}


} // namespace SCIRun

#endif // SCI_project_PropertyManager_h
