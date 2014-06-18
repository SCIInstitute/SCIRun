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

   Author: Spencer Frisby
   Date:   June 2014
*/

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixComparison.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc0() {return AlgorithmParameterName("Elc0");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc1() {return AlgorithmParameterName("Elc1");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc2() {return AlgorithmParameterName("Elc2");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc3() {return AlgorithmParameterName("Elc3");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc4() {return AlgorithmParameterName("Elc4");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc5() {return AlgorithmParameterName("Elc5");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc6() {return AlgorithmParameterName("Elc6");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc7() {return AlgorithmParameterName("Elc7");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc8() {return AlgorithmParameterName("Elc8");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc9() {return AlgorithmParameterName("Elc9");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc10() {return AlgorithmParameterName("Elc10");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc11() {return AlgorithmParameterName("Elc11");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc12() {return AlgorithmParameterName("Elc12");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc13() {return AlgorithmParameterName("Elc13");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc14() {return AlgorithmParameterName("Elc14");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc15() {return AlgorithmParameterName("Elc15");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc16() {return AlgorithmParameterName("Elc16");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc17() {return AlgorithmParameterName("Elc17");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc18() {return AlgorithmParameterName("Elc18");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc19() {return AlgorithmParameterName("Elc19");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc20() {return AlgorithmParameterName("Elc20");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc21() {return AlgorithmParameterName("Elc21");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc22() {return AlgorithmParameterName("Elc22");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc23() {return AlgorithmParameterName("Elc23");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc24() {return AlgorithmParameterName("Elc24");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc25() {return AlgorithmParameterName("Elc25");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc26() {return AlgorithmParameterName("Elc26");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc27() {return AlgorithmParameterName("Elc27");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc28() {return AlgorithmParameterName("Elc28");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc29() {return AlgorithmParameterName("Elc29");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc30() {return AlgorithmParameterName("Elc30");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc31() {return AlgorithmParameterName("Elc31");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc32() {return AlgorithmParameterName("Elc32");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc33() {return AlgorithmParameterName("Elc33");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc34() {return AlgorithmParameterName("Elc34");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc35() {return AlgorithmParameterName("Elc35");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc36() {return AlgorithmParameterName("Elc36");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc37() {return AlgorithmParameterName("Elc37");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc38() {return AlgorithmParameterName("Elc38");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc39() {return AlgorithmParameterName("Elc39");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc40() {return AlgorithmParameterName("Elc40");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc41() {return AlgorithmParameterName("Elc41");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc42() {return AlgorithmParameterName("Elc42");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc43() {return AlgorithmParameterName("Elc43");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc44() {return AlgorithmParameterName("Elc44");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc45() {return AlgorithmParameterName("Elc45");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc46() {return AlgorithmParameterName("Elc46");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc47() {return AlgorithmParameterName("Elc47");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc48() {return AlgorithmParameterName("Elc48");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc49() {return AlgorithmParameterName("Elc49");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc50() {return AlgorithmParameterName("Elc50");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc51() {return AlgorithmParameterName("Elc51");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc52() {return AlgorithmParameterName("Elc52");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc53() {return AlgorithmParameterName("Elc53");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc54() {return AlgorithmParameterName("Elc54");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc55() {return AlgorithmParameterName("Elc55");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc56() {return AlgorithmParameterName("Elc56");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc57() {return AlgorithmParameterName("Elc57");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc58() {return AlgorithmParameterName("Elc58");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc59() {return AlgorithmParameterName("Elc59");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc60() {return AlgorithmParameterName("Elc60");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc61() {return AlgorithmParameterName("Elc61");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc62() {return AlgorithmParameterName("Elc62");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc63() {return AlgorithmParameterName("Elc63");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc64() {return AlgorithmParameterName("Elc64");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc65() {return AlgorithmParameterName("Elc65");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc66() {return AlgorithmParameterName("Elc66");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc67() {return AlgorithmParameterName("Elc67");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc68() {return AlgorithmParameterName("Elc68");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc69() {return AlgorithmParameterName("Elc69");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc70() {return AlgorithmParameterName("Elc70");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc71() {return AlgorithmParameterName("Elc71");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc72() {return AlgorithmParameterName("Elc72");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc73() {return AlgorithmParameterName("Elc73");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc74() {return AlgorithmParameterName("Elc74");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc75() {return AlgorithmParameterName("Elc75");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc76() {return AlgorithmParameterName("Elc76");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc77() {return AlgorithmParameterName("Elc77");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc78() {return AlgorithmParameterName("Elc78");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc79() {return AlgorithmParameterName("Elc79");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc80() {return AlgorithmParameterName("Elc80");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc81() {return AlgorithmParameterName("Elc81");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc82() {return AlgorithmParameterName("Elc82");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc83() {return AlgorithmParameterName("Elc83");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc84() {return AlgorithmParameterName("Elc84");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc85() {return AlgorithmParameterName("Elc85");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc86() {return AlgorithmParameterName("Elc86");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc87() {return AlgorithmParameterName("Elc87");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc88() {return AlgorithmParameterName("Elc88");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc89() {return AlgorithmParameterName("Elc89");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc90() {return AlgorithmParameterName("Elc90");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc91() {return AlgorithmParameterName("Elc91");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc92() {return AlgorithmParameterName("Elc92");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc93() {return AlgorithmParameterName("Elc93");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc94() {return AlgorithmParameterName("Elc94");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc95() {return AlgorithmParameterName("Elc95");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc96() {return AlgorithmParameterName("Elc96");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc97() {return AlgorithmParameterName("Elc97");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc98() {return AlgorithmParameterName("Elc98");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc99() {return AlgorithmParameterName("Elc99");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc100() {return AlgorithmParameterName("Elc100");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc101() {return AlgorithmParameterName("Elc101");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc102() {return AlgorithmParameterName("Elc102");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc103() {return AlgorithmParameterName("Elc103");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc104() {return AlgorithmParameterName("Elc104");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc105() {return AlgorithmParameterName("Elc105");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc106() {return AlgorithmParameterName("Elc106");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc107() {return AlgorithmParameterName("Elc107");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc108() {return AlgorithmParameterName("Elc108");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc109() {return AlgorithmParameterName("Elc109");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc110() {return AlgorithmParameterName("Elc110");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc111() {return AlgorithmParameterName("Elc111");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc112() {return AlgorithmParameterName("Elc112");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc113() {return AlgorithmParameterName("Elc113");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc114() {return AlgorithmParameterName("Elc114");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc115() {return AlgorithmParameterName("Elc115");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc116() {return AlgorithmParameterName("Elc116");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc117() {return AlgorithmParameterName("Elc117");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc118() {return AlgorithmParameterName("Elc118");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc119() {return AlgorithmParameterName("Elc119");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc120() {return AlgorithmParameterName("Elc120");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc121() {return AlgorithmParameterName("Elc121");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc122() {return AlgorithmParameterName("Elc122");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc123() {return AlgorithmParameterName("Elc123");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc124() {return AlgorithmParameterName("Elc124");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc125() {return AlgorithmParameterName("Elc125");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc126() {return AlgorithmParameterName("Elc126");} AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc127() {return AlgorithmParameterName("Elc127");}

SetupRHSforTDCSandTMSAlgorithm::SetupRHSforTDCSandTMSAlgorithm()
{
  addParameter(Elc0(), 1.0); addParameter(Elc1(), -1.0); addParameter(Elc2(), 0.0); addParameter(Elc3(), 0.0); addParameter(Elc4(), 0.0); addParameter(Elc5(), 0.0); addParameter(Elc6(), 0.0); addParameter(Elc7(), 0.0); addParameter(Elc8(), 0.0); addParameter(Elc9(), 0.0); addParameter(Elc10(), 0.0); addParameter(Elc11(), 0.0); addParameter(Elc12(), 0.0); addParameter(Elc13(), 0.0); addParameter(Elc14(), 0.0); addParameter(Elc15(), 0.0); addParameter(Elc16(), 0.0); addParameter(Elc17(), 0.0); addParameter(Elc18(), 0.0); addParameter(Elc19(), 0.0); addParameter(Elc20(), 0.0); addParameter(Elc21(), 0.0); addParameter(Elc22(), 0.0); addParameter(Elc23(), 0.0); addParameter(Elc24(), 0.0); addParameter(Elc25(), 0.0); addParameter(Elc26(), 0.0); addParameter(Elc27(), 0.0); addParameter(Elc28(), 0.0); addParameter(Elc29(), 0.0); addParameter(Elc30(), 0.0); addParameter(Elc31(), 0.0); addParameter(Elc32(), 0.0); addParameter(Elc33(), 0.0); addParameter(Elc34(), 0.0); addParameter(Elc35(), 0.0); addParameter(Elc36(), 0.0); addParameter(Elc37(), 0.0); addParameter(Elc38(), 0.0); addParameter(Elc39(), 0.0); addParameter(Elc40(), 0.0); addParameter(Elc41(), 0.0); addParameter(Elc42(), 0.0); addParameter(Elc43(), 0.0); addParameter(Elc44(), 0.0); addParameter(Elc45(), 0.0); addParameter(Elc46(), 0.0); addParameter(Elc47(), 0.0); addParameter(Elc48(), 0.0); addParameter(Elc49(), 0.0); addParameter(Elc50(), 0.0); addParameter(Elc51(), 0.0); addParameter(Elc52(), 0.0); addParameter(Elc53(), 0.0); addParameter(Elc54(), 0.0); addParameter(Elc55(), 0.0); addParameter(Elc56(), 0.0); addParameter(Elc57(), 0.0); addParameter(Elc58(), 0.0); addParameter(Elc59(), 0.0); addParameter(Elc60(), 0.0); addParameter(Elc61(), 0.0); addParameter(Elc62(), 0.0); addParameter(Elc63(), 0.0); addParameter(Elc64(), 0.0); addParameter(Elc65(), 0.0); addParameter(Elc66(), 0.0); addParameter(Elc67(), 0.0); addParameter(Elc68(), 0.0); addParameter(Elc69(), 0.0); addParameter(Elc70(), 0.0); addParameter(Elc71(), 0.0); addParameter(Elc72(), 0.0); addParameter(Elc73(), 0.0); addParameter(Elc74(), 0.0); addParameter(Elc75(), 0.0); addParameter(Elc76(), 0.0); addParameter(Elc77(), 0.0); addParameter(Elc78(), 0.0); addParameter(Elc79(), 0.0); addParameter(Elc80(), 0.0); addParameter(Elc81(), 0.0); addParameter(Elc82(), 0.0); addParameter(Elc83(), 0.0); addParameter(Elc84(), 0.0); addParameter(Elc85(), 0.0); addParameter(Elc86(), 0.0); addParameter(Elc87(), 0.0); addParameter(Elc88(), 0.0); addParameter(Elc89(), 0.0); addParameter(Elc90(), 0.0); addParameter(Elc91(), 0.0); addParameter(Elc92(), 0.0); addParameter(Elc93(), 0.0); addParameter(Elc94(), 0.0); addParameter(Elc95(), 0.0); addParameter(Elc96(), 0.0); addParameter(Elc97(), 0.0); addParameter(Elc98(), 0.0); addParameter(Elc99(), 0.0); addParameter(Elc100(), 0.0); addParameter(Elc101(), 0.0); addParameter(Elc102(), 0.0); addParameter(Elc103(), 0.0); addParameter(Elc104(), 0.0); addParameter(Elc105(), 0.0); addParameter(Elc106(), 0.0); addParameter(Elc107(), 0.0); addParameter(Elc108(), 0.0); addParameter(Elc109(), 0.0); addParameter(Elc110(), 0.0); addParameter(Elc111(), 0.0); addParameter(Elc112(), 0.0); addParameter(Elc113(), 0.0); addParameter(Elc114(), 0.0); addParameter(Elc115(), 0.0); addParameter(Elc116(), 0.0); addParameter(Elc117(), 0.0); addParameter(Elc118(), 0.0); addParameter(Elc119(), 0.0); addParameter(Elc120(), 0.0); addParameter(Elc121(), 0.0); addParameter(Elc122(), 0.0); addParameter(Elc123(), 0.0); addParameter(Elc124(), 0.0); addParameter(Elc125(), 0.0); addParameter(Elc126(), 0.0); addParameter(Elc127(), 0.0);
}

AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_COIL_POSITIONS_AND_NORMAL("ELECTRODE_COIL_POSITIONS_AND_NORMAL");
AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_COUNT("ELECTRODE_COUNT");
AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::RHS("RHS");
//const AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_TRIANGULATION("ELECTRODE_TRIANGULATION");
//const AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODE_TRIANGULATION2("ELECTRODE_TRIANGULATION2");
//const AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::COIL("COIL");
//const AlgorithmInputName SetupRHSforTDCSandTMSAlgorithm::COIL2("COIL2");
//const AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::ELECTRODES_FIELD("ELECTRODES_FIELD");
//const AlgorithmOutputName SetupRHSforTDCSandTMSAlgorithm::COILS_FIELD("COILS_FIELD");

AlgorithmOutput SetupRHSforTDCSandTMSAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto elc_coil_pos_and_normal = input.get<Field>(ELECTRODE_COIL_POSITIONS_AND_NORMAL);
  auto elc_count = input.get<Matrix>(ELECTRODE_COUNT);
  // get matrix info here then pass that information to run
  DenseMatrixHandle elc_count_dense (new DenseMatrix(matrix_cast::as_dense(elc_count)->block(0,0,elc_count->nrows(),elc_count->ncols()))); 
  int elc_elem_num = elc_count_dense->coeff(0,0);

//  auto tri = input.get<Field>(ELECTRODE_TRIANGULATION);
//  auto tri2 = input.get<Field>(ELECTRODE_TRIANGULATION2);
//  auto coil = input.get<Field>(COIL);
//  auto coil2 = input.get<Field>(COIL2);
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(pos_orient, "ELECTRODE_COIL_POSITIONS_AND_NORMAL input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri, "ELECTRODE_TRIANGULATION input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri2, "ELECTRODE_TRIANGULATION2 input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil, "COIL input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil2, "COIL2 input field");
  
  DenseMatrixHandle vector = run(elc_coil_pos_and_normal, elc_elem_num);
    
//  output[ELECTRODES_FIELD] = out1;
//  output[COILS_FIELD] = out2;
  AlgorithmOutput output;
  output[RHS] = vector; // run is a Matrix, wonder if it is complaining that it is a DenseMatrixHandle
  return output;
}

DenseMatrixHandle SetupRHSforTDCSandTMSAlgorithm::run(FieldHandle fh, int num_of_elc) const
{  
  if (num_of_elc > 128) { THROW_ALGORITHM_INPUT_ERROR("Number of electrodes given exceeds what is possible ");}
  else if (num_of_elc < 0) { THROW_ALGORITHM_INPUT_ERROR("Negative number of electrodes given ");}
  
  if (!fh) THROW_ALGORITHM_INPUT_ERROR("Input field was not allocated ");
  
  VField* vfield = fh->vfield();

  double electrode_values[128] = {get(Elc0()).getDouble(), get(Elc1()).getDouble(), get(Elc2()).getDouble(), get(Elc3()).getDouble(), get(Elc4()).getDouble(), get(Elc5()).getDouble(), get(Elc6()).getDouble(), get(Elc7()).getDouble(), get(Elc8()).getDouble(), get(Elc9()).getDouble(), get(Elc10()).getDouble(), get(Elc11()).getDouble(), get(Elc12()).getDouble(), get(Elc13()).getDouble(), get(Elc14()).getDouble(), get(Elc15()).getDouble(), get(Elc16()).getDouble(), get(Elc17()).getDouble(), get(Elc18()).getDouble(), get(Elc19()).getDouble(), get(Elc20()).getDouble(), get(Elc21()).getDouble(), get(Elc22()).getDouble(), get(Elc23()).getDouble(), get(Elc24()).getDouble(), get(Elc25()).getDouble(), get(Elc26()).getDouble(), get(Elc27()).getDouble(), get(Elc28()).getDouble(), get(Elc29()).getDouble(), get(Elc30()).getDouble(), get(Elc31()).getDouble(), get(Elc32()).getDouble(), get(Elc33()).getDouble(), get(Elc34()).getDouble(), get(Elc35()).getDouble(), get(Elc36()).getDouble(), get(Elc37()).getDouble(), get(Elc38()).getDouble(), get(Elc39()).getDouble(), get(Elc40()).getDouble(), get(Elc41()).getDouble(), get(Elc42()).getDouble(), get(Elc43()).getDouble(), get(Elc44()).getDouble(), get(Elc45()).getDouble(), get(Elc46()).getDouble(), get(Elc47()).getDouble(), get(Elc48()).getDouble(), get(Elc49()).getDouble(), get(Elc50()).getDouble(), get(Elc51()).getDouble(), get(Elc52()).getDouble(), get(Elc53()).getDouble(), get(Elc54()).getDouble(), get(Elc55()).getDouble(), get(Elc56()).getDouble(), get(Elc57()).getDouble(), get(Elc58()).getDouble(), get(Elc59()).getDouble(), get(Elc60()).getDouble(), get(Elc61()).getDouble(), get(Elc62()).getDouble(), get(Elc63()).getDouble(), get(Elc64()).getDouble(), get(Elc65()).getDouble(), get(Elc66()).getDouble(), get(Elc67()).getDouble(), get(Elc68()).getDouble(), get(Elc69()).getDouble(), get(Elc70()).getDouble(), get(Elc71()).getDouble(), get(Elc72()).getDouble(), get(Elc73()).getDouble(), get(Elc74()).getDouble(), get(Elc75()).getDouble(), get(Elc76()).getDouble(), get(Elc77()).getDouble(), get(Elc78()).getDouble(), get(Elc79()).getDouble(), get(Elc80()).getDouble(), get(Elc81()).getDouble(), get(Elc82()).getDouble(), get(Elc83()).getDouble(), get(Elc84()).getDouble(), get(Elc85()).getDouble(), get(Elc86()).getDouble(), get(Elc87()).getDouble(), get(Elc88()).getDouble(), get(Elc89()).getDouble(), get(Elc90()).getDouble(), get(Elc91()).getDouble(), get(Elc92()).getDouble(), get(Elc93()).getDouble(), get(Elc94()).getDouble(), get(Elc95()).getDouble(), get(Elc96()).getDouble(), get(Elc97()).getDouble(), get(Elc98()).getDouble(), get(Elc99()).getDouble(), get(Elc100()).getDouble(), get(Elc101()).getDouble(), get(Elc102()).getDouble(), get(Elc103()).getDouble(), get(Elc104()).getDouble(), get(Elc105()).getDouble(), get(Elc106()).getDouble(), get(Elc107()).getDouble(), get(Elc108()).getDouble(), get(Elc109()).getDouble(), get(Elc110()).getDouble(), get(Elc111()).getDouble(), get(Elc112()).getDouble(), get(Elc113()).getDouble(), get(Elc114()).getDouble(), get(Elc115()).getDouble(), get(Elc116()).getDouble(), get(Elc117()).getDouble(), get(Elc118()).getDouble(), get(Elc119()).getDouble(), get(Elc120()).getDouble(), get(Elc121()).getDouble(), get(Elc122()).getDouble(), get(Elc123()).getDouble(), get(Elc124()).getDouble(), get(Elc125()).getDouble(), get(Elc126()).getDouble(), get(Elc127()).getDouble()};
  
  // making sure current intensities of the electrodes together are greater than 10e-6
  double check_value = 0;
  for (int i=0; i<num_of_elc; i++)
  {
    double temp = electrode_values[i]/1000.0;
    if (temp < 0.0)
      temp = temp*(-1.0);
    check_value += temp;
  }
  if (check_value < 0.00001) THROW_ALGORITHM_INPUT_ERROR("Electrode current intensities are negligible ");
  
  int node_elements  = vfield->vmesh()->num_nodes();
  int total_elements = node_elements + num_of_elc;
  
  DenseMatrixHandle output (boost::make_shared<DenseMatrix>(total_elements,1));
  int cnt = 0;
  for (int i=0; i < total_elements; i++)
  {
    if (i < node_elements)
      (*output)(i,0) = 0.0;
    else
      (*output)(i,0) = electrode_values[i-node_elements]/1000.0; // converting to Amps
    
    cnt++;
    if (cnt == total_elements/4)
    {
      cnt = 0;
      update_progress((double)i/total_elements);
    }
  }
  return output;
}
