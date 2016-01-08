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

#include <Modules/Legacy/Bundle/InsertFieldsIntoBundle.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Bundles;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

const ModuleLookupInfo InsertFieldsIntoBundle::staticInfo_("InsertFieldsIntoBundle", "Bundle", "SCIRun");
const AlgorithmParameterName InsertFieldsIntoBundle::NumFields("NumFields");
const AlgorithmParameterName InsertFieldsIntoBundle::FieldNames("FieldNames");
const AlgorithmParameterName InsertFieldsIntoBundle::FieldReplace("FieldReplace");

InsertFieldsIntoBundle::InsertFieldsIntoBundle() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputBundle);
  INITIALIZE_PORT(OutputBundle);
  INITIALIZE_PORT(InputFields);
}

void InsertFieldsIntoBundle::setStateDefaults()
{

}

void InsertFieldsIntoBundle::portAddedSlot(const ModuleId& mid, const PortId& pid)
{
  //TODO: redesign with non-virtual slot method and virtual hook that ensures module id is the same as this
  if (mid == id_)
  {
    int fields = num_input_ports() - 2; // -1 for empty end, -1 for bundle port 0
    get_state()->setTransientValue(NumFields, fields);
  }
}

void InsertFieldsIntoBundle::portRemovedSlot(const ModuleId& mid, const PortId& pid)
{
  //TODO: redesign with non-virtual slot method and virtual hook that ensures module id is the same as this
  if (mid == id_)
  {
    int fields = num_input_ports() - 2; // -1 for empty end, -1 for bundle port 0
    get_state()->setTransientValue(NumFields, fields);
  }
}

void InsertFieldsIntoBundle::execute()
{
  auto bundleOption = getOptionalInput(InputBundle);
  auto fields = getRequiredDynamicInputs(InputFields);

  //if (inputs_changed_ || guifield1name_.changed() || 
  //  guifield2name_.changed() || guifield3name_.changed() ||
  //  guifield4name_.changed() || guifield5name_.changed() ||
  //  guifield6name_.changed() || 
  //  guireplace1_.changed() || guireplace2_.changed() ||
  //  guireplace3_.changed() || guireplace4_.changed() ||
  //  guireplace5_.changed() || guireplace6_.changed() ||
  //  guibundlename_.changed() || !oport_cached("bundle"))
  if (needToExecute())
  {
    update_state(Executing);
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    std::string bundlename = guibundlename_.get();
#endif
    BundleHandle bundle;
    if (bundleOption && *bundleOption)
    {
      bundle.reset((*bundleOption)->clone());
    }
    else
    {
      bundle.reset(new Bundle());
      if (!bundle)
      {
        THROW_ALGORITHM_PROCESSING_ERROR("Could not allocate new bundle");
      }
    }

    //TODO: instead grab a vector of tuple<string,bool>. need to modify Variable::Value again
    auto fieldNames = get_state()->getValue(FieldNames).toVector();
    auto replace = get_state()->getValue(FieldReplace).toVector();

    for (int i = 0; i < fields.size(); ++i)
    {
      auto field = fields[i];
      if (field)
      {
        auto name = i < fieldNames.size() ? fieldNames[i].toString() : ("field" + boost::lexical_cast<std::string>(i));
        auto replaceField = i < replace.size() ? replace[i].toBool() : true;
        if (replaceField || !bundle->isField(name))
        {
          bundle->set(name, field);
        }
      }
    }

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (bundlename != "")
    {
      handle->set_property("name",bundlename,false);
    }
#endif

    sendOutput(OutputBundle, bundle);
  }
}

#if SCIRUN4_CODE_TO_BE_ENABLED_LATER

private:
  GuiString     guifield6name_;
  
  GuiInt        guireplace6_;
  
  GuiString     guibundlename_;
};


  InsertFieldsIntoBundle::InsertFieldsIntoBundle(GuiContext* ctx)
    : Module("InsertFieldsIntoBundle", ctx, Filter, "Bundle", "SCIRun"),
      guifield1name_(get_ctx()->subVar("field1-name"), "field1"),
      guifield2name_(get_ctx()->subVar("field2-name"), "field2"),
      guifield3name_(get_ctx()->subVar("field3-name"), "field3"),
      guifield4name_(get_ctx()->subVar("field4-name"), "field4"),
      guifield5name_(get_ctx()->subVar("field5-name"), "field5"),
      guifield6name_(get_ctx()->subVar("field6-name"), "field6"),
      guireplace1_(get_ctx()->subVar("replace1"),1),
      guireplace2_(get_ctx()->subVar("replace2"),1),
      guireplace3_(get_ctx()->subVar("replace3"),1),
      guireplace4_(get_ctx()->subVar("replace4"),1),
      guireplace5_(get_ctx()->subVar("replace5"),1),
      guireplace6_(get_ctx()->subVar("replace6"),1),
      guibundlename_(get_ctx()->subVar("bundlename",false), "")
{
}

#endif