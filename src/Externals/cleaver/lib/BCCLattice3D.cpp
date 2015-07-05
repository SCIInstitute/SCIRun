//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- BCC Lattice
//
// Author: Jonathan Bronson (bronson@sci.utah.edu)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
//  Copyright (C) 2011, 2012, Jonathan Bronson
//  Scientific Computing  &  Imaging Institute
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


#include <cstdlib>
#include <cmath>
#include <fstream>
#include "BCCLattice3D.h"
#include "GeneralizedStencilTable.h"
#include "TetMesh.h"
#include "Volume.h"
#include "ScalarField.h"


using namespace std;
using namespace Cleaver;


int BCCLattice3D::MaxNumTets = 24;

//======================================================
// Adjacent Triangle Lookup Tables
// Index by Edge ID
// Lookup Gives 26 ctree Quadrant + Face ID  [Oct][Face]
// CORRECTNESS : VERFIED through visualization 9/30/11
//======================================================             // Dual Edges
const int dualLongEdgeFaceGroup[6][4] = {{FLUF ,FLUB, FLLF ,FLLB},   // -CL
                                         {FRUF, FRUB, FRLF, FRLB},   // -CR
                                         {FUFL, FUFR, FUBL, FUBR},   // -CU
                                         {FDFL, FDFR, FDBL, FDBR},   // -CD
                                         {FFUL, FFUR, FFLL, FFLR},   // -CF
                                         {FBUL, FBUR, FBLL, FBLR}};  // -CB

                                               // Top edges
const int primalLongEdgeFaceGroup[12][4][2] = {{{CC,FUL},{CL,FUR},{CU,FLL},{UL,FLR}},   // -UL
                                               {{CC,FUR},{CR,FUL},{CU,FLR},{UR,FLL}},   // -UR
                                               {{CC,FUF},{CF,FUB},{CU,FLF},{UF,FLB}},   // -UF
                                               {{CC,FUB},{CB,FUF},{CU,FLB},{UB,FLF}},   // -UB
                                               // Bottom Face Edges
                                               {{CC,FLL},{CL,FLR},{CD,FUL},{LL,FUR}},    // -LL
                                               {{CC,FLR},{CR,FLL},{CD,FUR},{LR,FUL}},    // -LR
                                               {{CC,FLF},{CF,FLB},{CD,FUF},{LF,FUB}},    // -LF
                                               {{CC,FLB},{CB,FLF},{CD,FUB},{LB,FUF}},    // -LB
                                               // Four Column Edges
                                               {{CC,FFL},{CL,FFR},{CF,FBL},{FL,FBR}},     // FL
                                               {{CC,FFR},{CR,FFL},{CF,FBR},{FR,FBL}},     // FR
                                               {{CC,FBL},{CL,FBR},{CB,FFL},{BL,FFR}},     // BL
                                               {{CC,FBR},{CR,FBL},{CB,FFR},{BR,FFL}}};    // BR



const int shortEdgeFaceGroup[8][6] = {{FUL,FUF,FFL,FFUL,FLUF,FUFL},   // DULF
                                      {FUL,FUB,FBL,FBUL,FLUB,FUBL},   // DULB
                                      {FUR,FUF,FFR,FFUR,FRUF,FUFR},   // DURF
                                      {FUR,FUB,FBR,FBUR,FRUB,FUBR},   // DURB

                                      {FLL,FLF,FFL,FFLL,FLLF,FDFL},   // DLLF
                                      {FLL,FLB,FBL,FBLL,FLLB,FDBL},   // DLLB
                                      {FLR,FLF,FFR,FFLR,FRLF,FDFR},   // DLRF
                                      {FLR,FLB,FBR,FBLR,FRLB,FDBR}};  // DLRB


//======================================================
// Adjacent Tet Lookup Tables
// Index by Edge ID
// Lookup Gives 26 ctree Quadrant + Tet ID  [Oct][Tet]
// CORRECTNESS : VERIFIED through Visualization 9/30/11
//======================================================
const int dualLongEdgeTetGroup[8][6] = {{TLU, TLL, TLF, TLB},            // CL
                                        {TRU, TRL, TRF, TRB},            // CR
                                        {TUF, TUB, TUL, TUR},            // CU
                                        {TDF, TDB, TDL, TDR},            // CD
                                        {TFT, TFB, TFL, TFR},            // CF
                                        {TBT, TBB, TBL, TBR}};           // CB

                                               // Top edges
const int primalLongEdgeTetGroup[12][4][2] =  {{{CC,TLU},{CC,TUL},{CL,TUR},{CU,TLL}},   // -UL
                                               {{CC,TRU},{CC,TUR},{CR,TUL},{CU,TRL}},   // -UR
                                               {{CC,TFT},{CC,TUF},{CF,TUB},{CU,TFB}},   // -UF
                                               {{CC,TBT},{CC,TUB},{CB,TUF},{CU,TBB}},   // -UB
                                               // Bottom Face Edges
                                               {{CC,TLL},{CC,TDL},{CL,TDR},{CD,TLU}},   // -LL
                                               {{CC,TRL},{CC,TDR},{CR,TDL},{CD,TRU}},   // -LR
                                               {{CC,TFB},{CC,TDF},{CF,TDB},{CD,TFT}},   // -LF
                                               {{CC,TBB},{CC,TDB},{CB,TDF},{CD,TBT}},   // -LB
                                               // Four Column Edges
                                               {{CC,TLF},{CC,TFL},{FL,TBR},{FL,TRB}},   // FL
                                               {{CC,TRF},{CC,TFR},{FR,TBL},{FR,TLB}},   // FR
                                               {{CC,TLB},{CC,TBL},{BL,TFR},{BL,TRF}},   // BL
                                               {{CC,TRB},{CC,TBR},{BR,TFL},{BR,TLF}}};  // BR

const int shortEdgeTetGroup[8][6] = {{TUL,TUF,TLU,TLF,TFL,TFT},   // DULF
                                     {TUL,TUB,TLU,TLB,TBL,TBT},   // DULB
                                     {TUR,TUF,TRU,TRF,TFR,TFT},   // DURF
                                     {TUR,TUB,TRU,TRB,TBR,TBT},   // DURB
                                     {TDL,TDF,TLL,TLF,TFL,TFB},   // DLLF
                                     {TDL,TDB,TLL,TLB,TBL,TBB},   // DLLB
                                     {TDR,TDF,TRL,TRF,TFR,TFB},   // DLRF
                                     {TDR,TDB,TRL,TRB,TBR,TBB}};  // DLRB

//========================================================
// Vertices of Face, Lookup Table
//   Index by face ID
//   Lookup Gives 3 Octree Quadrant + Vert ID [Oct][Vert]
// CORRECTNESS : VERIFIED through Visualization  9/28/11
//========================================================
const int faceVertexGroup[36][3][2] =
{
                                     // Triangle Faces Touching Upper Lattice Edges
    {{CC,C},{CC,ULF},{CC,ULB}},   // FUL
    {{CC,C},{CC,URF},{CC,URB}},   // FUR
    {{CC,C},{CC,ULF},{CC,URF}},   // FUF
    {{CC,C},{CC,ULB},{CC,URB}},   // FUB
                                    // Triangle Faces Touching Lower Lattice Edges
    {{CC,C},{CC,LLF},{CC,LLB}},   //FLL
    {{CC,C},{CC,LRF},{CC,LRB}},   //FLR
    {{CC,C},{CC,LLF},{CC,LRF}},   //FLF
    {{CC,C},{CC,LLB},{CC,LRB}},   //FLB
                                    // Triangle Faces Touching Four Column Edges
    {{CC,C},{CC,LLF},{CC,ULF}},   //FFL
    {{CC,C},{CC,LRF},{CC,URF}},   //FFR
    {{CC,C},{CC,LLB},{CC,ULB}},   //FBL
    {{CC,C},{CC,LRB},{CC,URB}},   //FBR,
                                    // Triangle Faces Cutting through Left  Face
    {{CC,C},{CC,ULF},{CL,C}},   //FLUF
    {{CC,C},{CC,ULB},{CL,C}},   //FLUB
    {{CC,C},{CC,LLF},{CL,C}},   //FLLF
    {{CC,C},{CC,LLB},{CL,C}},   //FLLB,
                                    // Triangle Faces Cutting through Right Face
    {{CC,C},{CC,URF},{CR,C}},   //FRUF
    {{CC,C},{CC,URB},{CR,C}},   //FRUB
    {{CC,C},{CC,LRF},{CR,C}},   //FRLF
    {{CC,C},{CC,LRB},{CR,C}},   //FRLB
                                    // Triangle Faces Cutting through Front Face
    {{CC,C},{CC,ULF},{CF,C}},   //FFUL
    {{CC,C},{CC,URF},{CF,C}},   //FFUR
    {{CC,C},{CC,LLF},{CF,C}},   //FFLL
    {{CC,C},{CC,LRF},{CF,C}},   //FFLR,
                                    // Triangle Faces Cutting through Back  Face
    {{CC,C},{CC,ULB},{CB,C}},   //FBUL
    {{CC,C},{CC,URB},{CB,C}},   //FBUR
    {{CC,C},{CC,LLB},{CB,C}},   //FBLL
    {{CC,C},{CC,LRB},{CB,C}},   //FBLR,
                                    // Triangle Faces Cutting through Upper Face
    {{CC,C},{CC,ULF},{CU,C}},   //FUFL
    {{CC,C},{CC,URF},{CU,C}},   //FUFR
    {{CC,C},{CC,ULB},{CU,C}},   //FUBL
    {{CC,C},{CC,URB},{CU,C}},   //FUBR,
                                    // Triangle Faces Cutting through Lower Face
    {{CC,C},{CC,LLF},{CD,C}},   //FDFL
    {{CC,C},{CC,LRF},{CD,C}},   //FDFR
    {{CC,C},{CC,LLB},{CD,C}},   //FDBL
    {{CC,C},{CC,LRB},{CD,C}}    //FDBR};
};

//========================================================
// Edges of Face, Lookup Table
//   Index by face ID
//   Lookup Gives 3 Octree Quadrant + Vert ID [Oct][Edge]
// CORRECTNESS : VERIFIED through Visualization  9/28/11
//========================================================
const int faceEdgeGroup[36][3][2] =
{
    // Triangle Faces Touching Upper Lattice Edges
   {{CC,UL},{CC,DULF},{CC,DULB}},   // FUL
   {{CC,UR},{CC,DURF},{CC,DURB}},   // FUR
   {{CC,UF},{CC,DULF},{CC,DURF}},   // FUF
   {{CC,UB},{CC,DULB},{CC,DURB}},   // FUB
                                    // Triangle Faces Touching Lower Lattice Edges
   {{CC,LL},{CC,DLLF},{CC,DLLB}},   //FLL
   {{CC,LR},{CC,DLRF},{CC,DLRB}},   //FLR
   {{CC,LF},{CC,DLLF},{CC,DLRF}},   //FLF
   {{CC,LB},{CC,DLLB},{CC,DLRB}},   //FLB
                                   // Triangle Faces Touching Four Column Edges
   {{CC,FL},{CC,DLLF},{CC,DULF}},   //FFL
   {{CC,FR},{CC,DLRF},{CC,DURF}},   //FFR
   {{CC,BL},{CC,DLLB},{CC,DULB}},   //FBL
   {{CC,BR},{CC,DLRB},{CC,DURB}},   //FBR,
                                   // Triangle Faces Cutting through Left  Face
   {{CC,CL},{CC,DULF},{CL,DURF}},   //FLUF
   {{CC,CL},{CC,DULB},{CL,DURB}},   //FLUB
   {{CC,CL},{CC,DLLF},{CL,DLRF}},   //FLLF
   {{CC,CL},{CC,DLLB},{CL,DLRB}},   //FLLB,
                                   // Triangle Faces Cutting through Right Face
   {{CC,CR},{CC,DURF},{CR,DULF}},   //FRUF
   {{CC,CR},{CC,DURB},{CR,DULB}},   //FRUB
   {{CC,CR},{CC,DLRF},{CR,DLLF}},   //FRLF
   {{CC,CR},{CC,DLRB},{CR,DLLB}},   //FRLB
                                   // Triangle Faces Cutting through Front Face
   {{CC,CF},{CC,DULF},{CF,DULB}},   //FFUL
   {{CC,CF},{CC,DURF},{CF,DURB}},   //FFUR
   {{CC,CF},{CC,DLLF},{CF,DLLB}},   //FFLL
   {{CC,CF},{CC,DLRF},{CF,DLRB}},   //FFLR,
                                   // Triangle Faces Cutting through Back  Face
   {{CC,CB},{CC,DULB},{CB,DULF}},   //FBUL
   {{CC,CB},{CC,DURB},{CB,DURF}},   //FBUR
   {{CC,CB},{CC,DLLB},{CB,DLLF}},   //FBLL
   {{CC,CB},{CC,DLRB},{CB,DLRF}},   //FBLR,
                                   // Triangle Faces Cutting through Upper Face
   {{CC,CU},{CC,DULF},{CU,DLLF}},   //FUFL
   {{CC,CU},{CC,DURF},{CU,DLRF}},   //FUFR
   {{CC,CU},{CC,DULB},{CU,DLLB}},   //FUBL
   {{CC,CU},{CC,DURB},{CU,DLRB}},   //FUBR,
                                   // Triangle Faces Cutting through Lower Face
   {{CC,CD},{CC,DLLF},{CD,DULF}},   //FDFL
   {{CC,CD},{CC,DLRF},{CD,DURF}},   //FDFR
   {{CC,CD},{CC,DLLB},{CD,DULB}},   //FDBL
   {{CC,CD},{CC,DLRB},{CD,DURB}}    //FDBR};
};

//======================================================
// Adjacent Tetrahedra Lookup Table
// Index by face ID
// Lookup Gives 2 adjacent Tet IDs  [Tet]
// CORRECTNESS : Verified Visually  9/30/11
//======================================================    // Triangle Faces Touching Upper Lattice Edges
const int faceTetGroup[36][2] = {{TLU,TUL},                 // FUL
                                 {TRU,TUR},                 // FUR
                                 {TFT,TUF},                 // FUF
                                 {TBT,TUB},                 // FUB
                                 /*..*/                     // Triangle Faces Touching Lower Lattice Edges
                                 {TLL,TDL},                 // FLL
                                 {TRL,TDR},                 // FLR
                                 {TFB,TDF},                 // FLF
                                 {TBB,TDB},                 // FLB
                                 /*..*/                     // Triangle Faces Touching Four Column Edges
                                 {TLF,TFL},                 // FFL
                                 {TRF,TFR},                 // FFR
                                 {TLB,TBL},                 // FBL
                                 {TRB,TBR},                 // FBR
                                 /*..*/                     // Triangle Faces Cutting through Left  Face
                                 {TLU,TLF},                 // FLUF
                                 {TLU,TLB},                 // FLUB
                                 {TLL,TLF},                 // FLLF
                                 {TLL,TLB},                 // FLLB
                                 /*..*/                     // Triangle Faces Cutting through Right Face
                                 {TRU,TRF},                 // FRUF
                                 {TRU,TRB},                 // FRUB
                                 {TRL,TRF},                 // FRLF
                                 {TRL,TRB},                 // FRLB,
                                 /*..*/                     // Triangle Faces Cutting through Front Face
                                 {TFT,TFL},                 // FFUL
                                 {TFT,TFR},                 // FFUR
                                 {TFB,TFL},                 // FFLL
                                 {TFB,TFR},                 // FFLR
                                 /*..*/                     // Triangle Faces Cutting through Back  Face
                                 {TBT,TBL},                 // FBUL
                                 {TBT,TBR},                 // FBUR
                                 {TBB,TBL},                 // FBLL
                                 {TBB,TBR},                 // FBLR
                                 /*..*/                     // Triangle Faces Cutting through Upper Face
                                 {TUF,TUL},                 // FUFL
                                 {TUF,TUR},                 // FUFR
                                 {TUB,TUL},                 // FUBL
                                 {TUB,TUR},                 // FUBR
                                 /*..*/                     // Triangle Faces Cutting through Bottom Face
                                 {TDF,TDL},                 // FDFL
                                 {TDF,TDR},                 // FDFR
                                 {TDB,TDL},                 // FDBL
                                 {TDB,TDR}};                // FDBR

//=================================================
//  Neighbor Cell Lookup Table
//    Index by Edge ID gives array which is
//    indexed by Oct Cell. Value is triplet
//    giving offsets from Cell owning input Vertex
//=================================================
const int edgeCellGroup[27][3] =
{
    // DULF        DULB        DURF        DURB
    {-1,+1,-1}, {-1,+1,+1}, {+1,+1,-1}, {+1,+1,+1},
    // DLLF        DLLB        DLRF        DLRB
    {-1,-1,-1}, {-1,-1,+1}, {+1,-1,-1}, {+1,-1,+1},
    //  CL          CR          CU,         CD          CF          CB
    {-1, 0, 0}, {+1, 0, 0}, { 0,+1, 0}, { 0,-1, 0}, { 0, 0,-1}, { 0, 0,+1},
    //  UL          UR          UF          UB
    {-1,+1, 0}, {+1,+1, 0}, { 0,+1,-1}, { 0,+1,+1},
    //  LL          LR          LF          LB
    {-1,-1, 0}, {+1,-1, 0}, { 0,-1,-1}, { 0,-1,+1},
    //  FL          FR          BL          BR          CC
    {-1, 0,-1}, {+1, 0,-1}, {-1, 0,+1}, {+1, 0,+1}, { 0, 0, 0}};

//=================================================
//  Neighbor Cell Lookup Table
//    Index by Vertex ID gives array which is
//    indexed by Oct Cell. Value is triplet
//    giving offsets from Cell owning input Vertex
//=================================================
const int vertexCellGroup[8][8][3] = {
    // ULFCell ,   ULBCell ,   URFCell ,   URBCell ,   LLFCell ,   LLBCell ,   LRFCell ,   LRBCell
    {{-1,+1,-1}, {-1,+1, 0}, { 0,+1,-1}, { 0,+1, 0}, {-1, 0,-1}, {-1, 0, 0}, { 0, 0,-1}, { 0, 0, 0}},  // ULF Vertex
    {{-1,+1, 0}, {-1,+1,+1}, { 0,+1, 0}, { 0,+1,+1}, {-1, 0, 0}, {-1, 0,+1}, { 0, 0, 0}, { 0, 0,+1}},  // ULB Vertex
    {{ 0,+1,-1}, { 0,+1, 0}, {+1,+1,-1}, {+1,+1, 0}, { 0, 0,-1}, { 0, 0, 0}, {+1, 0,-1}, {+1, 0, 0}},  // URF Vertex
    {{ 0,+1, 0}, { 0,+1,+1}, {+1,+1, 0}, {+1,+1,+1}, { 0, 0, 0}, { 0, 0,+1}, {+1, 0, 0}, {+1, 0,+1}},  // URB Vertex
    {{-1, 0,-1}, {-1, 0, 0}, { 0, 0,-1}, { 0, 0, 0}, {-1,-1,-1}, {-1,-1, 0}, { 0,-1,-1}, { 0,-1, 0}},  // LLF Vertex
    {{-1, 0, 0}, {-1, 0,+1}, { 0, 0, 0}, { 0, 0,+1}, {-1,-1, 0}, {-1,-1,+1}, { 0,-1, 0}, { 0,-1,+1}},  // LLB Vertex
    {{ 0, 0,-1}, { 0, 0, 0}, {+1, 0,-1}, {+1, 0, 0}, { 0,-1,-1}, { 0,-1, 0}, {+1,-1,-1}, {+1,-1, 0}},  // LRF Vertex
    {{ 0, 0, 0}, { 0, 0,+1}, {+1, 0, 0}, {+1, 0,+1}, { 0,-1, 0}, { 0,-1,+1}, {+1,-1, 0}, {+1,-1,+1}}   // LRB Vertex
};


//========================================================
// Adjacent Edge Lookup Table
//   Index by vertex ID
//   Lookup Gives 14 Octree Quadrant + Edge ID [Oct][Edge]
//   6 Primal Edges (x,y,z) followed by 8 Diagonals
// CORRECTNESS : VERIFIED through Visualization  9/30/11
//========================================================
const int vertexEdgeGroup[14][2] = {{URB,LF},{URB,FL},{URB,LL},
                                    {LLF,UB},{LLF,UR},{LLF,BR},
                                    {LRB,DULF},{LRF,DULB},{LLB,DURF},{LLF,DURB},
                                    {URB,DLLF},{URF,DLLB},{ULB,DLRF},{ULF,DLRB}};

//===========================================================
// Adjacent Triangle Lookup Table
//   Index by vertex ID
//   Lookup Gives 36 Octree Quadrant + Triple ID [Oct][Trip]
// CORRECTNESS : VERFIED through visualization 9/30/11
//===========================================================
const int vertexFaceGroup[36][2] = {{ULF,FBLR},{URB,FFLL},{ULF,FRLB},{URB,FLLF},// Touching Upper Edges
                                    {LLF,FBUR},{LRB,FFUL},{LLF,FRUB},{LRB,FLUF},// Touching Lower Edges
                                    {LLF,FUBR},{LRF,FUBL},{LLB,FUFR},{LRB,FUFL},// Touching 4 Columns
                                    {ULF,FLB},{ULB,FLF},{LLF,FUB},{LLB,FUF},    // Left Face
                                    {URF,FLB},{URB,FLF},{LRF,FUB},{LRB,FUF},    // Right Face
                                    {ULF,FLR},{URF,FLL},{LLF,FUR},{LRF,FUL},    // Front Face
                                    {ULB,FLR},{URB,FLL},{LLB,FUR},{LRB,FUL},    // Back Face
                                    {ULF,FBR},{URF,FBL},{ULB,FFR},{URB,FFL},    // Upper Face
                                    {LLF,FBR},{LRF,FBL},{LLB,FFR},{LRB,FFL}};   // Lower Face


//===========================================================
// Adjacent Triangle's Vertex Lookup Table (For Primals)
//   Index by vertex ID
//   Lookup Gives 36 Octree Quadrant + Vertex ID  pairs.
//   of the form [Oct][Vertex], [Oct][Vertex]
//   Third value is vertex at origin of octants.
//===========================================================
const int primalVertexFaceGroupVertices[36][6] = {{ULF,C,ULB,C},{URF,C,URB,C},{ULF,C,URF,C},{ULB,C,URB,C},// Touching Upper Edges
                                                  {LLF,C,LLB,C},{LRF,C,LRB,C},{LLF,C,LRF,C},{LLB,C,LRB,C},// Touching Lower Edges
                                                  {LLF,C,ULF,C},{LRF,C,URF,C},{LLB,C,ULB,C},{LRB,C,URB,C},// Touching 4 Columns
                                                  {ULF,C,ULF,LLB},{ULB,C,ULB,LLF},{LLF,C,LLF,ULB},{LLB,C,LLB,ULF},    // Left Face
                                                  {URF,C,URF,LRB},{URB,C,URB,LRF},{LRF,C,LRF,URB},{LRB,C,LRB,URF},    // Right Face
                                                  {ULF,C,ULF,LRF},{URF,C,URF,LLF},{LLF,C,LLF,URF},{LRF,C,LRF,ULF},    // Front Face  (TODO: Double Check This One)
                                                  {ULB,C,ULB,LRB},{URB,C,URB,LLB},{LLB,C,LLB,URB},{LRB,C,LRB,ULB},    // Back Face
                                                  {ULF,C,ULF,URB},{URF,C,URF,ULB},{ULB,C,ULB,URF},{URB,C,URB,ULF},    // Upper Face
                                                  {LLF,C,LLF,LRB},{LRF,C,LRF,LLB},{LLB,C,LLB,LRF},{LRB,C,LRB,LLF}};   // Lower Face

//===========================================================
// Adjacent Triangle's Vertex Lookup Table (For Duals)
//   Index by vertex ID
//   Lookup Gives 36 Octree Quadrant + Vertex ID  pairs.
//   of the form [Oct][Vertex], [Oct][Vertex]
//   Third value is vertex at origin of octants.
//===========================================================
const int dualVertexFaceGroupVertices[36][6] = {{CENTER, ULF, CENTER, ULB}, {CENTER, URF, CENTER, URB}, {CENTER, ULF, CENTER, URF}, {CENTER, ULB, CENTER, URB}, // Touching Upper Edges
                                                {CENTER, LLF, CENTER, LLB}, {CENTER, LRF, CENTER, LRB}, {CENTER, LLF, CENTER, LRF}, {CENTER, LLB, CENTER, LRB}, // Touching Lower Edges
                                                {CENTER, LLF, CENTER, ULF}, {CENTER, LRF, CENTER, URF}, {CENTER, LLB, CENTER, ULB}, {CENTER, LRB, CENTER, URB}, // Touching 4 Columns
                                                {LEFT,  C, CENTER, ULF}, {LEFT,  C, CENTER, ULB}, {LEFT,  C, CENTER, LLF}, {LEFT,  C, CENTER, LLB},             // Left Face
                                                {RIGHT, C, CENTER, URF}, {RIGHT, C, CENTER, URB}, {RIGHT, C, CENTER, LRF}, {RIGHT, C, CENTER, LRB},             // Right Face
                                                {FRONT, C, CENTER, ULF}, {FRONT, C, CENTER, URF}, {FRONT, C, CENTER, LLF}, {FRONT, C, CENTER, LRF},             // Front Face
                                                {BACK,  C, CENTER, ULB}, {BACK,  C, CENTER, URB}, {BACK,  C, CENTER, LLB}, {BACK,  C, CENTER, LRB},             // Back Face
                                                {UP,    C, CENTER, ULF}, {UP,    C, CENTER, URF}, {UP,    C, CENTER, ULB}, {UP,    C, CENTER, URB},             // Upper Face
                                                {DOWN,  C, CENTER, LLF}, {DOWN,  C, CENTER, LRF}, {DOWN,  C, CENTER, LLB}, {DOWN,  C, CENTER, LRB}};            // Lower Face



//======================================================
// Adjacent Tetrahedra Lookup Table
// Index by vertex ID
// Lookup Gives 24 Octree Quadrant + Tet ID  [Oct][Tet]
// CORRECTNESS : VERFIED through visualization 9/30/11
//======================================================
const int vertexTetGroup[24][2] = {{LLF,TBT},{LLF,TUB},{ULB,TFB},{ULB,TDF},     // Left  Face Tets
                                   {LRF,TBT},{LRF,TUB},{URB,TFB},{URB,TDF},     // Right Face Tets
                                   {ULF,TBR},{ULF,TRB},{URB,TFL},{URB,TLF},     // Up    Face Tets
                                   {LLF,TBR},{LLF,TRB},{LRB,TFL},{LRB,TLF},     // Down  Face Tets
                                   {LLF,TUR},{LLF,TRU},{URF,TDL},{URF,TLL},     // Front Face Tets
                                   {LLB,TUR},{LLB,TRU},{URB,TDL},{URB,TLL}};    // Back  Face Tets



//===============================================================
// Share Tet Table
//
// Index by TRUE tet index
// Returns Direction of shared CELL, and corresponding tet index
//===============================================================
const int TetTetGroup[24][2] = {
    {CL, TRU}, {CL, TRL}, {CL, TRF}, {CL,TRB},  // Left  Face Tets  -  TLU, TLL, TLF, TLB
    {CR, TLU}, {CR, TLL}, {CR, TLF}, {CR,TLB},  // Right Face Tets  -  TRU, TRL, TRF, TRB
    {CF, TBT}, {CF, TBB}, {CF, TBL}, {CF,TBR},  // Front Face Tets  -  TFT, TFB, TFL, TFR
    {CB, TFT}, {CB, TFB}, {CB, TFL}, {CB,TFR},  // Back  Face Tets  -  TBT, TBB, TBL, TBR
    {CD, TUF}, {CD, TUB}, {CD, TUL}, {CD,TUR},  // Down  Face Tets  -  TDF, TDB, TDL, TDR
    {CU, TDF}, {CU, TDB}, {CU, TDL}, {CU,TDR}   // Upper Face Tets  -  TUF, TUB, TUL, TUR
};

//===============================================================
// Share Face Table
//
// Index by TRUE face index
// Returns Direction of shared CELL, and corresponding face index
//===============================================================
const int FaceFaceGroup[36][2] = {
    {CC,FUL}, {CC,FUR}, {CC,FUF}, {CC,FUB},   // Triangle Faces Touching Upper Lattice Edges - FUL, FUR, FUF, FUB
    {CC,FLL}, {CC,FLR}, {CC,FLF}, {CC,FLB},        // Triangle Faces Touching Lower Lattice Edges - FLL, FLR, FLF, FLB
    {CC,FFL}, {CC,FFR}, {CC,FBL}, {CC,FBR},        // Triangle Faces Touching Four Column Edges - FFL, FFR, FBL, FBR
    {CL,FRUF}, {CL,FRUB}, {CL,FRLF}, {CL,FRLB},    // Triangle Faces Cutting through Left  Face - FLUF, FLUB, FLLF, FLLB
    {CR,FLUF}, {CR,FLUB}, {CR,FLLF}, {CR,FLLB},    // Triangle Faces Cutting through Right Face - FRUF, FRUB, FRLF, FRLB
    {CF,FBUL}, {CF,FBUR}, {CF,FBLL}, {CF,FBLR},    // Triangle Faces Cutting through Front Face - FFUL, FFUR, FFLL, FFLR
    {CB,FFUL}, {CB,FFUR}, {CB,FFLL}, {CB,FFLR},    // Triangle Faces Cutting through Back  Face - FBUL, FBUR, FBLL, FBLR
    {CU,FDFL}, {CU,FDFR}, {CU,FDBL}, {CU,FDBR},    // Triangle Faces Cutting through Upper Face - FUFL, FUFR, FUBL, FUBR
    {CD,FUFL}, {CD,FUFR}, {CD,FUBL}, {CD,FUBR},   // Triangle Faces Cutting through Lower Face - FDFL, FDFR, FDBL, FDBR
};

//=======================================================================
// Share Edge Table
//
// Index by TRUE edge index
// Returns set of Direction of shared CELL, and corresponding edge index
//=======================================================================
const int EdgeEdgeGroup[26][4][2] = {

    // 1 cell contains diagonal edges
    {{CC, DULF}, {_O,_O}, {_O,_O}, {_O,_O}}, // DULF
    {{CC, DULB}, {_O,_O}, {_O,_O}, {_O,_O}}, // DULB
    {{CC, DURF}, {_O,_O}, {_O,_O}, {_O,_O}}, // DURF
    {{CC, DURB}, {_O,_O}, {_O,_O}, {_O,_O}}, // DURB
    {{CC, DLLF}, {_O,_O}, {_O,_O}, {_O,_O}}, // DLLF
    {{CC, DLLB}, {_O,_O}, {_O,_O}, {_O,_O}}, // DLLB
    {{CC, DLRF}, {_O,_O}, {_O,_O}, {_O,_O}}, // DLRF
    {{CC, DLRB}, {_O,_O}, {_O,_O}, {_O,_O}}, // DLRB

    // 2 cells contain the dual edges
    {{CC, CL}, {CL, CR}, {_O,_O}, {_O,_O}},  // CL
    {{CC, CR}, {CR, CL}, {_O,_O}, {_O,_O}},  // CR
    {{CC, CU}, {CU, CD}, {_O,_O}, {_O,_O}},  // CU
    {{CC, CD}, {CD, CU}, {_O,_O}, {_O,_O}},  // CD
    {{CC, CF}, {CF, CB}, {_O,_O}, {_O,_O}},  // CF
    {{CC, CB}, {CB, CF}, {_O,_O}, {_O,_O}},  // CB

    // 4 cells contain primal edges
    {{CC, UL}, {UL,LR}, {CU,LL}, {CL,UR}}, // UL
    {{CC, UR}, {UR,LL}, {CU,LR}, {CR,UL}}, // UR
    {{CC, UF}, {UF,LB}, {CU,LF}, {CF,UB}}, // UF
    {{CC, UB}, {UB,LF}, {CU,LB}, {CB,UF}}, // UB

    {{CC, LL}, {LL,UR}, {CD,UL}, {CL,LR}}, // LL
    {{CC, LR}, {LR,UL}, {CD,UR}, {CR,LL}}, // LR
    {{CC, LF}, {LF,UB}, {CD,UF}, {CF,LB}}, // LF
    {{CC, LB}, {LB,UF}, {CD,UB}, {CB,LF}}, // LB

    {{CC, FL}, {FL,BR}, {CF,BL}, {CL,FR}}, // FL
    {{CC, FR}, {FR,BL}, {CF,BR}, {CR,FL}}, // FR
    {{CC, BL}, {BL,FR}, {CB,FL}, {CL,BR}}, // BL
    {{CC, BR}, {BR,FL}, {CB,FR}, {CR,BL}}, // BR
};

//=================================================
//  Neighbor Cell Lookup Table
//    Index by Vertex ID gives array which is
//    indexed by Oct Cell. Value is triplet
//    giving offsets from Cell owning input Vertex
//  as well as index of that vert in that cell
//=================================================
const int VertexVertexGroup[8][8][4] = {
    // ULFCell,        ULBCell,        URFCell,          URBCell,         LLFCell,         LLBCell,         LRFCell,         LRBCell
    {{-1,+1,-1, LRB}, {-1,+1, 0, LRF}, { 0,+1,-1, LLB}, { 0,+1, 0, LLF}, {-1, 0,-1, URB}, {-1, 0, 0, URF}, { 0, 0,-1, ULB}, { 0, 0, 0, ULF}},  // ULF Vertex
    {{-1,+1, 0, LRB}, {-1,+1,+1, LRF}, { 0,+1, 0, LLB}, { 0,+1,+1, LLF}, {-1, 0, 0, URB}, {-1, 0,+1, URF}, { 0, 0, 0, ULB}, { 0, 0,+1, ULF}},  // ULB Vertex
    {{ 0,+1,-1, LRB}, { 0,+1, 0, LRF}, {+1,+1,-1, LLB}, {+1,+1, 0, LLF}, { 0, 0,-1, URB}, { 0, 0, 0, URF}, {+1, 0,-1, ULB}, {+1, 0, 0, ULF}},  // URF Vertex
    {{ 0,+1, 0, LRB}, { 0,+1,+1, LRF}, {+1,+1, 0, LLB}, {+1,+1,+1, LLF}, { 0, 0, 0, URB}, { 0, 0,+1, URF}, {+1, 0, 0, ULB}, {+1, 0,+1, ULF}},  // URB Vertex
    {{-1, 0,-1, LRB}, {-1, 0, 0, LRF}, { 0, 0,-1, LLB}, { 0, 0, 0, LLF}, {-1,-1,-1, URB}, {-1,-1, 0, URF}, { 0,-1,-1, ULB}, { 0,-1, 0, ULF}},  // LLF Vertex
    {{-1, 0, 0, LRB}, {-1, 0,+1, LRF}, { 0, 0, 0, LLB}, { 0, 0,+1, LLF}, {-1,-1, 0, URB}, {-1,-1,+1, URF}, { 0,-1, 0, ULB}, { 0,-1,+1, ULF}},  // LLB Vertex
    {{ 0, 0,-1, LRB}, { 0, 0, 0, LRF}, {+1, 0,-1, LLB}, {+1, 0, 0, LLF}, { 0,-1,-1, URB}, { 0,-1, 0, URF}, {+1,-1,-1, ULB}, {+1,-1, 0, ULF}},  // LRF Vertex
    {{ 0, 0, 0, LRB}, { 0, 0,+1, LRF}, {+1, 0, 0, LLB}, {+1, 0,+1, LLF}, { 0,-1, 0, URB}, { 0,-1,+1, URF}, {+1,-1, 0, ULB}, {+1,-1,+1, ULF}}   // LRB Vertex
};



#define CELL_ID 0
#define VERT_ID 1
#define EDGE_ID 1
#define FACE_ID 1
#define TET_ID  1



// Edge3D Deconstuctor
Edge3D::~Edge3D()
{
    // only delete cut if it was TRULY allocated as one
    // and if it not used in the output mesh
     if(cut && cut->original_order() == CUT && cut->tm_v_index < 0)
    {
        delete cut;
        cut = NULL;
    }    
}

// Face3D Deconstructor
Face3D::~Face3D()
{
    // only delete triple if it was TRULY allocated as one
    // and if it not used in the output mesh
    if(triple && triple->original_order() == TRIP && triple->tm_v_index < 0)
    {
        delete triple;
        triple = NULL;
    }
}

// Tet3D Deconstructor
Tet3D::~Tet3D()
{    
    // only delete quad if it was TRULY allocated as one
    // and if it not used in the output mesh
    if(quad && quad->original_order() == QUAD && quad->tm_v_index < 0)
    {
        delete quad;
        quad = NULL;
    }
}

BCCLattice3D::BCCLattice3D(const AbstractVolume *volume) : volume(volume)
{
    m_iNumMaterials = volume->numberOfMaterials();
    m_iWidth  = volume->width()+1;
    m_iHeight = volume->height()+1;
    m_iDepth  = volume->depth()+1;

    verts = new std::vector<Vertex3D*>();
    tets = new std::vector<Tet*>();

    this->tree = new Octree(m_iWidth-1,m_iHeight-1,m_iDepth-1, *verts, *tets);
    this->labels = new unsigned char[m_iWidth*m_iHeight*m_iDepth];

    m_bDataLoaded = false;
    m_bCutsComputed = false;
    m_bTriplesComputed = false;
    m_bQuadsComputed = false;
    m_bGeneralized = false;
    m_bStenciled = false;
    m_bPhase1Complete = false;
    m_bPhase2Complete = false;
    m_bPhase2ViolationsFound = false;
    m_bPhase3Complete = false;
    m_bPhase3ViolationsFound = false;
    m_bDebugMode = false;
    m_bPadded = false;
}

BCCLattice3D::~BCCLattice3D()
{
    if (labels != NULL) {
       delete[] labels;
       labels = NULL;
    }

    // clean up any data on leaves of tree
    // must do this before we destroy the tree
    unsigned int total_cells = cut_cells.size() + buffer_cells.size();


    // must delete ALL tets, followed by ALL faces, followed by ALL edges, followed by ALL verts
    // reason behind this is generalization may have edges pointing to vertices, and if vertices
    // are deleted, edge->cut pointer is now invalid and could do goofy things.


    // delete tets
    for(unsigned int c=0; c < total_cells; c++)
    {
        OTCell *cell = NULL;
        if(c < cut_cells.size())
            cell = cut_cells[c];
        else
            cell = buffer_cells[c - cut_cells.size()];

        if(cell->tets)
        {
            for(int t=0; t < 24; t++)
            {
                // if tet exists
                if(cell->tets[t])
                {
                    // set to NULL in adjacent cells that shares it
                    int cell_index = TetTetGroup[t][CELL_ID];
                    int tet_index  = TetTetGroup[t][TET_ID];
                    OTCell *nCell = tree->getNeighbor(cell, edgeCellGroup[cell_index]);

                    if(nCell && nCell->tets)
                        nCell->tets[tet_index] = NULL;

                    // delete tet
                    delete cell->tets[t];
                    cell->tets[t] = NULL;
                }
            }
            delete[] cell->tets;
            cell->tets = NULL;
        }
    }

    // delete faces
    for(unsigned int c=0; c < total_cells; c++)
    {
        OTCell *cell = NULL;
        if(c < cut_cells.size())
            cell = cut_cells[c];
        else
            cell = buffer_cells[c - cut_cells.size()];

        // delete faces
        if(cell->face)
        {
            for(int f=0; f < 36; f++)
            {
                // if face exists
                if(cell->face[f])
                {
                    // first 12 faces are interior, just delete them, otherwise
                    // set to NULL in adjacent cells that shares it first
                    if(f >= 12)
                    {
                        int cell_index = FaceFaceGroup[f][CELL_ID];
                        int face_index = FaceFaceGroup[f][FACE_ID];
                        OTCell *nCell = tree->getNeighbor(cell, edgeCellGroup[cell_index]);

                        if(nCell && nCell->face)
                            nCell->face[face_index] = NULL;
                    }

                    // delete face
                    delete cell->face[f];
                    cell->face[f] = NULL;
                }
            }
            delete[] cell->face;
            cell->face = NULL;
        }
    }

    // delete edges
    for(unsigned int c=0; c < total_cells; c++)
    {
        OTCell *cell = NULL;
        if(c < cut_cells.size())
            cell = cut_cells[c];
        else
            cell = buffer_cells[c - cut_cells.size()];

        // delete edges
        if(cell->edge)
        {
            for(int e=0; e < 26; e++)
            {
                // if edge exists
                if(cell->edge[e])
                {
                    Edge3D *edge = cell->edge[e];

                    // set to NULL in adjacent cells that shares it
                    if(e >= 8)
                    {
                        int group_count = e < 14 ? 2 : 4;

                        for(int n=0; n < group_count; n++)
                        {
                            int cell_index = EdgeEdgeGroup[e][n][CELL_ID];
                            int edge_index = EdgeEdgeGroup[e][n][EDGE_ID];
                            OTCell *nCell = tree->getNeighbor(cell, edgeCellGroup[cell_index]);

                            if(nCell && nCell->edge)
                                nCell->edge[edge_index] = NULL;
                        }
                    }

                    // delete edge
                    delete edge;
                    cell->edge[e] = NULL;
                }
            }

            delete[] cell->edge;
            cell->edge = NULL;
        }
    }


    // delete vertex 3ds
    for(unsigned int c=0; c < total_cells; c++)
    {
        OTCell *cell = NULL;
        if(c < cut_cells.size())
            cell = cut_cells[c];
        else
            cell = buffer_cells[c - cut_cells.size()];

        // delete vertex arrays
        if(cell->vert)
        {
            // Delete Primal Vertices
            for(int v=0; v < 8; v++)
            {
                if(cell->vert[v])
                {
                    // save pointer to memory
                    Vertex3D *vertex = cell->vert[v];

                    // null out all 8 possible references
                    for(int n=0; n < 8; n++)
                    {
                        const int *cell_offset = VertexVertexGroup[v][n];
                        int vert_index = VertexVertexGroup[v][n][3];
                        OTCell *nCell = tree->getNeighbor(cell, cell_offset);

                        if(nCell && nCell->vert)
                            nCell->vert[vert_index] = NULL;
                    }

                    // free memory if not used in output mesh
                    if(vertex->tm_v_index < 0)
                        delete vertex;
                    cell->vert[v] = NULL;  // redundant safety
                }
            }


            // Delete Dual Vertex if not used in output Mesh
            if(cell->vert[C] && cell->vert[C]->tm_v_index < 0)
            {
                delete cell->vert[C];
                cell->vert[C] = NULL;
            }

            // Delete the Array
            delete[] cell->vert;
            cell->vert = NULL;
        }
    }

    // finally delete tree
    if(tree != NULL){
        delete tree;
        tree = NULL;
    }
}



unsigned char BCCLattice3D::generalizedKey(const Tet3D *tet)
{
    Vertex3D *verts[15];
    getRightHandedVertexList(tet, verts);

    unsigned char key = 0; // 64;
    verts[4] ? key |= 32 : 0 ;
    verts[5] ? key |= 16 : 0 ;
    verts[6] ? key |=  8 : 0 ;
    verts[7] ? key |=  4 : 0 ;
    verts[8] ? key |=  2 : 0 ;
    verts[9] ? key |=  1 : 0 ;
    return key;
}

// given new edge list ordering, give generalized key
unsigned char BCCLattice3D::keyFromAdjacentEdges(Edge3D *edges[6])
{
    unsigned char key = 0; // 64;
    (edges[0]->cut && edges[0]->cut->order() == CUT) ? key |= 32 : 0;
    (edges[1]->cut && edges[1]->cut->order() == CUT) ? key |= 16 : 0 ;
    (edges[2]->cut && edges[2]->cut->order() == CUT) ? key |=  8 : 0 ;
    (edges[3]->cut && edges[3]->cut->order() == CUT) ? key |=  4 : 0 ;
    (edges[4]->cut && edges[4]->cut->order() == CUT) ? key |=  2 : 0 ;
    (edges[5]->cut && edges[5]->cut->order() == CUT) ? key |=  1 : 0 ;

    return key;
}

bool BCCLattice3D::isKeyValid(unsigned char key)
{
    // new key ordering
    if (key ==  0 || key == 11 || key == 22 || key == 29 || key == 31 ||
        key == 37 || key == 46 || key == 47 ||
        key == 51 || key == 55 || key == 56 || key == 59 || key == 61 || key == 62 ||
        key == 63)
        return true;
    else
        return false;

    /*
    // old key ordering
    if (key == 0 || key == 14 || key == 21 || key == 35 || key == 56 ||
       key == 27 || key == 45 || key == 54 ||
       key == 31 || key == 47 || key == 55 || key == 59 || key == 61 || key == 62 ||
       key == 63)
        return true;
    else
        return false;
    */
}


void BCCLattice3D::getVertsAroundFace(const OTCell *cell, int face_index, Vertex3D *verts[3])
{
    for(int i=0; i < 3; i++)
    {
        int cell_id = faceVertexGroup[face_index][i][CELL_ID];
        int vert_id = faceVertexGroup[face_index][i][VERT_ID];
        OTCell *vcell = tree->getNeighbor(cell, edgeCellGroup[cell_id]);
        verts[i] = vcell->vert[vert_id];
    }
}

void BCCLattice3D::getVertsAroundFace(const Face3D *face, Vertex3D *verts[3])
{
    for(int i=0; i < 3; i++)
    {
        int cell_id = faceVertexGroup[face->face_index][i][CELL_ID];
        int vert_id = faceVertexGroup[face->face_index][i][VERT_ID];
        OTCell *cell = tree->getNeighbor(face->cell, edgeCellGroup[cell_id]);
        verts[i] = cell->vert[vert_id];

    }
}

void BCCLattice3D::getEdgesAroundFace(const OTCell *cell, int face_index, Edge3D *edges[3])
{
    for(int i=0; i < 3; i++)
    {
        int cell_id = faceEdgeGroup[face_index][i][CELL_ID];
        int edge_id = faceEdgeGroup[face_index][i][EDGE_ID];
        OTCell *vcell = tree->getNeighbor(cell, edgeCellGroup[cell_id]);
        edges[i] = vcell->edge[edge_id];
    }
}

void BCCLattice3D::getEdgesAroundFace(const Face3D *face, Edge3D *edges[3])
{
    for(int i=0; i < 3; i++)
    {
        int cell_id = faceEdgeGroup[face->face_index][i][CELL_ID];
        int edge_id = faceEdgeGroup[face->face_index][i][EDGE_ID];
        OTCell *cell = tree->getNeighbor(face->cell, edgeCellGroup[cell_id]);
        edges[i] = cell->edge[edge_id];
    }
}

void BCCLattice3D::getTetsAroundFace(const Face3D *face, Tet3D *tets[2]) // , int index)
{
    for(int i=0; i < 2; i++)
    {
        int tet_id = faceTetGroup[face->face_index][i];
        tets[i] = face->cell->tets[tet_id];
    }
}

void BCCLattice3D::getVertsAroundTet(const Tet3D *tet, Vertex3D *verts[4])
{
    OTCell *cell = tet->cell;

    switch(tet->tet_index)
    {
        // right tets
        case TRU:
        {
            // upper
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[URB];
            verts[2] = Rcell->vert[C];
            verts[3] = cell->vert[URF];

            break;
        }
        case TRL:
        {
            // lower
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[LRF];
            verts[2] = Rcell->vert[C];
            verts[3] = cell->vert[LRB];

            break;
        }
        case TRF:
        {
            // front
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[URF];
            verts[2] = Rcell->vert[C];
            verts[3] = cell->vert[LRF];

            break;
        }
        case TRB:
        {
            // back
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[LRB];
            verts[2] = Rcell->vert[C];
            verts[3] = cell->vert[URB];

            break;
        }
        // upper tets
        case TUF:
        {
            // front
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[ULF];
            verts[2] = Ucell->vert[C];
            verts[3] = cell->vert[URF];

            break;
        }
        case TUB:
        {
            // back
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[URB];
            verts[2] = Ucell->vert[C];
            verts[3] = cell->vert[ULB];

            break;
        }
        case TUL:
        {
            // left
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[ULB];
            verts[2] = Ucell->vert[C];
            verts[3] = cell->vert[ULF];

            break;
        }
        case TUR:
        {
            // right
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[URF];
            verts[2] = Ucell->vert[C];
            verts[3] = cell->vert[URB];

            break;
        }
        // back tets
        case TBT:
        {
            // top
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[ULB];
            verts[2] = Bcell->vert[C];
            verts[3] = cell->vert[URB];

            break;
        }
        case TBB:
        {
            // bottom
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[LRB];
            verts[2] = Bcell->vert[C];
            verts[3] = cell->vert[LLB];

            break;
        }
        case TBL:
        {
            // left
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[LLB];
            verts[2] = Bcell->vert[C];
            verts[3] = cell->vert[ULB];

            break;
        }
        case TBR:
        {
            // right
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[URB];
            verts[2] = Bcell->vert[C];
            verts[3] = cell->vert[LRB];

            break;
        }
    }
}

void BCCLattice3D::getEdgesAroundTet(const Tet3D *tet, Edge3D *edges[6])
{
    OTCell *cell = tet->cell;

    switch(tet->tet_index)
    {
        // right tets
        case TRU:
        {
            // upper
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            edges[0] = cell->edge[DURB];
            edges[1] = cell->edge[CR];
            edges[2] = cell->edge[URF];
            edges[3] = Rcell->edge[DULB];
            edges[4] = Rcell->edge[DULF];
            edges[5] = cell->edge[UR];

            break;
        }
        case TRL:
        {
            // lower
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            edges[0] = cell->edge[DLRF];
            edges[1] = cell->edge[CR];
            edges[2] = cell->edge[DLRB];
            edges[3] = Rcell->edge[DLLF];
            edges[4] = Rcell->edge[DLLB];
            edges[5] = cell->edge[LR];

            break;
        }
        case TRF:
        {
            // front
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            edges[0] = cell->edge[DURF];
            edges[1] = cell->edge[CR];
            edges[2] = cell->edge[DLRF];
            edges[3] = Rcell->edge[DULF];
            edges[4] = Rcell->edge[DLLF];
            edges[5] = cell->edge[FR];

            break;
        }
        case TRB:
        {
            // back
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            edges[0] = cell->edge[DLRB];
            edges[1] = cell->edge[CR];
            edges[2] = cell->edge[DURB];
            edges[3] = Rcell->edge[DLLB];
            edges[4] = Rcell->edge[DULB];
            edges[5] = cell->edge[BR];

            break;
        }
        // upper tets
        case TUF:
        {
            // front
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            edges[0] = cell->edge[DULF];
            edges[1] = cell->edge[CU];
            edges[2] = cell->edge[DURF];
            edges[3] = Ucell->edge[DLLF];
            edges[4] = Ucell->edge[DLRF];
            edges[5] = cell->edge[UF];

            break;
        }
        case TUB:
        {
            // back
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            edges[0] = cell->edge[DURB];
            edges[1] = cell->edge[CU];
            edges[2] = cell->edge[DULB];
            edges[3] = Ucell->edge[DLRB];
            edges[4] = Ucell->edge[DLLB];
            edges[5] = cell->edge[UB];

            break;
        }
        case TUL:
        {
            // left
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            edges[0] = cell->edge[DULB];
            edges[1] = cell->edge[CU];
            edges[2] = cell->edge[DULF];
            edges[3] = Ucell->edge[DLLB];
            edges[4] = Ucell->edge[DLLF];
            edges[5] = cell->edge[UL];

            break;
        }
        case TUR:
        {
            // right
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            edges[0] = cell->edge[DURF];
            edges[1] = cell->edge[CU];
            edges[2] = cell->edge[DURB];
            edges[3] = Ucell->edge[DLRF];
            edges[4] = Ucell->edge[DLRB];
            edges[5] = cell->edge[UR];

            break;
        }
        // back tets
        case TBT:
        {
            // top
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            edges[0] = cell->edge[DULB];
            edges[1] = cell->edge[CB];
            edges[2] = cell->edge[DURB];
            edges[3] = Bcell->edge[DULF];
            edges[4] = Bcell->edge[DURF];
            edges[5] = cell->edge[UB];

            break;
        }
        case TBB:
        {
            // bottom
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            edges[0] = cell->edge[DLRB];
            edges[1] = cell->edge[CB];
            edges[2] = cell->edge[DLLB];
            edges[3] = Bcell->edge[DLRF];
            edges[4] = Bcell->edge[DLLF];
            edges[5] = cell->edge[LB];

            break;
        }
        case TBL:
        {
            // left
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            edges[0] = cell->edge[DLLB];
            edges[1] = cell->edge[CB];
            edges[2] = cell->edge[DULB];
            edges[3] = Bcell->edge[DLLF];
            edges[4] = Bcell->edge[DULF];
            edges[5] = cell->edge[BL];

            break;
        }
        case TBR:
        {
            // right
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            edges[0] = cell->edge[DURB];
            edges[1] = cell->edge[CB];
            edges[2] = cell->edge[DLRB];
            edges[3] = Bcell->edge[DURF];
            edges[4] = Bcell->edge[DLRF];
            edges[5] = cell->edge[BR];

            break;
        }
    }
}

void BCCLattice3D::getFacesAroundTet(const Tet3D *tet, Face3D *faces[4])
{
    OTCell *cell = tet->cell;

    switch(tet->tet_index)
    {
        // right tets
        case TRU:
        {
            // upper
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            faces[0] = cell->face[FRUB];
            faces[1] = cell->face[FRUF];
            faces[2] = cell->face[FUR];
            faces[3] = Rcell->face[FUL];

            break;
        }
        case TRL:
        {
            // lower
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            faces[0] = cell->face[FRLF];
            faces[1] = cell->face[FRLB];
            faces[2] = cell->face[FLR];
            faces[3] = Rcell->face[FLL];

            break;
        }
        case TRF:
        {
            // front
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            faces[0] = cell->face[FRUF];
            faces[1] = cell->face[FRLF];
            faces[2] = cell->face[FFR];
            faces[3] = Rcell->face[FFL];

            break;
        }
        case TRB:
        {
            // back
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            faces[0] = cell->face[FRLB];
            faces[1] = cell->face[FRUB];
            faces[2] = cell->face[FBR];
            faces[3] = Rcell->face[FBL];

            break;
        }
        // upper tets
        case TUF:
        {
            // front
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            faces[0] = cell->face[FUFL];
            faces[1] = cell->face[FUFR];
            faces[2] = cell->face[FUF];
            faces[3] = Ucell->face[FLF];

            break;
        }
        case TUB:
        {
            // back
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            faces[0] = cell->face[FUBR];
            faces[1] = cell->face[FUBL];
            faces[2] = cell->face[FUB];
            faces[3] = Ucell->face[FLB];

            break;
        }
        case TUL:
        {
            // left
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            faces[0] = cell->face[FUBL];
            faces[1] = cell->face[FUFL];
            faces[2] = cell->face[FUL];
            faces[3] = Ucell->face[FLL];

            break;
        }
        case TUR:
        {
            // right
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            faces[0] = cell->face[FUFR];
            faces[1] = cell->face[FUBR];
            faces[2] = cell->face[FUR];
            faces[3] = Ucell->face[FLR];

            break;
        }
        // back tets
        case TBT:
        {
            // top
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            faces[0] = cell->face[FBUL];
            faces[1] = cell->face[FBUR];
            faces[2] = cell->face[FUB];
            faces[3] = Bcell->face[FUF];

            break;
        }
        case TBB:
        {
            // bottom
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            faces[0] = cell->face[FBLR];
            faces[1] = cell->face[FBLL];
            faces[2] = cell->face[FLB];
            faces[3] = Bcell->face[FLF];

            break;
        }
        case TBL:
        {
            // left
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            faces[0] = cell->face[FBLL];
            faces[1] = cell->face[FBUL];
            faces[2] = cell->face[FBL];
            faces[3] = Bcell->face[FFL];

            break;
        }
        case TBR:
        {
            // right
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            faces[0] = cell->face[FBUR];
            faces[1] = cell->face[FBLR];
            faces[2] = cell->face[FBR];
            faces[3] = Bcell->face[FFR];

            break;
        }
    }
}


void BCCLattice3D::getAdjacencyLists(const Face3D *face, Vertex3D *verts[3], Edge3D *edges[3])
{
    getVertsAroundFace(face, verts);
    getEdgesAroundFace(face, edges);
}

void BCCLattice3D::getAdjacencyLists(const Tet3D *tet, Vertex3D *verts[4], Edge3D *edges[6], Face3D *faces[4])
{
    OTCell *cell = tet->cell;

    switch(tet->tet_index)
    {
        // right tets
        case TRU:
        {
            // upper
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[URB];
            verts[2] = Rcell->vert[C];
            verts[3] = cell->vert[URF];

            edges[0] = cell->edge[DURB];
            edges[1] = cell->edge[CR];
            edges[2] = cell->edge[URF];
            edges[3] = Rcell->edge[DULB];
            edges[4] = Rcell->edge[DULF];
            edges[5] = cell->edge[UR];

            faces[0] = cell->face[FRUB];
            faces[1] = cell->face[FRUF];
            faces[2] = cell->face[FUR];
            faces[3] = Rcell->face[FUL];

            break;
        }
        case TRL:
        {
            // lower
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[LRF];
            verts[2] = Rcell->vert[C];
            verts[3] = cell->vert[LRB];

            edges[0] = cell->edge[DLRF];
            edges[1] = cell->edge[CR];
            edges[2] = cell->edge[DLRB];
            edges[3] = Rcell->edge[DLLF];
            edges[4] = Rcell->edge[DLLB];
            edges[5] = cell->edge[LR];

            faces[0] = cell->face[FRLF];
            faces[1] = cell->face[FRLB];
            faces[2] = cell->face[FLR];
            faces[3] = Rcell->face[FLL];

            break;
        }
        case TRF:
        {
            // front
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[URF];
            verts[2] = Rcell->vert[C];
            verts[3] = cell->vert[LRF];

            edges[0] = cell->edge[DURF];
            edges[1] = cell->edge[CR];
            edges[2] = cell->edge[DLRF];
            edges[3] = Rcell->edge[DULF];
            edges[4] = Rcell->edge[DLLF];
            edges[5] = cell->edge[FR];

            faces[0] = cell->face[FRUF];
            faces[1] = cell->face[FRLF];
            faces[2] = cell->face[FFR];
            faces[3] = Rcell->face[FFL];

            break;
        }
        case TRB:
        {
            // back
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[LRB];
            verts[2] = Rcell->vert[C];
            verts[3] = cell->vert[URB];

            edges[0] = cell->edge[DLRB];
            edges[1] = cell->edge[CR];
            edges[2] = cell->edge[DURB];
            edges[3] = Rcell->edge[DLLB];
            edges[4] = Rcell->edge[DULB];
            edges[5] = cell->edge[BR];

            faces[0] = cell->face[FRLB];
            faces[1] = cell->face[FRUB];
            faces[2] = cell->face[FBR];
            faces[3] = Rcell->face[FBL];

            break;
        }
        // upper tets
        case TUF:
        {
            // front
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[ULF];
            verts[2] = Ucell->vert[C];
            verts[3] = cell->vert[URF];

            edges[0] = cell->edge[DULF];
            edges[1] = cell->edge[CU];
            edges[2] = cell->edge[DURF];
            edges[3] = Ucell->edge[DLLF];
            edges[4] = Ucell->edge[DLRF];
            edges[5] = cell->edge[UF];

            faces[0] = cell->face[FUFL];
            faces[1] = cell->face[FUFR];
            faces[2] = cell->face[FUF];
            faces[3] = Ucell->face[FLF];

            break;
        }
        case TUB:
        {
            // back
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[URB];
            verts[2] = Ucell->vert[C];
            verts[3] = cell->vert[ULB];

            edges[0] = cell->edge[DURB];
            edges[1] = cell->edge[CU];
            edges[2] = cell->edge[DULB];
            edges[3] = Ucell->edge[DLRB];
            edges[4] = Ucell->edge[DLLB];
            edges[5] = cell->edge[UB];

            faces[0] = cell->face[FUBR];
            faces[1] = cell->face[FUBL];
            faces[2] = cell->face[FUB];
            faces[3] = Ucell->face[FLB];

            break;
        }
        case TUL:
        {
            // left
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[ULB];
            verts[2] = Ucell->vert[C];
            verts[3] = cell->vert[ULF];

            edges[0] = cell->edge[DULB];
            edges[1] = cell->edge[CU];
            edges[2] = cell->edge[DULF];
            edges[3] = Ucell->edge[DLLB];
            edges[4] = Ucell->edge[DLLF];
            edges[5] = cell->edge[UL];

            faces[0] = cell->face[FUBL];
            faces[1] = cell->face[FUFL];
            faces[2] = cell->face[FUL];
            faces[3] = Ucell->face[FLL];

            break;
        }
        case TUR:
        {
            // right
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[URF];
            verts[2] = Ucell->vert[C];
            verts[3] = cell->vert[URB];

            edges[0] = cell->edge[DURF];
            edges[1] = cell->edge[CU];
            edges[2] = cell->edge[DURB];
            edges[3] = Ucell->edge[DLRF];
            edges[4] = Ucell->edge[DLRB];
            edges[5] = cell->edge[UR];

            faces[0] = cell->face[FUFR];
            faces[1] = cell->face[FUBR];
            faces[2] = cell->face[FUR];
            faces[3] = Ucell->face[FLR];

            break;
        }
        // back tets
        case TBT:
        {
            // top
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[ULB];
            verts[2] = Bcell->vert[C];
            verts[3] = cell->vert[URB];

            edges[0] = cell->edge[DULB];
            edges[1] = cell->edge[CB];
            edges[2] = cell->edge[DURB];
            edges[3] = Bcell->edge[DULF];
            edges[4] = Bcell->edge[DURF];
            edges[5] = cell->edge[UB];

            faces[0] = cell->face[FBUL];
            faces[1] = cell->face[FBUR];
            faces[2] = cell->face[FUB];
            faces[3] = Bcell->face[FUF];

            break;
        }
        case TBB:
        {
            // bottom
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[LRB];
            verts[2] = Bcell->vert[C];
            verts[3] = cell->vert[LLB];

            edges[0] = cell->edge[DLRB];
            edges[1] = cell->edge[CB];
            edges[2] = cell->edge[DLLB];
            edges[3] = Bcell->edge[DLRF];
            edges[4] = Bcell->edge[DLLF];
            edges[5] = cell->edge[LB];

            faces[0] = cell->face[FBLR];
            faces[1] = cell->face[FBLL];
            faces[2] = cell->face[FLB];
            faces[3] = Bcell->face[FLF];

            break;
        }
        case TBL:
        {
            // left
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[LLB];
            verts[2] = Bcell->vert[C];
            verts[3] = cell->vert[ULB];

            edges[0] = cell->edge[DLLB];
            edges[1] = cell->edge[CB];
            edges[2] = cell->edge[DULB];
            edges[3] = Bcell->edge[DLLF];
            edges[4] = Bcell->edge[DULF];
            edges[5] = cell->edge[BL];

            faces[0] = cell->face[FBLL];
            faces[1] = cell->face[FBUL];
            faces[2] = cell->face[FBL];
            faces[3] = Bcell->face[FFL];

            break;
        }
        case TBR:
        {
            // right
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[0] = cell->vert[C];
            verts[1] = cell->vert[URB];
            verts[2] = Bcell->vert[C];
            verts[3] = cell->vert[LRB];

            edges[0] = cell->edge[DURB];
            edges[1] = cell->edge[CB];
            edges[2] = cell->edge[DLRB];
            edges[3] = Bcell->edge[DURF];
            edges[4] = Bcell->edge[DLRF];
            edges[5] = cell->edge[BR];

            faces[0] = cell->face[FBUR];
            faces[1] = cell->face[FBLR];
            faces[2] = cell->face[FBR];
            faces[3] = Bcell->face[FFR];

            break;
        }
    }
}

Tet3D* BCCLattice3D::getOppositeTet(const Tet3D *tet, const Face3D *face)
{
    Tet3D *tets[TETS_PER_FACE];
    getTetsAroundFace(face, tets);

    if(tets[0] == tet)
        return tets[1];
    else
        return tets[0];
}

void BCCLattice3D::getFacesAroundEdgeOnTet(const Tet3D *tet, const Edge3D *edge, Face3D *faces[2])
{
    unsigned int face_count;
    Face3D *all_faces[MAX_FACES_PER_EDGE];
    getFacesAroundEdge(edge, all_faces, &face_count);

    unsigned int idx = 0;
    for(unsigned int f=0; f < face_count; f++)
    {
        if(contains(tet, all_faces[f]))
            faces[idx++] = all_faces[f];
    }
}

Vertex3D* BCCLattice3D::getGeneralizedVertex(const Tet3D *tet, const int index)
{
    Vertex3D *verts[15];

    getRightHandedVertexList(tet, verts);

    return verts[index];
}

// Rule is as follows:
// V1 (A) will always be Dual Vertex of Cell
// V2 (B) will always be Primal Vertex with Smaller Coordinate
// V3 (C) will always be Dual Vertex of Neighbor Cell
// V4 (D) will always be Primal Vertex with Larger Coordinate
// BUT: Tet Vertex ordering must always be Right-Handed
//      This will flip coordinate system for neighboring tets...
// Solution:     Appears to work if we keep all Tet's oriented
//               Right-handed, and always choose either smaller or
//               larger coordinate of V2/V4 as Primal Vertex

//#define SAFE_PTR(x) (x && x->exists) ? x : NULL


void BCCLattice3D::getRightHandedVertexList(const Tet3D *tet, Vertex3D *verts[15])
{
    OTCell *cell = tet->cell;

    switch(tet->tet_index)
    {
        // right tets
        case TRU:
        {
            // upper
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[URB];
            verts[   _C] = Rcell->vert[C];
            verts[   _D] = cell->vert[URF];
            verts[  _AB] = cell->edge[DURB]->cut;
            verts[  _AC] = cell->edge[CR]->cut;
            verts[  _AD] = cell->edge[URF]->cut;
            verts[  _CD] = Rcell->edge[DULF]->cut;
            verts[  _BD] = cell->edge[UR]->cut;
            verts[  _BC] = Rcell->edge[DULB]->cut;
            verts[ _BCD] = Rcell->face[FUL]->triple;
            verts[ _ACD] = cell->face[FRUF]->triple;
            verts[ _ABD] = cell->face[FUR]->triple;
            verts[ _ABC] = cell->face[FRUB]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        case TRL:
        {
            // lower
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[LRF];
            verts[   _C] = Rcell->vert[C];
            verts[   _D] = cell->vert[LRB];
            verts[  _AB] = cell->edge[DLRF]->cut;
            verts[  _AC] = cell->edge[CR]->cut;
            verts[  _AD] = cell->edge[DLRB]->cut;
            verts[  _CD] = Rcell->edge[DLLB]->cut;
            verts[  _BD] = cell->edge[LR]->cut;
            verts[  _BC] = Rcell->edge[DLLF]->cut;
            verts[ _BCD] = Rcell->face[FLL]->triple;
            verts[ _ACD] = cell->face[FRLB]->triple;
            verts[ _ABD] = cell->face[FLR]->triple;
            verts[ _ABC] = cell->face[FRLF]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        case TRF:
        {
            // front
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[URF];
            verts[   _C] = Rcell->vert[C];
            verts[   _D] = cell->vert[LRF];
            verts[  _AB] = cell->edge[DURF]->cut;
            verts[  _AC] = cell->edge[CR]->cut;
            verts[  _AD] = cell->edge[DLRF]->cut;
            verts[  _CD] = Rcell->edge[DLLF]->cut;
            verts[  _BD] = cell->edge[FR]->cut;
            verts[  _BC] = Rcell->edge[DULF]->cut;
            verts[ _BCD] = Rcell->face[FFL]->triple;
            verts[ _ACD] = cell->face[FRLF]->triple;
            verts[ _ABD] = cell->face[FFR]->triple;
            verts[ _ABC] = cell->face[FRUF]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        case TRB:
        {
            // back
            OTCell *Rcell = tree->getNeighbor(cell, 1, 0, 0);

            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[LRB];
            verts[   _C] = Rcell->vert[C];
            verts[   _D] = cell->vert[URB];
            verts[  _AB] = cell->edge[DLRB]->cut;
            verts[  _AC] = cell->edge[CR]->cut;
            verts[  _AD] = cell->edge[DURB]->cut;
            verts[  _CD] = Rcell->edge[DULB]->cut;
            verts[  _BD] = cell->edge[BR]->cut;
            verts[  _BC] = Rcell->edge[DLLB]->cut;
            verts[ _BCD] = Rcell->face[FBL]->triple;
            verts[ _ACD] = cell->face[FRUB]->triple;
            verts[ _ABD] = cell->face[FBR]->triple;
            verts[ _ABC] = cell->face[FRLB]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        // upper tets
        case TUF:
        {
            // front
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[ULF];
            verts[   _C] = Ucell->vert[C];
            verts[   _D] = cell->vert[URF];
            verts[  _AB] = cell->edge[DULF]->cut;
            verts[  _AC] = cell->edge[CU]->cut;
            verts[  _AD] = cell->edge[DURF]->cut;
            verts[  _CD] = Ucell->edge[DLRF]->cut;
            verts[  _BD] = cell->edge[UF]->cut;
            verts[  _BC] = Ucell->edge[DLLF]->cut;
            verts[ _BCD] = Ucell->face[FLF]->triple;
            verts[ _ACD] = cell->face[FUFR]->triple;
            verts[ _ABD] = cell->face[FUF]->triple;
            verts[ _ABC] = cell->face[FUFL]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        case TUB:
        {
            // back
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[URB];
            verts[   _C] = Ucell->vert[C];
            verts[   _D] = cell->vert[ULB];
            verts[  _AB] = cell->edge[DURB]->cut;
            verts[  _AC] = cell->edge[CU]->cut;
            verts[  _AD] = cell->edge[DULB]->cut;
            verts[  _CD] = Ucell->edge[DLLB]->cut;
            verts[  _BD] = cell->edge[UB]->cut;
            verts[  _BC] = Ucell->edge[DLRB]->cut;
            verts[ _BCD] = Ucell->face[FLB]->triple;
            verts[ _ACD] = cell->face[FUBL]->triple;
            verts[ _ABD] = cell->face[FUB]->triple;
            verts[ _ABC] = cell->face[FUBR]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        case TUL:
        {
            // left
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);
            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[ULB];
            verts[   _C] = Ucell->vert[C];
            verts[   _D] = cell->vert[ULF];
            verts[  _AB] = cell->edge[DULB]->cut;
            verts[  _AC] = cell->edge[CU]->cut;
            verts[  _AD] = cell->edge[DULF]->cut;
            verts[  _CD] = Ucell->edge[DLLF]->cut;
            verts[  _BD] = cell->edge[UL]->cut;
            verts[  _BC] = Ucell->edge[DLLB]->cut;
            verts[ _BCD] = Ucell->face[FLL]->triple;
            verts[ _ACD] = cell->face[FUFL]->triple;
            verts[ _ABD] = cell->face[FUL]->triple;
            verts[ _ABC] = cell->face[FUBL]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        case TUR:
        {
            // right
            OTCell *Ucell = tree->getNeighbor(cell, 0, 1, 0);

            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[URF];
            verts[   _C] = Ucell->vert[C];
            verts[   _D] = cell->vert[URB];
            verts[  _AB] = cell->edge[DURF]->cut;
            verts[  _AC] = cell->edge[CU]->cut;
            verts[  _AD] = cell->edge[DURB]->cut;
            verts[  _CD] = Ucell->edge[DLRB]->cut;
            verts[  _BD] = cell->edge[UR]->cut;
            verts[  _BC] = Ucell->edge[DLRF]->cut;
            verts[ _BCD] = Ucell->face[FLR]->triple;
            verts[ _ACD] = cell->face[FUBR]->triple;
            verts[ _ABD] = cell->face[FUR]->triple;
            verts[ _ABC] = cell->face[FUFR]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        // back tets
        case TBT:
        {
            // top
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[ULB];
            verts[   _C] = Bcell->vert[C];
            verts[   _D] = cell->vert[URB];
            verts[  _AB] = cell->edge[DULB]->cut;
            verts[  _AC] = cell->edge[CB]->cut;
            verts[  _AD] = cell->edge[DURB]->cut;
            verts[  _CD] = Bcell->edge[DURF]->cut;
            verts[  _BD] = cell->edge[UB]->cut;
            verts[  _BC] = Bcell->edge[DULF]->cut;
            verts[ _BCD] = Bcell->face[FUF]->triple;
            verts[ _ACD] = cell->face[FBUR]->triple;
            verts[ _ABD] = cell->face[FUB]->triple;
            verts[ _ABC] = cell->face[FBUL]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        case TBB:
        {
            // bottom
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[LRB];
            verts[   _C] = Bcell->vert[C];
            verts[   _D] = cell->vert[LLB];
            verts[  _AB] = cell->edge[DLRB]->cut;
            verts[  _AC] = cell->edge[CB]->cut;
            verts[  _AD] = cell->edge[DLLB]->cut;
            verts[  _CD] = Bcell->edge[DLLF]->cut;
            verts[  _BD] = cell->edge[LB]->cut;
            verts[  _BC] = Bcell->edge[DLRF]->cut;
            verts[ _BCD] = Bcell->face[FLF]->triple;
            verts[ _ACD] = cell->face[FBLL]->triple;
            verts[ _ABD] = cell->face[FLB]->triple;
            verts[ _ABC] = cell->face[FBLR]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        case TBL:
        {
            // left
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[LLB];
            verts[   _C] = Bcell->vert[C];
            verts[   _D] = cell->vert[ULB];
            verts[  _AB] = cell->edge[DLLB]->cut;
            verts[  _AC] = cell->edge[CB]->cut;
            verts[  _AD] = cell->edge[DULB]->cut;
            verts[  _CD] = Bcell->edge[DULF]->cut;
            verts[  _BD] = cell->edge[BL]->cut;
            verts[  _BC] = Bcell->edge[DLLF]->cut;
            verts[ _BCD] = Bcell->face[FFL]->triple;
            verts[ _ACD] = cell->face[FBUL]->triple;
            verts[ _ABD] = cell->face[FBL]->triple;
            verts[ _ABC] = cell->face[FBLL]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        case TBR:
        {
            // right
            OTCell * Bcell = tree->getNeighbor(cell, 0, 0, 1);

            verts[   _A] = cell->vert[C];
            verts[   _B] = cell->vert[URB];
            verts[   _C] = Bcell->vert[C];
            verts[   _D] = cell->vert[LRB];
            verts[  _AB] = cell->edge[DURB]->cut;
            verts[  _AC] = cell->edge[CB]->cut;
            verts[  _AD] = cell->edge[DLRB]->cut;
            verts[  _CD] = Bcell->edge[DLRF]->cut;
            verts[  _BD] = cell->edge[BR]->cut;
            verts[  _BC] = Bcell->edge[DURF]->cut;
            verts[ _BCD] = Bcell->face[FFR]->triple;
            verts[ _ACD] = cell->face[FBLR]->triple;
            verts[ _ABD] = cell->face[FBR]->triple;
            verts[ _ABC] = cell->face[FBUR]->triple;
            verts[_ABCD] = tet->quad;
            break;
        }
        default:
        {
            cerr << "Fatal Error: InvalidTet Index!!" << endl;
            exit(51);
        }
    }
}


void BCCLattice3D::getCellsAroundVertex(const OTCell *cell, int index, OTCell *cell_list[8])
{
    memset(cell_list, 0, 8*sizeof(OTCell*));
    if(index == C)
    {
        cell_list[0] = const_cast<OTCell*>(cell);
    }
    else{
        for(int i=0; i < 8; i++)
        {
            cell_list[i] = tree->getNeighbor(cell, vertexCellGroup[index][i]);
        }
    }
}

void BCCLattice3D::getCellsAroundVertex(Vertex3D *vertex, OTCell *cell_list[8])
{
    return getCellsAroundVertex(vertex->cell, vertex->vert_index, cell_list);
}


void BCCLattice3D::getEdgesAroundVertex(const OTCell *cell, int index, Edge3D *edges[14])
{
    // Dual Vertex Edges Are Easy
    if(index == C)
    {
        memcpy(edges, cell->edge, EDGES_PER_VERT*sizeof(Edge3D*));
        return;
    }

    OTCell* CELL[8];
    for(int c=0; c < 8; c++){
        CELL[c] = tree->getNeighbor(cell, vertexCellGroup[index][c]);
    }

    for(unsigned int e=0; e < EDGES_PER_VERT; e++)
    {
        OTCell *pCell = CELL[vertexEdgeGroup[e][CELL_ID]];
        if(pCell)
            edges[e] = pCell->edge[vertexEdgeGroup[e][EDGE_ID]];
        else
            edges[e] = NULL;
    }
}

void BCCLattice3D::getEdgesAroundVertex(const Vertex3D *vertex, Edge3D *edges[14])
{
    // Dual Vertex Edges Are Easy
    if(vertex->vert_index == C)
    {
        memcpy(edges, vertex->cell->edge, EDGES_PER_VERT*sizeof(Edge3D*));
        return;
    }

    OTCell* CELL[8];
    for(int c=0; c < 8; c++){
        CELL[c] = tree->getNeighbor(vertex->cell, vertexCellGroup[vertex->vert_index][c]);
    }

    for(unsigned int e=0; e < EDGES_PER_VERT; e++)
    {
        OTCell *pCell = CELL[vertexEdgeGroup[e][CELL_ID]];
        if(pCell)
            edges[e] = pCell->edge[vertexEdgeGroup[e][EDGE_ID]];
        else
            edges[e] = NULL;
    }
}

void BCCLattice3D::getFacesAroundVertex(const OTCell *cell, int index, Face3D *faces[36])
{
    // Dual Vertex Faces Are Easy
    if(index == C)
    {
        memcpy(faces, cell->face, FACES_PER_VERT*sizeof(Face3D*));
        return;
    }

    OTCell* CELL[8];
    for(int c=0; c < 8; c++){
        CELL[c] = tree->getNeighbor(cell, vertexCellGroup[index][c]);
    }

    for(int f=0; f < 36; f++)
    {
        OTCell *pCell = CELL[vertexFaceGroup[f][CELL_ID]];
        if(pCell)
            faces[f] = pCell->face[vertexFaceGroup[f][FACE_ID]];
        else
            faces[f] = NULL;
    }
}

void BCCLattice3D::getFacesAroundVertex(const Vertex3D *vertex, Face3D *faces[36])
{
    // Dual Vertex Faces Are Easy
    if(vertex->vert_index == C)
    {
        memcpy(faces, vertex->cell->face, 36*sizeof(Face3D*));
        return;
    }

    OTCell* CELL[8];
    for(int c=0; c < 8; c++){
        CELL[c] = tree->getNeighbor(vertex->cell, vertexCellGroup[vertex->vert_index][c]);
    }

    for(int f=0; f < 36; f++)
    {
        OTCell *pCell = CELL[vertexFaceGroup[f][CELL_ID]];
        faces[f] = pCell->face[vertexFaceGroup[f][FACE_ID]];
    }
}

void BCCLattice3D::getTetsAroundVertex(const OTCell *cell, int index, Tet3D *tets[24])
{
    // Dual Vertex Tets Are Easy
    if(index == C)
    {
        memcpy(tets, cell->tets, 24*sizeof(Tet3D*));
        return;
    }

    OTCell* CELL[8];
    for(int c=0; c < 8; c++){
        CELL[c] = tree->getNeighbor(cell, vertexCellGroup[index][c]);
    }

    for(int t=0; t < 24; t++)
    {
        OTCell *pCell = CELL[vertexTetGroup[t][CELL_ID]];
        tets[t] = pCell->tets[vertexTetGroup[t][TET_ID]];
    }
}

void BCCLattice3D::getTetsAroundVertex(const Vertex3D *vertex, Tet3D *tets[24])
{
    // Dual Vertex Tets Are Easy
    if(vertex->vert_index == C)
    {
        memcpy(tets, vertex->cell->tets, 24*sizeof(Tet3D*));
        return;
    }

    OTCell* CELL[8];
    for(int c=0; c < 8; c++){
        CELL[c] = tree->getNeighbor(vertex->cell, vertexCellGroup[vertex->vert_index][c]);
    }

    for(int t=0; t < 24; t++)
    {
        OTCell *pCell = CELL[vertexTetGroup[t][CELL_ID]];
        tets[t] = pCell->tets[vertexTetGroup[t][TET_ID]];
    }
}

// # of faces returned could be either 4 or 6, depending on edge type
void BCCLattice3D::getFacesAroundEdge(const Edge3D *edge, Face3D *faces[6], unsigned int *count) // , int index)
{
    OTCell *cell = edge->cell;

    //------------------
    //  Diagonal Edges
    //------------------
    if(edge->edge_index < 8)
    {
        *count = 6;
        for(unsigned int i=0; i < *count; i++)
        {
            faces[i] = cell->face[shortEdgeFaceGroup[edge->edge_index][i]];
        }
    }
    //---------------
    //   Dual Edges
    //---------------
    else if(edge->edge_index < 14)
    {
        *count = 4;
        for(unsigned int i=0; i < *count; i++)
        {
            faces[i] = cell->face[dualLongEdgeFaceGroup[edge->edge_index-8][i]];
        }
    }
    //---------------------------------------------
    //   Top Edges / Bottom Edges / Column Edges
    //---------------------------------------------
    else
    {
        *count = 4;

        for(unsigned int i=0; i < *count; i++)
        {
            int cell_id = primalLongEdgeFaceGroup[edge->edge_index-14][i][CELL_ID];
            OTCell *vcell = tree->getNeighbor(cell, edgeCellGroup[cell_id]);
            faces[i] = vcell->face[primalLongEdgeFaceGroup[edge->edge_index-14][i][FACE_ID]];
        }
    }
}

// # of tets returned could be either 4 or 6, depending on edge type
void BCCLattice3D::getTetsAroundEdge(const Edge3D *edge, Tet3D *tets[6], unsigned int *count) // , int index)
{
    OTCell *cell = edge->cell;

    //------------------
    //  Diagonal Edges
    //------------------
    if(edge->edge_index < 8)
    {
        *count = 6;
        for(unsigned int i=0; i < *count; i++)
        {
            tets[i] = cell->tets[shortEdgeTetGroup[edge->edge_index][i]];
        }
    }
    //---------------
    //   Dual Edges
    //---------------
    else if(edge->edge_index < 14)
    {
        *count = 4;
        for(unsigned int i=0; i < *count; i++)
        {
            tets[i] = cell->tets[dualLongEdgeTetGroup[edge->edge_index-8][i]];
        }
    }
    //---------------------------------------------
    //   Top Edges / Bottom Edges / Column Edges
    //---------------------------------------------
    else
    {
        *count = 4;

        for(unsigned int i=0; i < *count; i++)
        {
            int cell_id = primalLongEdgeTetGroup[edge->edge_index-14][i][CELL_ID];
            OTCell *vcell = tree->getNeighbor(cell, edgeCellGroup[cell_id]);
            tets[i] = vcell->tets[primalLongEdgeTetGroup[edge->edge_index-14][i][FACE_ID]];
        }
    }
}


//--------------------------------------------------------------------------------------------
//  triangle_intersect()
//
//  This method computes the intersection of a ray and triangle. The intersection point
//  is stored in 'pt', while a boolean returned indicates whether or not the intersection
//  occurred in the triangle. Epsilon tolerance is given to boundary case.
//--------------------------------------------------------------------------------------------
bool triangle_intersection(Vertex3D *v1, Vertex3D *v2, Vertex3D *v3, vec3 origin, vec3 ray, vec3 &pt, float epsilon = 1E-8)
{
    float epsilon2 = (float)1E-3;

    //-------------------------------------------------
    // if v1, v2, and v3 are not unique, return FALSE
    //-------------------------------------------------
    if(v1 == v2 || v2 == v3 || v1 == v3)
        return false;
    else if(L2(v1->pos() - v2->pos()) < epsilon || L2(v2->pos() - v3->pos()) < epsilon || L2(v1->pos() - v3->pos()) < epsilon)
        return false;

    //----------------------------------------------
    // compute intersection with plane, store in pt
    //----------------------------------------------
    vec3 e1 = v1->pos() - v3->pos();
    vec3 e2 = v2->pos() - v3->pos();

    ray = normalize(ray);
    vec3 r1 = ray.cross(e2);
    double denom = e1.dot(r1);

    if( fabs(denom) < epsilon)
        return false;

    double inv_denom = 1.0 / denom;
    vec3 s = origin - v3->pos();
    double b1 = s.dot(r1) * inv_denom;

    if(b1 < (0.0 - epsilon2) || b1 > (1.0 + epsilon2))
        return false;

    vec3 r2 = s.cross(e1);
    double b2 = ray.dot(r2) * inv_denom;

    if(b2 < (0.0 - epsilon2) || (b1 + b2) > (1.0 + 2*epsilon2))
        return false;

    double t = e2.dot(r2) * inv_denom;
    pt = origin + t*ray;


    if(t < 0.01)
        return false;
    else
        return true;
}


//=====================================================================
//  contains()  - face contains vert
//
//  Helper function returns true if face contains
//  the given vertex, false if not.
//=====================================================================
bool BCCLattice3D::contains(const Face3D *face, Vertex3D *vertex)
{
    Vertex3D *v[VERTS_PER_FACE];
    getVertsAroundFace(face, v);

    if(v[0]->isEqualTo(vertex) || v[1]->isEqualTo(vertex) || v[2]->isEqualTo(vertex))
        return true;
    else
        return false;
}

//=====================================================================
//  contains()  - face contains edge
//
//  Helper function returns true if face contains
//  the given edge, false if not.
//=====================================================================
bool BCCLattice3D::contains(const Face3D *face, Edge3D *edge)
{
    Edge3D *e[EDGES_PER_FACE];
    getEdgesAroundFace(face, e);

    if(e[0] == edge || e[1] == edge || e[2] == edge)
        return true;
    else
        return false;
}

//=====================================================================
//  contains()  - tet contains vertex
//
//  Helper function returns true if tet contains
//  the given vertex, false if not.
//=====================================================================
bool BCCLattice3D::contains(const Tet3D *tet, Vertex3D *vertex)
{
    Vertex3D *v[VERTS_PER_TET];
    getVertsAroundTet(tet, v);

    if(v[0]->isEqualTo(vertex) || v[1]->isEqualTo(vertex) ||
       v[2]->isEqualTo(vertex) || v[3]->isEqualTo(vertex))
        return true;
    else
        return false;
}


//=======================================================
//  contains()  - tet contains edge
//
//  Helper function returns true if tet contains
//  the given edge, false if not.
//=======================================================
bool BCCLattice3D::contains(const Tet3D *tet, const Edge3D *edge)
{
    Edge3D *e[EDGES_PER_TET];
    getEdgesAroundTet(tet, e);

    if(e[0] == edge || e[1] == edge || e[2] == edge ||
       e[3] == edge || e[4] == edge || e[5] == edge)
            return true;
    return false;
}

//=====================================================================
//  contains()  - tet contains face
//
//  Helper function returns true if tet contains
//  the given face, false if not.
//=====================================================================
bool BCCLattice3D::contains(const Tet3D  *tet,  const Face3D *face)
{
    Face3D *f[FACES_PER_TET];
    getFacesAroundTet(tet, f);

    if((f[0] == face) || (f[1] == face) || (f[2] == face) || (f[3] == face))
        return true;
    else
        return false;
}



//-------------------------------------------
// Switches the location of two vertices
//-------------------------------------------
void swap(Vertex3D* &v1, Vertex3D* &v2)
{
    Vertex3D *temp = v1;
    v1 = v2;
    v2 = temp;
}

//--------------------------------------------------------------------------------------------------
//  getInnerTet( Face3D*.....)
//
//  This method determines which lattice Tet should take care of projection
//  the triple on the participating face tied to the current lattice warp.
//--------------------------------------------------------------------------------------------------
Tet3D* BCCLattice3D::getInnerTet(const Face3D *face, const Vertex3D *warp_vertex, const vec3 warp_pt)
{
    vec3 dmy_pt;
    vec3 ray = normalize(warp_pt - face->triple->pos());
    Tet3D *tets[2];
    Vertex3D *verts_a[4];
    Vertex3D *verts_b[4];

    getTetsAroundFace(face, tets);
    getVertsAroundTet(tets[0], verts_a);
    getVertsAroundTet(tets[1], verts_b);

    // sort them so exterior vertex is first
    for(int v=0; v < 4; v++){
        if(!contains(face, verts_a[v]))
            swap(verts_a[0], verts_a[v]);

        if(!contains(face, verts_b[v]))
            swap(verts_b[0], verts_b[v]);
    }

    vec3 vec_a = normalize(verts_a[0]->pos() - face->triple->pos());
    vec3 vec_b = normalize(verts_b[0]->pos() - face->triple->pos());
    vec3 n = normalize(cross(verts_a[3]->pos() - verts_a[1]->pos(), verts_a[2]->pos() - verts_a[1]->pos()));

    float dot1 = (float)dot(vec_a, ray);
    float dot2 = (float)dot(vec_b, ray);

    if(dot1 > dot2)
        return tets[0];
    else
        return tets[1];


    // if neither hit, we have a problem
    cerr << "Fatal Error:  Failed to find Inner Tet for Face" << endl;
    exit(-1);
    return NULL;
}

//--------------------------------------------------------------------------------------------------
//  getInnerTet( Edge3D*.....)
//
//  This method determines which lattice Tet should take care of projection
//  the cut on the participating edge tied to the current lattice warp.
//--------------------------------------------------------------------------------------------------
Tet3D* BCCLattice3D::getInnerTet(const Edge3D *edge, const Vertex3D *warp_vertex, const vec3 warp_pt)
{
    // MAJOR TODO: Go Back and Structure This Function like getInnerTet (Face)

    Tet3D *tets[6];
    unsigned int tet_count = 0;
    getTetsAroundEdge(edge, tets, &tet_count);
    vec3 hit_pt = vec3::zero;

    Vertex3D *static_vertex;

    if(edge->v1 == warp_vertex)
        static_vertex = edge->v2;
    else
        static_vertex = edge->v1;

    vec3 origin = 0.5*(edge->v1->pos() + edge->v2->pos());  //edge->cut->pos(); //static_vertex->pos(); //
    vec3 ray = warp_pt - origin;

    for(unsigned int t=0; t < tet_count; t++)
    {
        Face3D *faces[4];
        getFacesAroundTet(tets[t], faces);

        for(int f=0; f < 4; f++)
        {
            Vertex3D *verts[3];
            getVertsAroundFace(faces[f], verts);

            if(triangle_intersection(verts[0], verts[1], verts[2], origin, ray, hit_pt))
            {
                if(L2(edge->cut->pos() - hit_pt) > 1E-3)
                    return tets[t];
            }
        }
    }


    // if none hit, make a less picky choice
    for(unsigned int t=0; t < tet_count; t++)
    {
        Face3D *faces[4];
        getFacesAroundTet(tets[t], faces);

        for(int f=0; f < 4; f++)
        {
            Vertex3D *verts[3];
            getVertsAroundFace(faces[f], verts);

            if(triangle_intersection(verts[0], verts[1], verts[2], origin, ray, hit_pt))
            {
                return tets[t];
            }
        }
    }

    // if STILL none hit, we have a problem
    cerr << "Fatal Error: Failed to find Inner Tet for Edge" << endl;
    exit(-1);

    return NULL;
}
