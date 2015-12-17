/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/IEPlugin/ObjToField_Plugin.h>
#include <Core/IEPlugin/NrrdField_Plugin.h>
#include <Core/IEPlugin/MatlabFiles_Plugin.h>
#include <Core/IEPlugin/SimpleTextFileToMatrix_Plugin.h>
#include <Core/IEPlugin/EcgsimFileToMatrix_Plugin.h>
#include <Core/IEPlugin/PointCloudField_Plugin.h>
#include <Core/IEPlugin/CurveField_Plugin.h>
#include <Core/IEPlugin/TriSurfField_Plugin.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/IEPlugin/IEPluginInit.h>

using namespace SCIRun;
using namespace SCIRun::Core::Logging;

void IEPluginManager::Initialize()
{
  static FieldIEPluginLegacyAdapter ObjToField_plugin("ObjToField", "*.obj", "", ObjToField_reader, FieldToObj_writer);

  static FieldIEPluginLegacyAdapter NrrdToField_plugin("NrrdFile","*.nhdr *.nrrd", "*.nrrd", NrrdToField_reader, FieldToNrrd_writer);
  static FieldIEPluginLegacyAdapter NodalNrrdToField_plugin("NrrdFile[DataOnNodes]","*.nhdr *.nrrd", "", Nodal_NrrdToField_reader, nullptr);
  static FieldIEPluginLegacyAdapter ModalNrrdToField_plugin("NrrdFile[DataOnElements]","*.nhdr *.nrrd", "", Modal_NrrdToField_reader, nullptr);
  static FieldIEPluginLegacyAdapter IPNodalNrrdToField_plugin("NrrdFile[DataOnNodes,InvertParity]","*.nhdr *.nrrd", "", IPNodal_NrrdToField_reader, nullptr);
  static FieldIEPluginLegacyAdapter IPModalNrrdToField_plugin("NrrdFile[DataOnElements,InvertParity]","*.nhdr *.nrrd", "", IPModal_NrrdToField_reader, nullptr);

  static FieldIEPluginLegacyAdapter MatlabField_plugin("Matlab Field", "*.mat", "*.mat", MatlabField_reader, MatlabField_writer);

  static MatrixIEPluginLegacyAdapter MatlabMatrix_plugin("Matlab Matrix","*.mat", "*.mat", MatlabMatrix_reader, MatlabMatrix_writer);
  //TODO
  //static NrrdIEPluginLegacyAdapter MatlabNrrd_plugin("Matlab Matrix",".mat", "*.mat",MatlabNrrd_reader,MatlabNrrd_writer);

  static MatrixIEPluginLegacyAdapter SimpleTextFileMatrix_plugin("SimpleTextFile","*.*", "",SimpleTextFileMatrix_reader,SimpleTextFileMatrix_writer);

  static FieldIEPluginLegacyAdapter PointCloudField_plugin("PointCloudField", "*.pts *.pos *.txt", "", TextToPointCloudField_reader, PointCloudFieldToText_writer);

  static FieldIEPluginLegacyAdapter CurveField_plugin("CurveField", "*.pts *.pos *.edge", "", TextToCurveField_reader, CurveFieldToTextBaseIndexZero_writer);

  static MatrixIEPluginLegacyAdapter EcgsimFileMatrix_plugin("ECGSimFile", "", "", EcgsimFileMatrix_reader, EcgsimFileMatrix_writer);

  static FieldIEPluginLegacyAdapter TriSurfField_plugin("TriSurfField", "*.fac *.tri *.pts *.pos", "", TextToTriSurfField_reader, TriSurfFieldToTextBaseIndexZero_writer);
  static FieldIEPluginLegacyAdapter TriSurfFieldBaseIndexOne_plugin("TriSurfField[BaseIndex 1]", "*.fac *.pts", "", nullptr, TriSurfFieldToTextBaseIndexOne_writer);
  static FieldIEPluginLegacyAdapter CVRTI_FacPtsFileToTriSurf_plugin("CVRTI_FacPtsFileToTriSurf", "*.fac *.tri *.pts *.pos", "", TextToTriSurfField_reader, TriSurfFieldToTextBaseIndexZero_writer);
  static FieldIEPluginLegacyAdapter TriSurfFieldToM_plugin("TriSurfFieldToM", "*.m", "", MToTriSurfField_reader, TriSurfFieldToM_writer);
  static FieldIEPluginLegacyAdapter TriSurfFieldVtk_plugin("TriSurfFieldToVtk", "*.vtk", "", nullptr, TriSurfFieldToVtk_writer);
  static FieldIEPluginLegacyAdapter VtkFromTriSurfField_plugin("VtkToTriSurfField", "*.vtk", "", VtkToTriSurfField_reader, nullptr);
  static FieldIEPluginLegacyAdapter TriSurfFieldToExotxt_plugin("TriSurfFieldToExotxt", "*.ex2", "", nullptr, TriSurfFieldToExotxt_writer);
  static FieldIEPluginLegacyAdapter TriSurfFieldToExotxtBaseIndexOne_plugin("TriSurfFieldToExotxt[BaseIndex 1]", "*.ex2", "", nullptr, TriSurfFieldToExotxtBaseIndexOne_writer);
}
