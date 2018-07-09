#ifndef MODULES_MATH_ResizeMatrix_H
#define MODULES_MATH_ResizeMatrix_H

#include <Dataflow/Network/Module.h>
#include <Modules/Math/share.h>

namespace SCIRun{
  namespace Modules{
    namespace Math{
      class SCISHARE ResizeMatrix:public SCIRun::Dataflow::Networks::Module,
      public Has1InputPort<MatrixPortTag>,
      public Has1OutputPort<MatrixPortTag>
      {
      public:
        ResizeMatrix();
        virtual void execute();
        virtual void setStateDefaults();
          
        INPUT_PORT(0,InputMatrix, Matrix);
        OUTPUT_PORT(0,OutputMatrix, Matrix);
      
        MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm);
      };
    }
  }
}
#endif

