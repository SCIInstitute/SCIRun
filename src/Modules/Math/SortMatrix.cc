#include<Modules/Math/SortMatrix.h>
#include<Core/Datatypes/Matrix.h>
#include<Dataflow/Network/Module.h>
#include<Core/Algorithms/Math/SortMatrixAlgo.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(SortMatrix,Math,SCIRun);

SortMatrix::SortMatrix() : Module(staticInfo_)
{
    INITIALIZE_PORT(InputMatrix);
    INITIALIZE_PORT(OutputMatrix);
}

void SortMatrix::setStateDefaults()
{
    setStateIntFromAlgo(Variables::Method);

}

void SortMatrix::execute()
{
    auto input=getRequiredInput(InputMatrix);
    
    if(needToExecute())
    {
        setAlgoIntFromState(Variables::Method);
        auto output = algo().run(withInputData((InputMatrix, input)));
        sendOutputFromAlgorithm(OutputMatrix,output);
    }
}
