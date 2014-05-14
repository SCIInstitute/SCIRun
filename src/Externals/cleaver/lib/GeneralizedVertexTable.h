//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Generalized Vertex Table
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


const int vertexTableOdd[64][15] = {
  {_A,_B,_C,_D,_A,_A,_A,_C,_C,_D,_A,_A,_A,_C,_A},	// key = 0 = 000000	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 1 = 000001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 2 = 000010	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 3 = 000011	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 4 = 000100	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 5 = 000101	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 6 = 000110	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 7 = 000111	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 8 = 001000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 9 = 001001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 10 = 001010	(invalid)
  {_A,_B,_C,_D,_A,_A,_AD,_C,_CD,_BD,_A,_AD,_AD,_CD,_AD},	// key = 11 = 001011	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 12 = 001100	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 13 = 001101	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 14 = 001110	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 15 = 001111	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 16 = 010000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 17 = 010001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 18 = 010010	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 19 = 010011	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 20 = 010100	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 21 = 010101	(invalid)
  {_A,_B,_C,_D,_A,_AC,_A,_BC,_CD,_D,_AC,_AC,_A,_CD,_AC},	// key = 22 = 010110	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 23 = 010111	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 24 = 011000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 25 = 011001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 26 = 011010	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 27 = 011011	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 28 = 011100	(invalid)
  {_A,_B,_C,_D,_A,_AC,_AD,_BC,_C,_BD,_AC,_AC,_AD,_BC,_AC},	// key = 29 = 011101	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 30 = 011110	(invalid)
  {_A,_B,_C,_D,_A,_AC,_AD,_BC,_CD,_BD,_AC,_ACD,_AD,_BCD,_ACD},	// key = 31 = 011111	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 32 = 100000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 33 = 100001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 34 = 100010	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 35 = 100011	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 36 = 100100	(invalid)
  {_A,_B,_C,_D,_AB ,_A,_A,_BC,_C,_BD,_AB ,_A,_AB ,_BC,_AB },	// key = 37 = 100101	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 38 = 100110	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 39 = 100111	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 40 = 101000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 41 = 101001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 42 = 101010	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 43 = 101011	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 44 = 101100	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 45 = 101101	(invalid)
  {_A,_B,_C,_D,_AB ,_A,_AD,_BC,_CD,_D,_AB ,_AD,_AD,_CD,_AD},	// key = 46 = 101110	(VALID)
  {_A,_B,_C,_D,_AB ,_A,_AD,_BC,_CD,_BD,_AB ,_AD,_ABD,_BCD,_ABD},	// key = 47 = 101111	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 48 = 110000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 49 = 110001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 50 = 110010	(invalid)
  {_A,_B,_C,_D,_AB ,_AC,_A,_C,_CD,_BD,_AC,_AC,_AB ,_CD,_AC},	// key = 51 = 110011	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 52 = 110100	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 53 = 110101	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 54 = 110110	(invalid)
  {_A,_B,_C,_D,_AB ,_AC,_A,_BC,_CD,_BD,_ABC,_AC,_AB ,_BCD,_ABC},	// key = 55 = 110111	(VALID)
  {_A,_B,_C,_D,_AB ,_AC,_AD,_C,_C,_D,_AC,_AC,_AD,_C,_AC},	// key = 56 = 111000	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 57 = 111001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 58 = 111010	(invalid)
  {_A,_B,_C,_D,_AB ,_AC,_AD,_C,_CD,_BD,_AC,_ACD,_ABD,_CD,_ACD},	// key = 59 = 111011	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 60 = 111100	(invalid)
  {_A,_B,_C,_D,_AB ,_AC,_AD,_BC,_C,_BD,_ABC,_AC,_ABD,_BC,_ABC},	// key = 61 = 111101	(VALID)
  {_A,_B,_C,_D,_AB ,_AC,_AD,_BC,_CD,_D,_ABC,_ACD,_AD,_CD,_ABC},	// key = 62 = 111110	(VALID)
  {_A,_B,_C,_D,_AB ,_AC,_AD,_BC,_CD,_BD,_ABC,_ACD,_ABD,_BCD,_ABCD}	// key = 63 = 111111	(VALID)
};

const int vertexTableEven[64][15] = {
  {_A,_B,_C,_D,_A,_A,_A,_C,_C,_B,_A,_A,_A,_C,_A},	// key = 0 = 000000	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 1 = 000001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 2 = 000010	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 3 = 000011	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 4 = 000100	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 5 = 000101	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 6 = 000110	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 7 = 000111	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 8 = 001000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 9 = 001001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 10 = 001010	(invalid)
  {_A,_B,_C,_D,_A,_A,_AD,_C,_CD,_BD,_A,_AD,_AD,_CD,_AD},	// key = 11 = 001011	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 12 = 001100	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 13 = 001101	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 14 = 001110	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 15 = 001111	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 16 = 010000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 17 = 010001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 18 = 010010	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 19 = 010011	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 20 = 010100	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 21 = 010101	(invalid)
  {_A,_B,_C,_D,_A,_AC,_A,_BC,_CD,_B,_AC,_AC,_A,_BC,_AC},	// key = 22 = 010110	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 23 = 010111	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 24 = 011000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 25 = 011001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 26 = 011010	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 27 = 011011	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 28 = 011100	(invalid)
  {_A,_B,_C,_D,_A,_AC,_AD,_BC,_C,_BD,_AC,_AC,_AD,_BC,_AC},	// key = 29 = 011101	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 30 = 011110	(invalid)
  {_A,_B,_C,_D,_A,_AC,_AD,_BC,_CD,_BD,_AC,_ACD,_AD,_BCD,_ACD},	// key = 31 = 011111	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 32 = 100000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 33 = 100001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 34 = 100010	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 35 = 100011	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 36 = 100100	(invalid)
  {_A,_B,_C,_D,_AB ,_A,_A,_BC,_C,_BD,_AB ,_A,_AB ,_BC,_AB },	// key = 37 = 100101	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 38 = 100110	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 39 = 100111	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 40 = 101000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 41 = 101001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 42 = 101010	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 43 = 101011	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 44 = 101100	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 45 = 101101	(invalid)
  {_A,_B,_C,_D,_AB ,_A,_AD,_BC,_CD,_B,_AB ,_AD,_AB ,_BC,_AB },	// key = 46 = 101110	(VALID)
  {_A,_B,_C,_D,_AB ,_A,_AD,_BC,_CD,_BD,_AB ,_AD,_ABD,_BCD,_ABD},	// key = 47 = 101111	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 48 = 110000	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 49 = 110001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 50 = 110010	(invalid)
  {_A,_B,_C,_D,_AB ,_AC,_A,_C,_CD,_BD,_AC,_AC,_AB ,_CD,_AC},	// key = 51 = 110011	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 52 = 110100	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 53 = 110101	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 54 = 110110	(invalid)
  {_A,_B,_C,_D,_AB ,_AC,_A,_BC,_CD,_BD,_ABC,_AC,_AB ,_BCD,_ABC},	// key = 55 = 110111	(VALID)
  {_A,_B,_C,_D,_AB ,_AC,_AD,_C,_C,_B,_AC,_AC,_AB ,_C,_AC},	// key = 56 = 111000	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 57 = 111001	(invalid)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 58 = 111010	(invalid)
  {_A,_B,_C,_D,_AB ,_AC,_AD,_C,_CD,_BD,_AC,_ACD,_ABD,_CD,_ACD},	// key = 59 = 111011	(VALID)
  {_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O,_O}, 	// key = 60 = 111100	(invalid)
  {_A,_B,_C,_D,_AB ,_AC,_AD,_BC,_C,_BD,_ABC,_AC,_ABD,_BC,_ABC},	// key = 61 = 111101	(VALID)
  {_A,_B,_C,_D,_AB ,_AC,_AD,_BC,_CD,_B,_ABC,_ACD,_AB ,_BC,_ABC},	// key = 62 = 111110	(VALID)
  {_A,_B,_C,_D,_AB ,_AC,_AD,_BC,_CD,_BD,_ABC,_ACD,_ABD,_BCD,_ABCD}	// key = 63 = 111111	(VALID)
};

