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


#ifndef INTERFACE_MODULES_RENDER_ES_CORE_HPP
#define INTERFACE_MODULES_RENDER_ES_CORE_HPP

#include <es-acorn/Acorn.hpp>
#include <gl-state/GLState.hpp>

namespace SCIRun {
namespace Render {

/// Entity system core sitting on top of Acorn.
  class ESCore final : public spire::Acorn
{
public:
  ESCore();
  ~ESCore() override;

  std::string toString(std::string prefix) const;

  void executeWithoutAdvancingClock();
  void execute(double constantFrameTime) override;
  void setBackgroundColor(float r, float g, float b, float a);
  void runGCOnNextExecution(){runGC = true;}
  bool hasShaderPromise() const;

  ESCore(const ESCore&) = delete;
  ESCore& operator=(const ESCore&) = delete;
private:
  bool hasGeomPromise() const;

  spire::GLState  mDefaultGLState;  ///< Default OpenGL state.
  double          mCurrentTime;     ///< Current system time calculated from constant frame time.
  bool            runGC {false};
  float           r_, g_, b_, a_;
};

} // namespace Render
} // namespace SCIRun

#endif
