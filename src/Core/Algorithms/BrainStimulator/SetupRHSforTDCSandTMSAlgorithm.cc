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
   Date: June 2014
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
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <iostream>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc0("Elc0");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc1("Elc1");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc2("Elc2");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc3("Elc3");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc4("Elc4");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc5("Elc5");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc6("Elc6");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc7("Elc7");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc8("Elc8");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc9("Elc9");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc10("Elc10");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc11("Elc11");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc12("Elc12");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc13("Elc13");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc14("Elc14");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc15("Elc15");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc16("Elc16");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc17("Elc17");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc18("Elc18");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc19("Elc19");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc20("Elc20");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc21("Elc21");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc22("Elc22");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc23("Elc23");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc24("Elc24");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc25("Elc25");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc26("Elc26");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc27("Elc27");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc28("Elc28");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc29("Elc29");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc30("Elc30");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc31("Elc31");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc32("Elc32");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc33("Elc33");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc34("Elc34");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc35("Elc35");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc36("Elc36");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc37("Elc37");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc38("Elc38");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc39("Elc39");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc40("Elc40");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc41("Elc41");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc42("Elc42");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc43("Elc43");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc44("Elc44");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc45("Elc45");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc46("Elc46");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc47("Elc47");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc48("Elc48");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc49("Elc49");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc50("Elc50");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc51("Elc51");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc52("Elc52");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc53("Elc53");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc54("Elc54");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc55("Elc55");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc56("Elc56");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc57("Elc57");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc58("Elc58");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc59("Elc59");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc60("Elc60");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc61("Elc61");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc62("Elc62");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc63("Elc63");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc64("Elc64");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc65("Elc65");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc66("Elc66");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc67("Elc67");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc68("Elc68");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc69("Elc69");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc70("Elc70");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc71("Elc71");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc72("Elc72");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc73("Elc73");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc74("Elc74");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc75("Elc75");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc76("Elc76");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc77("Elc77");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc78("Elc78");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc79("Elc79");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc80("Elc80");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc81("Elc81");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc82("Elc82");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc83("Elc83");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc84("Elc84");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc85("Elc85");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc86("Elc86");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc87("Elc87");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc88("Elc88");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc89("Elc89");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc90("Elc90");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc91("Elc91");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc92("Elc92");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc93("Elc93");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc94("Elc94");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc95("Elc95");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc96("Elc96");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc97("Elc97");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc98("Elc98");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc99("Elc99");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc100("Elc100");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc101("Elc101");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc102("Elc102");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc103("Elc103");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc104("Elc104");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc105("Elc105");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc106("Elc106");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc107("Elc107");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc108("Elc108");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc109("Elc109");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc110("Elc110");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc111("Elc111");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc112("Elc112");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc113("Elc113");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc114("Elc114");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc115("Elc115");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc116("Elc116");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc117("Elc117");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc118("Elc118");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc119("Elc119");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc120("Elc120");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc121("Elc121");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc122("Elc122");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc123("Elc123");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc124("Elc124");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc125("Elc125");
AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Elc126("Elc126");

SetupRHSforTDCSandTMSAlgorithm::SetupRHSforTDCSandTMSAlgorithm()
{
  addParameter(Elc0, 1.0);
  addParameter(Elc1, -1.0);
  addParameter(Elc2, 0.0);
  addParameter(Elc3, 0.0);
  addParameter(Elc4, 0.0);
  addParameter(Elc5, 0.0);
  addParameter(Elc6, 0.0);
  addParameter(Elc7, 0.0);
  addParameter(Elc8, 0.0);
  addParameter(Elc9, 0.0);
  addParameter(Elc10, 0.0);
  addParameter(Elc11, 0.0);
  addParameter(Elc12, 0.0);
  addParameter(Elc13, 0.0);
  addParameter(Elc14, 0.0);
  addParameter(Elc15, 0.0);
  addParameter(Elc16, 0.0);
  addParameter(Elc17, 0.0);
  addParameter(Elc18, 0.0);
  addParameter(Elc19, 0.0);
  addParameter(Elc20, 0.0);
  addParameter(Elc21, 0.0);
  addParameter(Elc22, 0.0);
  addParameter(Elc23, 0.0);
  addParameter(Elc24, 0.0);
  addParameter(Elc25, 0.0);
  addParameter(Elc26, 0.0);
  addParameter(Elc27, 0.0);
  addParameter(Elc28, 0.0);
  addParameter(Elc29, 0.0);
  addParameter(Elc30, 0.0);
  addParameter(Elc31, 0.0);
  addParameter(Elc32, 0.0);
  addParameter(Elc33, 0.0);
  addParameter(Elc34, 0.0);
  addParameter(Elc35, 0.0);
  addParameter(Elc36, 0.0);
  addParameter(Elc37, 0.0);
  addParameter(Elc38, 0.0);
  addParameter(Elc39, 0.0);
  addParameter(Elc40, 0.0);
  addParameter(Elc41, 0.0);
  addParameter(Elc42, 0.0);
  addParameter(Elc43, 0.0);
  addParameter(Elc44, 0.0);
  addParameter(Elc45, 0.0);
  addParameter(Elc46, 0.0);
  addParameter(Elc47, 0.0);
  addParameter(Elc48, 0.0);
  addParameter(Elc49, 0.0);
  addParameter(Elc50, 0.0);
  addParameter(Elc51, 0.0);
  addParameter(Elc52, 0.0);
  addParameter(Elc53, 0.0);
  addParameter(Elc54, 0.0);
  addParameter(Elc55, 0.0);
  addParameter(Elc56, 0.0);
  addParameter(Elc57, 0.0);
  addParameter(Elc58, 0.0);
  addParameter(Elc59, 0.0);
  addParameter(Elc60, 0.0);
  addParameter(Elc61, 0.0);
  addParameter(Elc62, 0.0);
  addParameter(Elc63, 0.0);
  addParameter(Elc64, 0.0);
  addParameter(Elc65, 0.0);
  addParameter(Elc66, 0.0);
  addParameter(Elc67, 0.0);
  addParameter(Elc68, 0.0);
  addParameter(Elc69, 0.0);
  addParameter(Elc70, 0.0);
  addParameter(Elc71, 0.0);
  addParameter(Elc72, 0.0);
  addParameter(Elc73, 0.0);
  addParameter(Elc74, 0.0);
  addParameter(Elc75, 0.0);
  addParameter(Elc76, 0.0);
  addParameter(Elc77, 0.0);
  addParameter(Elc78, 0.0);
  addParameter(Elc79, 0.0);
  addParameter(Elc80, 0.0);
  addParameter(Elc81, 0.0);
  addParameter(Elc82, 0.0);
  addParameter(Elc83, 0.0);
  addParameter(Elc84, 0.0);
  addParameter(Elc85, 0.0);
  addParameter(Elc86, 0.0);
  addParameter(Elc87, 0.0);
  addParameter(Elc88, 0.0);
  addParameter(Elc89, 0.0);
  addParameter(Elc90, 0.0);
  addParameter(Elc91, 0.0);
  addParameter(Elc92, 0.0);
  addParameter(Elc93, 0.0);
  addParameter(Elc94, 0.0);
  addParameter(Elc95, 0.0);
  addParameter(Elc96, 0.0);
  addParameter(Elc97, 0.0);
  addParameter(Elc98, 0.0);
  addParameter(Elc99, 0.0);
  addParameter(Elc100, 0.0);
  addParameter(Elc101, 0.0);
  addParameter(Elc102, 0.0);
  addParameter(Elc103, 0.0);
  addParameter(Elc104, 0.0);
  addParameter(Elc105, 0.0);
  addParameter(Elc106, 0.0);
  addParameter(Elc107, 0.0);
  addParameter(Elc108, 0.0);
  addParameter(Elc109, 0.0);
  addParameter(Elc110, 0.0);
  addParameter(Elc111, 0.0);
  addParameter(Elc112, 0.0);
  addParameter(Elc113, 0.0);
  addParameter(Elc114, 0.0);
  addParameter(Elc115, 0.0);
  addParameter(Elc116, 0.0);
  addParameter(Elc117, 0.0);
  addParameter(Elc118, 0.0);
  addParameter(Elc119, 0.0);
  addParameter(Elc120, 0.0);
  addParameter(Elc121, 0.0);
  addParameter(Elc122, 0.0);
  addParameter(Elc123, 0.0);
  addParameter(Elc124, 0.0);
  addParameter(Elc125, 0.0);
  addParameter(Elc126, 0.0);
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

DenseMatrixHandle SetupRHSforTDCSandTMSAlgorithm::run(FieldHandle fh, MatrixHandle elc) const
{
  // storing all the electrode values, only the indicies determined by the matrix are used
  double electrod_values[128] = {get(Elc0).getDouble(),get(Elc1).getDouble(),get(Elc2).getDouble(),get(Elc3).getDouble(),get(Elc4).getDouble(),get(Elc5).getDouble(),get(Elc6).getDouble(),get(Elc7).getDouble(),get(Elc8).getDouble(),get(Elc9).getDouble(),get(Elc10).getDouble(),get(Elc11).getDouble(),get(Elc12).getDouble(),get(Elc13).getDouble(),get(Elc14).getDouble(),get(Elc15).getDouble(),get(Elc16).getDouble(),get(Elc17).getDouble(),get(Elc18).getDouble(),get(Elc19).getDouble(),get(Elc20).getDouble(),get(Elc21).getDouble(),get(Elc22).getDouble(),get(Elc23).getDouble(),get(Elc24).getDouble(),get(Elc25).getDouble(),get(Elc26).getDouble(),get(Elc27).getDouble(),get(Elc28).getDouble(),get(Elc29).getDouble(),get(Elc30).getDouble(),get(Elc31).getDouble(),get(Elc32).getDouble(),get(Elc33).getDouble(),get(Elc34).getDouble(),get(Elc35).getDouble(),get(Elc36).getDouble(),get(Elc37).getDouble(),get(Elc38).getDouble(),get(Elc39).getDouble(),get(Elc40).getDouble(),get(Elc41).getDouble(),get(Elc42).getDouble(),get(Elc43).getDouble(),get(Elc44).getDouble(),get(Elc45).getDouble(),get(Elc46).getDouble(),get(Elc47).getDouble(),get(Elc48).getDouble(),get(Elc49).getDouble(),get(Elc50).getDouble(),get(Elc51).getDouble(),get(Elc52).getDouble(),get(Elc53).getDouble(),get(Elc54).getDouble(),get(Elc55).getDouble(),get(Elc56).getDouble(),get(Elc57).getDouble(),get(Elc58).getDouble(),get(Elc59).getDouble(),get(Elc60).getDouble(),get(Elc61).getDouble(),get(Elc62).getDouble(),get(Elc63).getDouble(),get(Elc64).getDouble(),get(Elc65).getDouble(),get(Elc66).getDouble(),get(Elc67).getDouble(),get(Elc68).getDouble(),get(Elc69).getDouble(),get(Elc70).getDouble(),get(Elc71).getDouble(),get(Elc72).getDouble(),get(Elc73).getDouble(),get(Elc74).getDouble(),get(Elc75).getDouble(),get(Elc76).getDouble(),get(Elc77).getDouble(),get(Elc78).getDouble(),get(Elc79).getDouble(),get(Elc80).getDouble(),get(Elc81).getDouble(),get(Elc82).getDouble(),get(Elc83).getDouble(),get(Elc84).getDouble(),get(Elc85).getDouble(),get(Elc86).getDouble(),get(Elc87).getDouble(),get(Elc88).getDouble(),get(Elc89).getDouble(),get(Elc90).getDouble(),get(Elc91).getDouble(),get(Elc92).getDouble(),get(Elc93).getDouble(),get(Elc94).getDouble(),get(Elc95).getDouble(),get(Elc96).getDouble(),get(Elc97).getDouble(),get(Elc98).getDouble(),get(Elc99).getDouble(),get(Elc100).getDouble(),get(Elc101).getDouble(),get(Elc102).getDouble(),get(Elc103).getDouble(),get(Elc104).getDouble(),get(Elc105).getDouble(),get(Elc106).getDouble(),get(Elc107).getDouble(),get(Elc108).getDouble(),get(Elc109).getDouble(),get(Elc110).getDouble(),get(Elc111).getDouble(),get(Elc112).getDouble(),get(Elc113).getDouble(),get(Elc114).getDouble(),get(Elc115).getDouble(),get(Elc116).getDouble(),get(Elc117).getDouble(),get(Elc118).getDouble(),get(Elc119).getDouble(),get(Elc120).getDouble(),get(Elc121).getDouble(),get(Elc122).getDouble(),get(Elc123).getDouble(),get(Elc124).getDouble(),get(Elc125).getDouble(),get(Elc126).getDouble()};
  
  // converting elc matrix handle to dense matrix to obtain number of electrodes to copy
  // converting field to be virtual to obtain number of nodes
  DenseMatrixHandle elc_dense (new DenseMatrix(matrix_cast::as_dense(elc)->block(0,0,elc->nrows(),elc->ncols())));
  VField* vfield = fh->vfield();
  int total_elements = vfield->vmesh()->num_nodes() + elc_dense->coeff(0,0);
  int node_elements  = vfield->vmesh()->num_nodes();
  DenseMatrixHandle output (boost::make_shared<DenseMatrix>(total_elements,1));
  
  // giving output its values
  for (int i=0; i < total_elements; i++)
  {
    if (i < node_elements)
      (*output)(i,0) = 0.0;
    else
      (*output)(i,0) = electrod_values[i-node_elements]/1000.00; // Amps
  }
  
  // DEBUG: displaying vector created
  for (int i=0; i<output->nrows(); i++)
    std::cout << i << " " << output->coeff(i,0) << std::endl;
  
  return output;
}

AlgorithmOutput SetupRHSforTDCSandTMSAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto pos_orient = input.get<Field>(ELECTRODE_COIL_POSITIONS_AND_NORMAL);
  auto num_of_elc = input.get<Matrix>(ELECTRODE_COUNT);

//  auto tri = input.get<Field>(ELECTRODE_TRIANGULATION);
//  auto tri2 = input.get<Field>(ELECTRODE_TRIANGULATION2);
//  auto coil = input.get<Field>(COIL);
//  auto coil2 = input.get<Field>(COIL2);
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(pos_orient, "ELECTRODE_COIL_POSITIONS_AND_NORMAL input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri, "ELECTRODE_TRIANGULATION input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(tri2, "ELECTRODE_TRIANGULATION2 input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil, "COIL input field");
//  ENSURE_ALGORITHM_INPUT_NOT_NULL(coil2, "COIL2 input field");
//old-style run call, just put algorithm code here
//auto outputs = run(boost::make_tuple(lhs, rhs), Option(get(Variables::AppendMatrixOption).getInt()));
// CODE HERE
//  FieldHandle out1,out2;
//  //Algorithm starts here:
//  //VField* vfield = elc_coil_pos_and_normal->vfield();
//   VMesh*  vmesh  = pos_orient->vmesh();
//   std::cout << "a: " << vmesh->num_nodes() << std::endl;
//   //for (int i=0;i<vmesh->num_nodes();;i++)
//   //{
//   //}
  
  DenseMatrixHandle vector = run(pos_orient, num_of_elc);
    
  AlgorithmOutput output;
//  output[ELECTRODES_FIELD] = out1;
//  output[COILS_FIELD] = out2;
  output[RHS] = vector;
  return output;
}
