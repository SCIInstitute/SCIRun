//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Generalized Stencil Table
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


// MACRO's to simplify Stencil Tables
#ifndef _O
#define _O -1        // ___      NO MORE VERTICES
#define _A 0         //
#define _B 1         //     \__  Lattice Vertices
#define _C 2         //     /
#define _D 3         // ___/
                     // ___
#define _AB 4        //
#define _AC 5        //
#define _AD 6        //      \__ Cutpoint Vertices
#define _BC 7        //      /
#define _CD 8        //
#define _BD 9        // ___/
                     // ___
#define _ABC 10      //
#define _ACD 11      //     \__  TriplePoint Vertices
#define _ABD 12      //     /
#define _BCD 13      // ___/

#define _ABCD 14     // QuadPoint Vertex

#define VERT 0
#define CUT  1
#define TRIP 2
#define QUAD 3

#endif


const int stencilTableEven[64][24][4] = {
{{_A,_B,_C,_D}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 0 = 00000000000	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 1 = 00000000001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 2 = 00000000010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 3 = 00000000011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 4 = 00000000100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 5 = 00000000101	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 6 = 00000000110	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 7 = 00000000111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 8 = 00000001000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 9 = 00000001001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 10 = 00000001010	(invalid)
{{_BD,_AD,_CD,_B}, {_B,_C,_AD,_CD}, {_A,_B,_C,_AD}, {_D,_AD,_CD,_BD}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 11 = 00000001011	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 12 = 00000001100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 13 = 00000001101	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 14 = 00000001110	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 15 = 00000001111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 16 = 00000010000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 17 = 00000010001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 18 = 00000010010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 19 = 00000010011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 20 = 00000010100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 21 = 00000010101	(invalid)
{{_CD,_D,_AC,_B}, {_AC,_B,_D,_A}, {_CD,_AC,_BC,_B}, {_C,_AC,_BC,_CD}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 22 = 00000010110	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 23 = 00000010111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 24 = 00000011000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 25 = 00000011001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 26 = 00000011010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 27 = 00000011011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 28 = 00000011100	(invalid)
{{_BD,_AC,_BC,_B}, {_B,_AC,_AD,_BD}, {_A,_B,_AC,_AD}, {_C,_D,_AC,_BC}, {_D,_AC,_BC,_BD}, {_BD,_AC,_AD,_D}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 29 = 00000011101	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 30 = 00000011110	(invalid)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 31 = 00000011111	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 32 = 00000100000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 33 = 00000100001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 34 = 00000100010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 35 = 00000100011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 36 = 00000100100	(invalid)
{{_BD,_AB ,_BC,_B}, {_C,_D,_AB ,_BC}, {_A,_C,_D,_AB }, {_D,_AB ,_BC,_BD}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 37 = 00000100101	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 38 = 00000100110	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 39 = 00000100111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 40 = 00000101000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 41 = 00000101001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 42 = 00000101010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 43 = 00000101011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 44 = 00000101100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 45 = 00000101101	(invalid)
{{_CD,_D,_AD,_B}, {_CD,_AD,_BC,_B}, {_B,_AB ,_AD,_BC}, {_C,_AD,_BC,_CD}, {_BC,_AB ,_AD,_C}, {_AD,_C,_AB ,_A}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 46 = 00000101110	(VALID)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 47 = 00000101111	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 48 = 00000110000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 49 = 00000110001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 50 = 00000110010	(invalid)
{{_BD,_AC,_CD,_B}, {_BD,_AB ,_AC,_B}, {_B,_C,_AC,_CD}, {_D,_AC,_CD,_BD}, {_D,_AB ,_AC,_BD}, {_A,_D,_AB ,_AC}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 51 = 00000110011	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 52 = 00000110100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 53 = 00000110101	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 54 = 00000110110	(invalid)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 55 = 00000110111	(VALID)
{{_A,_AB ,_AC,_AD}, {_AC,_C,_D,_B}, {_B,_D,_AC,_AD}, {_AD,_AB ,_AC,_B}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 56 = 00000111000	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 57 = 00000111001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 58 = 00000111010	(invalid)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 59 = 00000111011	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 60 = 00000111100	(invalid)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 61 = 00000111101	(VALID)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 62 = 00000111110	(VALID)
{{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
 {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}}	// key = 63 = 00000111111	(VALID)
};


const int stencilTableOdd[64][24][4] = {
{{_A,_B,_C,_D}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 0 = 00000000000	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 1 = 00000000001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 2 = 00000000010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 3 = 00000000011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 4 = 00000000100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 5 = 00000000101	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 6 = 00000000110	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 7 = 00000000111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 8 = 00000001000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 9 = 00000001001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 10 = 00000001010	(invalid)
{{_BD,_AD,_CD,_B}, {_B,_C,_AD,_CD}, {_A,_B,_C,_AD}, {_D,_AD,_CD,_BD}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 11 = 00000001011	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 12 = 00000001100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 13 = 00000001101	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 14 = 00000001110	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 15 = 00000001111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 16 = 00000010000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 17 = 00000010001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 18 = 00000010010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 19 = 00000010011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 20 = 00000010100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 21 = 00000010101	(invalid)
{{_C,_AC,_BC,_CD}, {_CD,_AC,_BC,_D}, {_BC,_D,_AC,_B}, {_AC,_B,_D,_A}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 22 = 00000010110	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 23 = 00000010111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 24 = 00000011000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 25 = 00000011001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 26 = 00000011010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 27 = 00000011011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 28 = 00000011100	(invalid)
{{_BD,_AC,_BC,_B}, {_B,_AC,_AD,_BD}, {_A,_B,_AC,_AD}, {_C,_D,_AC,_BC}, {_D,_AC,_BC,_BD}, {_BD,_AC,_AD,_D}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 29 = 00000011101	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 30 = 00000011110	(invalid)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 31 = 00000011111	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 32 = 00000100000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 33 = 00000100001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 34 = 00000100010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 35 = 00000100011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 36 = 00000100100	(invalid)
{{_BD,_AB ,_BC,_B}, {_C,_D,_AB ,_BC}, {_A,_C,_D,_AB }, {_D,_AB ,_BC,_BD}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 37 = 00000100101	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 38 = 00000100110	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 39 = 00000100111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 40 = 00000101000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 41 = 00000101001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 42 = 00000101010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 43 = 00000101011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 44 = 00000101100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 45 = 00000101101	(invalid)
{{_C,_AB ,_BC,_CD}, {_CD,_AB ,_AD,_C}, {_AD,_C,_AB ,_A}, {_CD,_AB ,_BC,_D}, {_D,_AB ,_AD,_CD}, {_BC,_D,_AB ,_B}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 46 = 00000101110	(VALID)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 47 = 00000101111	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 48 = 00000110000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 49 = 00000110001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 50 = 00000110010	(invalid)
{{_BD,_AC,_CD,_B}, {_BD,_AB ,_AC,_B}, {_B,_C,_AC,_CD}, {_D,_AC,_CD,_BD}, {_D,_AB ,_AC,_BD}, {_A,_D,_AB ,_AC}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 51 = 00000110011	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 52 = 00000110100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 53 = 00000110101	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 54 = 00000110110	(invalid)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 55 = 00000110111	(VALID)
{{_A,_AB ,_AC,_AD}, {_AC,_C,_D,_B}, {_AC,_D,_AB ,_B}, {_AD,_AB ,_AC,_D}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 56 = 00000111000	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 57 = 00000111001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 58 = 00000111010	(invalid)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 59 = 00000111011	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 60 = 00000111100	(invalid)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 61 = 00000111101	(VALID)
    {{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
     {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}},	// key = 62 = 00000111110	(VALID)
{{_ABCD,_AB ,_ABD,_A}, {_A,_AB ,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_A}, {_A,_AD,_ABD,_ABCD}, {_ABCD,_AD,_ACD,_A}, {_A,_AC,_ACD,_ABCD}, {_B,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_B}, {_B,_AB ,_ABD,_ABCD}, {_ABCD,_BC,_BCD,_B}, {_B,_BC,_ABC,_ABCD}, {_ABCD,_AB ,_ABC,_B},
 {_C,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_C}, {_C,_AC,_ABC,_ABCD}, {_ABCD,_CD,_BCD,_C}, {_C,_CD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_C}, {_D,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_D}, {_D,_AD,_ACD,_ABCD}, {_ABCD,_BD,_BCD,_D}, {_D,_BD,_ABD,_ABCD}, {_ABCD,_AD,_ABD,_D}}	// key = 63 = 00000111111	(VALID)
};



/*
const int stencilTable[64][24][4] = {
{{_D,_B,_C,_A}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 0 = 00000000000	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 1 = 00000000001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 2 = 00000000010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 3 = 00000000011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 4 = 00000000100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 5 = 00000000101	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 6 = 00000000110	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 7 = 00000000111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 8 = 00000001000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 9 = 00000001001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 10 = 00000001010	(invalid)
{{_B,_AD,_CD,_BD}, {_CD,_C,_AD,_B}, {_AD,_B,_C,_A}, {_BD,_AD,_CD,_D}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 11 = 00000001011	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 12 = 00000001100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 13 = 00000001101	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 14 = 00000001110	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 15 = 00000001111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 16 = 00000010000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 17 = 00000010001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 18 = 00000010010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 19 = 00000010011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 20 = 00000010100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 21 = 00000010101	(invalid)
{{_CD,_AC,_BC,_C}, {_D,_AC,_BC,_CD}, {_B,_D,_AC,_BC}, {_A,_B,_D,_AC}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 22 = 00000010110	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 23 = 00000010111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 24 = 00000011000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 25 = 00000011001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 26 = 00000011010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 27 = 00000011011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 28 = 00000011100	(invalid)
{{_B,_AC,_BC,_BD}, {_BD,_AC,_AD,_B}, {_AD,_B,_AC,_A}, {_BC,_D,_AC,_C}, {_BD,_AC,_BC,_D}, {_D,_AC,_AD,_BD}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 29 = 00000011101	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 30 = 00000011110	(invalid)
{{_A,_AB ,_ABD,_ABCD}, {_ABCD,_AB ,_ABC,_A}, {_A,_AC,_ABC,_ABCD}, {_ABCD,_AD,_ABD,_A}, {_A,_AD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_A}, {_ABCD,_BD,_BCD,_B}, {_B,_BD,_ABD,_ABCD}, {_ABCD,_AB ,_ABD,_B}, {_B,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_B}, {_B,_AB ,_ABC,_ABCD},
 {_ABCD,_BC,_BCD,_C}, {_C,_BC,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_C}, {_C,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_C}, {_C,_AC,_ACD,_ABCD}, {_ABCD,_CD,_BCD,_D}, {_D,_CD,_ACD,_ABCD}, {_ABCD,_AD,_ACD,_D}, {_D,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_D}, {_D,_AD,_ABD,_ABCD}},	// key = 31 = 00000011111	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 32 = 00000100000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 33 = 00000100001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 34 = 00000100010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 35 = 00000100011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 36 = 00000100100	(invalid)
{{_B,_AB ,_BC,_BD}, {_BC,_D,_AB ,_C}, {_AB ,_C,_D,_A}, {_BD,_AB ,_BC,_D}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 37 = 00000100101	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 38 = 00000100110	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 39 = 00000100111	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 40 = 00000101000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 41 = 00000101001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 42 = 00000101010	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 43 = 00000101011	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 44 = 00000101100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 45 = 00000101101	(invalid)
{{_CD,_AB ,_BC,_C}, {_C,_AB ,_AD,_CD}, {_A,_C,_AB ,_AD}, {_D,_AB ,_BC,_CD}, {_CD,_AB ,_AD,_D}, {_B,_D,_AB ,_BC}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 46 = 00000101110	(VALID)
    {{_A,_AB ,_ABD,_ABCD}, {_ABCD,_AB ,_ABC,_A}, {_A,_AC,_ABC,_ABCD}, {_ABCD,_AD,_ABD,_A}, {_A,_AD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_A}, {_ABCD,_BD,_BCD,_B}, {_B,_BD,_ABD,_ABCD}, {_ABCD,_AB ,_ABD,_B}, {_B,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_B}, {_B,_AB ,_ABC,_ABCD},
     {_ABCD,_BC,_BCD,_C}, {_C,_BC,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_C}, {_C,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_C}, {_C,_AC,_ACD,_ABCD}, {_ABCD,_CD,_BCD,_D}, {_D,_CD,_ACD,_ABCD}, {_ABCD,_AD,_ACD,_D}, {_D,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_D}, {_D,_AD,_ABD,_ABCD}},	// key = 47 = 00000101111	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 48 = 00000110000	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 49 = 00000110001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 50 = 00000110010	(invalid)
{{_B,_AC,_CD,_BD}, {_B,_AB ,_AC,_BD}, {_CD,_C,_AC,_B}, {_BD,_AC,_CD,_D}, {_BD,_AB ,_AC,_D}, {_AC,_D,_AB ,_A}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 51 = 00000110011	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 52 = 00000110100	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 53 = 00000110101	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 54 = 00000110110	(invalid)
{{_A,_AB ,_ABD,_ABCD}, {_ABCD,_AB ,_ABC,_A}, {_A,_AC,_ABC,_ABCD}, {_ABCD,_AD,_ABD,_A}, {_A,_AD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_A}, {_ABCD,_BD,_BCD,_B}, {_B,_BD,_ABD,_ABCD}, {_ABCD,_AB ,_ABD,_B}, {_B,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_B}, {_B,_AB ,_ABC,_ABCD},
 {_ABCD,_BC,_BCD,_C}, {_C,_BC,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_C}, {_C,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_C}, {_C,_AC,_ACD,_ABCD}, {_ABCD,_CD,_BCD,_D}, {_D,_CD,_ACD,_ABCD}, {_ABCD,_AD,_ACD,_D}, {_D,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_D}, {_D,_AD,_ABD,_ABCD}},	// key = 55 = 00000110111	(VALID)
{{_AD,_AB ,_AC,_A}, {_B,_C,_D,_AC}, {_B,_D,_AB ,_AC}, {_D,_AB ,_AC,_AD}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 56 = 00000111000	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 57 = 00000111001	(invalid)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 58 = 00000111010	(invalid)
{{_A,_AB ,_ABD,_ABCD}, {_ABCD,_AB ,_ABC,_A}, {_A,_AC,_ABC,_ABCD}, {_ABCD,_AD,_ABD,_A}, {_A,_AD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_A}, {_ABCD,_BD,_BCD,_B}, {_B,_BD,_ABD,_ABCD}, {_ABCD,_AB ,_ABD,_B}, {_B,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_B}, {_B,_AB ,_ABC,_ABCD},
 {_ABCD,_BC,_BCD,_C}, {_C,_BC,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_C}, {_C,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_C}, {_C,_AC,_ACD,_ABCD}, {_ABCD,_CD,_BCD,_D}, {_D,_CD,_ACD,_ABCD}, {_ABCD,_AD,_ACD,_D}, {_D,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_D}, {_D,_AD,_ABD,_ABCD}},	// key = 59 = 00000111011	(VALID)
{{_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O},
 {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}, {_O,_O,_O,_O}},	// key = 60 = 00000111100	(invalid)
{{_A,_AB ,_ABD,_ABCD}, {_ABCD,_AB ,_ABC,_A}, {_A,_AC,_ABC,_ABCD}, {_ABCD,_AD,_ABD,_A}, {_A,_AD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_A}, {_ABCD,_BD,_BCD,_B}, {_B,_BD,_ABD,_ABCD}, {_ABCD,_AB ,_ABD,_B}, {_B,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_B}, {_B,_AB ,_ABC,_ABCD},
 {_ABCD,_BC,_BCD,_C}, {_C,_BC,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_C}, {_C,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_C}, {_C,_AC,_ACD,_ABCD}, {_ABCD,_CD,_BCD,_D}, {_D,_CD,_ACD,_ABCD}, {_ABCD,_AD,_ACD,_D}, {_D,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_D}, {_D,_AD,_ABD,_ABCD}},	// key = 61 = 00000111101	(VALID)
{{_A,_AB ,_ABD,_ABCD}, {_ABCD,_AB ,_ABC,_A}, {_A,_AC,_ABC,_ABCD}, {_ABCD,_AD,_ABD,_A}, {_A,_AD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_A}, {_ABCD,_BD,_BCD,_B}, {_B,_BD,_ABD,_ABCD}, {_ABCD,_AB ,_ABD,_B}, {_B,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_B}, {_B,_AB ,_ABC,_ABCD},
 {_ABCD,_BC,_BCD,_C}, {_C,_BC,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_C}, {_C,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_C}, {_C,_AC,_ACD,_ABCD}, {_ABCD,_CD,_BCD,_D}, {_D,_CD,_ACD,_ABCD}, {_ABCD,_AD,_ACD,_D}, {_D,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_D}, {_D,_AD,_ABD,_ABCD}},	// key = 62 = 00000111110	(VALID)
{{_A,_AB ,_ABD,_ABCD}, {_ABCD,_AB ,_ABC,_A}, {_A,_AC,_ABC,_ABCD}, {_ABCD,_AD,_ABD,_A}, {_A,_AD,_ACD,_ABCD}, {_ABCD,_AC,_ACD,_A}, {_ABCD,_BD,_BCD,_B}, {_B,_BD,_ABD,_ABCD}, {_ABCD,_AB ,_ABD,_B}, {_B,_BC,_BCD,_ABCD}, {_ABCD,_BC,_ABC,_B}, {_B,_AB ,_ABC,_ABCD},
 {_ABCD,_BC,_BCD,_C}, {_C,_BC,_ABC,_ABCD}, {_ABCD,_AC,_ABC,_C}, {_C,_CD,_BCD,_ABCD}, {_ABCD,_CD,_ACD,_C}, {_C,_AC,_ACD,_ABCD}, {_ABCD,_CD,_BCD,_D}, {_D,_CD,_ACD,_ABCD}, {_ABCD,_AD,_ACD,_D}, {_D,_BD,_BCD,_ABCD}, {_ABCD,_BD,_ABD,_D}, {_D,_AD,_ABD,_ABCD}}	// key = 63 = 00000111111	(VALID)
};
*/
