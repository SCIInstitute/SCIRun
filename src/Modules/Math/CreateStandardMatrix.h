#ifndef MODULES_MATH_CreateStandardMatrix_H
#define MODULES_MATH_CreateStandardMatrix_H

#include <Dataflow/Network/Module.h>
#include <Modules/Math/share.h>


namespace SCIRun{
    namespace Modules{
        namespace Math{
            class SCISHARE CreateStandardMatrix:public SCIRun::Dataflow::Networks::Module,
            public HasNoInputPorts,
            public Has1OutputPort<MatrixPortTag>
            {
            public:
                CreateStandardMatrix();
                virtual void execute();
                virtual void setStateDefaults();
                
                
                //INPUT_PORT(0,InputMatrix, Matrix);
                OUTPUT_PORT(0,OutputMatrix, Matrix);
            
                MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm);
            };
        }
    }
}
#endif

