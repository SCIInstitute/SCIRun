/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


/// @todo Documentation Modules/DataIO/WriteG3D.cc


#include <Modules/DataIO/WriteG3D.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Logging/Log.h>


#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Algorithms/Legacy/DataIO/ObjToFieldReader.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

const Dataflow::Networks::ModuleLookupInfo WriteG3D::staticInfo_("WriteG3D", "DataIO", "SCIRun");

WriteG3D::WriteG3D()
  : my_base(staticInfo_.module_name_, staticInfo_.category_name_, staticInfo_.package_name_, "Filename")
{
  INITIALIZE_PORT(FieldToWrite);
  //INITIALIZE_PORT(ColorMapObject);
  filetype_ = "Binary";
  objectPortName_ = &FieldToWrite;

  FieldIEPluginManager mgr;
  //TODO: change from hard coded types to getting the correct group from the list exporter
  auto types = makeGuiTypesListForExport(mgr);
  types = "IV3D (*.g3d);;ObjToField (*.obj)";
  get_state()->setTransientValue(Variables::FileTypeList, types);
}

void WriteG3D::setStateDefaults()
{
  auto  state = get_state();
  state->setValue(EnableTransparency, false);
  state->setValue(TransparencyValue, 0.65f);
  state->setValue(Coloring, 0);
  state->setValue(DefaultColor, ColorRGB(0.5, 0.5, 0.5).toString());

  //Call base class to ensure the inherited defaults are set
  my_base::setStateDefaults();
}

bool WriteG3D::call_exporter(const std::string& filename)
{
  ObjToFieldReader todo(getLogger());
  return todo.write(filename, getRequiredInput(FieldToWrite));
}

void WriteG3D::execute()
{
  calculateColors();

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  //get the current file name
  const std::string oldfilename=filename_.get();

  //determine if we should increment an index in the file name
  if (gui_increment_.get())
  {

    //warn the user if they try to use 'Increment' incorrectly
    const std::string::size_type loc2 = oldfilename.find("%d");
    if(loc2 == std::string::npos)
    {
      remark("To use the increment function, there must be a '%d' in the file name.");
    }

    char buf[1024];

    int current=gui_current_.get();
    sprintf(buf, filename_.get().c_str(), current);

    filename_.set(buf);
    gui_current_.set(current+1);
  }
#endif

  my_base::execute();

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   if (gui_increment_.get())
    filename_.set(oldfilename);
#endif
}

bool WriteG3D::useCustomExporter(const std::string& filename) const
{
  return true;
}

std::string WriteG3D::defaultFileTypeName() const
{
  return "*.g3d";
}

void WriteG3D::calculateColors()
{
  auto field = getRequiredInput(FieldToWrite);
  //auto colorMap = getOptionalInput(ColorMapObject);

  if (needToExecute())
  {
    VField* fld = field->vfield();
    VMesh*  mesh = field->vmesh();

    Vector vval;
    Tensor tval;

    float transparency = 1.0f;
    auto state = get_state();
    if (state->getValue(EnableTransparency).toBool())
    {
      transparency = static_cast<float>(state->getValue(TransparencyValue).toDouble());
    }

    ColorRGB node_color;

    mesh->synchronize(Mesh::NODES_E);

    VMesh::Node::iterator eiter, eiter_end;
    mesh->begin(eiter);
    mesh->end(eiter_end);

    while (eiter != eiter_end)
    {
      checkForInterruption();

      Point p;
      mesh->get_point(p, *eiter);

      //coloring options
      //Default color
      if (state->getValue(Coloring).toInt() == 0)
      {
        ColorRGB defaultColor = ColorRGB(state->getValue(DefaultColor).toString());
        //TODO: extract method
        defaultColor = (defaultColor.r() > 1.0 || defaultColor.g() > 1.0 || defaultColor.b() > 1.0) ?
          ColorRGB(defaultColor.r() / 255., defaultColor.g() / 255., defaultColor.b() / 255.) : defaultColor;
        node_color = defaultColor;
      }
      // Color map lookup
      /*else if (state->getValue(Coloring).toInt() == 1)
      {
        ColorMapHandle map = colorMap.get();
        if (fld->is_scalar())
        {
          fld->get_value(sval, *eiter);
          node_color = map->valueToColor(sval);
        }
        else if (fld->is_vector())
        {
          fld->get_value(vval, *eiter);
          node_color = map->valueToColor(vval);
        }
        else if (fld->is_tensor())
        {
          fld->get_value(tval, *eiter);
          node_color = map->valueToColor(tval);
        }
      }*/
      // RGB conversion
      else
      {
        if (fld->is_scalar())
        {
          Vector colorVector = Vector(p.x(), p.y(), p.z()).normal();
          node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
        }
        else if (fld->is_vector())
        {
          fld->get_value(vval, *eiter);
          Vector colorVector = vval.normal();
          node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
        }
        else if (fld->is_tensor())
        {
          fld->get_value(tval, *eiter);
          Vector colorVector = tval.get_eigenvector1().normal();
          node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
        }
      }
      colors_.push_back(ColorRGB(node_color.r(), node_color.g(), node_color.b(), transparency));
      ++eiter;
    }
  }
}

const AlgorithmParameterName WriteG3D::EnableTransparency("EnableTransparency");
const AlgorithmParameterName WriteG3D::TransparencyValue("TransparencyValue");
const AlgorithmParameterName WriteG3D::Coloring("Coloring");
const AlgorithmParameterName WriteG3D::DefaultColor("DefaultColor");
