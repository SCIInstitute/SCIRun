#ifndef _SOLVER_H
#define _SOLVER_H

#include "World.h"

class PotentialSolver 
{
public:
	/*constructor, sets world*/
	PotentialSolver(World &world, int max_it, double tol): 
		world(world), max_solver_it(max_it), tolerance(tol) {}
	
	/*solves potential using Gauss-Seidel*/
	bool solve();

	/*computes electric field = -gradient(phi)*/
	void computeEF();

protected:
	World &world;
	unsigned max_solver_it;	//maximum number of solver iterations
	double tolerance;		//solver tolerance
};
#endif
