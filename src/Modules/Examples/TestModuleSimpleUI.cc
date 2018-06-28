#include <Modules/String/TestModuleSimpleUI.h>
#include <Core/Datatypes/String.h>

using namespace SCIRun;
using namespace SCIRun::Modules::StringManip;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

const ModuleLookupInfo TestModuleSimpleUI::staticInfo_("TestModuleSimpleUI","String","SCIRun");

TestModuleSimpleUI::TestModuleSimpleUI () : Module(staticInfo_)
{
    INITIALIZE_PORT(InputString);
    INITIALIZE_PORT(OutputString);
}

SCIRun::Core::Algorithms::AlgorithmParameterName TestModuleSimpleUI::FormatString("FormatString");

void TestModuleSimpleUI::setStateDefaults()
{
    auto state = get_state();
    state->setValue(FormatString,std::string("[Insert your message here]"));
}

void TestModuleSimpleUI::execute()
{
    std::string message_string;
    auto stringH=getOptionalInput(InputString);
    auto state=get_state();
    
    if(stringH && *stringH)
    {
        state->setValue(FormatString, (*stringH)->value());
    }
    
    message_string=state->getValue(FormatString).toString();
    
    StringHandle msH(new String(message_string));
    sendOutput(OutputString,msH);
}

