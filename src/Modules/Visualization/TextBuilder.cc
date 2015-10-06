/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
#include <Modules/Visualization/TextBuilder.h>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Geometry;

TextBuilder::TextBuilder(const std::string& text, const double scale,
  const Vector& shift)
  : text_(text), scale_(scale), shift_(shift) {}

void TextBuilder::getStringVerts(std::vector<Vector> &verts, std::vector<Vector> &coords) {
  Vector shift = shift_;
  for (auto a : text_) {
    std::vector<Vector> tmp;
    std::vector<Vector> tmp2;
    getCharVerts(a, tmp, tmp2);
    for (auto v : tmp) {
      verts.push_back(v + shift);
    }
    for (auto v : tmp2) {
      coords.push_back(v);
    }
    shift = shift + Vector(scale_, 0., 0.);
  }
}

void TextBuilder::getCharVerts(const char c, std::vector<Vector> &verts, std::vector<Vector> &coords) {
  char idx = c - 32;
  if (idx < 0 || idx >= 96) idx = 95;
  //get the offset into the font array.
  size_t row = 6 - idx / 16;
  size_t col = c % 16;
  double left, right, top, bottom;
  left = static_cast<double>(col) / 16.;
  right = left + 1. / 16.;
  top = static_cast<double>(row) / 6.;
  bottom = top - 1. / 6.;
  Vector ll = Vector(left, bottom, 0.);
  Vector lr = Vector(right, bottom, 0.);
  Vector ur = Vector(right, top, 0.);
  Vector ul = Vector(left, top, 0.);
  //triangle 1
  verts.push_back(scale_ * Vector(0., 0., 0.));
  coords.push_back(ll);
  verts.push_back(scale_ * Vector(0., 1., 0.));
  coords.push_back(ul);
  verts.push_back(scale_ * Vector(1., 0., 0.));
  coords.push_back(lr);
  //triangle 2
  verts.push_back(scale_ * Vector(0., 1., 0.));
  coords.push_back(ul);
  verts.push_back(scale_ * Vector(1., 0., 0.));
  coords.push_back(lr);
  verts.push_back(scale_ * Vector(1., 1., 0.));
  coords.push_back(ur);
}

void TextBuilder::reset(const std::string& text, const double scale, const Vector& shift) {
  text_ = text;
  scale_ = scale;
  shift_ = shift;
}
