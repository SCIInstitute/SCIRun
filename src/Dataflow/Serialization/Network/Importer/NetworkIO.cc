//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2015 Scientific Computing and Imaging Institute,
//  University of Utah.
//
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//    File   : NetworkIO.cc
//    Author : Martin Cole
//    Date   : Mon Feb  6 14:32:15 2006


// TODO: change string concatenation to string streams

// Disable concepts compliance detection for problem compiler.
// See boost/range/concepts.hpp for other problem compilers.
#if defined __clang__
#if __clang_major__ == 4 && __clang_minor__ == 2

#define BOOST_RANGE_ENABLE_CONCEPT_ASSERT 0

#endif
#endif

#include <Dataflow/Serialization/Network/Importer/NetworkIO.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/ModuleFactory.h>
#include <Core/XMLUtil/XMLUtil.h>
//#include <Dataflow/Network/NetworkEditor.h>
#include <Dataflow/Network/Module.h>
//#include <Core/Util/Environment.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <sci_debug.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>

#include <libxml/catalog.h>
#include <Core/Logging/Log.h>
#include <iostream>
#include <sstream>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;

LegacyNetworkIO::LegacyNetworkIO(const std::string& dtdpath, const ModuleFactory& modFactory,
  std::ostringstream& simpleLog) :
net_file_("new.srn"),
done_writing_(false),
doc_(0),
out_fname_(""),
sn_count_(0),
sn_ctx_(0),
dtdPath_(dtdpath),
modFactory_(modFactory),
simpleLog_(simpleLog)
{
  netid_to_modid_.push(id_map_t());
  netid_to_conid_.push(id_map_t());
}

std::string
LegacyNetworkIO::get_mod_id(const std::string& id)
{
  id_map_t &mmap = netid_to_modid_.top();
  const std::string sn("Subnet");
  return (id == sn) ? sn : mmap[id];
}

std::string
LegacyNetworkIO::gui_push_subnet_ctx()
{
  std::string cmmd = "set sn_ctx $Subnet(Loading)";
  //std::string s = TCLInterface::eval(cmmd);
  return cmmd;
}


void
LegacyNetworkIO::gui_pop_subnet_ctx(const std::string& ctx)
{
  std::string cmmd = "set Subnet(Loading) " + ctx;
  simpleLog_ << "TCLInterface::eval " << cmmd << std::endl;
#if 0
  --sn_ctx_;
  netid_to_modid_.pop();
  netid_to_conid_.pop();
  #endif
}

void
LegacyNetworkIO::gui_add_subnet_at_position(const std::string &mod_id,
const std::string &module,
const std::string& x,
const std::string &y)
{
  #if 0
  ++sn_count_;
  // map the subnet to a local var before we push maps.
  id_map_t &mmap = netid_to_modid_.top();

  std::ostringstream snic;
  snic << "SubnetIcon" << sn_count_;
  mmap[mod_id] = snic.str();

  netid_to_modid_.push(id_map_t());
  netid_to_conid_.push(id_map_t());

  std::ostringstream cmmd;

  cmmd << "set Subnet(Loading) [makeSubnetEditorWindow "
    << sn_ctx_ << " " << x  << " " << y << "]";
  TCLInterface::eval(cmmd.str());
  ++sn_ctx_;

  std::ostringstream cmmd1;
  cmmd1 << "set Subnet(Subnet" << sn_count_ << "_Name) \"" << module << "\"";
  TCLInterface::eval(cmmd1.str());
  #endif
}

void
LegacyNetworkIO::gui_add_module_at_position(const std::string &mod_id,
const std::string &cpackage,
const std::string &ccategory,
const std::string &moduleNameOrig,
const std::string &cversion,
const std::string& x,
const std::string &y)
{
  if (!xmlData_)
    return;

  const std::string cmodule = checkForModuleRename(moduleNameOrig);

  std::vector<int> existingIdsWithThisModuleName;
  boost::copy(moduleIdMap_
    | boost::adaptors::map_values
    | boost::adaptors::filtered([&](const ModuleId& mid) { return mid.name_ == cmodule; })
    | boost::adaptors::transformed([&](const ModuleId& mid) { return mid.idNumber_; }),
    std::back_inserter(existingIdsWithThisModuleName));
  int nextId = 0;
  if (!existingIdsWithThisModuleName.empty())
    nextId = *std::max_element(existingIdsWithThisModuleName.begin(), existingIdsWithThisModuleName.end()) + 1;
  moduleIdMap_[mod_id] = ModuleId(cmodule, nextId);

  ModuleLookupInfoXML& mod = xmlData_->network.modules[moduleIdMap_[mod_id]].module;
  mod.package_name_ = cpackage;
  mod.category_name_ = ccategory;
  mod.module_name_= cmodule;

  const double guiScalingFactor = 1.5;
  xmlData_->modulePositions.modulePositions[moduleIdMap_[mod_id]] =
    { boost::lexical_cast<double>(x) * guiScalingFactor,
      boost::lexical_cast<double>(y) * guiScalingFactor};
}

//TODO: move into a config file
const std::map<std::string, std::string> LegacyNetworkIO::moduleRenameMap_ =
  {
    {"MapFieldDataOntoElems", "MapFieldDataOntoElements"},
    {"GetColumnOrRowFromMatrix", "GetMatrixSlice"},
    {"CreateStandardColorMaps", "CreateStandardColorMap"}
  };

std::string LegacyNetworkIO::checkForModuleRename(const std::string& originalName)
{
  auto rename = moduleRenameMap_.find(originalName);
  if (rename != moduleRenameMap_.end())
    return rename->second;
  return originalName;
}

void
LegacyNetworkIO::createConnectionNew(const std::string& from, const std::string& to,
  const std::string& from_port, const std::string& to_port, const std::string& con_id)
{
  auto fromId = moduleIdMap_[from];
  auto toId = moduleIdMap_[to];

  if (!xmlData_)
    return;

  try
  {
    auto fromDesc = modFactory_.lookupDescription(ModuleLookupInfo(fromId.name_, "TODO", "SCIRun"));
    auto toDesc = modFactory_.lookupDescription(ModuleLookupInfo(toId.name_, "TODO", "SCIRun"));

    auto& connections = xmlData_->network.connections;
    OutgoingConnectionDescription out;
    out.moduleId_ = fromId;

    auto fromIndex = boost::lexical_cast<int>(from_port);
    if (fromIndex >= fromDesc.output_ports_.size() && fromDesc.output_ports_.back().isDynamic)
    {
      out.portId_ = fromDesc.output_ports_.back().id;
      out.portId_.id = fromIndex;
    }
    else
      out.portId_ = fromDesc.output_ports_.at(fromIndex).id;
    IncomingConnectionDescription in;
    in.moduleId_ = toId;

    auto toIndex = boost::lexical_cast<int>(to_port);

    if (toIndex >= toDesc.input_ports_.size() && toDesc.input_ports_.back().isDynamic)
    {
      in.portId_ = toDesc.input_ports_.back().id;
      in.portId_.id = toIndex;
    }
    else
      in.portId_ = toDesc.input_ports_.at(toIndex).id;

    ConnectionDescriptionXML conn;
    conn.out_ = out;
    conn.in_ = in;
    connections.push_back(conn);
    connectionIdMap_[con_id] = ConnectionId::create(conn).id_;
  }
  catch (Core::InvalidArgumentException& e)
  {
    simpleLog_ << "File conversion error: connection not created between modules " << fromId << " and " << toId << std::endl;
  }
}

void
LegacyNetworkIO::gui_add_connection(const std::string &con_id,
const std::string &from_id,
const std::string &from_port,
const std::string &to_id,
const std::string &to_port0)
{
  std::string to_port = to_port0;
  std::string from = get_mod_id(from_id);
  std::string to = get_mod_id(to_id);
  if (from.find("Subnet") == std::string::npos &&
    to.find("Subnet") == std::string::npos)
  {
    createConnectionNew(from_id, to_id, from_port, to_port0, con_id);
  }
}

#if 0
void
NetworkIO::gui_set_connection_disabled(const std::string &con_id)
{
  id_map_t &cmap = netid_to_conid_.top();
  std::string con = cmap[con_id];
  std::string cmmd = "set Disabled(" + con + ") {1}";
  TCLInterface::eval(cmmd);
}


void
NetworkIO::gui_set_module_port_caching(const std::string &mid, const std::string &pid,
const std::string &val)
{
  std::string modid = get_mod_id(mid);
  std::string cmmd = "setPortCaching " + modid + " " + pid + " " + val;
  TCLInterface::eval(cmmd);
}


void
NetworkIO::gui_call_module_callback(const std::string &id, const std::string &call)
{
  std::string modid = get_mod_id(id);
  std::string cmmd = modid + " " + call;
  TCLInterface::eval(cmmd);
}
#endif

void
LegacyNetworkIO::gui_set_modgui_variable(const std::string &mod_id, const std::string &var,
const std::string &val)
{
  std::string cmmd;
  std::string mod = get_mod_id(mod_id);

  // Some variables in tcl are quoted strings with spaces, so in that
  // case insert the module identifying std::string after the first quote.
  size_t pos = var.find_first_of("\"");
  if (pos == std::string::npos)
  {
    cmmd = "set " + mod + "-" + var +  " " + val;
  }
  else
  {
    std::string v = var;
    v.insert(++pos, mod + "-");
    cmmd = "set " + v +  " " + val;
  }

  if (!xmlData_)
    return;

  std::string moduleName = xmlData_->network.modules[moduleIdMap_[mod_id]].module.module_name_;
  auto& stateXML = xmlData_->network.modules[moduleIdMap_[mod_id]].state;

  auto moduleNameMapIter = nameLookup_.find(moduleName);
  if (moduleNameMapIter == nameLookup_.end())
  {
    simpleLog_ << "STATE CONVERSION TO IMPLEMENT: module " << moduleName << ", mod_id: " << moduleIdMap_[mod_id] << " var: " << var << " val: " << val << std::endl;
    return;
  }
  auto valueConverterForModuleIter = valueConverter_.find(moduleName);
  if (valueConverterForModuleIter == valueConverter_.end())
  {
    simpleLog_ << "STATE CONVERSION TO IMPLEMENT: module " << moduleName << ", mod_id: " << moduleIdMap_[mod_id] << " var: " << var << " val: " << val << std::endl;
    return;
  }
  std::string stripBraces(val.begin() + 1, val.end() - 1);
  stateXML.setValue(moduleNameMapIter->second[var], valueConverterForModuleIter->second[var](stripBraces));
}

namespace
{
  ValueConverter toInt = [](const std::string& s) { return boost::lexical_cast<int>(s); };
  ValueConverter toDouble = [](const std::string& s) { return boost::lexical_cast<double>(s); };
  ValueConverter toPercent = [](const std::string& s) { return boost::lexical_cast<double>(s) / 100.0; };

  //TODO: mapping macro or find a boost lib to do pattern matching with funcs easily
  ValueConverter data_at = [](const std::string& s)
  {
    if (s == "Nodes") return 0;
    if (s == "Cells") return 1;
    return 2;
  };
  ValueConverter element_size = [](const std::string& s)
  {
    if (s == "Mesh") return 0;
    //if (s == "Element")
    return 1;
  };

  ValueConverter throwAway = [](const std::string& s) { return 0; };
}

NameLookup LegacyNetworkIO::nameLookup_ =
{
  {
    "CreateLatVol",
    {
      { "sizex", Name("XSize") },
      { "sizey", Name("YSize") },
      { "sizez", Name("ZSize") },
      { "padpercent", Name("PadPercent") },
      { "data-at", Name("DataAtLocation") },
      { "element-size", Name("ElementSizeNormalized") }
    }
  }
};

ValueConverterMap LegacyNetworkIO::valueConverter_ =
{
  {
    "CreateLatVol",
    {
      { "sizex", toInt },
      { "sizey", toInt },
      { "sizez", toInt },
      { "padpercent", toPercent },
      { "data-at", data_at },
      { "element-size", element_size },
      { "ui_geometry", throwAway }
    }
  }
};

#if 0
void
NetworkIO::gui_call_mod_post_read(const std::string &mod_id)
{
  std::string modid = get_mod_id(mod_id);
  ModuleHandle mod = net_->get_module_by_id(modid);
  if (mod.get_rep()) { mod->post_read(); }
}

#endif

void
LegacyNetworkIO::gui_set_module_note(const std::string &mod_id, const std::string &pos,
const std::string &col, const std::string &note)
{
  if (!xmlData_)
    return;

  std::string stripBraces(note.begin() + 1, note.end() - 1);
  int position = getNotePosition(pos);
  NoteXML noteXml(stripBraces, position, stripBraces);
  xmlData_->moduleNotes.notes[moduleIdMap_[mod_id]] = noteXml;
}

int LegacyNetworkIO::getNotePosition(const std::string& position) const
{
  // expected format is {p}
  if (position.length() < 3)
    return 0;
  char oldPos = position[1];
  /* copied from gui header; should move type down here
  enum NotePosition
  {
    Default,  0
    None,     1
    Tooltip,  2
    Top,      3
    Left,     4
    Right,    5
    Bottom    6
  };
  */
  switch (oldPos)
  {
  case 'n':
    return 3;
  case 's':
    return 6;
  case 'e':
    return 5;
  case 'w':
    return 4;
  case 't':
    return 2;
  default:
    return 0;
  }
}

void
LegacyNetworkIO::gui_set_connection_note(const std::string &con_id, const std::string &pos,
const std::string &col, const std::string &note)
{
  std::string stripBraces(note.begin() + 1, note.end() - 1);
  NoteXML noteXml(stripBraces, 0, stripBraces);
  //std::cout << "TODO NOT WORKING YET:::SETTING CONNECTION note: " << connectionIdMap_[con_id] << " note is " << stripBraces << std::endl;
  xmlData_->connectionNotes.notes[connectionIdMap_[con_id]] = noteXml;
}

void
LegacyNetworkIO::gui_set_variable(const std::string &var, const std::string &val)
{
  std::string cmd = "set " + var +  " " + val;
  simpleLog_ << "TCLInterface::eval " << cmd << std::endl;
}

void
LegacyNetworkIO::gui_open_module_gui(const std::string &mod_id)
{
  std::string mod = get_mod_id(mod_id);
  std::string cmmd = mod + " initialize_ui";
  simpleLog_ << "TCLInterface::eval " << cmmd << std::endl;
}

void
LegacyNetworkIO::process_environment(const xmlNodePtr enode)
{
  xmlNodePtr node = enode->children;
  for (; node != 0; node = node->next)
  {
    if (std::string(to_char_ptr(node->name)) == std::string("var"))
    {
      xmlAttrPtr name_att = get_attribute_by_name(node, "name");
      xmlAttrPtr val_att = get_attribute_by_name(node, "val");
      env_subs_[std::string(to_char_ptr(name_att->children->content))] =
        std::string(to_char_ptr(val_att->children->content));
    }
  }
}

void
LegacyNetworkIO::process_modules_pass1(const xmlNodePtr enode)
{
  xmlNodePtr node = enode->children;
  for (; node != 0; node = node->next)
  {
    if (std::string(to_char_ptr(node->name)) == std::string("module") ||
      std::string(to_char_ptr(node->name)) == std::string("subnet"))
    {
      bool do_subnet = std::string(to_char_ptr(node->name)) == std::string("subnet");
      xmlNodePtr network_node = 0;

      std::string x,y;
      xmlAttrPtr id_att = get_attribute_by_name(node, "id");
      xmlAttrPtr package_att = get_attribute_by_name(node, "package");
      xmlAttrPtr category_att = get_attribute_by_name(node, "category");
      xmlAttrPtr name_att = get_attribute_by_name(node, "name");
      xmlAttrPtr version_att = get_attribute_by_name(node, "version");

      std::string mname = std::string(to_char_ptr(name_att->children->content));
      std::string mid = std::string(to_char_ptr(id_att->children->content));
      xmlNodePtr pnode = node->children;
      for (; pnode != 0; pnode = pnode->next)
      {
        if (std::string(to_char_ptr(pnode->name)) == std::string("position"))
        {
          xmlAttrPtr x_att = get_attribute_by_name(pnode, "x");
          xmlAttrPtr y_att = get_attribute_by_name(pnode, "y");
          x = std::string(to_char_ptr(x_att->children->content));
          y = std::string(to_char_ptr(y_att->children->content));

          if (do_subnet)
          {
            std::string old_ctx = gui_push_subnet_ctx();
            gui_add_subnet_at_position(mid, mname, x, y);

            ASSERT(network_node != 0);
            process_network_node(network_node);
            gui_pop_subnet_ctx(old_ctx);
          }
          else
          {
            std::string package = std::string(to_char_ptr(package_att->children->content));
            std::string category = std::string(to_char_ptr(category_att->children->content));
            std::string version = "1.0";
            if (version_att != 0) version = std::string(to_char_ptr(version_att->children->content));

            gui_add_module_at_position(mid, package, category,
              mname, version, x, y);
          }
        }
        else if (std::string(to_char_ptr(pnode->name)) == std::string("network"))
        {
          network_node = pnode;
        }
        else if (std::string(to_char_ptr(pnode->name)) == std::string("note"))
        {
          xmlAttrPtr pos_att = get_attribute_by_name(pnode, "position");
          xmlAttrPtr col_att = get_attribute_by_name(pnode, "color");
          std::string pos, col, note;
          if (pos_att)
            pos = std::string(to_char_ptr(pos_att->children->content));
          if (col_att)
            col = std::string(to_char_ptr(col_att->children->content));

          note = std::string(to_char_ptr(pnode->children->content));
          gui_set_module_note(mid, pos, col, note);
        }
        else if (std::string(to_char_ptr(pnode->name)) == std::string("port_caching"))
        {
          xmlNodePtr pc_node = pnode->children;
          for (; pc_node != 0; pc_node = pc_node->next)
          {
            if (std::string(to_char_ptr(pc_node->name)) == std::string("port"))
            {
              #if 0
              xmlAttrPtr pid_att = get_attribute_by_name(pc_node, "id");
              xmlAttrPtr val_att = get_attribute_by_name(pc_node, "val");
              gui_set_module_port_caching(mid,
                std::string(to_char_ptr(pid_att->children->content)),
                std::string(to_char_ptr(val_att->children->content)));
              #endif
            }
          }
        }
      }
    }
  }
}

void
LegacyNetworkIO::process_modules_pass2(const xmlNodePtr enode)
{
  xmlNodePtr node = enode->children;
  for (; node != 0; node = node->next)
  {
    if (std::string(to_char_ptr(node->name)) == std::string("module"))
    {
      std::string x,y;
      xmlAttrPtr id_att = get_attribute_by_name(node, "id");
      xmlAttrPtr visible_att = get_attribute_by_name(node, "gui_visible");

      xmlNodePtr pnode;

      pnode = node->children;
      for (; pnode != 0; pnode = pnode->next)
      {
        if (std::string(to_char_ptr(pnode->name)) == std::string("var"))
        {
          xmlAttrPtr name_att = get_attribute_by_name(pnode, "name");
          xmlAttrPtr val_att = get_attribute_by_name(pnode, "val");
          xmlAttrPtr filename_att = get_attribute_by_name(pnode,"filename");
          xmlAttrPtr substitute_att= get_attribute_by_name(pnode,"substitute");

          std::string val = std::string(to_char_ptr(val_att->children->content));

          std::string filename = "no";
          if (filename_att != 0) filename =
            std::string(to_char_ptr(filename_att->children->content));
          std::string substitute = "yes";
          if (substitute_att != 0) substitute =
            std::string(to_char_ptr(substitute_att->children->content));

          if (filename == "yes")
          {
            if (substitute == "yes") val = process_substitute(val);
            val = process_filename(val);
            if (val.size() > 0 && val[0] == '{')
              val = std::string("\"")+val.substr(1,val.size()-2)+std::string("\"");
          }
          else
          {
            if (substitute == "yes") val = process_substitute(val);
          }

          gui_set_modgui_variable(
            std::string(to_char_ptr(id_att->children->content)),
            std::string(to_char_ptr(name_att->children->content)),
            val);
        }
      }

      bool has_gui_callback = false;
      pnode = node->children;
      for (; pnode != 0; pnode = pnode->next)
      {
        if (std::string(to_char_ptr(pnode->name)) == std::string("gui_callback"))
        {
          has_gui_callback = true;
          xmlNodePtr gc_node = pnode->children;
          for (; gc_node != 0; gc_node = gc_node->next)
          {
            if (std::string(to_char_ptr(gc_node->name)) == std::string("callback"))
            {
              std::string call = std::string(to_char_ptr(gc_node->children->content));
              #if 0
              gui_call_module_callback(
                std::string(to_char_ptr(id_att->children->content)),
                call);
              #endif

            }
          }
        }
      }

      if (has_gui_callback)
      {
        pnode = node->children;
        for (; pnode != 0; pnode = pnode->next)
        {
          if (std::string(to_char_ptr(pnode->name)) == std::string("var"))
          {
            xmlAttrPtr name_att = get_attribute_by_name(pnode, "name");
            xmlAttrPtr val_att = get_attribute_by_name(pnode, "val");
            xmlAttrPtr filename_att = get_attribute_by_name(pnode,"filename");
            xmlAttrPtr substitute_att= get_attribute_by_name(pnode,"substitute");

            std::string val = std::string(to_char_ptr(val_att->children->content));

            std::string filename = "no";
            if (filename_att != 0) filename =
              std::string(to_char_ptr(filename_att->children->content));
            std::string substitute = "yes";
            if (substitute_att != 0) substitute =
              std::string(to_char_ptr(substitute_att->children->content));

            if (filename == "yes")
            {
              if (substitute == "yes") val = process_substitute(val);
              val = process_filename(val);
              if (val.size() > 0 && val[0] == '{')
                val = std::string("\"")+val.substr(1,val.size()-2)+std::string("\"");
            }
            else
            {
              if (substitute == "yes") val = process_substitute(val);
            }

            gui_set_modgui_variable(
              std::string(to_char_ptr(id_att->children->content)),
              std::string(to_char_ptr(name_att->children->content)),
              val);
          }
        }
      }

      #if 0
      gui_call_mod_post_read(std::string(to_char_ptr(id_att->children->content)));
      #endif

      if (visible_att &&
        std::string(to_char_ptr(visible_att->children->content)) == "yes")
      {
        gui_open_module_gui(std::string(to_char_ptr(id_att->children->content)));
      }
    }
  }
}

void
LegacyNetworkIO::process_connections(const xmlNodePtr enode)
{
  xmlNodePtr node = enode->children;
  for (; node != 0; node = node->next) {
    if (std::string(to_char_ptr(node->name)) == std::string("connection")) {
      xmlAttrPtr id_att = get_attribute_by_name(node, "id");
      xmlAttrPtr from_att = get_attribute_by_name(node, "from");
      xmlAttrPtr fromport_att = get_attribute_by_name(node, "fromport");
      xmlAttrPtr to_att = get_attribute_by_name(node, "to");
      xmlAttrPtr toport_att = get_attribute_by_name(node, "toport");
      xmlAttrPtr dis_att = get_attribute_by_name(node, "disabled");

      std::string id = std::string(to_char_ptr(id_att->children->content));

      gui_add_connection(id,
        std::string(to_char_ptr(from_att->children->content)),
        std::string(to_char_ptr(fromport_att->children->content)),
        std::string(to_char_ptr(to_att->children->content)),
        std::string(to_char_ptr(toport_att->children->content)));

      if (dis_att &&
        std::string(to_char_ptr(dis_att->children->content)) == "yes")
      {
        #if 0
        gui_set_connection_disabled(id);
        #endif
      }


      xmlNodePtr cnode = node->children;
      for (; cnode != 0; cnode = cnode->next)
      {
        if (std::string(to_char_ptr(cnode->name)) == std::string("route"))
        {
          #if 0
          gui_set_connection_route(id,
            std::string(to_char_ptr(cnode->children->content)));
          #endif
        }
        else if (std::string(to_char_ptr(cnode->name)) == std::string("note"))
        {
          xmlAttrPtr pos_att = get_attribute_by_name(cnode, "position");
          xmlAttrPtr col_att = get_attribute_by_name(cnode, "color");
          std::string pos, col, note;
          if (pos_att)
            pos = std::string(to_char_ptr(pos_att->children->content));
          if (col_att)
            col = std::string(to_char_ptr(col_att->children->content));

          note = std::string(to_char_ptr(cnode->children->content));
          gui_set_connection_note(id, pos, col, note);
        }
      }
    }
  }
}

std::string
LegacyNetworkIO::process_filename(const std::string &orig)
{
  // This function reinterprets a filename

  // Copy the string and remove TCL brackets
  std::string filename = orig.substr(1,orig.size()-2);


  // Remove blanks and tabs from the input
  // (Some could have editted the XML file manually and may have left spaces)

  if (filename.size() > 0)
  {
    if (filename[0] == '{')
    {
      filename = filename.substr(1,filename.size()-2);
    }
  }

  while (filename.size() > 0 &&
    ((filename[0] == ' ')||(filename[0] == '\t'))) {
    filename = filename.substr(1);
  }

  while (filename.size() > 0 &&
    ((filename[filename.size()-1] == ' ')||
    (filename[filename.size()-1] == '\t'))) {
    filename = filename.substr(1,filename.size()-1);
  }

  // Check whether filename is absolute:

  if ( filename.size() > 0 && filename[0] == '/') return (std::string("{")+filename+std::string("}")); // Unix absolute path
  if ( filename.size() > 1 && filename[1] == ':') return (std::string("{")+filename+std::string("}")); // Windows absolute path

  // If not substitute:

  // Create a dynamic substitute called NETWORKDIR for relative path names
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  FullFileName fnet_file(net_file_);
  std::string net_file = fnet_file.get_abs_filename();
  std::string net_path = fnet_file.get_abs_path();
  env_subs_[std::string("scisub_networkdir")] = std::string("SCIRUN_NETWORKDIR");
  sci_putenv("SCIRUN_NETWORKDIR",net_path);

  auto iter = env_subs_.begin();
  while (iter != env_subs_.end())
  {
    const auto& kv = *iter++;
    const std::string &key = kv.first;

    id_map_t::size_type idx = filename.find(key);

    if (idx != std::string::npos)
    {
      const std::string &env_var = kv.second;
      const char* env = sci_getenv(env_var);
      std::string subst = (env != 0)?env:"";

      if (env_var == "SCIRUN_DATASET" && subst.empty())
      {
        subst = "sphere";
      }
      while (idx != std::string::npos)
      {
        filename = filename.replace(idx, key.size(), subst);
        idx = filename.find(key);
      }
    }
  }
  #endif

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  FullFileName ffn(filename);
  filename = ffn.get_abs_filename();
#endif

  return "{" + filename + "}";
}

std::string
LegacyNetworkIO::process_substitute(const std::string &orig)
{
  //std::cout << "TO BE IMPLEMENTED: process_substitute " << orig << std::endl;
  return orig;
  #if SCIRUN4_CODE_TO_BE_ENABLED_LATER
  std::string src = orig;
  id_map_t::const_iterator iter = env_subs_.begin();
  while (iter != env_subs_.end())
  {
    const std::pair<const std::string, std::string> &kv = *iter++;
    const std::string &key = kv.first;

    id_map_t::size_type idx = src.find(key);

    if (idx != std::string::npos) {
      const std::string &env_var = kv.second;
      const char* env = sci_getenv(env_var);
      std::string subst = (env != 0)?env:"";

      if (env_var == std::string("SCIRUN_DATASET") && subst.size() == 0)
      {
        subst = std::string("sphere");
      }
      while (idx != std::string::npos) {
        src = src.replace(idx, key.size(), subst);
        idx = src.find(key);
      }
    }
  }

  return (src);
  #endif
}

NetworkFileHandle
LegacyNetworkIO::load_net(const std::string &net)
{
//  FullFileName netfile(net);
  net_file_ = net;
  //sci_putenv("SCIRUN_NETFILE", net);
  if (!load_network())
    return nullptr;

  return xmlData_;
}

void
LegacyNetworkIO::process_network_node(xmlNode* network_node)
{
  // have to multi pass this document to workaround tcl timing issues.
  // PASS 1 - create the modules and connections
  xmlNode* node = network_node;
  for (; node != 0; node = node->next) {
    // skip all but the component node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("network"))
    {
      //! set attributes
      //xmlAttrPtr version_att = get_attribute_by_name(node, "version");
      xmlAttrPtr name_att = get_attribute_by_name(node, "name");
      gui_set_variable(std::string("name"),
        std::string(to_char_ptr(name_att->children->content)));
      xmlAttrPtr bbox_att = get_attribute_by_name(node, "bbox");
      gui_set_variable(std::string("bbox"),
        std::string(to_char_ptr(bbox_att->children->content)));
      xmlAttrPtr cd_att = get_attribute_by_name(node, "creationDate");
      gui_set_variable(std::string("creationDate"),
        std::string(to_char_ptr(cd_att->children->content)));
      xmlAttrPtr ct_att = get_attribute_by_name(node, "creationTime");
      gui_set_variable(std::string("creationTime"),
        std::string(to_char_ptr(ct_att->children->content)));
      xmlAttrPtr geom_att = get_attribute_by_name(node, "geometry");
      gui_set_variable(std::string("geometry"),
        std::string(to_char_ptr(geom_att->children->content)));


      xmlAttrPtr netversion_att = get_attribute_by_name(node, "netversion");
      if (netversion_att)
      {
        gui_set_variable(std::string("netversion"),
          std::string(to_char_ptr(netversion_att->children->content)));
      }
      else
      {
        gui_set_variable(std::string("netversion"), "0");
      }

      xmlNode* enode = node->children;
      for (; enode != 0; enode = enode->next) {

        if (enode->type == XML_ELEMENT_NODE &&
          std::string(to_char_ptr(enode->name)) == std::string("environment"))
        {
          process_environment(enode);
        } else if (enode->type == XML_ELEMENT_NODE &&
          std::string(to_char_ptr(enode->name)) == std::string("modules"))
        {
          process_modules_pass1(enode);
        } else if (enode->type == XML_ELEMENT_NODE &&
          std::string(to_char_ptr(enode->name)) == std::string("connections"))
        {
          process_connections(enode);
        } else if (enode->type == XML_ELEMENT_NODE &&
          std::string(to_char_ptr(enode->name)) == std::string("note"))
        {
          gui_set_variable(std::string("notes"),
            std::string(to_char_ptr(enode->children->content)));
        }
      }
    }
  }

  // PASS 2 -- call the callbacks and set the variables
  node = network_node;
  for (; node != 0; node = node->next) {
    // skip all but the component node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("network"))
    {
      xmlNode* enode = node->children;
      for (; enode != 0; enode = enode->next) {

        if (enode->type == XML_ELEMENT_NODE &&
          std::string(to_char_ptr(enode->name)) == std::string("modules"))
        {
          process_modules_pass2(enode);
        }
      }
    }
  }
}

bool
LegacyNetworkIO::load_network()
{
  /*
   * this initializes the library and checks potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */

  // Reset some variables
  // FullFileName netfile(net_file_);
  // net_file_ = netfile.get_abs_filename();
  // sci_putenv("SCIRUN_NETFILE", net_file_);
  // sn_count_ = 0;
  // sn_ctx_ = 0;

  LIBXML_TEST_VERSION;

  xmlParserCtxtPtr ctxt; /* the parser context */
  xmlDocPtr doc; /* the resulting document tree */

  auto dtdfile = boost::filesystem::path(dtdPath_) / "network.dtd";
  xmlInitializeCatalog();
  xmlCatalogAdd(XMLUtil::char_to_xmlChar("public"),
    XMLUtil::char_to_xmlChar("-//SCIRun/Network DTD"),
    XMLUtil::char_to_xmlChar(dtdfile.string().c_str()));

  /* create a parser context */
  ctxt = xmlNewParserCtxt();
  if (!ctxt) {
    simpleLog_ << "LegacyNetworkIO.cc: Failed to allocate parser context"
      << std::endl;
    return false;
  }

  // parse the file, activating the DTD validation option (disabled - see below)
  // XML_PARSE_DTDVALID

  int flags = XML_PARSE_DTDATTR;
  // TODO: replacing the DTD URI in SCIRun network files is currently broken.
  // See bug 157 in the GForge CIBC project bug tracker.
  // Until this can be fixed, suppress the warning unless debugging.
#ifdef DEBUG
  flags |= XML_PARSE_PEDANTIC;
#else
  flags |= XML_PARSE_NOWARNING;
#endif
  doc = xmlCtxtReadFile(ctxt, net_file_.c_str(), 0, flags);
  /* check if parsing suceeded */
  if (doc == 0) {
    simpleLog_ << "LegacyNetworkIO.cc: Failed to parse " << net_file_
      << std::endl;
    return false;
  } else {
    /* check if validation suceeded */
    if (ctxt->valid == 0) {
      simpleLog_ << "LegacyNetworkIO.cc: Failed to validate " << net_file_
        << std::endl;
      return false;
    }
  }

  // TCLInterface::eval("netedit dontschedule");

  xmlData_.reset(new NetworkFile);

  // parse the doc at network node.
  process_network_node(doc->children);

  xmlFreeDoc(doc);
  /* free up the parser context */
  xmlFreeParserCtxt(ctxt);
#ifndef _WIN32
  // there is a problem on windows when using Uintah
  // which is either caused or exploited by this
  xmlCleanupParser();
#endif

  // TCLInterface::eval("setGlobal NetworkChanged 0");
  // TCLInterface::eval("set netedit_savefile {" + net_file_ + "}");
  // TCLInterface::eval("netedit scheduleok");
  // TCLInterface::eval("update_network_editor_title \"" + net_file_ + "\"");
  //

  return true;
}

void LegacyNetworkIO::listModuleIdMapping()
{
  std::cout << "Module id mapping: {\n";
  for (const auto& mm : moduleIdMap_)
  {
    std::cout << mm.first << " --> " << mm.second << std::endl;
  }
  std::cout << "}" << std::endl;
}

#if 0
// push a new network root node.
void
NetworkIO::push_subnet_scope(const std::string &id, const std::string &name)
{
  // this is a child node of the network.
  xmlNode* mod_node = 0;
  xmlNode* net_node = 0;
  xmlNode* node = subnets_.top();
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("network"))
    {
      net_node = node;
      xmlNode* mnode = node->children;
      for (; mnode != 0; mnode = mnode->next) {
        if (std::string(to_char_ptr(mnode->name)) == std::string("modules")) {
          mod_node = mnode;
          break;
        }
      }
    }
  }
  if (! mod_node) {
    if (! net_node) {
      simpleLog_ << "ERROR: could not find top level node." << std::endl;
      return;
    }
    mod_node = xmlNewChild(net_node, 0, BAD_CAST "modules", 0);
  }
  xmlNodePtr tmp = xmlNewChild(mod_node, 0, BAD_CAST "subnet", 0);
  xmlNewProp(tmp, BAD_CAST "id", BAD_CAST id.c_str());
  xmlNewProp(tmp, BAD_CAST "package", BAD_CAST "subnet");
  xmlNewProp(tmp, BAD_CAST "category", BAD_CAST "subnet");
  xmlNewProp(tmp, BAD_CAST "name", BAD_CAST name.c_str());
  xmlNewProp(tmp, BAD_CAST "version", BAD_CAST "1.0");

  xmlNodePtr sn_node = xmlNewChild(tmp, 0, BAD_CAST "network", 0);
  xmlNewProp(sn_node, BAD_CAST "version", BAD_CAST "contained");

  subnets_.push(sn_node);
}


void
NetworkIO::pop_subnet_scope()
{
  subnets_.pop();
}


void
NetworkIO::start_net_doc(const std::string &fname, const std::string &vers, const std::string &netversion)
{
  out_fname_ = fname;
  xmlNodePtr root_node = 0; /* node pointers */
  xmlDtdPtr dtd = 0;        /* DTD pointer */

  LIBXML_TEST_VERSION;

  /*
   * Creates a new document, a node and set it as a root node
   */
  doc_ = xmlNewDoc(BAD_CAST "1.0");
  root_node = xmlNewNode(0, BAD_CAST "network");
  subnets_.push(root_node);
  xmlDocSetRootElement(doc_, root_node);

  /*
   * Creates a DTD declaration.
   */
  std::string dtdstr = std::string("network.dtd");

  dtd = xmlCreateIntSubset(doc_, BAD_CAST "network",
    BAD_CAST "-//SCIRun/Network DTD",
    BAD_CAST dtdstr.c_str());

  /*
   * xmlNewChild() creates a new node, which is "attached" as child node
   * of root_node node.
   */
  xmlNewProp(root_node, BAD_CAST "version", BAD_CAST vers.c_str());
  xmlNewProp(root_node, BAD_CAST "netversion", BAD_CAST netversion.c_str());

  FullFileName netfile(fname);
  net_file_ = netfile.get_abs_filename();

  // Filter out backup names
  std::string filename;
  for (size_t k=0; k < fname.size(); k++) if (fname[k] != '#') filename += fname[k];
  sci_putenv("SCIRUN_NETFILE", filename);
}


void
NetworkIO::write_net_doc()
{
  // write the file
  xmlSaveFormatFileEnc(out_fname_.c_str(), doc_, "UTF-8", 1);

  // free the document
  xmlFreeDoc(doc_);
  doc_ = 0;
  out_fname_ = "";
  done_writing_ = true;
}


void
NetworkIO::add_net_var(const std::string &var, const std::string &val)
{
  // add these as attributes of the network node.
  xmlNode* node = subnets_.top();
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("network"))
    {
      break;
    }
  }
  if (! node) {
    simpleLog_ << "ERROR: could not find top level node." << std::endl;
    return;
  }
  xmlNewProp(node, BAD_CAST var.c_str(), BAD_CAST val.c_str());
}


void
NetworkIO::add_environment_sub(const std::string &var, const std::string &val)
{
  // this is a child node of the network.
  xmlNode* env_node = 0;
  xmlNode* net_node = 0;
  xmlNode* node = subnets_.top();
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("network"))
    {
      net_node = node;
      xmlNode* enode = node->children;
      for (; enode != 0; enode = enode->next) {
        if (std::string(to_char_ptr(enode->name)) == std::string("environment")) {
          env_node = enode;
          break;
        }
      }
    }
  }
  if (! env_node) {
    if (! net_node) {
      simpleLog_ << "ERROR: could not find top level node." << std::endl;
      return;
    }
    env_node = xmlNewChild(net_node, 0, BAD_CAST "environment", 0);
  }
  xmlNodePtr tmp = xmlNewChild(env_node, 0, BAD_CAST "var", 0);
  xmlNewProp(tmp, BAD_CAST "name", BAD_CAST var.c_str());
  xmlNewProp(tmp, BAD_CAST "val", BAD_CAST val.c_str());
}


void
NetworkIO::add_net_note(const std::string &val)
{
  // this is a child node of the network, must come after
  // environment node if it exists.
  xmlNode* node = subnets_.top();
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("network"))
    {
      break;
    }
  }
  if (! node) {
    simpleLog_ << "ERROR: could not find 'network' node." << std::endl;
    return;
  }

  xmlNewTextChild(node, 0, BAD_CAST "note", BAD_CAST val.c_str());
}


void
NetworkIO::add_module_node(const std::string &id, const std::string &pack,
const std::string &cat, const std::string &mod, const std::string& ver)
{
  // this is a child node of the network.
  xmlNode* mod_node = 0;
  xmlNode* net_node = 0;
  xmlNode* node = subnets_.top();
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("network"))
    {
      net_node = node;
      xmlNode* mnode = node->children;
      for (; mnode != 0; mnode = mnode->next) {
        if (std::string(to_char_ptr(mnode->name)) == std::string("modules")) {
          mod_node = mnode;
          break;
        }
      }
    }
  }
  if (! mod_node) {
    if (! net_node) {
      simpleLog_ << "ERROR: could not find top level node." << std::endl;
      return;
    }
    mod_node = xmlNewChild(net_node, 0, BAD_CAST "modules", 0);
  }
  xmlNodePtr tmp = xmlNewChild(mod_node, 0, BAD_CAST "module", 0);
  xmlNewProp(tmp, BAD_CAST "id", BAD_CAST id.c_str());
  xmlNewProp(tmp, BAD_CAST "package", BAD_CAST pack.c_str());
  xmlNewProp(tmp, BAD_CAST "category", BAD_CAST cat.c_str());
  xmlNewProp(tmp, BAD_CAST "name", BAD_CAST mod.c_str());
  xmlNewProp(tmp, BAD_CAST "version", BAD_CAST ver.c_str());
}


xmlNode*
NetworkIO::get_module_node(const std::string &id)
{
  xmlNode* mid_node = 0;
  xmlNode* node = subnets_.top();
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("network"))
    {
      xmlNode* msnode = node->children;
      for (; msnode != 0; msnode = msnode->next) {

        if (std::string(to_char_ptr(msnode->name)) == std::string("modules")) {
          xmlNode* mnode = msnode->children;
          for (; mnode != 0; mnode = mnode->next) {

            if (std::string(to_char_ptr(mnode->name)) == std::string("module") ||
              std::string(to_char_ptr(mnode->name)) == std::string("subnet"))
            {
              xmlAttrPtr name_att = get_attribute_by_name(mnode, "id");
              std::string mid = std::string(to_char_ptr(name_att->children->content));
              if (mid == id)
              {
                mid_node = mnode;
                break;
              }
            }
          }
        }
      }
    }
  }
  return mid_node;
}


void
NetworkIO::add_module_variable(const std::string &id, const std::string &var,
const std::string &val, bool filename, bool substitute, bool userelfilenames)
{
  xmlNode* node = get_module_node(id);

  if (! node)
  {
    simpleLog_ << "ERROR: could not find module node with id (module variable): " << id << std::endl;
    return;
  }
  xmlNodePtr tmp = xmlNewChild(node, 0, BAD_CAST "var", 0);
  xmlNewProp(tmp, BAD_CAST "name", BAD_CAST var.c_str());

  std::string nval = val;
  if (filename && userelfilenames)
  {
    if ((nval.size() >0) &&  (nval[0] == '{'))
    {
      FullFileName relname(nval.substr(1, nval.size() - 2));
      nval = relname.get_rel_filename(out_fname_);
      nval = "{"+nval+"}";
    }
    else
    {
      FullFileName relname(nval);
      nval = relname.get_rel_filename(out_fname_);
    }
  }

  xmlNewProp(tmp, BAD_CAST "val", BAD_CAST nval.c_str());
  if (filename) xmlNewProp(tmp, BAD_CAST "filename", BAD_CAST "yes");

  if (substitute) xmlNewProp(tmp, BAD_CAST "substitute", BAD_CAST "yes");
  else xmlNewProp(tmp, BAD_CAST "substitute", BAD_CAST "no");
}


void
NetworkIO::set_module_gui_visible(const std::string &id)
{
  xmlNode* node = get_module_node(id);

  if (! node) {
    simpleLog_ << "ERROR: could not find module node with id (module gui visible): " << id << std::endl;
    return;
  }
  xmlNewProp(node, BAD_CAST "gui_visible", BAD_CAST "yes");
}


void
NetworkIO::add_module_gui_callback(const std::string &id, const std::string &call)
{
  xmlNode* gc_node = 0;
  xmlNode* mod_node = get_module_node(id);
  if (! mod_node) {
    simpleLog_ << "ERROR: could not find node for module id: " << id << std::endl;
    return;
  }
  xmlNode *node = mod_node->children;
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("gui_callback")) {
      gc_node = node;
      break;
    }
  }

  if (! gc_node) {
    gc_node = xmlNewChild(mod_node, 0, BAD_CAST "gui_callback", 0);
  }
  xmlNewTextChild(gc_node, 0, BAD_CAST "callback", BAD_CAST call.c_str());
}


void
NetworkIO::add_module_position(const std::string &id, const std::string &x,
const std::string &y)
{
  xmlNode* mid_node = get_module_node(id);

  if (! mid_node) {
    simpleLog_ << "ERROR: could not find module node with id (add_module_position): " << id << std::endl;
    return;
  }
  xmlNodePtr tmp = xmlNewChild(mid_node, 0, BAD_CAST "position", 0);
  xmlNewProp(tmp, BAD_CAST "x", BAD_CAST x.c_str());
  xmlNewProp(tmp, BAD_CAST "y", BAD_CAST y.c_str());

}

void
NetworkIO::add_module_note_position(const std::string &id, const std::string &pos)
{
  bool found = false;
  xmlNode* node = get_module_node(id);
  if (! node) {
    simpleLog_ << "ERROR: could not find node for module id: " << id << std::endl;
    return;
  }
  node = node->children;
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("note")) {
      found = true;
      break;
    }
  }

  if (! found) {
    simpleLog_ << "ERROR: could not find note node for module id: " << id << std::endl;
    return;
  }
  xmlNewProp(node, BAD_CAST "position", BAD_CAST pos.c_str());
}


void
NetworkIO::add_module_note_color(const std::string &id, const std::string &col)
{
  bool found = false;
  xmlNode* node = get_module_node(id);
  if (! node) {
    simpleLog_ << "ERROR: could not find node for module id: " << id << std::endl;
    return;
  }
  node = node->children;
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("note")) {
      found = true;
      break;
    }
  }

  if (! found) {
    simpleLog_ << "ERROR: could not find note node for module id: " << id << std::endl;
    return;
  }
  xmlNewProp(node, BAD_CAST "color", BAD_CAST col.c_str());
}


void
NetworkIO::add_connection_node(const std::string &id, const std::string &fmod,
const std::string &fport, const std::string &tmod,
const std::string &tport)
{
  // this is a child node of the network.
  xmlNode* con_node= 0;
  xmlNode* net_node= 0;
  xmlNode* node = subnets_.top();
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("network"))
    {
      net_node = node;
      xmlNode* cnode = node->children;
      for (; cnode != 0; cnode = cnode->next) {
        if (std::string(to_char_ptr(cnode->name)) == std::string("connections")) {
          con_node = cnode;
          break;
        }
      }
    }
  }
  if (! con_node) {
    if (! net_node) {
      simpleLog_ << "ERROR: could not find top level node." << std::endl;
      return;
    }
    con_node = xmlNewChild(net_node, 0, BAD_CAST "connections", 0);
  }
  xmlNodePtr tmp = xmlNewChild(con_node, 0, BAD_CAST "connection", 0);
  xmlNewProp(tmp, BAD_CAST "id", BAD_CAST id.c_str());
  xmlNewProp(tmp, BAD_CAST "from", BAD_CAST fmod.c_str());
  xmlNewProp(tmp, BAD_CAST "fromport", BAD_CAST fport.c_str());
  xmlNewProp(tmp, BAD_CAST "to", BAD_CAST tmod.c_str());
  xmlNewProp(tmp, BAD_CAST "toport", BAD_CAST tport.c_str());
}


xmlNode*
NetworkIO::get_connection_node(const std::string &id)
{
  xmlNode* cid_node = 0;
  xmlNode* node = subnets_.top();
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("network"))
    {
      xmlNode* msnode = node->children;
      for (; msnode != 0; msnode = msnode->next) {

        if (std::string(to_char_ptr(msnode->name)) == std::string("connections")) {
          xmlNode* mnode = msnode->children;
          for (; mnode != 0; mnode = mnode->next) {

            if (std::string(to_char_ptr(mnode->name)) == std::string("connection")) {
              xmlAttrPtr name_att = get_attribute_by_name(mnode, "id");
              std::string cid = std::string(to_char_ptr(name_att->children->content));
              if (cid == id) {
                cid_node = mnode;
                break;
              }
            }
          }
        }
      }
    }
  }
  return cid_node;
}


void
NetworkIO::set_disabled_connection(const std::string &id)
{
  xmlNode* cid_node = get_connection_node(id);

  if (! cid_node) {
    simpleLog_ << "ERROR: could not find connection node with id: " << id << std::endl;
    return;
  }
  xmlNewProp(cid_node, BAD_CAST "disabled", BAD_CAST "yes");
}


void
NetworkIO::add_connection_route(const std::string &id, const std::string &route)
{
  xmlNode* cid_node = get_connection_node(id);

  if (! cid_node) {
    simpleLog_ << "ERROR: could not find connection node with id: " << id << std::endl;
    return;
  }

  xmlNewTextChild(cid_node, 0, BAD_CAST "route", BAD_CAST route.c_str());
}

void
NetworkIO::add_connection_note_position(const std::string &id, const std::string &pos)
{
  bool found = false;
  xmlNode* node = get_connection_node(id);

  if (! node) {
    simpleLog_ << "ERROR: could not find node for connection id: " << id << std::endl;
    return;
  }
  node = node->children;
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("note")) {
      found = true;
      break;
    }
  }

  if (! found) {
    simpleLog_ << "ERROR: could not find note node for module id: " << id << std::endl;
    return;
  }
  xmlNewProp(node, BAD_CAST "position", BAD_CAST pos.c_str());
}


void
NetworkIO::add_connection_note_color(const std::string &id, const std::string &col)
{
  bool found = false;
  xmlNode* node = get_connection_node(id);

  if (! node) {
    simpleLog_ << "ERROR: could not find node for connection id: " << id << std::endl;
    return;
  }
  node = node->children;
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("note")) {
      found = true;
      break;
    }
  }

  if (! found) {
    simpleLog_ << "ERROR: could not find note node for module id: " << id << std::endl;
    return;
  }
  xmlNewProp(node, BAD_CAST "color", BAD_CAST col.c_str());
}


void
NetworkIO::set_port_caching(const std::string &id, const std::string &port,
const std::string &val)
{
  xmlNode* mnode = get_module_node(id);
  xmlNode* pcnode = 0;
  if (! mnode) {
    simpleLog_ << "ERROR: could not find module node with id (set_port_caching): " << id << std::endl;
    return;
  }

  xmlNode* node = mnode->children;
  for (; node != 0; node = node->next) {
    // skip all but the network node.
    if (node->type == XML_ELEMENT_NODE &&
      std::string(to_char_ptr(node->name)) == std::string("port_caching"))
    {
      pcnode = node;
    }
  }

  if (! pcnode) {
    pcnode = xmlNewChild(mnode, 0, BAD_CAST "port_caching", 0);
  }

  xmlNode *tmp;
  tmp = xmlNewChild(pcnode, 0, BAD_CAST "port", 0);
  xmlNewProp(tmp, BAD_CAST "id", BAD_CAST port.c_str());
  xmlNewProp(tmp, BAD_CAST "val", BAD_CAST val.c_str());
}

#endif
