/*
* Basic Electrostatic PIC simulation
*
* Compile with mpic++ *.cpp
* 
* Run with:  ./a.out
*
*/


#include <math.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "World.h"
#include "PotentialSolver.h"
#include "Species.h"
#include "Output.h"

using namespace std;		//to avoid having to write std::cout
using namespace Const;		//to avoid having to write Const::ME

/*program execution starts here*/
int main(int argc, char *args[])
{
	/*initialize domain*/
    World world(21,21,21);
    world.setExtents({-0.1,-0.1,0},{0.1,0.1,0.2});
//    world.setTime(2e-10,10000);
    world.setTime(2e-10,100);

	/*set up particle species*/
	vector<Species> species;
	species.reserve(2);	//pre-allocate space for two species
	species.push_back(Species("O+", 16*AMU, QE, world));
	species.push_back(Species("e-", ME, -1*QE, world));

	cout<<"Size of species "<<species.size()<<endl;

	/*create particles*/
	int3 np_ions_grid = {41,41,41};
	int3 np_eles_grid = {21,21,21};
	species[0].loadParticlesBoxQS(world.getX0(),world.getXm(),1e11,np_ions_grid);	//ions
	species[1].loadParticlesBoxQS(world.getX0(),world.getXc(),1e11,np_eles_grid);	//electrons

	/*initialize potential solver and solve initial potential*/
    PotentialSolver solver(world,10000,1e-4);
    solver.solve();

    /*obtain initial electric field*/
    solver.computeEF();

    /* main loop*/
	while(world.advanceTime())
        {
        /*move particles*/
		for (Species &sp:species)
		    {
			sp.advance();
			sp.computeNumberDensity();
		    }

		/*compute charge density*/
		world.computeChargeDensity(species);

        /*update potential*/
        solver.solve();

        /*obtain electric field*/
        solver.computeEF();

		/*screen and file output*/
        Output::screenOutput(world,species);
        Output::diagOutput(world,species);

		/*periodically write out results*/
        if (world.getTs()%100==0 || world.isLastTimeStep()) Output::fields(world, species);
        }
	
	/* grab starting time*/
	cout<<"Simulation took "<<world.getWallTime()<<" seconds\n";
	return 0;		//indicate normal exit
}
