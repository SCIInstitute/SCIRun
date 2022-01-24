/*Defines flying material data*/

#ifndef _SPECIES_H
#define _SPECIES_H

#include <vector>
#include "Field.h"
#include "World.h"

/** Data structures for particle storage **/
struct Particle
{
	double3 pos;			/*position*/
	double3 vel;			/*velocity*/
	double mpw;				/*macroparticle weight*/
	
	Particle(double3 x, double3 v, double mpw):
	pos{x}, vel{v}, mpw{mpw} { }
};

/*species container*/
class Species 
{
public:
	Species(std::string name, double mass, double charge, World &world) :
		name(name), mass(mass), charge(charge), den(world.ni,world.nj,world.nk),
		world(world) { 	}

	/*returns the number of simulation particles*/
	size_t getNp()	{return particles.size();}

	/*returns the number of real particles*/
	double getRealCount();

	/*returns the species momentum*/
	double3 getMomentum();

	/*returns the species kinetic energy*/
	double getKE();

	/*moves all particles using electric field ef[]*/
	void advance();

	/*compute number density*/
	void computeNumberDensity();

	/*adds a new particle*/
	void addParticle(double3 pos, double3 vel, double mpwt);

	/*random load of particles in a x1-x2 box representing num_den number density*/
	void loadParticlesBox(double3 x1, double3 x2, double num_den, int num_mp);

	/*quiet start load of particles in a x1-x2 box representing num_den number density*/
	void loadParticlesBoxQS(double3 x1, double3 x2, double num_den, int3 num_mp);

	const std::string name;			/*species name*/
	const double mass;			/*particle mass in kg*/
	const double charge;		/*particle charge in Coulomb*/
	
	std::vector<Particle> particles;	/*contiguous array for storing particles*/
	Field den;			/*number density*/

protected:
	World &world;
};


#endif
