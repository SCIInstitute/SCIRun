/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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

#include<Core/Algorithms/ParticleInCell/ElectroStaticAlgo.h>
#include<Core/Datatypes/MatrixTypeConversions.h>
#include <chrono>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

ALGORITHM_PARAMETER_DEF(ParticleInCell, NumTimeSteps);                                        //here 1

const AlgorithmOutputName ElectroStaticAlgo::x_coordinates("x_coordinates");
const AlgorithmOutputName ElectroStaticAlgo::y_coordinates("y_coordinates");
const AlgorithmOutputName ElectroStaticAlgo::z_coordinates("z_coordinates");

ElectroStaticAlgo::ElectroStaticAlgo()
    {
    addParameter(Variables::Method,0);;
    addParameter(Parameters::NumTimeSteps,5000);                                     //here 2
    }

AlgorithmOutput ElectroStaticAlgo::run(const AlgorithmInput&) const
    {
    AlgorithmOutput output;

/*
************************************************ Variable Setup
*/

using namespace std;

    int num_particles       = 10000;                  //should be set by User Interface input
    double delta_t          = 2e-10;                  //should be set by User Interface input

    auto iterations         = get(Parameters::NumTimeSteps).toInt();                  //here 3
    printf("Debug 4: iterations is %d\n", iterations);

//    int iterations          = 10000;
//    int iterations          = 5000;

//    const int sample_size_p = 1000;                                        //should be set by User Interface input
    const int sample_size_p = 100;                    //Using 100 for sample_size_p, 10,000 for num_particles, 5,000 iterations and 5 for sample_size_i, takes 108 seconds to run

//    const int sample_size_i = 100;                    //should be set by User Interface input

//    const int sample_size_i = 5;                    //Using 5 for sample_size_i, 5,000 iterations, 10,000 for num_particles and 100 for sample_size_p, takes 53 seconds
    const int sample_size_i = 10;                   //Using 10 for sample_size_i, 5,000 iterations, 10,000 for num_particles and 100 for sample_size_p, takes 53 seconds
//    const int sample_size_i = 50;                   //Using 50 for sample_size_i, 5,000 iterations, 10,000 for num_particles and 100 for sample_size_p, takes ?? seconds
    

    int output_count        = 0;
    int buffer_index        = 0;
    int pot_buffer_index    = 0;
    int iterations_index    = 0;
    const int buffer_size   = (iterations/sample_size_i)*(num_particles/sample_size_p);
    auto buffer_pos_x       = new double[buffer_size];
    auto buffer_pos_y       = new double[buffer_size];
    auto buffer_pos_z       = new double[buffer_size];
    auto buffer_potential   = new double[(iterations/(sample_size_i*100))+1][21][21][21];

/*
************************************************ Output Data Setup
*/

    DenseMatrixHandle output_mat_0(new DenseMatrix(buffer_size, 1));
    DenseMatrixHandle output_mat_1(new DenseMatrix(buffer_size, 1));
    DenseMatrixHandle output_mat_2(new DenseMatrix(buffer_size, 1));
                          
    auto start_wall  = chrono::high_resolution_clock::now();    //Set up collection of execution time
    auto outputTimes = get(Variables::Method).toInt();          //outputTimes is used to show or not show the execution time

/*
************************************************ Simulation Code
                     includes collecting buffered data in buffer_pos_x, buffer_pos_y and buffer_pos_z
*/

using namespace Const;

//program execution starts here

	//initialize domain
    World world(21,21,21);
    world.setExtents({-0.1,-0.1,0},{0.1,0.1,0.2});
//    world.setTime(2e-10,10000);
//    world.setTime(2e-10,100);
    world.setTime(delta_t,iterations);

	//set up particle species
	vector<Species> species;
	species.reserve(2);	//pre-allocate space for two species
	species.push_back(Species("O+", 16*AMU, QE, world));
	species.push_back(Species("e-", ME, -1*QE, world));

	cout<<"Size of species "<<species.size()<<endl;

	//create particles
	int3 np_ions_grid = {41,41,41};
	int3 np_eles_grid = {21,21,21};
	species[0].loadParticlesBoxQS(world.getX0(),world.getXm(),1e11,np_ions_grid);	//ions
    species[1].loadParticlesBoxQS(world.getX0(),world.getXc(),1e11,np_eles_grid);	//electrons

	//initialize potential solver and solve initial potential
    PotentialSolver solver(world,10000,1e-4);
    solver.solve();

    //obtain initial electric field
    solver.computeEF();

    // main loop
	while(world.advanceTime())
        {
        //Save electric potential for output to SCIRun
//        if (world.getTs()%500==0 || world.isLastTimeStep())
//        if (!(iterations_index%(sample_size_i*100)) && (pot_buffer_index < iterations/sample_size_i))
        if (!(world.getTs()%(sample_size_i*100)) && (pot_buffer_index < iterations/sample_size_i))
            {
            cout<<"pot_buffer_index is "<<pot_buffer_index<<"\n";
            cout<<"world.getTs()/(sample_size_i*100) is "<<world.getTs()/(sample_size_i*100)<<"\n";
            for (int i=0;i<world.ni-1;i++)
                for (int j=0;j<world.nj-1;j++)
                    for (int k=0;k<world.nk-1;k++)
                        {
                        buffer_potential[pot_buffer_index][i][j][k]=world.phi[i][j][k];
                        }
            pot_buffer_index++;
            }


        //move particles
        int species_index = 0;
		for (Species &sp:species)
		    {
//			sp.advance();
            sp.advance(sample_size_p, sample_size_i, buffer_size, buffer_index, iterations_index, species_index, buffer_pos_x, buffer_pos_y, buffer_pos_z);
			sp.computeNumberDensity();
            species_index++;
		    }

		//compute charge density
		world.computeChargeDensity(species);

        //update potential
        solver.solve();

        //obtain electric field
        solver.computeEF();

        if(outputTimes)
            {
		    //screen and file output
//            Output::screenOutput(world,species);
//            Output::diagOutput(world,species);

		    //periodically write out results
            if (world.getTs()%(sample_size_i*100)==0 || world.isLastTimeStep())
                {
                cout<<"Simulation timestep is "<<world.getTs()<<"\n";
                Output::fields(world, species);
                output_count++;
                }
            }

        iterations_index++;
        }

	// grab starting time
	cout<<"Simulation took "<<world.getWallTime()<<" seconds\n";
    cout<<"There were "<< iterations/sample_size_i<< " time slices with "<<num_particles/sample_size_p<<" particles, for a total of "<<buffer_index<<" data elements sent to each SCIRun output port\n";
	if(outputTimes) cout<<"There were "<<output_count<<" sample(s) output to file\n";

/*
************************************************End of the simulation code
*/

/*
************************************************Wrap Up
                  includes posting time of execution and passing buffered data to SCIRun
*/

    double *data0=output_mat_0->data();
    double *data1=output_mat_1->data();
    double *data2=output_mat_2->data();

    std::copy(buffer_pos_x, buffer_pos_x+buffer_size, data0);
    std::copy(buffer_pos_y, buffer_pos_y+buffer_size, data1);
    std::copy(buffer_pos_z, buffer_pos_z+buffer_size, data2);

    output[x_coordinates] = output_mat_0;
    output[y_coordinates] = output_mat_1;
    output[z_coordinates] = output_mat_2;

    if(outputTimes)
        {
        auto end_wall = chrono::high_resolution_clock::now();       //Execution time
        chrono::duration<double> time_taken = end_wall - start_wall;
        printf("Program took %.6f seconds Wall Clock time\n", time_taken.count());
        }

    return output;
    }
