//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Generalized Interface Table
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


//======================================================
// parity_flip
// Index by Tet ID
// Lookup Gives BOOLEAN indicating if tet is even/odd
//======================================================
static bool parity_flip[24] = {false,  // 0
                               true,   // 1
                               false,  // 2
                               true,   // 3
                               false,  // 4
                               true,   // 5
                               false,  // 6
                               true,   // 7
                               false,  // 8
                               true,   // 9
                               false,  // 10
                               true,   // 11
                               false,  // 12
                               true,   // 13
                               true,   // 14
                               false,  // 15
                               false,  // 16
                               true,   // 17
                               false,  // 18
                               true,   // 19
                               false,  // 20
                               true,   // 21
                               false,  // 22
                               true};  // 23


//=======================================================
// Tet Parity Table
// Index by Tet ID
// Lookup Gives BOOLEAN indicating if tet should flip
//======================================================
const bool tetParityTable[24] = {            //----- Left  Face Tets
                                  true,      //  TLU
                                  false,     //  TLL
                                  true,      //  TLF
                                  false,     //  TLB,
                                             //----- Right Face Tets
                                  true,      //  TRU
                                  false,     //  TRL
                                  true,      //  TRF
                                  false,     //  TRB
                                             //----- Front Face Tets
                                  false,     //  TFT
                                  true,      //  TFB
                                  false,     //  TFL
                                  true,      //  TFR
                                             //----- Back  Face Tets
                                  false,     //  TBT
                                  true,      //  TBB
                                  false,     //  TBL
                                  true,      //  TBR
                                             //----- Down  Face Tets
                                  false,     // TDF
                                  true,      // TDB
                                  true,      // TDL
                                  false,     // TDR
                                             //----- Upper Face Tets
                                  false,     //  TUF
                                  true,      //  TUB
                                  true,      //  TUL
                                  false};    //  TUR


const int interfaceTableEven[64][12][2] = {
    {{_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 0 = 00000000000	(VALID)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 1 = 00000000001	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 2 = 00000000010	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 3 = 00000000011	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 4 = 00000000100	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 5 = 00000000101	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 6 = 00000000110	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 7 = 00000000111	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 8 = 00000001000	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 9 = 00000001001	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 10 = 00000001010	(invalid)
    {{_BD,_CD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 11 = 00000001011	(VALID)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 12 = 00000001100	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 13 = 00000001101	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 14 = 00000001110	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 15 = 00000001111	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 16 = 00000010000	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 17 = 00000010001	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 18 = 00000010010	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 19 = 00000010011	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 20 = 00000010100	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 21 = 00000010101	(invalid)
    {{_CD,_BC}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 22 = 00000010110	(VALID)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 23 = 00000010111	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 24 = 00000011000	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 25 = 00000011001	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 26 = 00000011010	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 27 = 00000011011	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 28 = 00000011100	(invalid)
    {{_BD,_AD}, {_BD,_BC}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 29 = 00000011101	(VALID)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 30 = 00000011110	(invalid)
    {{_BC,_AC}, {_BC,_BCD}, {_CD,_BCD}, {_BD,_AD}, {_BD,_BCD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 31 = 00000011111	(VALID)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 32 = 00000100000	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 33 = 00000100001	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 34 = 00000100010	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 35 = 00000100011	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 36 = 00000100100	(invalid)
    {{_BD,_BC}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 37 = 00000100101	(VALID)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 38 = 00000100110	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 39 = 00000100111	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 40 = 00000101000	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 41 = 00000101001	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 42 = 00000101010	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 43 = 00000101011	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 44 = 00000101100	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 45 = 00000101101	(invalid)
    {{_CD,_AD}, {_CD,_BC}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 46 = 00000101110	(VALID)
    {{_BC,_AB }, {_BC,_BCD}, {_CD,_AD}, {_CD,_BCD}, {_BD,_BCD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 47 = 00000101111	(VALID)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 48 = 00000110000	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 49 = 00000110001	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 50 = 00000110010	(invalid)
    {{_BD,_AB }, {_BD,_CD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 51 = 00000110011	(VALID)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 52 = 00000110100	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 53 = 00000110101	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 54 = 00000110110	(invalid)
    {{_BC,_BCD}, {_CD,_AC}, {_CD,_BCD}, {_BD,_AB }, {_BD,_BCD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 55 = 00000110111	(VALID)
    {{_AD,_AB }, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 56 = 00000111000	(VALID)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 57 = 00000111001	(invalid)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 58 = 00000111010	(invalid)
    {{_AB ,_AC}, {_AB ,_ABD}, {_AD,_ABD}, {_BD,_ABD}, {_BD,_CD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 59 = 00000111011	(VALID)
    {{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 60 = 00000111100	(invalid)
    {{_AB ,_ABD}, {_AD,_ABD}, {_AD,_AC}, {_BD,_ABD}, {_BD,_BC}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 61 = 00000111101	(VALID)
    {{_AC,_ACD}, {_AD,_AB }, {_AD,_ACD}, {_CD,_ACD}, {_CD,_BC}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 62 = 00000111110	(VALID)
    {{_AB ,_ABC}, {_AB ,_ABD}, {_AC,_ABC}, {_AC,_ACD}, {_AD,_ABD}, {_AD,_ACD}, {_BC,_ABC}, {_BC,_BCD}, {_CD,_ACD}, {_CD,_BCD}, {_BD,_ABD}, {_BD,_BCD}}	// key = 63 = 00000111111	(VALID)
};


const int interfaceTableOdd[64][12][2] = {
{{_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 0 = 00000000000	(VALID)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 1 = 00000000001	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 2 = 00000000010	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 3 = 00000000011	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 4 = 00000000100	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 5 = 00000000101	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 6 = 00000000110	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 7 = 00000000111	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 8 = 00000001000	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 9 = 00000001001	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 10 = 00000001010	(invalid)
{{_BD,_CD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 11 = 00000001011	(VALID)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 12 = 00000001100	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 13 = 00000001101	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 14 = 00000001110	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 15 = 00000001111	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 16 = 00000010000	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 17 = 00000010001	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 18 = 00000010010	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 19 = 00000010011	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 20 = 00000010100	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 21 = 00000010101	(invalid)
{{_BC,_CD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 22 = 00000010110	(VALID)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 23 = 00000010111	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 24 = 00000011000	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 25 = 00000011001	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 26 = 00000011010	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 27 = 00000011011	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 28 = 00000011100	(invalid)
{{_BD,_AD}, {_BD,_BC}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 29 = 00000011101	(VALID)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 30 = 00000011110	(invalid)
{{_BC,_AC}, {_BC,_BCD}, {_CD,_BCD}, {_BD,_AD}, {_BD,_BCD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 31 = 00000011111	(VALID)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 32 = 00000100000	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 33 = 00000100001	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 34 = 00000100010	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 35 = 00000100011	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 36 = 00000100100	(invalid)
{{_BD,_BC}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 37 = 00000100101	(VALID)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 38 = 00000100110	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 39 = 00000100111	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 40 = 00000101000	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 41 = 00000101001	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 42 = 00000101010	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 43 = 00000101011	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 44 = 00000101100	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 45 = 00000101101	(invalid)
{{_BC,_AB }, {_BC,_CD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 46 = 00000101110	(VALID)
{{_BC,_AB }, {_BC,_BCD}, {_CD,_AD}, {_CD,_BCD}, {_BD,_BCD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 47 = 00000101111	(VALID)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 48 = 00000110000	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 49 = 00000110001	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 50 = 00000110010	(invalid)
{{_BD,_AB }, {_BD,_CD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 51 = 00000110011	(VALID)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 52 = 00000110100	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 53 = 00000110101	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 54 = 00000110110	(invalid)
{{_BC,_BCD}, {_CD,_AC}, {_CD,_BCD}, {_BD,_AB }, {_BD,_BCD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 55 = 00000110111	(VALID)
{{_AB ,_AD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 56 = 00000111000	(VALID)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 57 = 00000111001	(invalid)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 58 = 00000111010	(invalid)
{{_AB ,_AC}, {_AB ,_ABD}, {_AD,_ABD}, {_BD,_ABD}, {_BD,_CD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 59 = 00000111011	(VALID)
{{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O},{_O,_O}}, 	// key = 60 = 00000111100	(invalid)
{{_AB ,_ABD}, {_AD,_ABD}, {_AD,_AC}, {_BD,_ABD}, {_BD,_BC}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 61 = 00000111101	(VALID)
{{_AB ,_AD}, {_AC,_ACD}, {_AD,_ACD}, {_BC,_CD}, {_CD,_ACD}, {_D,_AD}, {_D,_CD}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}, {_O,_O}},	// key = 62 = 00000111110	(VALID)
{{_AB ,_ABC}, {_AB ,_ABD}, {_AC,_ABC}, {_AC,_ACD}, {_AD,_ABD}, {_AD,_ACD}, {_BC,_ABC}, {_BC,_BCD}, {_CD,_ACD}, {_CD,_BCD}, {_BD,_ABD}, {_BD,_BCD}}	// key = 63 = 00000111111	(VALID)
};



const int completeInterfaceTable[12][2] = {{_AB, _ABC},
                                           {_AB, _ABD},
                                           {_AC, _ABC},
                                           {_AC, _ACD},
                                           {_AD, _ABD},
                                           {_AD, _ACD},
                                           {_BC, _ABC},
                                           {_BC, _BCD},
                                           {_BD, _ABD},
                                           {_BD, _BCD},
                                           {_CD, _ACD},
                                           {_CD, _BCD}};

const int simpleInterfaceTable[4][6] = {{ _AB, _ABC, _AC, _ACD, _AD, _ABD },
                                        { _AB, _ABC, _BC, _BCD, _BD, _ABD },
                                        { _BC, _ABC, _AC, _ACD, _CD, _BCD },
                                        { _CD, _ACD, _AD, _ABD, _BD, _BCD }};

