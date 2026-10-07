/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2026 Scientific Computing and Imaging Institute,
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


#include <Dataflow/Serialization/Network/NetworkToPython.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/PortInterface.h>
#include <Dataflow/Network/ConnectionId.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <locale>
#include <map>
#include <sstream>
#include <tuple>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

namespace
{
  std::string quoted(const std::string& s)
  {
    std::string out = "\"";
    for (const unsigned char c : s)
    {
      switch (c)
      {
      case '\\': out += "\\\\"; break;
      case '"': out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (c < 0x20 || c == 0x7f)
        {
          char buf[5];
          std::snprintf(buf, sizeof(buf), "\\x%02x", c);
          out += buf;
        }
        else
          out += static_cast<char>(c);  // UTF-8 passes through; the script is written as UTF-8
      }
    }
    return out + "\"";
  }

  // Shortest precision that reads back to the same double, so 0.1 prints as 0.1.
  std::string doubleLiteral(double d)
  {
    if (std::isinf(d))
      return d > 0 ? "float('inf')" : "-float('inf')";
    std::string s;
    for (int precision = 15; precision <= 17; ++precision)
    {
      std::ostringstream ostr;
      ostr.imbue(std::locale::classic());
      ostr.precision(precision);
      ostr << d;
      s = ostr.str();
      // Not strtod: Qt sets the C locale from the environment, which can make the decimal point a comma.
      std::istringstream istr(s);
      istr.imbue(std::locale::classic());
      double back = 0;
      if (istr >> back && back == d)
        break;
    }
    if (s.find_first_of(".en") == std::string::npos)
      s += ".0";
    return s;
  }

  struct LiteralVisitor : boost::static_visitor<std::optional<std::string>>
  {
    std::optional<std::string> operator()(int v) const { return std::to_string(v); }
    std::optional<std::string> operator()(double v) const { return doubleLiteral(v); }
    std::optional<std::string> operator()(const std::string& v) const { return quoted(v); }
    std::optional<std::string> operator()(bool v) const { return std::string(v ? "True" : "False"); }
    std::optional<std::string> operator()(const AlgoOption&) const { return {}; }
    std::optional<std::string> operator()(const Variable::List& list) const
    {
      std::string out = "[";
      for (size_t i = 0; i < list.size(); ++i)
      {
        auto element = pythonLiteral(list[i]);
        if (!element)
          return {};
        if (i > 0)
          out += ", ";
        out += *element;
      }
      return out + "]";
    }
  };

  std::string variableName(const ModuleId& id)
  {
    auto name = id.name_;
    if (!name.empty())
      name[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(name[0])));
    return name + "_" + std::to_string(id.idNumber_);
  }

  struct ResolvedConnection
  {
    ModuleId from, to;
    size_t fromIndex, toIndex;
  };
}

std::optional<std::string> SCIRun::Dataflow::Networks::pythonLiteral(const Variable& var)
{
  return boost::apply_visitor(LiteralVisitor(), var.value());
}

std::string SCIRun::Dataflow::Networks::networkToPythonScript(const NetworkStateInterface& network,
  const NetworkFile* layout, const std::string& sourceName)
{
  std::ostringstream py;
  py.imbue(std::locale::classic());
  py << "# SCIRun network exported to Python" << (sourceName.empty() ? "" : " from " + sourceName) << ".\n"
     << "# Run it with `scirun -s <this file>`, or paste it into the SCIRun Python console.\n"
     << "# Module IDs are reassigned when the script runs, so modules are referred to by variable.\n";

  std::vector<ModuleHandle> modules;
  std::map<std::string, size_t> order;
  for (size_t i = 0; i < network.nmodules(); ++i)
  {
    modules.push_back(network.module(i));
    order[modules.back()->id().id_] = i;
  }

  py << "\n"
     << "skipped = []\n"
     << "def set_state(module, key, value):\n"
     << "    # Saved networks can hold keys or value types that this version's modules no longer accept.\n"
     << "    try:\n"
     << "        scirun_set_module_state(module, key, value)\n"
     << "    except (ValueError, RuntimeError) as e:\n"
     << "        skipped.append(f\"{module} {key}: {e}\")\n";

  py << "\n";
  for (const auto& module : modules)
    py << variableName(module->id()) << " = scirun_add_module(" << quoted(module->name()) << ")\n";

  // Keys named after an input port (dynamic port labels) only exist once that port is connected.
  auto isPortKey = [](const ModuleHandle& module, const std::string& key)
  {
    const auto ports = module->inputPorts();
    return std::any_of(ports.begin(), ports.end(), [&key](const InputPortHandle& port)
      { return port && (port->externalId().toString() == key || port->internalId().toString() == key); });
  };
  auto emitState = [&](const ModuleHandle& module, bool portKeys, bool& headerWritten)
  {
    auto state = module->get_state();
    if (!state)
      return;
    const auto var = variableName(module->id());
    for (const auto& key : state->getKeys())
    {
      if (isPortKey(module, key.name()) != portKeys)
        continue;
      if (!headerWritten)
        py << "\n# " << module->id().id_ << "\n";
      headerWritten = true;
      const auto literal = pythonLiteral(state->getValue(key));
      if (literal)
        py << "set_state(" << var << ", " << quoted(key.name()) << ", " << *literal << ")\n";
      else
        py << "# " << key.name() << ": value has no python form, left at its default\n";
    }
  };

  for (const auto& module : modules)
  {
    bool headerWritten = false;
    emitState(module, false, headerWritten);
  }

  std::vector<ResolvedConnection> connections;
  for (const auto& desc : network.connections(false))
  {
    auto from = network.lookupModule(desc.out_.moduleId_);
    auto to = network.lookupModule(desc.in_.moduleId_);
    auto outPort = from ? from->getOutputPort(desc.out_.portId_) : nullptr;
    auto inPort = to ? to->getInputPort(desc.in_.portId_) : nullptr;
    if (!outPort || !inPort)
    {
      py << "# could not resolve connection " << ConnectionId::create(desc).id_ << "\n";
      continue;
    }
    connections.push_back({ desc.out_.moduleId_, desc.in_.moduleId_, outPort->getIndex(), inPort->getIndex() });
  }
  // Dynamic input ports appear one at a time as they are connected, so connect in port order.
  // Module order rather than ID keeps the output stable when a rebuilt network renumbers IDs.
  std::sort(connections.begin(), connections.end(), [&order](const ResolvedConnection& a, const ResolvedConnection& b)
  {
    return std::make_tuple(order[a.to.id_], a.toIndex, order[a.from.id_], a.fromIndex)
      < std::make_tuple(order[b.to.id_], b.toIndex, order[b.from.id_], b.fromIndex);
  });

  if (!connections.empty())
    py << "\n";
  for (const auto& c : connections)
    py << "scirun_connect_modules(" << variableName(c.from) << ", " << c.fromIndex << ", "
       << variableName(c.to) << ", " << c.toIndex << ")\n";

  for (const auto& module : modules)
  {
    bool headerWritten = false;
    emitState(module, true, headerWritten);
  }

  if (layout)
  {
    // The GUI keys disabled connections by module pair only, "to--from" (NetworkEditor::connectionNoteId).
    const auto& disabled = layout->disabledComponents.disabledConnections;
    for (const auto& c : connections)
    {
      if (std::find(disabled.begin(), disabled.end(), c.to.id_ + "--" + c.from.id_) != disabled.end())
        py << "scirun_disable_connection(" << variableName(c.from) << ", " << c.fromIndex << ", "
           << variableName(c.to) << ", " << c.toIndex << ")\n";
    }

    for (const auto& id : layout->disabledComponents.disabledModules)
      py << "# " << id << " is disabled in the network; the python API cannot disable modules.\n";

    const auto& positions = layout->modulePositions.modulePositions;
    bool first = true;
    for (const auto& module : modules)
    {
      auto pos = positions.find(module->id().id_);
      if (pos == positions.end())
        continue;
      if (first)
        py << "\n";
      first = false;
      py << "scirun_move_module(" << variableName(module->id()) << ", "
         << doubleLiteral(pos->second.first) << ", " << doubleLiteral(pos->second.second) << ")\n";
    }
  }

  py << "\nif skipped:\n"
     << "    print(\"Saved state not applied:\\n  \" + \"\\n  \".join(skipped))\n";
  return py.str();
}
