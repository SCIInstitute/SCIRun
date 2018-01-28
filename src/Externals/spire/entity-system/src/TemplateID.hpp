/// \author James Hughes
/// \date   December 2013

#ifndef SPIRE_ENTITY_SYSTEM_TEMPLATEID_HPP
#define SPIRE_ENTITY_SYSTEM_TEMPLATEID_HPP

#include <es-log/trace-log.h>
#include <cstdint>
#include <spire/scishare.h>

namespace spire {

class SCISHARE TemplateIDHelper
{
public:

  static const int InitialStartingID = 0;

  static uint64_t getNewTypeID()
  {
    ++mCurrentTypeID;
    return mCurrentTypeID;
  }

  static void setCounter(uint64_t newCount) {mCurrentTypeID = newCount;}
  static uint64_t getCurrentTypeID()        {return mCurrentTypeID;}

private:
  static uint64_t mCurrentTypeID;
};

/// Simple templated class to extract a unique ID from types. Used mostly
/// for sorting purposes.
template <typename T>
class TemplateID
{
public:

  static uint64_t getID()
  {
    // Assign ourselves a new static ID if we don't already have one.
    if (mStaticTypeID == 0)
      mStaticTypeID = TemplateIDHelper::getNewTypeID();

    return mStaticTypeID;
  }

  static void setID(uint64_t id)
  {
    mStaticTypeID = id;
  }

  static uint64_t mStaticTypeID;
};

template <typename T> uint64_t TemplateID<T>::mStaticTypeID = 0;

} // namespace spire

#endif
