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
