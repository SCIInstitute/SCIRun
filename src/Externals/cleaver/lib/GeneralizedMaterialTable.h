//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Generalized Material Table
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


const int materialTableEven[64][24] = {
{_A,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 0 = 00000000000	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 1 = 00000000001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 2 = 00000000010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 3 = 00000000011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 4 = 00000000100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 5 = 00000000101	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 6 = 00000000110	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 7 = 00000000111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 8 = 00000001000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 9 = 00000001001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 10 = 00000001010	(invalid)
{_B,_B,_A,_D,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 11 = 00000001011	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 12 = 00000001100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 13 = 00000001101	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 14 = 00000001110	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 15 = 00000001111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 16 = 00000010000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 17 = 00000010001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 18 = 00000010010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 19 = 00000010011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 20 = 00000010100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 21 = 00000010101	(invalid)
{_D,_B,_B,_C,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 22 = 00000010110	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 23 = 00000010111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 24 = 00000011000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 25 = 00000011001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 26 = 00000011010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 27 = 00000011011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 28 = 00000011100	(invalid)
{_B,_B,_A,_C,_D,_D, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 29 = 00000011101	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 30 = 00000011110	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 31 = 00000011111	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 32 = 00000100000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 33 = 00000100001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 34 = 00000100010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 35 = 00000100011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 36 = 00000100100	(invalid)
{_B,_C,_A,_D,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 37 = 00000100101	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 38 = 00000100110	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 39 = 00000100111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 40 = 00000101000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 41 = 00000101001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 42 = 00000101010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 43 = 00000101011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 44 = 00000101100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 45 = 00000101101	(invalid)
{_D,_B,_B,_C,_C,_C, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 46 = 00000101110	(VALID)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 47 = 00000101111	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 48 = 00000110000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 49 = 00000110001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 50 = 00000110010	(invalid)
{_B,_B,_B,_D,_D,_A, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 51 = 00000110011	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 52 = 00000110100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 53 = 00000110101	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 54 = 00000110110	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 55 = 00000110111	(VALID)
{_A,_C,_B,_B,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 56 = 00000111000	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 57 = 00000111001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 58 = 00000111010	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 59 = 00000111011	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 60 = 00000111100	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 61 = 00000111101	(VALID)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 62 = 00000111110	(VALID)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D}	// key = 63 = 00000111111	(VALID)
};

const int materialTableOdd[64][24] = {
{_A,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 0 = 00000000000	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 1 = 00000000001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 2 = 00000000010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 3 = 00000000011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 4 = 00000000100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 5 = 00000000101	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 6 = 00000000110	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 7 = 00000000111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 8 = 00000001000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 9 = 00000001001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 10 = 00000001010	(invalid)
{_B,_B,_A,_D,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 11 = 00000001011	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 12 = 00000001100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 13 = 00000001101	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 14 = 00000001110	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 15 = 00000001111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 16 = 00000010000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 17 = 00000010001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 18 = 00000010010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 19 = 00000010011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 20 = 00000010100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 21 = 00000010101	(invalid)
{_C,_D,_D,_B,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 22 = 00000010110	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 23 = 00000010111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 24 = 00000011000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 25 = 00000011001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 26 = 00000011010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 27 = 00000011011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 28 = 00000011100	(invalid)
{_B,_B,_A,_C,_D,_D, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 29 = 00000011101	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 30 = 00000011110	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 31 = 00000011111	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 32 = 00000100000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 33 = 00000100001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 34 = 00000100010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 35 = 00000100011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 36 = 00000100100	(invalid)
{_B,_C,_A,_D,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 37 = 00000100101	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 38 = 00000100110	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 39 = 00000100111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 40 = 00000101000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 41 = 00000101001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 42 = 00000101010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 43 = 00000101011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 44 = 00000101100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 45 = 00000101101	(invalid)
{_C,_C,_C,_D,_D,_D, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 46 = 00000101110	(VALID)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 47 = 00000101111	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 48 = 00000110000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 49 = 00000110001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 50 = 00000110010	(invalid)
{_B,_B,_B,_D,_D,_A, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 51 = 00000110011	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 52 = 00000110100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 53 = 00000110101	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 54 = 00000110110	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 55 = 00000110111	(VALID)
{_A,_C,_D,_D,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 56 = 00000111000	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 57 = 00000111001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 58 = 00000111010	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 59 = 00000111011	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 60 = 00000111100	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 61 = 00000111101	(VALID)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 62 = 00000111110	(VALID)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D}	// key = 63 = 00000111111	(VALID)
};


/*
const int materialTable[64][24] = {
{_A,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 0 = 00000000000	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 1 = 00000000001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 2 = 00000000010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 3 = 00000000011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 4 = 00000000100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 5 = 00000000101	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 6 = 00000000110	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 7 = 00000000111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 8 = 00000001000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 9 = 00000001001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 10 = 00000001010	(invalid)
{_B,_B,_A,_D,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 11 = 00000001011	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 12 = 00000001100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 13 = 00000001101	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 14 = 00000001110	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 15 = 00000001111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 16 = 00000010000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 17 = 00000010001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 18 = 00000010010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 19 = 00000010011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 20 = 00000010100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 21 = 00000010101	(invalid)
{_C,_D,_D,_B,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 22 = 00000010110	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 23 = 00000010111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 24 = 00000011000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 25 = 00000011001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 26 = 00000011010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 27 = 00000011011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 28 = 00000011100	(invalid)
{_B,_B,_A,_C,_D,_D, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 29 = 00000011101	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 30 = 00000011110	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 31 = 00000011111	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 32 = 00000100000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 33 = 00000100001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 34 = 00000100010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 35 = 00000100011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 36 = 00000100100	(invalid)
{_B,_C,_A,_D,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 37 = 00000100101	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 38 = 00000100110	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 39 = 00000100111	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 40 = 00000101000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 41 = 00000101001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 42 = 00000101010	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 43 = 00000101011	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 44 = 00000101100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 45 = 00000101101	(invalid)
{_C,_C,_C,_D,_D,_D, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 46 = 00000101110	(VALID)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 47 = 00000101111	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 48 = 00000110000	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 49 = 00000110001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 50 = 00000110010	(invalid)
{_B,_B,_B,_D,_D,_A, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 51 = 00000110011	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 52 = 00000110100	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 53 = 00000110101	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 54 = 00000110110	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 55 = 00000110111	(VALID)
{_A,_C,_D,_D,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 56 = 00000111000	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 57 = 00000111001	(invalid)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 58 = 00000111010	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 59 = 00000111011	(VALID)
{_O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O, _O,_O,_O,_O,_O,_O},	// key = 60 = 00000111100	(invalid)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 61 = 00000111101	(VALID)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D},	// key = 62 = 00000111110	(VALID)
{_A,_A,_A,_A,_A,_A, _B,_B,_B,_B,_B,_B, _C,_C,_C,_C,_C,_C, _D,_D,_D,_D,_D,_D}	// key = 63 = 00000111111	(VALID)
};
*/
