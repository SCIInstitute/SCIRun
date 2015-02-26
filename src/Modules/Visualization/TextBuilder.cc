/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

TextBuilder::TextBuilder(const char * text, const double scale,
            const Core::Geometry::Vector shift)
: text_(text), scale_(scale), shift_(shift) {
    
}

void TextBuilder::getStringVerts(std::vector<Core::Geometry::Vector> &verts) {
    Vector shift = shift_;
    for (auto a : text_) {
        std::vector<Vector> tmp;
        getCharVerts(a, tmp);
        for (auto v : tmp) {
            verts.push_back(v + shift);
        }
        shift = shift + Vector(scale_,0.,0.);
    }
}

void TextBuilder::getCharVerts(const char c, std::vector<Core::Geometry::Vector> &verts) {
    const std::vector<Vector> *b = NULL;
    switch (c) {
        case '_':
            b = &underscore_verts_;
            break;
        case '-':
            b = &dash_verts_;
            break;
        case '|':
            b = &pipe_verts_;
            break;
        case '0':
            b = &zero_verts_;
            break;
        case '1':
            b = &one_verts_;
            break;
        case '2':
            b = &two_verts_;
            break;
        case '3':
            b = &three_verts_;
            break;
        case '4':
            b = &four_verts_;
            break;
        case '5':
            b = &five_verts_;
            break;
        case '6':
            b = &six_verts_;
            break;
        case '7':
            b = &seven_verts_;
            break;
        case '8':
            b = &eight_verts_;
            break;
        case '9':
            b = &nine_verts_;
            break;
        case '.':
            b = &decimal_verts_;
            break;
        case '\n':
        case '\t':
        case ' ':
            return;
        default:
            b = &unknown_verts_;
            break;
    }
    for (auto a : *b) {
        verts.push_back(a * scale_);
    }
}

void TextBuilder::reset(const char * text, const double scale,
            const Core::Geometry::Vector shift) {
    text_ = text;
    scale_ = scale;
    shift_ = shift;
}

const std::vector<Vector> TextBuilder::underscore_verts_ = {{
    Vector(0.0,0.,0.),  Vector(1.0,0.,0.) }};
    
const std::vector<Vector> TextBuilder::pipe_verts_ = {{
    Vector(0.0,0.,0.),  Vector(0.0,1.,0.) }};

const std::vector<Vector> TextBuilder::dash_verts_ = {{
    Vector(0.2,0.5,0.),  Vector(0.8,0.5,0.) }};

const std::vector<Vector> TextBuilder::unknown_verts_ = {{
    Vector(0.2,0.,0.),  Vector(0.8,0.,0.), Vector(0.8,0.,0.),
    Vector(0.8,0.8,0.), Vector(0.8,0.8,0.), Vector(0.2,0.8,0.),
    Vector(0.2,0.8,0.), Vector(0.2,0.,0.) }};


const std::vector<Vector> TextBuilder::zero_verts_ = {{
    Vector(0.2,0.,0.),  Vector(0.8,0.,0.), Vector(0.8,0.,0.),
    Vector(0.8,0.8,0.), Vector(0.8,0.8,0.), Vector(0.2,0.8,0.),
    Vector(0.2,0.8,0.), Vector(0.2,0.,0.), Vector(0.2,0.,0.),
    Vector(0.8,0.8,0.) }};

const std::vector<Vector> TextBuilder::one_verts_ = {{
    Vector(0.35,0.65,0.),  Vector(0.7,0.8,0.),
    Vector(0.7,0.8,0.),  Vector(0.7,0.0,0.) }};
    
const std::vector<Vector> TextBuilder::two_verts_ = {{
    Vector(0.2,0.45,0.),  Vector(0.2,0.0,0.),
    Vector(0.2,0.8,0.),  Vector(0.8,0.8,0.),
    Vector(0.8,0.8,0.),  Vector(0.8,0.45,0.),
    Vector(0.8,0.0,0.),  Vector(0.2,0.0,0.),
    Vector(0.2,0.45,0.),  Vector(0.8,0.45,0.)  }};
    
const std::vector<Vector> TextBuilder::three_verts_ = {{
    Vector(0.2,0.8,0.),  Vector(0.8,0.8,0.),
    Vector(0.8,0.8,0.),  Vector(0.8,0.0,0.),
    Vector(0.8,0.0,0.),  Vector(0.2,0.0,0.),
    Vector(0.2,0.45,0.),  Vector(0.8,0.45,0.) }};
    
const std::vector<Vector> TextBuilder::four_verts_ = {{
    Vector(0.8,0.8,0.),  Vector(0.8,0.,0.),
    Vector(0.8,0.8,0.),  Vector(0.2,0.5,0.),
    Vector(0.2,0.5,0.),  Vector(0.8,0.5,0.) }};
    
const std::vector<Vector> TextBuilder::five_verts_ = {{
    Vector(0.2,0.8,0.),  Vector(0.2,0.45,0.),
    Vector(0.2,0.8,0.),  Vector(0.8,0.8,0.),
    Vector(0.8,0.45,0.),  Vector(0.8,0.0,0.),
    Vector(0.8,0.0,0.),  Vector(0.2,0.0,0.),
    Vector(0.2,0.45,0.),  Vector(0.8,0.45,0.)  }};
    
const std::vector<Vector> TextBuilder::six_verts_ = {{
    Vector(0.2,0.8,0.),  Vector(0.2,0.0,0.),
    Vector(0.2,0.8,0.),  Vector(0.8,0.8,0.),
    Vector(0.8,0.45,0.),  Vector(0.8,0.0,0.),
    Vector(0.8,0.0,0.),  Vector(0.2,0.0,0.),
    Vector(0.2,0.45,0.),  Vector(0.8,0.45,0.) }};

const std::vector<Vector> TextBuilder::seven_verts_ = {{
    Vector(0.2,0.8,0.),  Vector(0.8,0.8,0.),
    Vector(0.8,0.8,0.),  Vector(0.8,0.,0.) }};

const std::vector<Vector> TextBuilder::eight_verts_ = {{
    Vector(0.2,0.8,0.),  Vector(0.2,0.0,0.),
    Vector(0.2,0.8,0.),  Vector(0.8,0.8,0.),
    Vector(0.8,0.8,0.),  Vector(0.8,0.0,0.),
    Vector(0.8,0.0,0.),  Vector(0.2,0.0,0.),
    Vector(0.2,0.45,0.),  Vector(0.8,0.45,0.)  }};

const std::vector<Vector> TextBuilder::nine_verts_ = {{
    Vector(0.2,0.8,0.),  Vector(0.8,0.8,0.),
    Vector(0.2,0.8,0.),  Vector(0.2,0.45,0.),
    Vector(0.8,0.45,0.),  Vector(0.2,0.45,0.),
    Vector(0.2,0.0,0.),  Vector(0.8,0.0,0.),
    Vector(0.8,0.8,0.),  Vector(0.8,0.0,0.)  }};

const std::vector<Vector> TextBuilder::decimal_verts_ = {{
    Vector(0.6,0.0,0.),  Vector(0.75,0.0,0.),
    Vector(0.75,0.0,0.),  Vector(0.75,0.15,0.),
    Vector(0.75,0.15,0.),  Vector(0.6,0.15,0.),
    Vector(0.6,0.15,0.),  Vector(0.6,0.0,0.) }};

