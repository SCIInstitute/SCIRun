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


#ifndef SPIRE_MATH_H__
#define SPIRE_MATH_H__

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>           // glm::value_ptr
#include <iostream>
#include <spire/scishare.h>

// Make the 4x4 matrix streamable.
std::ostream& operator<< (std::ostream& os, const glm::mat4& m);
std::ostream& operator<< (std::ostream& os, const glm::vec4& v);
std::ostream& operator<< (std::ostream& os, const glm::vec3& v);
std::ostream& operator<< (std::ostream& os, const glm::vec2& v);
std::ostream& operator<< (std::ostream& os, const glm::quat& q);

#define eieo(t)		( 0.5f * (1 - cosf( (t) * glm::pi<float>() )) )
#define fieo(t)		( sinf( (t) * (glm::pi<float>() / 2.0f ) )
#define eifo(t)		( 1.0f - sinf( (1.0f - (t)) * (glm::pi<float>() / 2.0f) ) )

#endif
