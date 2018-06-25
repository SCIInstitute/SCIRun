#include<Modules/Math/GenerateStandardMatrix.h>
#include<Core/Datatypes/Matrix.h>
#include<Core/Datatypes/DenseMatrix.h>
#include<Dataflow/Network/Module.h>
#include<Core/Algorithms/Math/GenerateStandardMatrixAlgo.h>


using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;


MODULE_INFO_DEF(GenerateStandardMatrix,Math,SCIRun);

GenerateStandardMatrix::GenerateStandardMatrix() : Module(staticInfo_)
{
    //INITIALIZE_PORT(InputMatrix);
    INITIALIZE_PORT(OutputMatrix);
}

void GenerateStandardMatrix::setStateDefaults()
{
   
    setStateStringFromAlgoOption(Parameters::MatrixType);
    setStateIntFromAlgo(Parameters::Rows);
    setStateIntFromAlgo(Parameters::Columns);
    setStateIntFromAlgo(Parameters::Size);
    
}

void GenerateStandardMatrix::execute()

{
    //auto input=getRequiredInput(InputMatrix);
    if(needToExecute())
    {
        setAlgoOptionFromState(Parameters::MatrixType);
        setAlgoIntFromState(Parameters::Rows);
        setAlgoIntFromState(Parameters::Columns);
        setAlgoIntFromState(Parameters::Size);
        
    
        AlgorithmInput input;
        auto output=algo().run(input);
        sendOutputFromAlgorithm(OutputMatrix,output);
    }
}
