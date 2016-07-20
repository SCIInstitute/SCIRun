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
/// @todo Documentation Modules/DataIO/WriteG3D.cc


#include <Modules/DataIO/WriteG3D.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Logging/Log.h>


#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

const Dataflow::Networks::ModuleLookupInfo WriteG3D::staticInfo_("WriteG3D", "DataIO", "SCIRun");

WriteG3D::WriteG3D()
  : my_base(staticInfo_.module_name_, staticInfo_.category_name_, staticInfo_.package_name_, "Filename")
    //gui_increment_(get_ctx()->subVar("increment"), 0),
    //gui_current_(get_ctx()->subVar("current"), 0)
{
  INITIALIZE_PORT(FieldToWrite);
  filetype_ = "Binary";
  objectPortName_ = &FieldToWrite;

  FieldIEPluginManager mgr;
  //TODO: change from hard coded types to getting the correct group from the list exporter
  auto types = makeGuiTypesListForExport(mgr);
  types = "IV3D (*.g3d);;ObjToField (*.obj)";
  get_state()->setValue(Variables::FileTypeList, types);
}

bool WriteG3D::call_exporter(const std::string& filename)
{
  ///@todo: how will this work via python? need more code to set the filetype based on the extension...
  FieldIEPluginManager mgr;
  auto pl = mgr.get_plugin(get_state()->getValue(Variables::FileTypeName).toString());
  if (pl)
  {
    //return pl->writeFile(handle_, filename, getLogger());
    return write(filename, handle_);
  }
  return false;
}

void WriteG3D::execute()
{
  auto field = getRequiredInput(FieldToWrite);
  //auto colorMap = getOptionalInput(ColorMapObject);

  if (needToExecute())
  {
  }

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
  auto ft = get_state()->getValue(Variables::FileTypeName).toString();
  LOG_DEBUG("WriteG3D with filetype " << ft);
  auto ret = boost::filesystem::extension(filename) != ".fld";
  
  filetype_ = ft.find("SCIRun Field ASCII") != std::string::npos ? "ASCII" : "Binary";

  return ret;
}

std::string WriteG3D::defaultFileTypeName() const
{
  FieldIEPluginManager mgr;
  return defaultImportTypeForFile(&mgr);
}
/*
void WriteG3D::calculateColors(
  boost::shared_ptr<Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap)
{

  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  double sval;
  Vector vval;
  Tensor tval;

  ColorScheme colorScheme;
  ColorRGB node_color;

  if (fld->basis_order() < 0 || (fld->basis_order() == 0 && mesh->dimensionality() != 0) || state.get(RenderState::USE_DEFAULT_COLOR_NODES))
    colorScheme = ColorScheme::COLOR_UNIFORM;
  else if (state.get(RenderState::USE_COLORMAP_ON_NODES))
    colorScheme = ColorScheme::COLOR_MAP;
  else
    colorScheme = ColorScheme::COLOR_IN_SITU;

  mesh->synchronize(Mesh::NODES_E);

  VMesh::Node::iterator eiter, eiter_end;
  mesh->begin(eiter);
  mesh->end(eiter_end);

  double radius = moduleState->getValue(ShowField::SphereScaleValue).toDouble();
  double num_strips = static_cast<double>(moduleState->getValue(ShowField::SphereResolution).toInt());
  if (radius < 0) radius = 1.;
  if (num_strips < 0) num_strips = 10.;
  std::stringstream ss;
  ss << state.get(RenderState::USE_SPHERE) << radius << num_strips << static_cast<int>(colorScheme);

  std::string uniqueNodeID = id + "node" + ss.str();

  nodeTransparencyValue_ = static_cast<float>(moduleState->getValue(ShowField::NodeTransparencyValue).toDouble());

  SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::POINTS;
  // Use spheres...
  if (state.get(RenderState::USE_SPHERE))
    primIn = SpireIBO::PRIMITIVE::TRIANGLES;

  GlyphGeom glyphs;
  while (eiter != eiter_end)
  {
    interruptible->checkForInterruption();

    Point p;
    mesh->get_point(p, *eiter);
    //coloring options
    if (colorScheme != ColorScheme::COLOR_UNIFORM)
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
    }
    //accumulate VBO or IBO data
    if (state.get(RenderState::USE_SPHERE))
    {
      glyphs.addSphere(p, radius, num_strips, node_color);
    }
    else
    {
      glyphs.addPoint(p, node_color);
    }

    ++eiter;
  }

  glyphs.buildObject(geom, uniqueNodeID, state.get(RenderState::USE_TRANSPARENT_NODES), nodeTransparencyValue_,
    colorScheme, state, primIn, mesh->get_bounding_box());
}
*/
//TODO: below method is to test functionality. Needs to be moved to Core\Algorithms\Legacy\DataIO\ObjToFieldReader.cc
bool WriteG3D::write(const std::string& filename, const FieldHandle& field)
{
  std::ofstream os;
  const VMesh* mesh = field->vmesh();

  if (mesh->num_nodes() == 0) { return false; }

  os.open(filename.c_str(), std::ios::out);
  if (!os) { return false; }

  os << "# written by SCIRun\n";

  {
    VMesh::Node::size_type iter;
    VMesh::Node::size_type end = mesh->num_nodes();
    for (iter = 0; iter != end; ++iter)
    {
      Point p;
      mesh->get_point(p, iter);
      os << "v " << p.x() << " " << p.y() << " " << p.z() << "\n";
    }
  }

  {
    VMesh::Face::iterator iter;
    VMesh::Face::iterator end;
    VMesh::Node::array_type faceNodes(4);
    mesh->end(end);
    for (mesh->begin(iter); iter != end; ++iter)
    {
      mesh->get_nodes(faceNodes, *iter);
      // OBJ face indices are 1-based.  Seriously.
      os << "f " << faceNodes[0] + 1 << " " << faceNodes[1] + 1 << " "
        << faceNodes[2] + 1 << "\n";
    }
  }
  os.close();

  return true;
}