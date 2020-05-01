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

#include <vector>
#include <cstdio>

#include <ospray/ospray.h>
#include <ospray/ospray_util.h>

class OSPRayRenderer
{
public:
  OSPRayRenderer();
  virtual ~OSPRayRenderer();

  void renderFrame(); //renders frame with current context
  void displayFrame();
  void resize(int width, int height);
  void addGroup();
  void addModelToGroup(float* vertexPositions, float* vertexColors, uint32_t vertexCount,
  uint32_t* indices, size_t indexCount);
  void addInstaceOfGroup();

private:
  static int osprayRendererInstances;

  int width  {16};
  int height {16};

  OSPFrameBuffer frameBuffer {nullptr};
  OSPRenderer    renderer    {nullptr};
  OSPCamera      camera      {nullptr};
  OSPWorld       world       {nullptr};

  OSPGroup group {nullptr};
};
