#include<Modules/Math/ResizeMatrix.h>
#include<Core/Datatypes/Matrix.h>
#include<Core/Datatypes/DenseMatrix.h>
#include<Dataflow/Network/Module.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Math/ResizeMatrixAlgo.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;

MODULE_INFO_DEF(ResizeMatrix,Math,SCIRun);

ResizeMatrix::ResizeMatrix() : Module(staticInfo_)
{
    INITIALIZE_PORT(InputMatrix);
    INITIALIZE_PORT(OutputMatrix);
}

void ResizeMatrix::setStateDefaults()
{
   
    setStateIntFromAlgo(Parameters::NoOfRows);
    setStateIntFromAlgo(Parameters::NoOfColumns);
    
}

void ResizeMatrix::execute()

{
    auto input=getRequiredInput(InputMatrix);
   
    if(needToExecute())
    {
        setAlgoIntFromState(Parameters::NoOfRows);
        setAlgoIntFromState(Parameters::NoOfColumns);
        
        auto output=algo().run(withInputData((InputMatrix,input)));
        sendOutputFromAlgorithm(OutputMatrix,output);
    }
}
