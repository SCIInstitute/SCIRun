#ifndef SPIRE_ENTITY_SYSTEM_EMPTYCOMPONENTCONTAINER_HPP
#define SPIRE_ENTITY_SYSTEM_EMPTYCOMPONENTCONTAINER_HPP

#include <es-log/trace-log.h>
#include <spire/scishare.h>

namespace spire {

// Component base class, used to verify component types at run-time.
class SCISHARE EmptyComponentContainer : public BaseComponentContainer
{
public:
  EmptyComponentContainer()           {}
  virtual ~EmptyComponentContainer()  {}

  std::string toString(std::string prefix) const override {return prefix + "  EMPTY\n";}

  void renormalize(bool stableSort) override {}
  uint64_t getLowerSequence() const override {return 0;}
  uint64_t getUpperSequence() const override {return 0;}
  uint64_t getNumComponents() const override {return 0;}
  int getNumComponentsWithSequence(uint64_t) const override {return 0;}
  void removeAll() override {}
  void removeAllImmediately() override {}
  void removeSequence(uint64_t) override {}
  void removeFirstSequence(uint64_t) override {}
  void removeLastSequence(uint64_t) override {}
  void removeSequenceWithIndex(uint64_t, int32_t) override {}
  bool isStatic() const override {return false;}
  uint64_t getSequenceFromIndex(int) const override {return 0;}
  std::string describe() const override { return "Empty"; }
};

} // namespace spire

#endif
