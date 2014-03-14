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

/**
 *TODO @todo Documentation Modules/Legacy/Fields/AlignMeshBoundingBoxes.cc/.h,
 *  ApplyFilterToFieldData.cc, ApplyMappingMatrix.cc, BuildMappingMatrix.cc,
 *  BuildMatrixOfSurfaceNormals.cc, CalculateDistanceToField.cc,
 *  CalculateDistanceToFieldBoundary.cc, CalculateFieldData.cc,
 *  CalculateFieldData2.cc, CalculateFieldData3.cc, CalculateFieldData4.cc,
 *  CalculateFieldData5.cc, CalculateFieldDataMetric.cc, 
 *  CalculateGradients.cc/.h, CalculateInsideWhichField.cc,
 *  CalculateIsInsideField.cc, CalculateLatVolGradientAtNodes.cc,
 *  CalculateMeshCenter.cc, CalculateMeshConnector.cc, CalculateMeshnodes.cc,
 *  CalculateNodeNormals.cc, CalculateProfile.cc, 
 *  CalculateSignedDistanceToField.cc/.h CalculateVectorMagnitudes.cc/.h,
 *  CleanupTetMesh.cc, ClipFieldByFunction.cc, ClipFieldByFunction2.cc,
 *  ClipFieldByFunction3.cc, ClipFieldByMesh.cc, CollectFields.cc,
 *  CollectPointClouds.cc, ConvertFieldBasis.cc, ConvertFieldDatatypes.cc,
 *  ConvertHexVolToTetVol.cc, ConvertIndiciesToFieldData.cc,
 *  ConverLatVolDataFromElemToNode.cc, ConvertLatVolFromNodeToElem.cc,
 *  ConvertMeshToIrregularMesh.cc, ConvertMeshToPointCloud.cc,
 *  ConvertMeshtoUnstructuredMesh.cc, ConverQuadSurfToTriSurf.cc/.h,
 *  CreateFieldData.cc, FairMesh.cc, FlipSurfaceNormals.cc, 
 *  GeneratedMedialAxisPoints.cc, GenerateSinglePointProbeFromFields.cc,
 *  GetAllSegmentationBoundaries.cc, GetDomainBoundary.cc,/.h, 
 *  GetDomainStructure.cc, GetFieldBoundary.cc/.h, GetFieldNodes.cc/.h,
 *  GetMeshQualityField.cc, JoinFields.cc, MapFieldDataFromElemToNode.cc,
 *  MapFieldDataFromNodeToElem.cc, MapFieldDataFromSourceToDestination.cc,
 *  MapFieldDataOntoElems.cc, MapFieldDataOntoNodes, 
 *  MapFieldDataOntoNodesRadialbasis.cc, MatchDomainLabels.cc, PadRegularMesh.cc,
 *  ProjectPointsOntoMesh.cc, RefineMesh.cc, RegesterWithCorrespondences.cc,
 *  RemoveUnused.cc, ReportFieldInfo.cc, ResampleRegularMesh.cc, 
 *  ScaleFieldMeshAndData.cc, SelectAndSetFieldData.cc, 
 *  SelectAndSetFieldData3.cc, SetFieldData.cc, SetFieldDatatToConstantValue.cc,
 *  SetFieldNodes.cc, SmoothVecFieldMedian.cc, SplitFieldByConnectionRegion.cc,
 *  SplitlFieldByDomain.cc, SplitNodesByDomain.cc, TransformMeshWithTransform.cc,
 */

#include <Modules/Legacy/Fields/AlignMeshBoundingBoxes.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Matrix.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;

AlignMeshBoundingBoxes::AlignMeshBoundingBoxes() :
  Module(ModuleLookupInfo("AlignMeshBoundingBoxes", "ChangeMesh", "SCIRun"), false)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(AlignmentField);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(TransformMatrix);
}

void AlignMeshBoundingBoxes::execute()
{
  FieldHandle ifield = getRequiredInput(InputField);
  FieldHandle objfield = getRequiredInput(AlignmentField);

  // inputs_changed_ || !oport_cached("Output") || !oport_cached("Transform")
  if (needToExecute())
  {
    update_state(Executing);

    auto output = algo().run_generic(make_input((InputField, ifield)(AlignmentField, objfield)));

    sendOutputFromAlgorithm(OutputField, output);
    sendOutputFromAlgorithm(TransformMatrix, output);
  }
}
