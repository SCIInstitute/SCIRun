/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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


// NOTE: This MatlabIO file is used in different projects as well. Please, do not
// make it depend on other scirun code. This way it is easier to maintain matlabIO 
// code among different projects. Thank you.


/*
 * FILE: matfiledefs.h
 * AUTH: Jeroen G Stinstra
// * DATE: 16 MAY 2005
// */
// 
//#ifndef CORE_MATLABIO_MATFILEDEFS_H
//#define CORE_MATLABIO_MATFILEDEFS_H 1
// 
///*
// * Definitions for compiling the code
// */
//
//// uncomment definitions to include or exclude options
//
//#define JGS_MATLABIO_USE_64INTS		1
//
//// define 64 bit integers
//
//#ifdef JGS_MATLABIO_USE_64INTS
//
//#ifdef _WIN32
//	typedef signed __int64 int64;
//	typedef unsigned __int64 uint64;
//  typedef unsigned int uint32_t;
//  typedef unsigned int u_int32_t;
//  typedef signed int int32_t;
//#else
//	typedef signed long long int64;
//	typedef unsigned long long uint64;
//#endif
//
//#endif
//
//
//#endif
//