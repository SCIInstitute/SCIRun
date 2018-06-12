#include <Modules/String/TestModuleSimple.h>
#include <Core/Datatypes/String.h>

using namespace SCIRun;
using namespace SCIRun::Modules::StringManip;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

const ModuleLookupInfo TestModuleSimple::staticInfo_("TestModuleSimple","String","SCIRun");

TestModuleSimple::TestModuleSimple () : Module(staticInfo_,false)
{
    INITIALIZE_PORT(OutputString);
}

void TestModuleSimple::execute()
{
    std::string message_string;
    message_string="Trying to develop a Module";
    
    StringHandle msH(new String(message_string));
    sendOutput(OutputString,msH);
}

