//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Matlab File Format Info
//
//  Author: Jonathan Bronson (bronson@sci.utah.edu)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
//  Copyright (C) 2011, 2012, Jonathan Bronson
//  Scientific Computing & Imaging Institute
//  University of Utah
//
//  Permission is  hereby  granted, free  of charge, to any person
//  obtaining a copy of this software and associated documentation
//  files  ( the "Software" ),  to  deal in  the  Software without
//  restriction, including  without limitation the rights to  use,
//  copy, modify,  merge, publish, distribute, sublicense,  and/or
//  sell copies of the Software, and to permit persons to whom the
//  Software is  furnished  to do  so,  subject  to  the following
//  conditions:
//
//  The above  copyright notice  and  this permission notice shall
//  be included  in  all copies  or  substantial  portions  of the
//  Software.
//
//  THE SOFTWARE IS  PROVIDED  "AS IS",  WITHOUT  WARRANTY  OF ANY
//  KIND,  EXPRESS OR IMPLIED, INCLUDING  BUT NOT  LIMITED  TO THE
//  WARRANTIES   OF  MERCHANTABILITY,  FITNESS  FOR  A  PARTICULAR
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT  SHALL THE AUTHORS OR
//  COPYRIGHT HOLDERS  BE  LIABLE FOR  ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
//  USE OR OTHER DEALINGS IN THE SOFTWARE.
//-------------------------------------------------------------------
//-------------------------------------------------------------------


#ifndef _MATLAB_H_
#define _MATLAB_H_

//--------------------------
//   MATLAB  Data Types
//--------------------------
#define   miINT8         1   //   8 bit, signed
#define   miUINT8        2   //   8 bit, unsigned
#define   miINT16        3   //   16-bit, signed
#define   miUINT16       4   //   16-bit, unsigned
#define   miINT32        5   //   32-bit, signed
#define   miUINT32       6   //   32-bit, unsigned
#define   miSINGLE       7   //   IEEE® 754 single format
//#define Reserved       8
#define   miDOUBLE       9   //   IEEE 754 double format
//#define Reserved      10
//#define Reserved      11
#define   miINT64       12   //   64-bit, signed
#define   miUINT64      13   //   64-bit, unsigned
#define   miMATRIX      14   //   MATLAB array
#define   miCOMPRESSED  15   //   Compressed Data
#define   miUTF8        16   //   Unicode UTF-8 Encoded Character Data
#define   miUTF16       17   //   Unicode UTF-16 Encoded Character Data
#define   miUTF32       18   //   Unicode UTF-32 Encoded Character Data


//--------------------------------
//  MATLAB Array Types (Classes)
//--------------------------------
#define   mxCELL_CLASS    1   // Cell array
#define   mxSTRUCT_CLASS  2   // Structure
#define   mxOBJECT_CLASS  3   // Object
#define   mxCHAR_CLASS    4   // Character array
#define   mxSPARSE_CLASS  5   // Sparse array
#define   mxDOUBLE_CLASS  6   // Double precision array
#define   mxSINGLE_CLASS  7   // Single precision array
#define   mxINT8_CLASS    8   // 8-bit, signed integer
#define   mxUINT8_CLASS   9   // 8-bit, unsigned integer
#define   mxINT16_CLASS   10  // 16-bit, signed integer
#define   mxUINT16_CLASS  11  // 16-bit, unsigned integer
#define   mxINT32_CLASS   12  // 32-bit, signed integer
#define   mxUINT32_CLASS  13  // 32-bit, unsigned integer
#define   mxINT64_CLASS   14  // 64-bit, signed integer
#define   mxUINT64_CLASS  15  // 64-bit, unsigned integer



#endif // _MATLAB_H_
