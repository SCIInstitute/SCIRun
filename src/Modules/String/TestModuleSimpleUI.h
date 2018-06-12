//
//  TestModuleSimple.h
//  Sample Module
//
//  Created by Barad-dûr on 6/8/18.
//  Copyright © 2018 Barad-dûr. All rights reserved.
//

#ifndef MODULES_STRING_TestModuleSimpleUI_h
#define MODULES_STRING_TestModuleSimpleUI_h

#include <Dataflow/Network/Module.h>
#include <Modules/Fields/share.h>


namespace SCIRun{
    namespace Modules{
        namespace StringManip{
            class SCISHARE TestModuleSimpleUI :
             public SCIRun::Dataflow::Networks::Module,
             public Has1InputPort<StringPortTag>,
             public Has1OutputPort<StringPortTag>
            {
              public:
                TestModuleSimpleUI();
                virtual void execute();
                virtual void setStateDefaults();
                
                INPUT_PORT(0,InputString, String);
                OUTPUT_PORT(0,OutputString, String);
                
                static Core::Algorithms::AlgorithmParameterName FormatString;
                MODULE_TRAITS_AND_INFO(NoAlgoOrUI);
            
            };
        }
    }
}

#endif /* TestModuleSimple_h */
