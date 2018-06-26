#include<Modules/Math/CreateStandardMatrix.h>
#include<Core/Datatypes/Matrix.h>
#include<Core/Datatypes/DenseMatrix.h>
#include<Dataflow/Network/Module.h>
#include<Core/Algorithms/Math/CreateStandardMatrixAlgo.h>


using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;


MODULE_INFO_DEF(CreateStandardMatrix,Math,SCIRun);

CreateStandardMatrix::CreateStandardMatrix() : Module(staticInfo_)
{
    //INITIALIZE_PORT(InputMatrix);
    INITIALIZE_PORT(OutputMatrix);
}

void CreateStandardMatrix::setStateDefaults()
{
   
    setStateStringFromAlgoOption(Parameters::MatrixType);
    setStateIntFromAlgo(Parameters::Rows);
    setStateIntFromAlgo(Parameters::Columns);
    setStateIntFromAlgo(Parameters::Size);
    setStateIntFromAlgo(Parameters::StartPointer);
    
}

void CreateStandardMatrix::execute()

{
    //auto input=getRequiredInput(InputMatrix);
    if(needToExecute())
    {
        setAlgoOptionFromState(Parameters::MatrixType);
        setAlgoIntFromState(Parameters::Rows);
        setAlgoIntFromState(Parameters::Columns);
        setAlgoIntFromState(Parameters::Size);
        setAlgoIntFromState(Parameters::StartPointer);
    
        AlgorithmInput input;
        auto output=algo().run(input);
        sendOutputFromAlgorithm(OutputMatrix,output);
    }
}
