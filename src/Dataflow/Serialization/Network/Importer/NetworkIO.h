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
//    File   : NetworkIO.h
//    Author : Martin Cole
//    Date   : Tue Jan 24 11:28:22 2006

#ifndef DATAFLOW_NETWORK_NETWORKIO_H
#define DATAFLOW_NETWORK_NETWORKIO_H 1

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Algorithms/Base/Variable.h>
#include <libxml/xmlreader.h>
#include <map>
#include <stack>

#include <Dataflow/Serialization/Network/Importer/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  typedef std::map<std::string, std::map<std::string, Core::Algorithms::Name>> NameLookup;
  typedef boost::function<Core::Algorithms::AlgorithmParameter::Value(std::string)> ValueConverter;
  typedef std::map<std::string, std::map<std::string, ValueConverter>> ValueConverterMap;

  class SCISHARE LegacyNetworkIO
  {
  public:
    LegacyNetworkIO(const std::string& dtdPath, const Networks::ModuleFactory& modFactory,
      std::ostringstream& simpleLog);
    NetworkFileHandle load_net(const std::string& legacyNetworkFilename);
  private:
    bool done_writing() const { return done_writing_; }

    bool load_network();

    void createConnectionNew(const std::string& from, const std::string& to,
      const std::string& from_port, const std::string& to_port, const std::string &con_id);

    //! Interface to build up an xml document for saving.
    void start_net_doc(const std::string &fname, const std::string &vers, const std::string &vnetersion);
    void write_net_doc();

    void add_net_var(const std::string &var, const std::string &val);
    void add_environment_sub(const std::string &var, const std::string &val);
    void add_net_note(const std::string &val);
    void add_module_node(const std::string &id, const std::string &pack,
                         const std::string &cat, const std::string &mod, const std::string &ver);
    void add_module_position(const std::string &id, const std::string &x,
                             const std::string &y);
    void add_module_note(const std::string &id, const std::string &note);
    void add_module_note_position(const std::string &id, const std::string &pos);
    void add_module_note_color(const std::string &id, const std::string &col);
    void add_module_variable(const std::string &id, const std::string &var,
                             const std::string &val, bool filename = false, bool substitute = false, bool userelfilenames = false);
    void set_module_gui_visible(const std::string &id);
    void add_module_gui_callback(const std::string &id, const std::string &call);

    void add_connection_node(const std::string &id, const std::string &fmod,
                             const std::string &fport, const std::string &tmod,
                             const std::string &tport);
    void set_disabled_connection(const std::string &id);
    void add_connection_route(const std::string &id, const std::string &route);
    void add_connection_note(const std::string &id, const std::string &note);
    void add_connection_note_position(const std::string &id, const std::string &pos);
    void add_connection_note_color(const std::string &id, const std::string &col);
    void set_port_caching(const std::string &id, const std::string &port,
                          const std::string &val);
    void push_subnet_scope(const std::string &id, const std::string &name);
    void pop_subnet_scope();

    void process_environment(const xmlNodePtr enode);
    void process_modules_pass1(const xmlNodePtr enode);
    void process_modules_pass2(const xmlNodePtr enode);
    void process_connections(const xmlNodePtr enode);
    void process_network_node(const xmlNodePtr nnode);

    std::string process_filename(const std::string &src);
    std::string process_substitute(const std::string &src);

    std::string get_mod_id(const std::string &id);

    //! Interface from xml reading to tcl.
    //! this could be virtualized and used to interface with another gui type.
    void gui_add_module_at_position(const std::string &mod_id,
                                    const std::string &package,
                                    const std::string &category,
                                    const std::string &module,
                                    const std::string &version,
                                    const std::string &x,
                                    const std::string &y);

    void gui_add_connection(const std::string &con_id,
                            const std::string &from_id, const std::string &from_port,
                            const std::string &to_id, const std::string &to_port);

    void gui_set_connection_disabled(const std::string &con_id);
    void gui_set_module_port_caching(const std::string &mid, const std::string &pid,
                                     const std::string &val);

    void gui_call_module_callback(const std::string &id, const std::string &call);

    void gui_set_variable(const std::string &var, const std::string &val);
    void gui_set_modgui_variable(const std::string &mod_id, const std::string &var,
                                 const std::string &val);
    void gui_call_mod_post_read(const std::string &mod_id);
    void gui_set_module_note(const std::string &mod_id, const std::string &pos,
                             const std::string &col, const std::string &note);
    void gui_set_connection_note(const std::string &mod_id, const std::string &pos,
                                 const std::string &col, const std::string &note);
    void gui_set_connection_route(const std::string &con_id, const std::string &route);
    void gui_open_module_gui(const std::string &mod_id);
    int getNotePosition(const std::string& position) const;

    void gui_add_subnet_at_position(const std::string &mod_id,
                                    const std::string &module,
                                    const std::string& x,
                                    const std::string &y);
    std::string gui_push_subnet_ctx();
    void gui_pop_subnet_ctx(const std::string& ctx);

    void listModuleIdMapping();
    static std::string checkForModuleRename(const std::string& originalName);

    xmlNode* get_module_node(const std::string &id);
    xmlNode* get_connection_node(const std::string &id);

    typedef std::map<std::string, std::string> id_map_t;

    std::stack<id_map_t> netid_to_modid_;
    std::stack<id_map_t> netid_to_conid_;
    //! the enviroment variable substitutions
    id_map_t env_subs_;

    std::string net_file_;
    bool done_writing_;
    NetworkFileHandle xmlData_;

    //! document for writing nets.
    xmlDocPtr                          doc_;
    std::stack<xmlNodePtr>                  subnets_;
    std::string                             out_fname_;
    int                                sn_count_;
    int                                sn_ctx_;
    std::string dtdPath_;
    std::ostringstream& simpleLog_;
    const Networks::ModuleFactory& modFactory_;
    std::map<std::string, ModuleId> moduleIdMap_;
    std::map<std::string, std::string> connectionIdMap_;
    static const std::map<std::string, std::string> moduleRenameMap_;
    static NameLookup nameLookup_;
    static ValueConverterMap valueConverter_;
  };

}}} // end namespace SCIRun

#endif //NetworkIO_h
